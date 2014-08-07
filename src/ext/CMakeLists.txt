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

add_library(opentxs-ext
  ${cxx-sources}
)

if(WIN32)
  # Windows ZMQ library has a difficult name and is harder to find
  target_link_libraries(opentxs-ext opentxs-core ${CMAKE_BINARY_DIR}/deps/libzmq_4/lib/$<CONFIGURATION>/libzmq-mt-gd-4_1_0.lib)
else()
  target_link_libraries(opentxs-ext opentxs-core libzmq)
endif()

if(WIN32)
  add_custom_command(TARGET opentxs-ext POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-ext.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-ext.dll")

  add_custom_command(TARGET opentxs-ext POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-ext.pdb"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-ext.pdb")
endif()

if(NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS opentxs-ext
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT)
endif()
