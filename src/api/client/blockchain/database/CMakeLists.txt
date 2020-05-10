# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
  BlockFilter.cpp
  BlockHeaders.cpp
  Blocks.cpp
  Database.cpp
  Peers.cpp
)
set(
  cxx-headers
  BlockFilter.hpp
  BlockHeaders.hpp
  Blocks.hpp
  Database.hpp
  Peers.hpp
)

add_library(opentxs-api-client-blockchain-database OBJECT ${cxx-sources}
                                                          ${cxx-headers})
target_include_directories(
  opentxs-api-client-blockchain-database SYSTEM
  PRIVATE ${LMDB_INCLUDE_DIR}
)
target_link_libraries(
  opentxs-api-client-blockchain-database
  PRIVATE opentxs::messages
)

if(OPENTXS_BLOCK_STORAGE_ENABLED)
  target_link_libraries(
    opentxs-api-client-blockchain-database
    PRIVATE Boost::iostreams Boost::headers Boost::filesystem
  )
endif()
set_property(
  TARGET opentxs-api-client-blockchain-database
  PROPERTY POSITION_INDEPENDENT_CODE 1
)

target_compile_definitions(
  opentxs-api-client-blockchain-database PRIVATE
  OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY=${OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY}
)
