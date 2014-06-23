# Copyright (c) Monetas AG, 2014

#add_subdirectory(ChaiScript)
add_subdirectory(gtest)
add_subdirectory(irrxml)

### Build lucre as library
set(lucre-sources
  ${CMAKE_CURRENT_SOURCE_DIR}/lucre/src/bankimp.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lucre/include/lucre/)

add_library(lucre
  ${lucre-sources}
)

### Build bigint as library
set(bigint-sources
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigInteger.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigIntegerAlgorithms.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigIntegerUtils.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigUnsigned.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigUnsignedInABase.cc
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/bigint/)

add_library(bigint
  ${bigint-sources}
)

### Build libzmq as static library
#execute_process(COMMAND sed "-i" "" "s|add_library(libzmq SHARED|add_library(libzmq STATIC|" "${CMAKE_CURRENT_SOURCE_DIR}/libzmq_4/CMakeLists.txt")
execute_process(COMMAND sed "-i.bak" "s|set(LIBRARY_OUTPUT_PATH  \${CMAKE_CURRENT_BINARY_DIR}/lib)| |" "${CMAKE_CURRENT_SOURCE_DIR}/libzmq_4/CMakeLists.txt")

set(ZMQ_BUILD_TESTS OFF CACHE BOOL "")

add_subdirectory(libzmq_4)
