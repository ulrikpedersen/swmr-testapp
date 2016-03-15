swmr-testapp
============

A small test application to read and verify data as it is written to a HDF5 file 
using the SWMR functionality

[![Build Status](https://travis-ci.org/ulrikpedersen/swmr-testapp.svg?branch=master)](https://travis-ci.org/ulrikpedersen/swmr-testapp)

Building
========

The build system is using cmake. The following libraries are required:

* cmake (vesion >= 2.8)
* [HDF5](http://www.hdfgroup.org): with the 
[SWMR functionality](http://www.hdfgroup.org/HDF5/docNewFeatures/NewFeaturesSwmrDocs.html)
(version >= 1.9.178)
* [Boost](http://www.boost.org): only the program_options component is used 
(version >= 1.41.0)
* [Log4CXX](http://logging.apache.org/log4cxx/): Configurable logger (version >= 0.10.0)

If the dependencies are installed in non-standard locations, use the following
cmake-variables to inform cmake where to find them:

* HDF5_ROOT
* BOOST_ROOT
* LOG4CXX_ROOT_DIR

The standard cmake variables can be used to configure boost and HDF5. For example
to do a static build with HDF5, set the variable HDF5_USE_STATIC_LIBRARIES=ON.

To install the swmr-testapp into a custom location, use the standard cmake
variable: CMAKE_INSTALL_PREFIX 

Configure and build like this:

    tar -zxf swmr-testapp.tar.gz
    cd swmr-testapp
    mkdir build
    cd build
    cmake -DHDF5_ROOT=/path/to/hdf5/swmr/installation \
          -DBOOST_ROOT=/path/to/boost/installation \
          -DCMAKE_INSTALL_PREFIX=/path/to/install/destination
          ..
    make VERBOSE=1
    make install

Example DLS build for distribution
----------------------------------

The build environment at DLS is 64bit RHEL6 with a custom installation of log4cxx which
is linked statically. The following build configuration is used:

    mkdir install
    cd swmr-testapp
    mkdir build
    cd build
    cmake -DHDF5_ROOT=../hdf5swmr/ \
          -DCMAKE_INSTALL_PREFIX=../../install/ \
          -DLOG4CXX_ROOT_DIR=/dls_sw/prod/tools/RHEL6-x86_64/log4cxx/0-10-0/prefix/ \
          -DBOOST_ROOT=/usr \
          -DCMAKE_VERBOSE_MAKEFILE=ON \
          -DBoost_NO_BOOST_CMAKE=ON \
          ..
    make VERBOSE=1
    make install
    
This results in the swmr binary, log configuration file and test data being installed
into the 'install' directory.

Installing
==========

The make install command will install the application into a system location.

If the dependent libraries are not installed in system directories it may be 
useful to configure the build with the cmake variable 
CMAKE_INSTALL_RPATH_USE_LINK_PATH=ON as this will retain the rpath in the binary
and it will remember where to find its dependencies.

Running
=======

The "swmr" application takes a subcommand, either: "read" or "write". 

Both reader and writer operate by loading in a reference dataset (a single 2D
image in a 3D dataset) from a file is one is supplied (--testdatafile). 
Writer and reader must use the same test data and DATAFILE to succeed. 
The 2D image chunking (the last 2 dims) configuration of the test dataset is u
sed in the writer when creating the new dataset in the output DATAFILE.

An example --testdatafile could look something like this:

    h5dump -pH testdata/img_16M.h5 
    HDF5 "testdata/img_16M.h5" {
    GROUP "/" {
        DATASET "data" {
            DATATYPE  H5T_STD_U32LE
            DATASPACE  SIMPLE { ( 1, 4096, 1024 ) / ( 1, 4096, 1024 ) }
            STORAGE_LAYOUT {
                CHUNKED ( 1, 4096, 1024 )
                SIZE 16777216
            }
            FILTERS {
                NONE
            }
            FILLVALUE {
                FILL_TIME H5D_FILL_TIME_ALLOC
                VALUE  0
            }
            ALLOCATION_TIME {
                H5D_ALLOC_TIME_INCR
            }
        }
    }
    }


The writer will repeatedly write the reference dataset into a growing 3D dataset
in the output DATAFILE. Multiple readers can be started after the writer has
started and output the message "##### SWMR mode ######". The readers will monitor
the growing 3D dataset and for each notification of size change, the latest 2D
image from DATAFILE will be read back and compared against the reference dataset.

The reader will output a report at the end, indicating how many images it compared
and a summary of the result of the comparisons.

Each subcommand provide it's own online help. For the reader:

    swmr read -h
    Usage:
      swmr read [options] [DATAFILE]
    
        DATAFILE: The HDF5 SWMR datafile to operate on.
    
    Option Groups:
    
    Common options:
      -h [ --help ]                    Produce help message and quit
      -s [ --dataset ] arg (=data)     Name of HDF5 SWMR dataset to use
      -f [ --testdatafile ] arg        HDF5 reference test data file name
      -d [ --testdataset ] arg (=data) HDF5 reference dataset name
      -l [ --logconfig ] arg           Log4CXX XML configuration file
    
    Command options:
      -n [ --nframes ] arg (=-1) Number of frames to expect in input dataset (-1: 
                                 unknown)
      -t [ --timeout ] arg (=2)  Timeout [sec] waiting for new data
      -p [ --polltime ] arg (=1) Monitor polling time [sec]

The writer:

    swmr write -h
    Usage:
      swmr write [options] [DATAFILE]
    
        DATAFILE: The HDF5 SWMR datafile to operate on.
    
    Option Groups:
    
    Common options:
      -h [ --help ]                    Produce help message and quit
      -s [ --dataset ] arg (=data)     Name of HDF5 SWMR dataset to use
      -f [ --testdatafile ] arg        HDF5 reference test data file name
      -d [ --testdataset ] arg (=data) HDF5 reference dataset name
      -l [ --logconfig ] arg           Log4CXX XML configuration file
    
    Command options:
      -n [ --niter ] arg (=2) Number of write iterations
      -c [ --chunk ] arg (=1) Number of chunked frames
