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
          -DHDF5_USE_STATIC_LIBRARIES=ON \
          -DBOOST_ROOT=/path/to/boost/installation \
          -DCMAKE_INSTALL_PREFIX=/path/to/install/destination
          ..
    make VERBOSE=1
    make install


Running
=======

The "swmr" application takes a subcommand, either: "read" or "write". 

Both reader and writer operate by loading in a reference dataset (a single 2D
image) from a file is one is supplied (--testdatafile). Writer and reader must
use the same test data and DATAFILE to succeed.

The writer will repeatedly write the reference dataset into a growing 3D dataset
in the output DATAFILE. Multiple readers can be started after the writer has
started and output the message "##### SWMR mode ######". The readers will monitor
the growing 3D dataset and for each notification of size change, the latest 2D
image from DATAFILE will be read back and compared against the reference dataset.

The reader will output a report at the end, indicating how many images it compared
and a summary of the result of the comparisons.

Each subcommand provide it's own online help. For the reader:

    swmr read -h
    Available subcommands: [help|read|write] 
    
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
    Available subcommands: [help|read|write] 
    
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
      -n [ --niter ] arg (=2)  Number of write iterations
      -c [ --chunk ] arg (=1)  Number of chunked frames
      -p [ --period ] arg (=1) Write loop period [sec]
