#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <cstring>
#include <assert.h>

#include <log4cxx/logger.h>
using namespace log4cxx;

#include <hdf5.h>
#include "frame.h"

using namespace std;

Frame::Frame()
: m_pdata(NULL), m_log(Logger::getLogger("Frame"))
{
}

Frame::Frame(const Frame& frame)
: m_pdata(NULL), m_log(Logger::getLogger("Frame"))
{
    this->copy(frame);
}


Frame::Frame(const std::vector<hsize_t>& dims, uint32_t* pdata)
: m_pdata(NULL), m_log(Logger::getLogger("Frame"))
{
    m_dims = dims;   // copy the input dimensions
    m_pdata = pdata; // copy (shallow) the data pointer
}

Frame::Frame(const std::vector<hsize_t>& dims, const uint32_t* pdata)
: m_pdata(NULL), m_log(Logger::getLogger("Frame"))
{
    m_dims = dims;   // copy the input dimensions

    unsigned long long nitems = Frame::num_items(dims);
    m_pdata = new uint32_t [nitems];
    memcpy(m_pdata, pdata, nitems * sizeof(uint32_t));
}

Frame::Frame(const std::string& fname, const std::string& dsetname)
: m_pdata(NULL), m_log(Logger::getLogger("Frame"))
{
    /* Create file access property list */
    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl >= 0);

    hid_t fid = H5Fopen(fname.c_str(), H5F_ACC_RDONLY, fapl);
    assert(fid >= 0);

    hid_t dset = H5Dopen2(fid, dsetname.c_str(), H5P_DEFAULT);
    assert(dset >= 0);

    /* Get the dataspace */
    hid_t dspace = H5Dget_space(dset);
    assert(dspace >= 0);

    int ndims = H5Sget_simple_extent_ndims(dspace);
    assert(ndims == 3);

    m_dims.clear();
    m_dims.resize(3, 0);
    assert( m_dims.size() == 3);
    hsize_t maxdims[3];
    hsize_t *dims = (hsize_t*)&(m_dims.front());
    H5Sget_simple_extent_dims(dspace, dims, maxdims);
    LOG4CXX_DEBUG(m_log, "Test data ("<< fname <<") Dims: " << dims[0]
                         << ", " << dims[1] << ", " << dims[2]);

    hsize_t offset[3] = { m_dims[0] - 1, 0, 0 };
    assert(offset[0] >= 0);
    m_dims[0] = 1; // We only want to read out one slice
    assert(H5Sselect_hyperslab(dspace, H5S_SELECT_SET, offset,
                               NULL, dims, NULL) >= 0);
    hid_t memspace;
    memspace = H5Screate_simple(2, dims+1, NULL);
    assert(memspace >= 0);
    herr_t status = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset+1,
                                        NULL, dims+1, NULL);
    assert(status >= 0);
    m_dims.erase(m_dims.begin());
    assert(m_dims.size() == 2);

    if (m_pdata != NULL) {
        delete [] m_pdata;
        m_pdata = NULL;
    }
    m_pdata = this->create_buffer(); // allocate some memory
    status = H5Dread(dset, H5T_NATIVE_UINT32,
                     memspace, dspace, H5P_DEFAULT,
                     static_cast<void*>(m_pdata));
    assert(status >= 0);

    assert(H5Dclose(dset) >= 0);
    assert(H5Sclose(dspace) >= 0);
    assert(H5Sclose(memspace) >= 0);
    assert(H5Pclose(fapl) >=0 );
    assert(H5Fclose(fid) >= 0);
}

Frame::~Frame()
{
    m_dims.clear();
}

const std::vector<hsize_t>& Frame::dimensions()
{
    return m_dims;
}

uint32_t* Frame::create_buffer()
{
    unsigned long long nitems = this->num_items();
    assert( nitems > 0);
    uint32_t * new_buffer = new uint32_t [nitems];
    return new_buffer;
}

const uint32_t* Frame::pdata()
{
    return m_pdata;
}

Frame& Frame::operator =(const Frame& src)
{
    // Check for self-assignment
    if (this == &src)
        return *this;
    // perform the copy
    this->copy(src);
    return *this;
}

bool Frame::operator ==(const Frame& cmp)
{
    return this->is_equal(cmp);
}

bool Frame::operator !=(const Frame& cmp)
{
    return not this->is_equal(cmp);
}

void Frame::copy(const Frame& src)
{
    m_dims = src.m_dims;
    m_pdata = src.m_pdata;
}

bool Frame::is_equal(const Frame& src)
{
    // first check whether the dimensions match up
    if (m_dims != src.m_dims) return false;

    // Then check each data element
    for (int i = 0; i < this->num_items(); i++) {
        if (*(m_pdata + i) != *(src.m_pdata + i))
            return false; // mismatch
    }
    return true; // all matched up
}

unsigned long long multiply(unsigned long long x, unsigned long long y)
{
    return x * y;
}

unsigned long long Frame::num_items()
{
    return this->num_items(m_dims);
}

unsigned long long Frame::num_items(const std::vector<hsize_t>& dims) const
{
    unsigned long long nitems;
    nitems = accumulate(dims.begin(), dims.end(), 1, multiply);
    return nitems;
}


