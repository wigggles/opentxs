# Copyright (c) Monetas AG, 2014

include_directories(SYSTEM
  ${ZEROMQ_INCLUDE_DIRS}
  ${CZMQ_INCLUDE_DIR}
)

set(cxx-sources
  commands/CmdAcceptInbox.cpp
  commands/CmdAcceptPayments.cpp
  commands/CmdAcceptReceipts.cpp
  commands/CmdAcceptTransfers.cpp
  commands/CmdBase.cpp
  commands/CmdBaseAccept.cpp
  commands/CmdBaseInstrument.cpp
  commands/CmdCancel.cpp
  commands/CmdConfirm.cpp
  commands/CmdDeposit.cpp
  commands/CmdDiscard.cpp
  commands/CmdExportCash.cpp
  commands/CmdPayInvoice.cpp
  commands/CmdSendCash.cpp
  commands/CmdShowNyms.cpp
  commands/CmdWithdrawCash.cpp
  Helpers.cpp
  MadeEasy.cpp
  OT_API.cpp
  OT_ME.cpp
  OTAPI_Exec.cpp
  OTAPI_Func.cpp
  OTAPI_Wrap.cpp
  OTClient.cpp
  OTMessageBuffer.cpp
  OTMessageOutbuffer.cpp
  OTRecord.cpp
  OTRecordList.cpp
  OTServerConnection.cpp
  OTWallet.cpp
  Utility.cpp
)

file(GLOB_RECURSE cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

set(MODULE_NAME opentxs-client)
if(WIN32)
  # suppress "decorated name length exceeded, name was truncated" warnings
  add_definitions("/wd4503")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME}
  ${cxx-sources}
  ${cxx-headers}
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME}
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

target_link_libraries(opentxs-client PUBLIC opentxs-ext opentxs-cash opentxs-core opentxs-storage PRIVATE czmq_local)
set_lib_property(opentxs-client)

add_library(opentxs-proto SHARED IMPORTED)
add_library(opentxs-verify SHARED IMPORTED)

set_property(TARGET opentxs-proto PROPERTY IMPORTED_LOCATION ${OPENTXS_PROTO})
set_property(TARGET opentxs-verify PROPERTY IMPORTED_LOCATION ${OPENTXS_VERIFY})

target_link_libraries(opentxs-client PRIVATE opentxs-proto)
target_link_libraries(opentxs-client PRIVATE opentxs-verify)

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
          EXPORT opentxsTargets
          COMPONENT main)
endif()
