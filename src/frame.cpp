#include <vector>
#include <string>
#include <numeric>
#include <algorithm>

#include <cstring>
#include <cstdlib>
#include <assert.h>

#include "frame.h"

using namespace std;

Frame::Frame()
: m_pdata(NULL)
{
}

Frame::Frame(const Frame& frame)
: m_pdata(NULL)
{
    this->copy(frame);
}


Frame::Frame(const std::vector<hsize_t>& dims, uint32_t* pdata)
: m_pdata(NULL)
{
    m_dims = dims;   // copy the input dimensions
    m_pdata = pdata; // copy (shallow) the data pointer
}

Frame::Frame(const std::vector<hsize_t>& dims, const uint32_t* pdata)
: m_pdata(NULL)
{
    m_dims = dims;   // copy the input dimensions

    unsigned long long nitems = Frame::num_items(dims);
    m_pdata = (uint32_t*)calloc(nitems, sizeof(uint32_t));
    memcpy(m_pdata, pdata, nitems * sizeof(uint32_t));
}

Frame::Frame(const std::string fname, const std::string dset)
: m_pdata(NULL)
{
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
    uint32_t * new_buffer = (uint32_t *)calloc(nitems, sizeof(uint32_t));
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


