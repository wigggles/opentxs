# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
  Block.cpp
  Header.cpp
  Input.cpp
  Inputs.cpp
  Output.cpp
  Outputs.cpp
  Script.cpp
  Transaction.cpp
)

set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Block.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Header.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Input.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Inputs.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Output.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Outputs.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Script.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/bitcoin/Transaction.hpp"
)

set(
  cxx-headers
  ${cxx-install-headers}
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/bitcoin/Bitcoin.hpp"
  Block.hpp
  Header.hpp
  Input.hpp
  Inputs.hpp
  Output.hpp
  Outputs.hpp
  Script.hpp
  Transaction.hpp
)

add_library(opentxs-blockchain-block-bitcoin OBJECT ${cxx-sources}
                                                    ${cxx-headers})
target_link_libraries(
  opentxs-blockchain-block-bitcoin
  PRIVATE Boost::headers opentxs::messages
)
set_property(
  TARGET opentxs-blockchain-block-bitcoin
  PROPERTY POSITION_INDEPENDENT_CODE 1
)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/blockchain/block/bitcoin
  COMPONENT dev
)
