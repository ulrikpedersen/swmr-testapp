# - Try to find Apache Portable Runtime Utility
# Once done this will define
#  LIBAPR-UTIL_FOUND - System has Apache Portable Runtime Utility
#  LIBAPR-UTIL_INCLUDE_DIRS - The Apache Portable Runtime Utility include directories
#  LIBAPR-UTIL_LIBRARIES - The libraries needed to use Apache Portable Runtime Utility
#  LIBAPR-UTIL_DEFINITIONS - Compiler switches required for using Apache Portable Runtime Utility

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_LIBAPR-UTIL REQUIRED apr-1)
set(LIBAPR-UTIL_DEFINITIONS ${PC_LIBAPR-UTIL_CFLAGS_OTHER})

find_path(LIBAPR-UTIL_INCLUDE_DIR apu.h
          HINTS ${PC_LIBAPR-UTIL_INCLUDEDIR} ${PC_LIBAPR-UTIL_INCLUDE_DIRS}
          PATH_SUFFIXES apr-1 apr-1.0)

find_library(LIBAPR-UTIL_LIBRARY NAMES aprutil-1 libaprutil-1
             HINTS ${PC_LIBAPR-UTIL_LIBDIR} ${PC_LIBAPR-UTIL_LIBRARY_DIRS})

set(LIBAPR-UTIL_LIBRARIES ${LIBAPR-UTIL_LIBRARY})
set(LIBAPR-UTIL_INCLUDE_DIRS ${LIBAPR-UTIL_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBAPR-UTIL_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibApr-Util DEFAULT_MSG
                                  LIBAPR-UTIL_LIBRARY LIBAPR-UTIL_INCLUDE_DIR)

mark_as_advanced(LIBAPR-UTIL_INCLUDE_DIR LIBAPR-UTIL_LIBRARY)

