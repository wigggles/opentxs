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


set(ZMQ_BUILD_TESTS OFF CACHE BOOL "")

if (NOT ZMQ_LIB_FOUND)
  add_subdirectory(libzmq_4)
endif()

if(ANDROID)
  # needed for czmq
  add_definitions(-D_CRT_ERRNO_DEFINED)
endif()
if(NOT WIN32)
  # Disable czmq tests. They cause trouble on Android MIPS
  # and should not be executed anyway for our project.
  execute_process(COMMAND sed "-i" "/^add_executable(czmq_selftest \${SOURCE_DIR}\\/src\\/czmq_selftest.c)$/d" "${CMAKE_CURRENT_SOURCE_DIR}/czmq/CMakeLists.txt")
  execute_process(COMMAND sed "-i" "/^target_link_libraries(czmq_selftest czmq \${ZEROMQ_LIBRARIES})$/d" "${CMAKE_CURRENT_SOURCE_DIR}/czmq/CMakeLists.txt")
  execute_process(COMMAND sed "-i" "/^add_test(czmq_selftest czmq_selftest)/d" "${CMAKE_CURRENT_SOURCE_DIR}/czmq/CMakeLists.txt")
endif()
add_subdirectory(czmq)
# Create czmq_local for use inside opentxs' CMake project. Since czmq_local is
# marked as IMPORTED, CMake will not complain about it not being exported.
add_library(czmq_local SHARED IMPORTED GLOBAL)
set_property(TARGET czmq_local PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/libczmq.so)
add_dependencies(czmq_local czmq)

if (NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS bigint lucre
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          EXPORT opentxsTargets
          COMPONENT main)
endif()
