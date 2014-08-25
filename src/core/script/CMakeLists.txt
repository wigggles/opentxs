# Copyright (c) Monetas AG, 2014

set(cxx-sources
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

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/script
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/
)

add_library(opentxs-script
  STATIC
  ${cxx-sources}
  ${cxx-headers}
)
