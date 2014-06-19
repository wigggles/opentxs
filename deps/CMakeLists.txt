# Copyright (c) Monetas AG, 2014

cmake_minimum_required(VERSION 2.8)

#add_subdirectory(ChaiScript) // we only use the Chai-Headers
add_subdirectory(gtest)
add_subdirectory(irrxml)


### Build libzmq as static library
execute_process(COMMAND sed "s/add_library(libzmq SHARED/add_library(libzmq STATIC/" "-i" "${CMAKE_CURRENT_SOURCE_DIR}/libzmq_4/CMakeLists.txt")
execute_process(COMMAND sed "s|set(LIBRARY_OUTPUT_PATH  \${CMAKE_CURRENT_BINARY_DIR}/lib)| |" "-i" "${CMAKE_CURRENT_SOURCE_DIR}/libzmq_4/CMakeLists.txt")

add_subdirectory(libzmq_4)

### Build lucre as library
set(lucre-sources
	${CMAKE_CURRENT_SOURCE_DIR}/lucre/src/bankimp.cpp
   )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lucre/include/lucre/)

add_library(lucre
	    STATIC
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
	    STATIC
	    ${bigint-sources}
)
