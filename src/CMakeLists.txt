set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package( Boost 1.41.0
              REQUIRED
              COMPONENTS program_options )

##### Find HDF5 with SWMR ###################
message("\nSearching for HDF5 with SWMR")
FIND_PATH(HDF5_INC_DIR
          NAMES hdf5.h
          PATHS /usr/include
                /usr/local/include
                ${swmr-testapp_SOURCE_DIR}/hdf5swmr/include)
FIND_LIBRARY(HDF5_LIBRARY
          NAMES hdf5
          PATHS 
                ${swmr-testapp_SOURCE_DIR}/hdf5swmr/lib
                ${HDF5_ROOT})

IF(NOT HDF5_INC_DIR)

   MESSAGE(FATAL_ERROR "Unable to find HDF5 SWMR install dir")

ELSE(NOT HDF5_INC_DIR)

   MESSAGE("Found HDF5 SWMR include files in: " ${HDF5_INC_DIR})

ENDIF(NOT HDF5_INC_DIR)
##### End of HDF5 with SWMR search ##########


##### Find other dependencies ############
find_package(Log4CXX 0.10.0 REQUIRED)
find_package(ZLIB REQUIRED)

# librt is really only required if glibc =< 2.16
FIND_LIBRARY(REALTIME_LIBRARY
             NAMES rt)
##### End of dependency search ###########


# Include the directory itself as a path to include directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${HDF5_INC_DIR} ${LOG4CXX_INCLUDE_DIRS}/.. ${Boost_INCLUDE_DIRS})

# create a list of .cpp files from this directory
file(GLOB swmr_SOURCES *.cpp)

# Create an executable file called helloworld from sources:
add_executable(swmr ${swmr_SOURCES})

target_link_libraries(swmr ${ZLIB_LIBRARIES} ${Boost_LIBRARIES} ${LOG4CXX_LIBRARIES} ${HDF5_LIBRARY} ${REALTIME_LIBRARY})

INSTALL(TARGETS swmr
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
