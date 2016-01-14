# Try to find the OpenDHT librairies
# OPENDHT_FOUND - system has OPENDHT lib
# OPENDHT_INCLUDE_DIR - the OPENDHT include directory
# OPENDHT_LIBRARIES - Libraries needed to use OPENDHT

if (OPENDHT_INCLUDE_DIR AND OPENDHT_LIBRARIES)
                # Already in cache, be silent
                set(OPENDHT_FIND_QUIETLY TRUE)
endif (OPENDHT_INCLUDE_DIR AND OPENDHT_LIBRARIES)

find_path(OPENDHT_INCLUDE_DIR NAMES opendht.h )
find_library(OPENDHT_LIBRARIES NAMES opendht libopendht )
MESSAGE(STATUS "OpenDHT libs: " ${OPENDHT_LIBRARIES} )

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENDHT DEFAULT_MSG OPENDHT_INCLUDE_DIR OPENDHT_LIBRARIES)

mark_as_advanced(OPENDHT_INCLUDE_DIR OPENDHT_LIBRARIES)
