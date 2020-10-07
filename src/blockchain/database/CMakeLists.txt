# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources
    Blocks.cpp
    Database.cpp
    Filters.cpp
    Headers.cpp
    Wallet.cpp
)

set(cxx-headers
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/database/Database.hpp"
    Blocks.hpp
    Database.hpp
    Filters.hpp
    Headers.hpp
    Wallet.hpp
)

add_library(opentxs-blockchain-database OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(
  opentxs-blockchain-database
  PRIVATE
    Boost::headers
    opentxs::messages
    lmdb
    opentxs-api-client-blockchain-database
)

set_property(
  TARGET opentxs-blockchain-database PROPERTY POSITION_INDEPENDENT_CODE 1
)
