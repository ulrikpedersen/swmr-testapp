# create a list of .cpp files from this directory
file(GLOB swmr_SOURCES *.cpp)

# Create an executable file called helloworld from sources:
add_executable(swmr ${swmr_SOURCES})

### Search for and configure the boost libraries ###
find_package( Boost 1.41.0
              REQUIRED
              COMPONENTS program_options)

IF (NOT Boost_FOUND)
  message(FATAL_ERROR "Unable to find the boost installation")
ELSE (NOT Boost_FOUND)

    ####### HACK ALERT ##############
    # The following IF() is basically a hack to work around a problem that
    # the default FindBoost.cmake implementation ignores BOOST_ROOT with respect
    # to finding the libraries. See thread: 
    #   http://www.cmake.org/pipermail/cmake/2010-March/035889.html
    IF (BOOST_ROOT)
      message("BOOST_ROOT defined. Fixing broken FindBoost.cmake results...")
      set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib)
      set(Boost_LIBRARY_DIRS ${BOOST_ROOT}/lib)
      find_library(BOOST_LIBS
                   NAMES boost_program_options
                   PATHS ${BOOST_ROOT}/lib
                   NO_DEFAULT_PATH)
      IF (NOT BOOST_LIBS)
        message(FATAL_ERROR "Did not find BOOST libs in " ${BOOST_ROOT}/lib)
      ELSE (NOT BOOST_LIBS)
        set(Boost_LIBRARIES ${BOOST_LIBS})
        message("Found:          " ${BOOST_LIBS})
      ENDIF (NOT BOOST_LIBS)
    ENDIF (BOOST_ROOT)
    ####### END OF HACK ALERT ########

  message("Boost libs:     " ${Boost_LIBRARIES})
  message("Boost inc dir:  " ${Boost_INCLUDE_DIRS})
ENDIF (NOT Boost_FOUND)
### End of boost search ###

##### Find HDF5 with SWMR ###################
message("\nSearching for HDF5 with SWMR")

IF (HDF5_ROOT)
  message("HDF5_ROOT set: " ${HDF5_ROOT})
  set( ENV{HDF5_ROOT} ${HDF5_ROOT} )
  find_path( HDF5_INCLUDE_DIRS
             NAMES hdf5.h hdf5_hl.h
             PATHS ${HDF5_ROOT}/include
             NO_DEFAULT_PATH )
  find_library( HDF5_LIBRARIES
                NAMES hdf5 hdf5_hl
                PATHS ${HDF5_ROOT}/lib
                NO_DEFAULT_PATH )
  find_library( HDF5HL_LIBRARIES
                NAMES hdf5_hl
                PATHS ${HDF5_ROOT}/lib
                NO_DEFAULT_PATH )
ELSE(HDF5_ROOT)
    set(HDF5HL_LIBRARIES "")
    find_package(HDF5
                 REQUIRED)

    IF (NOT HDF5_FOUND)
      message(FATAL_ERROR "Unable to find HDF5 installation")
    ELSE (NOT HDF5_FOUND)
      message("HDF5 lib dir:        " ${HDF5_LIBRARY_DIRS})
    ENDIF (NOT HDF5_FOUND)
ENDIF(HDF5_ROOT)

message("HDF5 include files:  " ${HDF5_INCLUDE_DIRS})
message("HDF5 libs:           " ${HDF5_LIBRARIES} ${HDF5HL_LIBRARIES})

##### End of HDF5 with SWMR search ##########


##### Find other dependencies ############
find_package(AprUtil REQUIRED)
message("apr libs: " ${LIBAPR-UTIL_LIBRARIES})
find_package(ZLIB REQUIRED)

# librt is really only required if glibc =< 2.16
FIND_LIBRARY(REALTIME_LIBRARY
             NAMES rt)

# Find a static library version of liblog4cxx
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
find_package(Log4CXX 0.10.0 REQUIRED)
##### End of dependency search ###########

# Include the directory itself as a path to include directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${HDF5_INCLUDE_DIRS} ${LOG4CXX_INCLUDE_DIRS}/.. ${Boost_INCLUDE_DIRS})
add_definitions(${HDF5_DEFINITIONS})

# List the libraries to link into the exe
target_link_libraries(swmr ${Boost_LIBRARIES} ${LOG4CXX_LIBRARIES} ${HDF5_LIBRARIES} ${HDF5HL_LIBRARIES} ${REALTIME_LIBRARY} ${ZLIB_LIBRARIES} ${CMAKE_DL_LIBS} ${LIBAPR-UTIL_LIBRARIES})

INSTALL(TARGETS swmr
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

