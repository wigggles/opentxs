#[[
// clang-format off
]]#
# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MODULE_NAME opentxs-api-client-blockchain)

set(cxx-sources
  BalanceList.cpp
  BalanceNode.cpp
  BalanceTree.tpp
  BalanceTree.cpp
  Deterministic.cpp
  HD.cpp
)

set(cxx-install-headers
  ../../../../include/opentxs/api/client/blockchain/BalanceList.hpp
  ../../../../include/opentxs/api/client/blockchain/BalanceNode.hpp
  ../../../../include/opentxs/api/client/blockchain/BalanceTree.hpp
  ../../../../include/opentxs/api/client/blockchain/Deterministic.hpp
  ../../../../include/opentxs/api/client/blockchain/HD.hpp
  ../../../../include/opentxs/api/client/blockchain/Imported.hpp
  ../../../../include/opentxs/api/client/blockchain/PaymentCode.hpp
)

set(cxx-headers
  ${cxx-install-headers}
  ../../../internal/api/client/blockchain/Blockchain.hpp
  BalanceList.hpp
  BalanceNode.hpp
  BalanceTree.hpp
  Deterministic.hpp
  HD.hpp
)

if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
    ../../../core/util/OTWindowsRegistryTools.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

install(FILES ${cxx-install-headers}
        DESTINATION include/opentxs/api/client/blockchain
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
#[[
// clang-format on
]]#
