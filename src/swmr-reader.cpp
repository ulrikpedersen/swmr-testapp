#include <iostream>
#include <vector>
#include <algorithm>
#include <assert.h>

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;

#include "swmr-testdata.h"
#include "timestamp.h"
#include "progressbar.h"
#include "swmr-reader.h"

using namespace std;

SWMRReader::SWMRReader()
{
    m_log = Logger::getLogger("SWMRReader");
    LOG4CXX_TRACE(m_log, "SWMRReader constructor");
    m_filename = "";
    m_fid = -1;
    m_pdata = NULL;
    m_latest_framenumber = 0;
}

SWMRReader::~SWMRReader()
{
    LOG4CXX_TRACE(m_log, "SWMRReader destructor");

    if (m_fid >= 0) {
        assert(H5Fclose(m_fid) >= 0);
        m_fid = -1;
    }

    if (m_pdata != NULL) {
        delete[] m_pdata;
        m_pdata = NULL;
    }
}

void SWMRReader::open_file(const string& fname, const string& dsetname)
{
    m_filename = fname;
    m_dsetname = dsetname;

    // sanity check
    assert(m_filename != "");

    /* Create file access property list */
    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl >= 0);
    /* Set to use the latest library format */
    assert(H5Pset_libver_bounds(fapl, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) >= 0);

    m_fid = H5Fopen(m_filename.c_str(),
                    H5F_ACC_RDONLY | H5F_ACC_SWMR_READ, fapl);
    assert(m_fid >= 0);
}

void SWMRReader::get_test_data()
{
    LOG4CXX_DEBUG(m_log, "Getting test data from swmr_testdata");
    vector<hsize_t> dims(2);
    dims[0] = swmr_testdata_cols;
    dims[1] = swmr_testdata_rows;
    m_testimg = Frame(dims, (const unsigned int*) (swmr_testdata[0]));

    // Allocate some space for our reading-in buffer
    m_pdata = m_testimg.create_buffer();
}

void SWMRReader::get_test_data(const string& fname, const string& dsetname)
{
    LOG4CXX_DEBUG(m_log, "Getting test data from: " << fname << "/" << dsetname);
    m_testimg = Frame(fname, dsetname);

    // Allocate some space for our reading-in buffer
    m_pdata = m_testimg.create_buffer();
}

unsigned long long SWMRReader::latest_frame_number()
{
    herr_t status;
    hid_t dset;
    // sanity check
    assert(m_dsetname != "");
    assert(m_fid >= 0);
    dset = H5Dopen2(m_fid, m_dsetname.c_str(), H5P_DEFAULT);
    assert(dset >= 0);

    /* Get the dataspace */
    hid_t dspace;
    dspace = H5Dget_space(dset);
    assert(dspace >= 0);

    /* Refresh the dataset, i.e. get the latest info from disk */
    assert(H5Drefresh(dset) >= 0);

    int ndims = H5Sget_simple_extent_ndims(dspace);
    assert(ndims == (1 + m_testimg.dimensions().size()));

    H5Sget_simple_extent_dims(dspace, m_dims, m_maxdims);
    /* Check the image dimensions match the test image */
    assert(m_testimg.dimensions()[0] == m_dims[1]);
    assert(m_testimg.dimensions()[1] == m_dims[2]);
    if (m_dims[0] > m_latest_framenumber) {
        LOG4CXX_DEBUG(m_log, "Got dimensions: " << m_dims[0] << ", "
                              << m_dims[1] << ", " << m_dims[2]);
    } else {
        LOG4CXX_TRACE(m_log, "No new data");
    }

    assert(H5Dclose(dset) >= 0);
    assert(H5Sclose(dspace) >= 0);

    return m_dims[0];
}

void SWMRReader::read_latest_frame()
{
    herr_t status;
    hid_t dset;
    // sanity check
    assert(m_dsetname != "");
    assert(m_fid >= 0);
    dset = H5Dopen2(m_fid, m_dsetname.c_str(), H5P_DEFAULT);
    assert(dset >= 0);

    /* Get the dataspace */
    hid_t dspace;
    dspace = H5Dget_space(dset);
    assert(dspace >= 0);

    hsize_t offset[3] = { m_dims[0] - 1, 0, 0 };
    assert(offset[0] >= 0);
    hsize_t img_size[3] = { 1, m_dims[1], m_dims[2] };
    assert(H5Sselect_hyperslab(dspace, H5S_SELECT_SET, offset,
                               NULL, img_size, NULL) >= 0);

    hid_t memspace;
    memspace = H5Screate_simple(2, m_dims+1, NULL);
    assert(memspace >= 0);
    status = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset+1,
                                 NULL, img_size+1, NULL);
    assert(status >= 0);

    LOG4CXX_DEBUG(m_log, "Reading dataset: size = "
                  << img_size[0] << ", " << img_size[1] << ", "<< img_size[2]
                  << " offset = "
                  << offset[0] << ", " << offset[1] << ", "<< offset[2]);
    status = H5Dread(dset, H5T_NATIVE_UINT32,
                     memspace, dspace, H5P_DEFAULT,
                     static_cast<void*>(m_pdata));
    assert(status >= 0);
    m_latest_framenumber = m_dims[0];

    // Cleanup
    this->print_open_objects();
    assert(H5Dclose(dset) >= 0);
    assert(H5Sclose(dspace) >= 0);
    assert(H5Sclose(memspace) >= 0);
}

bool SWMRReader::check_dataset()
{
    LOG4CXX_TRACE(m_log, "Creating new Frame with read data");
    Frame readimg(m_testimg.dimensions(), m_pdata);
    assert(readimg.dimensions()[0] == m_testimg.dimensions()[0]);
    assert(readimg.dimensions()[1] == m_testimg.dimensions()[1]);
    assert(readimg.dimensions()[0] == m_dims[1]);
    assert(readimg.dimensions()[1] == m_dims[2]);

    bool result = readimg == m_testimg;
    if (result != true) {
        LOG4CXX_WARN(m_log, "Data mismatch. Frame = " << m_latest_framenumber);
    }
    return result;
}

void SWMRReader::monitor_dataset(double timeout, double polltime, int expected)
{
    bool carryon = true;
    bool check_result;
    LOG4CXX_DEBUG(m_log, "Starting monitoring");
    TimeStamp ts;

    bool show_pbar = not m_log->isDebugEnabled();
    while (carryon) {
        if (this->latest_frame_number() > m_latest_framenumber) {
            this->read_latest_frame();
            check_result = this->check_dataset();
            m_checks.push_back(check_result);
            if (expected > 0) {
                if (show_pbar) progressbar(this->m_latest_framenumber, expected);
                if (m_latest_framenumber >= expected) carryon = false;
            }
            ts.reset();
        } else {
            double secs = ts.seconds_until_now();
            if (timeout > 0 && secs > timeout) {
                LOG4CXX_WARN(m_log, "Timeout: it's been " << secs
                             << " seconds since last read");
                carryon = false;
            } else {
                usleep((unsigned int) (polltime * 1000000));
            }
        }
    }
}


int SWMRReader::report()
{
    ostringstream oss;
    int fail_count = count(m_checks.begin(), m_checks.end(), false);
    oss << endl << "======= SWMR reader report ========" << endl << endl
        << " Number of checks: " << m_checks.size() << endl
        << " Number of frames: " << m_latest_framenumber << endl;
    if ( fail_count == 0 ) {
        oss << " Result: Success! No failed checks" << endl;
    } else {
        oss << " Result: Failed checks: " << fail_count << endl;
    }
    if (not m_log->isDebugEnabled()) cout << oss.str();
    LOG4CXX_DEBUG(m_log, oss.str());
    return fail_count;
}


void SWMRReader::print_open_objects()
{
    LOG4CXX_TRACE(m_log, "    DataSets open:    " << H5Fget_obj_count( m_fid, H5F_OBJ_DATASET ));
    LOG4CXX_TRACE(m_log, "    Groups open:      " << H5Fget_obj_count( m_fid, H5F_OBJ_GROUP ));
    LOG4CXX_TRACE(m_log, "    Attributes open:  " << H5Fget_obj_count( m_fid, H5F_OBJ_ATTR ));
    LOG4CXX_TRACE(m_log, "    Datatypes open:   " << H5Fget_obj_count( m_fid, H5F_OBJ_DATATYPE ));
    LOG4CXX_TRACE(m_log, "    Files open:       " << H5Fget_obj_count( m_fid, H5F_OBJ_FILE ));
    LOG4CXX_TRACE(m_log, "    Sum objects open: " << H5Fget_obj_count( m_fid, H5F_OBJ_ALL ));
}

