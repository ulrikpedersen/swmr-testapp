# swmr-testapp project cmake build system
# 
# This project builds a single executable binary using a standard cmake build.
#
# Build instructions:
#   cd swmr-testapp
#   mkdir build
#   cmake ..
#   make
#   sudo make install
#
# The cmake command will search for the dependencies in the usual places. If you
# have custom installations of these libraries and headers, you can tell cmake
# about it with the usual macros (cmake -D<macro>=/path/to/installation ..)
#
# The supported macros for the relevant dependencies are:
#   HDF5_ROOT
#   BOOST_ROOT
#   LOG4CXX_ROOT_DIR


# Project name 
project(swmr-testapp)

# States that CMake required version must be greater than 2.8
cmake_minimum_required(VERSION 2.8)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Appends the cmake/modules path inside the MAKE_MODULE_PATH variable which stores the
# directories of additional CMake modules (ie. MacroOutOfSourceBuild.cmake):
set(CMAKE_MODULE_PATH ${swmr-testapp_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# The macro below forces the build directory to be different from source directory:
#include(MacroOutOfSourceBuild)
#macro_ensure_out_of_source_build("${PROJECT_NAME} requires an out of source build.")

add_subdirectory(src)
install(FILES   swmr_log_conf.xml DESTINATION etc  COMPONENT config)
install(FILES  testdata/img_4M.h5 DESTINATION data COMPONENT config)
install(FILES testdata/img_16M.h5 DESTINATION data COMPONENT config)

