# Copyright (c) Monetas AG, 2014

cmake_minimum_required(VERSION 2.8)

set(cxx-sources
  OTSocket.cpp
)

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/
)

include_directories(
  ${dependency_include_dir}
  ${dependency_include_dir}/libzmq_4/include
  ${dependency_include_dir}/cppzmq
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/otlib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/otextensions
)

add_library(opentxs-ext
  STATIC
  ${cxx-sources}
)

target_link_libraries(opentxs-ext opentxs-core zmq)
