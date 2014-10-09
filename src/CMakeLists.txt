set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package( Boost 1.41.0
              REQUIRED
              COMPONENTS program_options )
find_package(Log4CXX 0.10.0 REQUIRED)

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


# Include the directory itself as a path to include directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#INCLUDE_DIRECTORIES(${HDF5_INC_DIR} ${log4cxx_INC_DIR})
INCLUDE_DIRECTORIES(${HDF5_INC_DIR} ${LOG4CXX_INCLUDE_DIRS}/.. ${Boost_INCLUDE_DIRS})

# Create a variable called swmr_SOURCES containing all .cpp files:
#set(swmr_SOURCES frame.cpp swmr-demo.cpp swmr-reader.cpp swmr-writer.cpp)

# For a large number of source files you can create it in a simpler way
# using file() function:
file(GLOB swmr_SOURCES *.cpp)

# Create an executable file called helloworld from sources:
add_executable(swmr ${swmr_SOURCES})
target_link_libraries(swmr z rt ${Boost_LIBRARIES} ${LOG4CXX_LIBRARIES} ${HDF5_LIBRARY})
