# Copyright (c) Monetas AG, 2014

set(cxx-sources
  OpenTransactions.cpp
  ot_commands_ot.cpp
  ot_made_easy_ot.cpp
  OT_ME.cpp
  ot_otapi_ot.cpp
  ot_utility_ot.cpp
  OTAPI.cpp
  OTAPI_Exec.cpp
  OTClient.cpp
  OTMeCpp.cpp
  OTRecord.cpp
  OTRecordList.cpp
  OTServerConnection.cpp
)

if(MSVC)
  # suppress "decorated name length exceeded, name was truncated" warnings 
  add_Definitions("/wd4503")
endif()

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/
)

include_directories(SYSTEM
  ${dependency_include_dir}
  ${dependency_include_dir}/ChaiScript/include
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ext
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/api
)

link_directories(
  ${CMAKE_BINARY_DIR}/lib
)

add_library(opentxs-api
  ${cxx-sources}
)

target_link_libraries(opentxs-api opentxs-ext opentxs-core)

if(WIN32)
  add_custom_command(TARGET opentxs-api POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-api.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-api.dll")

  add_custom_command(TARGET opentxs-api POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-api.pdb"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-api.pdb")
endif()

if(NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS opentxs-api DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

