# Copyright (c) Monetas AG, 2014

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

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/script/*.hpp")

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/
)

include_directories(SYSTEM
  ${dependency_include_dir}
  ${dependency_include_dir}/ChaiScript/include/
)

if (WIN32)
  # suppress "decorated name length exceeded, name was truncated" warnings
  add_definitions(/wd4503)

  # OTScriptChai needs this flag to avoid error C1128: number of sections exceeded object file format limit
  add_definitions(/bigobj)
endif()

set(MODULE_NAME opentxs-script)
add_library(${MODULE_NAME}
  OBJECT
  ${cxx-sources}
  ${cxx-headers}
)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
