# Copyright (c) Monetas AG, 2014

set(cxx-sources
  ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/anyoption/anyoption.cpp
  Helpers.cpp
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
  OTWallet.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/*.hpp")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client
)

if(WIN32)
  # suppress "decorated name length exceeded, name was truncated" warnings 
  add_definitions("/wd4503")
endif()

add_library(opentxs-client
  ${cxx-sources}
  ${cxx-headers}
)

target_link_libraries(opentxs-client opentxs-ext opentxs-core)

if(WIN32)
  add_custom_command(TARGET opentxs-client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-client.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-client.dll")

  add_custom_command(TARGET opentxs-client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-client.pdb"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-client.pdb")
endif()

if(NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS opentxs-client
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT)
endif()

