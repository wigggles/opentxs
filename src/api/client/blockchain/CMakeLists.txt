# Copyright (c) 2010-2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
  BalanceList.cpp
  BalanceNode.cpp
  BalanceTree.tpp
  BalanceTree.cpp
  Deterministic.cpp
  HD.cpp
)
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/BalanceList.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/BalanceNode.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/BalanceTree.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/Deterministic.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/HD.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/Imported.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/api/client/blockchain/PaymentCode.hpp"
)
set(
  cxx-headers
  ${cxx-install-headers}
  "${opentxs_SOURCE_DIR}/src/internal/api/client/blockchain/Blockchain.hpp"
  BalanceList.hpp
  BalanceNode.hpp
  BalanceTree.hpp
  Deterministic.hpp
  HD.hpp
)

add_library(opentxs-api-client-blockchain OBJECT ${cxx-sources} ${cxx-headers})
set_property(
  TARGET opentxs-api-client-blockchain
  PROPERTY POSITION_INDEPENDENT_CODE 1
)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/api/client/blockchain
  COMPONENT dev
)
