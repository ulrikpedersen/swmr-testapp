#include <iostream>
#include <string>
#include <assert.h>

#include <cstdlib>
#include <ctime>

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;

#include "hdf5.h"
#include "swmr-testdata.h"
#include "swmr-writer.h"

using namespace std;


SWMRWriter::SWMRWriter(const string& fname)
{

    this->log = Logger::getLogger("SWMRWriter");
    LOG4CXX_DEBUG(log, "SWMRWriter constructor. Filename: " << fname);
    this->filename = fname;
    this->fid = -1;
    this->delay = 0.2;
}

void SWMRWriter::create_file()
{
    hid_t fapl; /* File access property list */
    hid_t fcpl;

    /* Create file access property list */
    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl >= 0);

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

void SWMRWriter::write_test_data(unsigned int niter, unsigned int nframes_cache)
{
    hid_t dataspace, dataset;
    hid_t filespace, memspace;
    hid_t prop;
    herr_t status;
    hsize_t chunk_dims[3];
    hsize_t max_dims[3];
    hsize_t img_dims[3];
    hsize_t offset[3] = { 0, 0, 0 };
    hsize_t size[3];

    assert(this->img.dimensions().size() == 2);
    chunk_dims[0] = nframes_cache;
    chunk_dims[1] = this->img.dimensions()[0];
    chunk_dims[2] = this->img.dimensions()[1];

    max_dims[0] = H5S_UNLIMITED;
    max_dims[1] = this->img.dimensions()[0];
    max_dims[2] = this->img.dimensions()[1];

    img_dims[0] = 1;
    img_dims[1] = this->img.dimensions()[0];
    img_dims[2] = this->img.dimensions()[1];
    size[0] = 1;
    size[1] = this->img.dimensions()[0];
    size[2] = this->img.dimensions()[1];

    /* Create the dataspace with the given dimensions - and max dimensions */
    dataspace = H5Screate_simple(3, img_dims, max_dims);
    assert(dataspace >= 0);

    /* Enable chunking  */
    prop = H5Pcreate(H5P_DATASET_CREATE);
    status = H5Pset_chunk(prop, 3, chunk_dims);
    assert(status >= 0);

    /* Create dataset  */
    LOG4CXX_DEBUG(log, "Creating dataset");
    dataset = H5Dcreate2(this->fid, "data", H5T_NATIVE_INT, dataspace,
    H5P_DEFAULT, prop, H5P_DEFAULT);

    /* Enable SWMR writing mode */
    assert(H5Fstart_swmr_write(this->fid) >= 0);
    LOG4CXX_INFO(log, "File writer in SWMR mode. Clients can start reading");

    LOG4CXX_DEBUG(log, "Starting write loop. Iterations: " << niter);
    for (int i = 0; i < niter; i++) {
        /* Extend the dataset  */
        LOG4CXX_TRACE(log, "Extending. Size: " << size[2]
                      << ", " << size[1] << ", " << size[0]);
        status = H5Dset_extent(dataset, size);
        assert(status >= 0);

        /* Select a hyperslab */
        filespace = H5Dget_space(dataset);
        status = H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL,
                                     img_dims, NULL);
        assert(status >= 0);

        /* Write the data to the hyperslab */
        LOG4CXX_TRACE(log, "Writing. Offset: " << offset[0] << ", "
                      << offset[1] << ", " << offset[2]);
        status = H5Dwrite(dataset, H5T_NATIVE_UINT32, dataspace, filespace,
        H5P_DEFAULT, this->img.pdata());
        assert(status >= 0);

        /* Increment offsets and dimensions as appropriate */
        offset[0]++;
        size[0]++;

        LOG4CXX_TRACE(log, "Flushing");
        assert(H5Dflush(dataset) >= 0);

        LOG4CXX_TRACE(log, "Sleeping " << this->delay << "s");
        usleep((unsigned int) (this->delay * 1000000));
    }

    LOG4CXX_DEBUG(log, "Closing intermediate open HDF objects");
    H5Dclose(dataset);
    H5Pclose(prop);
    H5Sclose(dataspace);
    H5Sclose(filespace);
}

SWMRWriter::~SWMRWriter()
{
    LOG4CXX_DEBUG(log, "SWMRWriter destructor");
    if (this->fid >= 0) {
        assert(H5Fclose(this->fid) >= 0);
        this->fid = -1;
    }
}

