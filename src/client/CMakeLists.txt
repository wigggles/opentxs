# Copyright (c) Monetas AG, 2014

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
  OTME_too.cpp
  OTRecord.cpp
  OTRecordList.cpp
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
  add_library(${MODULE_NAME} OBJECT
  ${cxx-sources}
  ${cxx-headers}
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
