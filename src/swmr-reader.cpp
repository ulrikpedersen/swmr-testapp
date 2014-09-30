#include <iostream>
#include <string>
#include <vector>
#include <assert.h>

#include <cstdlib>
#include <ctime>

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/ndc.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

#include "hdf5.h"
#include "swmr-testdata.h"
#include "frame.h"

using namespace std;

class TimeStamp {
public:
    TimeStamp();
    ~TimeStamp()
    {
    }
    ;
    void reset();
    double seconds_until_now();
    double tsdiff(timespec& start, timespec& end) const;
    private:
    timespec start;
};

TimeStamp::TimeStamp()
{
    this->start.tv_nsec = 0;
    this->start.tv_sec = 0;
    this->reset();
}

void TimeStamp::reset()
{
    clock_gettime( CLOCK_REALTIME, &this->start);
}

double TimeStamp::tsdiff(timespec& start, timespec& end) const
                         {
    timespec result;
    double retval = 0.0;
    /* Perform the carry for the later subtraction by updating */
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        result.tv_sec = end.tv_sec - start.tv_sec - 1;
        result.tv_nsec = (long int) (1E9) + end.tv_nsec - start.tv_nsec;
    } else {
        result.tv_sec = end.tv_sec - start.tv_sec;
        result.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    retval = result.tv_sec;
    retval += (double) result.tv_nsec / (double) 1E9;
    return retval;
}

double TimeStamp::seconds_until_now()
{
    timespec now;
    clock_gettime( CLOCK_REALTIME, &now);
    double stampdiff = this->tsdiff(this->start, now);
    return stampdiff;
}

class SWMRReader {
public:
    SWMRReader(const char * fname);
    ~SWMRReader();
    void open_file();
    void get_test_data();
    unsigned long long latest_frame_number();
    void read_latest_frame();
    bool check_dataset();
    void monitor_dataset(double timeout = 2.0);

private:
    void print_open_objects();

    LoggerPtr m_log;
    string m_filename;
    hid_t m_fid;
    hsize_t m_dims[3];
    hsize_t m_maxdims[3];

    Frame m_testimg;
    unsigned int * m_pdata;
    unsigned long long m_latest_framenumber;
    double m_polltime;
    std::vector<bool> m_checks;
};

SWMRReader::SWMRReader(const char * fname)
{
    this->m_log = Logger::getLogger("SWMRReader");
    LOG4CXX_DEBUG(m_log, "SWMRReader constructor. Filename: " << fname);
    this->m_filename = fname;
    this->m_fid = -1;
    this->m_polltime = 0.2;
    this->m_pdata = NULL;
    this->m_latest_framenumber = 0;
}

SWMRReader::~SWMRReader()
{
    LOG4CXX_DEBUG(m_log, "SWMRReader destructor");

    if (this->m_fid >= 0) {
        assert(H5Fclose(this->m_fid) >= 0);
        this->m_fid = -1;
    }

    if (this->m_pdata != NULL) {
        delete[] this->m_pdata;
        this->m_pdata = NULL;
    }
}

void SWMRReader::open_file()
{
    // sanity check
    assert(this->m_filename != "");

    /* Create file access property list */
    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl >= 0);
    /* Set to use the latest library format */
    assert(H5Pset_libver_bounds(fapl, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) >= 0);

    this->m_fid = H5Fopen(this->m_filename.c_str(),
                        H5F_ACC_RDONLY | H5F_ACC_SWMR_READ, fapl);
    assert(this->m_fid >= 0);
}

void SWMRReader::get_test_data()
{
    LOG4CXX_DEBUG(m_log, "Getting test data from swmr_testdata");
    vector<hsize_t> dims(2);
    dims[0] = swmr_testdata_cols;
    dims[1] = swmr_testdata_rows;
    this->m_testimg = Frame(dims, (const unsigned int*)(swmr_testdata[0]));


    // Allocate some space for our reading-in buffer
    this->m_pdata = this->m_testimg.create_buffer();
}

unsigned long long SWMRReader::latest_frame_number()
{
    herr_t status;
    hid_t dset;
    assert(this->m_fid >= 0);
    dset = H5Dopen2(m_fid, "data", H5P_DEFAULT);
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
    assert(m_testimg.dimensions()[0] == m_dims[0]);
    assert(m_testimg.dimensions()[1] == m_dims[1]);
    LOG4CXX_DEBUG(m_log, "Got dimensions: " << m_dims[0] << ", "
                  << m_dims[1] << ", " << m_dims[2] << ", ");

    assert(H5Dclose(dset) >= 0);
    assert(H5Sclose(dspace) >= 0);

    return m_dims[2];
}

void SWMRReader::read_latest_frame()
{
    herr_t status;
    hid_t dset;
    assert(this->m_fid >= 0);
    dset = H5Dopen2(m_fid, "data", H5P_DEFAULT);
    assert(dset >= 0);

    /* Get the dataspace */
    hid_t dspace;
    dspace = H5Dget_space(dset);
    assert(dspace >= 0);

    hsize_t offset[3] = { 0, 0, m_dims[2] - 1 };
    assert(offset[2] >= 0);
    hsize_t img_size[3] = { m_dims[0], m_dims[1], 1 };
    assert(H5Sselect_hyperslab(dspace, H5S_SELECT_SET, offset,
                               NULL, img_size, NULL) >= 0);

    hid_t memspace;
    memspace = H5Screate_simple(2, m_dims, NULL);
    assert(memspace >= 0);
    status = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset,
                                NULL, img_size, NULL);
    assert(status >= 0);
    LOG4CXX_DEBUG(m_log, "Reading dataset");
    status = H5Dread(dset, H5T_NATIVE_UINT32,
                     memspace, dspace, H5P_DEFAULT, static_cast<void*>(this->m_pdata));
    assert(status >= 0);
    this->m_latest_framenumber = m_dims[2];

    // Cleanup
    this->print_open_objects();
    assert(H5Dclose(dset) >= 0);
    assert(H5Sclose(dspace) >= 0);
    assert(H5Sclose(memspace) >= 0);
}

bool SWMRReader::check_dataset()
{
    Frame readimg(m_testimg.dimensions(), this->m_pdata);
    bool result = readimg == m_testimg;
    if (result != true) {
        LOG4CXX_WARN(m_log, "Data mismatch. Frame = " << m_latest_framenumber);
    }
    return result;
}

void SWMRReader::monitor_dataset(double timeout)
{
    bool carryon = true;
    bool check_result;
    LOG4CXX_DEBUG(m_log, "Starting monitoring");
    TimeStamp ts;

    while (carryon) {
        if (this->latest_frame_number() > this->m_latest_framenumber) {
            this->read_latest_frame();
            check_result = this->check_dataset();
            this->m_checks.push_back(check_result);
            ts.reset();
        } else {
            double secs = ts.seconds_until_now();
            if (timeout > 0 && secs > timeout) {
                LOG4CXX_WARN(m_log, "Timeout: it's been " << secs
                             << " seconds since last read");
                carryon = false;
            } else {
                usleep((unsigned int) (this->m_polltime * 1000000));
            }
        }
    }
}

void SWMRReader::print_open_objects()
{
    LOG4CXX_TRACE(m_log, "    DataSets open:    " << H5Fget_obj_count( this->m_fid, H5F_OBJ_DATASET ));
    LOG4CXX_TRACE(m_log, "    Groups open:      " << H5Fget_obj_count( this->m_fid, H5F_OBJ_GROUP ));
    LOG4CXX_TRACE(m_log, "    Attributes open:  " << H5Fget_obj_count( this->m_fid, H5F_OBJ_ATTR ));
    LOG4CXX_TRACE(m_log, "    Datatypes open:   " << H5Fget_obj_count( this->m_fid, H5F_OBJ_DATATYPE ));
    LOG4CXX_TRACE(m_log, "    Files open:       " << H5Fget_obj_count( this->m_fid, H5F_OBJ_FILE ));
    LOG4CXX_TRACE(m_log, "    Sum objects open: " << H5Fget_obj_count( this->m_fid, H5F_OBJ_ALL ));
}

int main()
{
    DOMConfigurator::configure("Log4cxxConfig.xml");
    LoggerPtr log(Logger::getLogger("main"));

    LOG4CXX_INFO(log, "Creating a SWMR Reader object");
    SWMRReader srd = SWMRReader("swmr.h5");

    LOG4CXX_INFO(log, "Opening file");
    srd.open_file();

    LOG4CXX_INFO(log, "Getting test data");
    srd.get_test_data();

    LOG4CXX_INFO(log, "Starting monitor");
    srd.monitor_dataset();

    return 0;
}

