# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MODULE_NAME opentxs-script)

set(cxx-sources
  OTStash.cpp
  OTStashItem.cpp
  OTAgent.cpp
  OTBylaw.cpp
  OTClause.cpp
  OTParty.cpp
  OTPartyAccount.cpp
  OTScript.cpp
  OTScriptable.cpp
  OTScriptChai.cpp
  OTSmartContract.cpp
  OTVariable.cpp
)

file(GLOB cxx-install-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/script/*.hpp")

set(cxx-headers
  ${cxx-install-headers}
)

if (WIN32)
  # suppress "decorated name length exceeded, name was truncated" warnings
  add_definitions(/wd4503)

  # OTScriptChai needs this flag to avoid error C1128: number of sections exceeded object file format limit
  add_definitions(/bigobj)
endif()

add_library(${MODULE_NAME}
  OBJECT
  ${cxx-sources}
  ${cxx-headers}
)

install(FILES ${cxx-install-headers}
        DESTINATION include/opentxs/core/script
        COMPONENT dev)
target_compile_options(${MODULE_NAME} PRIVATE -Wno-switch-default) # ChaiScript
set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
target_include_directories(${MODULE_NAME} PRIVATE "../../../deps/ChaiScript/include")
target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/")
