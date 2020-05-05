# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(bitcoin)

set(cxx-sources "Address.cpp" "Peer.cpp")

set(cxx-headers "${opentxs_SOURCE_DIR}/src/internal/blockchain/p2p/P2P.hpp"
                "Address.hpp" "Peer.hpp")

set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/p2p/Address.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/p2p/Peer.hpp"
)

add_library(opentxs-blockchain-p2p OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(
  opentxs-blockchain-p2p
  PRIVATE Boost::headers opentxs::messages
)
set_property(TARGET opentxs-blockchain-p2p PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/blockchain/p2p
  COMPONENT dev
)
