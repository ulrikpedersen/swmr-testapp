#include <iostream>
#include <string>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <assert.h>

#include <cstdlib>

#include <log4cxx/logger.h>
using namespace log4cxx;

#include "hdf5.h"
#include "hdf5_hl.h"
#include "timestamp.h"
#include "swmr-testdata.h"
#include "progressbar.h"
#include "swmr-writer.h"

using namespace std;


SWMRWriter::SWMRWriter(const string& fname)
{

    this->log = Logger::getLogger("SWMRWriter");
    LOG4CXX_TRACE(log, "SWMRWriter constructor. Filename: " << fname);
    this->filename = fname;
    this->fid = -1;
    dt_start = 0.0;
    nframes = 0;
}

void SWMRWriter::create_file()
{
    hid_t fapl; /* File access property list */
    hid_t fcpl;

    /* Create file access property list */
    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl >= 0);

    assert(H5Pset_fclose_degree(fapl, H5F_CLOSE_STRONG) >= 0);

    /* Set chunk boundary alignment to 4MB */
    assert( H5Pset_alignment( fapl, 65536, 4*1024*1024 ) >= 0);

    /* Set to use the latest library format */
    assert(H5Pset_libver_bounds(fapl, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) >= 0);

    /* Create file creation property list */
    if ((fcpl = H5Pcreate(H5P_FILE_CREATE)) < 0)
    assert(fcpl >= 0);

    /* Creating the file with SWMR write access*/
    LOG4CXX_INFO(log, "Creating file: " << filename);
    unsigned int flags = H5F_ACC_TRUNC;
    this->fid = H5Fcreate(this->filename.c_str(), flags, fcpl, fapl);
    assert(this->fid >= 0);

    /* Close file access property list */
    assert(H5Pclose(fapl) >= 0);
}

void SWMRWriter::get_test_data()
{
    LOG4CXX_DEBUG(log, "Getting test data from swmr_testdata");
    vector<hsize_t> dims(2);
    dims[0] = swmr_testdata_cols;
    dims[1] = swmr_testdata_rows;
    this->img = Frame(dims, (const uint32_t*)(swmr_testdata[0]));
}

void SWMRWriter::get_test_data(const string& fname, const string& dsetname)
{
    LOG4CXX_DEBUG(log, "Getting test data from file/dset: "
                  << fname << "/" << dsetname);
    this->img = Frame(fname, dsetname);
}

void SWMRWriter::write_test_data(unsigned int niter,
                                 unsigned int nframes_cache)
{
    hid_t dataspace=0, dataset=0;
    hid_t filespace=0, memspace=0;
    hid_t prop=0;
    herr_t status=0;
    hsize_t chunk_dims[3];
    hsize_t max_dims[3];
    hsize_t img_dims[3];
    hsize_t offset[3] = { 0, 0, 0 };
    hsize_t size[3];

    assert(this->img.dimensions().size() == 2);
    chunk_dims[0] = nframes_cache;
    chunk_dims[1] = this->img.chunks()[0];
    chunk_dims[2] = this->img.chunks()[1];

    max_dims[0] = H5S_UNLIMITED;
    max_dims[1] = this->img.dimensions()[0];
    max_dims[2] = this->img.dimensions()[1];

    img_dims[0] = 1;
    img_dims[1] = this->img.dimensions()[0];
    img_dims[2] = this->img.dimensions()[1];
    size[0] = 1;
    size[1] = this->img.dimensions()[0];
    size[2] = this->img.dimensions()[1];

    double full_cache_size = sizeof(uint32_t) *
                             this->img.dimensions()[0] *
                             this->img.dimensions()[1] *
                             nframes_cache;
    full_cache_size = full_cache_size / (1024. * 1024.); // in MegaBytes

    /* Create the dataspace with the given dimensions - and max dimensions */
    dataspace = H5Screate_simple(3, img_dims, max_dims);
    assert(dataspace >= 0);

    /* Enable chunking  */
    LOG4CXX_DEBUG(log, "Chunking=" << chunk_dims[0] << ","
                       << chunk_dims[1] << ","
                       << chunk_dims[2]);
    prop = H5Pcreate(H5P_DATASET_CREATE);
    status = H5Pset_chunk(prop, 3, chunk_dims);
    assert(status >= 0);

    /* dataset access property list */
    hid_t dapl = H5Pcreate(H5P_DATASET_ACCESS);
    size_t nbytes = img_dims[1] * img_dims[2] * sizeof(uint32_t) * chunk_dims[0];
    size_t nslots = static_cast<size_t>(ceil((double)max_dims[1] / chunk_dims[1]) * niter);
    nslots *= 13;
    LOG4CXX_DEBUG(log, "Chunk cache nslots=" << nslots << " nbytes=" << nbytes);
    assert( H5Pset_chunk_cache( dapl, nslots, nbytes, 1.0) >= 0);

    /* Create dataset  */
    LOG4CXX_DEBUG(log, "Creating dataset");
    dataset = H5Dcreate2(this->fid, "data", H5T_NATIVE_INT, dataspace,
    H5P_DEFAULT, prop, dapl);

    /* Enable SWMR writing mode */
    assert(H5Fstart_swmr_write(this->fid) >= 0);
    LOG4CXX_INFO(log, "##### SWMR mode ######");
    LOG4CXX_INFO(log, "Clients can start reading");
    if (!log->isInfoEnabled()) cout << "##### SWMR mode ######" << endl;

	size_t databuf_nbytes = this->img.num_bytes_chunk(); // used for direct chunk write

    TimeStamp ts;
    LOG4CXX_DEBUG(log, "Starting write loop. Iterations: " << niter);
    bool show_pbar = not log->isDebugEnabled();
    if (show_pbar) progressbar(0, niter);
    double writetime = 0.;
    double writerate = 0.;
    TimeStamp globaltime;
    globaltime.reset();
    ts.reset();
    for (int i = 0; i < niter; i++) {
        /* Extend the dataset  */
        LOG4CXX_TRACE(log, "Extending. Size: " << size[2]
                      << ", " << size[1] << ", " << size[0]);
        status = H5Dset_extent(dataset, size);
        assert(status >= 0);

        if (true) {
        	uint32_t filter_mask = 0x0;
        	status = H5DOwrite_chunk(dataset, H5P_DEFAULT,
        							 filter_mask, offset,
        							 databuf_nbytes, this->img.pdata());
            assert(status >= 0);
        } else {
            /* Select a hyperslab */
            filespace = H5Dget_space(dataset);
            assert(filespace >= 0);
            status = H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL,
                                         img_dims, NULL);
            assert(status >= 0);

            /* Write the data to the hyperslab */
            LOG4CXX_DEBUG(log, "Writing. Offset: " << offset[0] << ", "
                          << offset[1] << ", " << offset[2]);
            status = H5Dwrite(dataset, H5T_NATIVE_UINT32, dataspace, filespace,
            H5P_DEFAULT, this->img.pdata());
            assert(status >= 0);
        }

        /* Increment offsets and dimensions as appropriate */
        offset[0]++;
        size[0]++;

        if ((i+1) % nframes_cache == 0)
        {
            LOG4CXX_TRACE(log, "Flushing");
            assert(H5Dflush(dataset) >= 0);
            writetime = ts.seconds_until_now();
            write_times.push_back(writetime);
            writerate = full_cache_size / writetime;
            LOG4CXX_DEBUG(log, "Writetime: " << writetime << " ["
                          << writerate << "MB/s]");
            ts.reset();
            dt_start = globaltime.seconds_until_now();
        }

        if (show_pbar) progressbar(i+1, niter, writerate);
    }

    dt_start = globaltime.seconds_until_now();
    nframes = niter;

    LOG4CXX_DEBUG(log, "Closing intermediate open HDF objects");
    assert( H5Dclose(dataset) >= 0);
    assert( H5Pclose(prop) >= 0);
    assert( H5Pclose(dapl) >= 0);
    assert( H5Sclose(dataspace) >= 0);
}

void SWMRWriter::report()
{
    ostringstream oss;
    double sum = accumulate(write_times.begin(), write_times.end(), 0.0);
    double mean = sum / write_times.size();

    double sq_sum = inner_product(write_times.begin(), write_times.end(), write_times.begin(), 0.0);
    double stdev = sqrt(sq_sum / write_times.size() - mean * mean);
    double imgsize = this->img.dimensions()[0] * this->img.dimensions()[1] * sizeof(uint32_t);
    imgsize = imgsize / (1024. * 1024); // in megabytes
    double dsetsize = imgsize * nframes;

    oss << endl << "======= SWMR writer report ========" << endl << endl
        << " Number of writes: " << write_times.size() << endl
        << fixed << setprecision(1)
        << "    Overall time:    " << dt_start << "s\n"
        << "            rate:    " << dsetsize/dt_start << "MB/s\n"
        << fixed << setprecision(3)
        << " Mean write time:    " << mean << "s (stddev: "<< stdev << "s)\n"
        << "             min:    " << *min_element(write_times.begin(), write_times.end()) << "s\n"
        << "             max:    " << *max_element(write_times.begin(), write_times.end()) << "s\n"
        << endl;
    if (not log->isDebugEnabled()) cout << oss.str();
    LOG4CXX_DEBUG(log, oss.str());
}

SWMRWriter::~SWMRWriter()
{
    LOG4CXX_TRACE(log, "SWMRWriter destructor");
    if (this->fid >= 0) {
        assert(H5Fclose(this->fid) >= 0);
        this->fid = -1;
    }
}

