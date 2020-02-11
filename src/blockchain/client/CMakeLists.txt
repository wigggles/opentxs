# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(bitcoin)

set(
  cxx-sources
  Client.cpp
  FilterOracle.cpp
  HeaderOracle.cpp
  Network.cpp
  PeerManager.cpp
  UpdateTransaction.cpp
)

set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/client/HeaderOracle.hpp"
)

set(
  cxx-headers
  ${cxx-install-headers}
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/client/Client.hpp"
  FilterOracle.hpp
  HeaderOracle.hpp
  Network.hpp
  PeerManager.hpp
  UpdateTransaction.hpp
)

add_library(opentxs-blockchain-client OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-blockchain-client PRIVATE Boost::headers)
set_property(
  TARGET opentxs-blockchain-client
  PROPERTY POSITION_INDEPENDENT_CODE 1
)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/blockchain/client
  COMPONENT dev
)
