# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(message)

set(
  cxx-sources
  Bitcoin.cpp
  Header.cpp
  Message.cpp
  Peer.cpp
)

set(
  cxx-headers
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/p2p/bitcoin/Factory.hpp"
  Header.hpp
  Message.hpp
  Peer.hpp
)

add_library(opentxs-blockchain-p2p-bitcoin OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(
  opentxs-blockchain-p2p-bitcoin
  PRIVATE Boost::headers opentxs::messages
)
set_property(
  TARGET opentxs-blockchain-p2p-bitcoin
  PROPERTY POSITION_INDEPENDENT_CODE 1
)
