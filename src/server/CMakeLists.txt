# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MODULE_NAME opentxs-server)

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

set(cxx-headers
  ConfigLoader.hpp
  Macros.hpp
  MainFile.hpp
  MessageProcessor.hpp
  Notary.hpp
  PayDividendVisitor.hpp
  ReplyMessage.hpp
  Server.hpp
  ServerSettings.hpp
  Transactor.hpp
  UserCommandProcessor.hpp
)

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

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../deps/")
