# Copyright (c) Monetas AG, 2014

set(cxx-sources
  OTSocket.cpp
  Socket_ZMQ4.cpp
)

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/
)

include_directories(SYSTEM
  ${dependency_include_dir}
  ${dependency_include_dir}/libzmq_4/include
  ${dependency_include_dir}/cppzmq
)

include_directories(
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
elseif(MSVC)
  # Windows ZMQ library has a difficult name
  target_link_libraries(opentxs-ext opentxs-core libzmq-mt-gd-4_1_0)
else()
  target_link_libraries(opentxs-ext opentxs-core zmq)
endif()

if (NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS opentxs-ext DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
