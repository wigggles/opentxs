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
  NymData.cpp
  OT_API.cpp
  OTAPI_Exec.cpp
  OTAPI_Func.cpp
  OTClient.cpp
  OTMessageOutbuffer.cpp
  OTRecord.cpp
  OTRecordList.cpp
  OTWallet.cpp
  SwigWrap.cpp
  Utility.cpp
)

set(cxx-install-headers
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/Helpers.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/NymData.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/OTAPI_Exec.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/OT_API.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/OTClient.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/OTMessageOutbuffer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/OTRecord.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/OTRecordList.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/OTWallet.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/ServerAction.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/SwigWrap.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/Utility.hpp
)

set(cxx-headers
  ${cxx-install-headers}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdAcceptInbox.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdAcceptPayments.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdAcceptReceipts.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdAcceptTransfers.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdBaseAccept.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdBase.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdBaseInstrument.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdCancel.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdConfirm.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdDeposit.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdDiscard.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdExportCash.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdPayInvoice.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdSendCash.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdShowNyms.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/commands/CmdWithdrawCash.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/OTAPI_Func.hpp
)

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

install(FILES ${cxx-install-headers}
        DESTINATION include/opentxs/client
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
