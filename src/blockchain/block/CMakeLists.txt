# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(bitcoin)

set(cxx-sources Block.cpp Header.cpp)

set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/Block.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/Header.hpp"
)

set(
  cxx-headers
  ${cxx-install-headers}
  "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/Block.hpp"
  Block.hpp
  Header.hpp
)

add_library(opentxs-blockchain-block OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(
  opentxs-blockchain-block
  PRIVATE Boost::headers opentxs::messages
)
set_property(
  TARGET opentxs-blockchain-block
  PROPERTY POSITION_INDEPENDENT_CODE 1
)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/blockchain/block
  COMPONENT dev
)
