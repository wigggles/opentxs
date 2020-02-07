# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
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
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTAgent.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTBylaw.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTClause.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTParty.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTPartyAccount.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTScript.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTScriptable.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTScriptChai.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTSmartContract.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTStash.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTStashItem.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/script/OTVariable.hpp"
)
set(cxx-headers ${cxx-install-headers})

add_library(opentxs-script OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-script SYSTEM
  PRIVATE
    "${opentxs_SOURCE_DIR}/deps/ChaiScript/include"
    "${opentxs_SOURCE_DIR}/deps/"
)
set_property(TARGET opentxs-script PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/core/script
  COMPONENT dev
)
