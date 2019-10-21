# Copyright (c) 2010-2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources CompactSize.cpp Inventory.cpp)

set(
  cxx-headers
  ${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/Bitcoin.hpp
  CompactSize.hpp Inventory.hpp
)

add_library(opentxs-blockchain-bitcoin OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-blockchain-bitcoin PRIVATE Boost::headers)
set_property(
  TARGET opentxs-blockchain-bitcoin
  PROPERTY POSITION_INDEPENDENT_CODE 1
)
