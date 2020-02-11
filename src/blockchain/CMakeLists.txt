# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(bitcoin)
add_subdirectory(block)
add_subdirectory(client)
add_subdirectory(p2p)
add_subdirectory(transaction)

set(
  cxx-sources
  Blockchain.cpp
  BloomFilter.cpp
  Database.cpp
  GCS.cpp
  NumericHash.cpp
  Work.cpp
)

set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Blockchain.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/BloomFilter.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Network.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/NumericHash.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Work.hpp"
)

set(
  cxx-headers
  ${cxx-install-headers}
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/Blockchain.hpp"
  Database.hpp
  GCS.hpp
  NumericHash.hpp
  Work.hpp
)

add_library(opentxs-blockchain OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-blockchain PRIVATE Boost::headers)
target_include_directories(
  opentxs-blockchain SYSTEM
  PRIVATE ${LMDB_INCLUDE_DIR}
)
set_property(TARGET opentxs-blockchain PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/blockchain
  COMPONENT dev
)
