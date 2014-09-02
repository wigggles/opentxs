# Copyright (c) Monetas AG, 2014

set(cxx-sources
  OTSocket.cpp
  Socket_ZMQ4.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ext/*.hpp")

include_directories(SYSTEM
  ${ZMQ_INCLUDE_DIR}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ext
)

add_library(opentxs-ext
  ${cxx-sources}
  ${cxx-headers}
)

target_link_libraries(opentxs-ext opentxs-core ${ZMQ_LIBRARY})

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
