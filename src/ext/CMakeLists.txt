# Copyright (c) Monetas AG, 2014

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
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ext
)

link_directories(
  ${CMAKE_BINARY_DIR}/lib
)

add_library(opentxs-ext
  ${cxx-sources}
)

if(APPLE)
  target_link_libraries(opentxs-ext opentxs-core ZeroMQ)
else()
  target_link_libraries(opentxs-ext opentxs-core zmq)
endif()
