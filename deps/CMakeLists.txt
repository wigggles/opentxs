# Copyright (c) Monetas AG, 2014

#add_subdirectory(ChaiScript)
add_subdirectory(gtest)
add_subdirectory(irrxml)

### Build lucre as library
set(lucre-sources
  ${CMAKE_CURRENT_SOURCE_DIR}/lucre/src/bankimp.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lucre/include/lucre/ ${OPENSSL_INCLUDE_DIR})


if(WIN32)
  # suppress sscanf/sprintf being unsafe warnings 
  add_definitions("/wd4996")

  add_library(lucre
    STATIC
    ${lucre-sources}
  )

  target_link_libraries(lucre ${LIB_EAY_RELEASE})
else()
  add_library(lucre
    ${lucre-sources}
  )

  target_link_libraries(lucre ${OPENSSL_LIBRARIES})
endif()


### Build bitcoin-base58 as library
add_library(bitcoin-base58
  STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin-base58/base58.cpp
)
target_link_libraries(bitcoin-base58 ${OPENSSL_LIBRARIES})


### Build bigint as library
set(bigint-sources
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigInteger.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigIntegerAlgorithms.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigIntegerUtils.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigUnsigned.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigUnsignedInABase.cc
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/bigint/)

if(WIN32)
  include_directories(${OPENSSL_INCLUDE_DIR})
  add_library(bigint
    STATIC
    ${bigint-sources}
  )
else()
  add_library(bigint
    ${bigint-sources}
  )
endif()


### Build anyoption as library
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(anyoption
  STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/anyoption/anyoption.cpp
)

if(ANDROID)
  set(ZMQ_BUILD_TESTS OFF CACHE BOOL "")
  add_definitions(-DHAVE_LIBSODIUM)
  add_subdirectory(libzmq_4)
  find_library(SODIUM_LIBRARY sodium)
  set_property(TARGET libzmq PROPERTY LINK_LIBRARIES "${SODIUM_LIBRARY}")
  # needed for czmq
  set(ZEROMQ_LIBRARIES libzmq)
  set(ZEROMQ_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/libzmq_4/include)
  add_definitions(-D_CRT_ERRNO_DEFINED)
endif()

# Disable czmq tests. They also cause trouble on Android MIPS
set(CZMQ_BUILD_TESTS false CACHE BOOL "")
add_subdirectory(czmq)
# Create czmq_local for use inside opentxs' CMake project. Since czmq_local is
# marked as IMPORTED, CMake will not complain about it not being exported.
add_library(czmq_local SHARED IMPORTED GLOBAL)
set(lib_ext so)
if (APPLE)
  set(lib_ext dylib)
endif()
set_property(TARGET czmq_local PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/libczmq.${lib_ext})
add_dependencies(czmq_local czmq)
if (NOT ANDROID)
  file(STRINGS "${ZEROMQ_INCLUDE_DIRS}/zmq.h" ZMQ_VERSION_MAJOR REGEX "^#define ZMQ_VERSION_MAJOR [0-9]$")
  string(REGEX REPLACE "^#define ZMQ_VERSION_MAJOR ([0-9])$" "\\1" ZMQ_VERSION_MAJOR "${ZMQ_VERSION_MAJOR}")
  if (NOT ZMQ_VERSION_MAJOR STREQUAL "4")
    message(FATAL_ERROR "Wrong ZeroMQ major version ${ZMQ_VERSION_MAJOR}. Required version is 4.x.x")
  endif()
else()
  # reset ZEROMQ_INCLUDE_DIRS because its value was overridden by czmq's CMakeLists.txt
  set(ZEROMQ_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/libzmq_4/include PARENT_SCOPE)
endif()
set(CZMQ_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/czmq/include PARENT_SCOPE)


if (NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS bigint lucre
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          EXPORT opentxsTargets
          COMPONENT main)
endif()
