# Copyright (c) Monetas AG, 2014

set(cxx-sources
  ConfigLoader.cpp
  MainFile.cpp
  MessageProcessor.cpp
  Notary.cpp
  PayDividendVisitor.cpp
  ReplyMessage.cpp
  Server.cpp
  ServerSettings.cpp
  Transactor.cpp
  UserCommandProcessor.cpp
)

file(GLOB cxx-install-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/server/*.hpp")

set(cxx-headers
  ${cxx-install-headers}
)

include_directories(SYSTEM ${CZMQ_INCLUDE_DIRS})

set(MODULE_NAME opentxs-server)
if(WIN32)
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
        DESTINATION include/opentxs/server
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
