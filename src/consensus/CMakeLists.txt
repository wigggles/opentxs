# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
  ClientContext.cpp
  Context.cpp
  ManagedNumber.cpp
  ServerContext.cpp
  TransactionStatement.cpp
)
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/consensus/ClientContext.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/consensus/Context.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/consensus/ManagedNumber.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/consensus/ServerContext.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/consensus/TransactionStatement.hpp"
)
set(
  cxx-headers
  ${cxx-install-headers}
  "${opentxs_SOURCE_DIR}/src/internal/consensus/Consensus.hpp"
  ClientContext.hpp
  Context.hpp
  ManagedNumber.hpp
  ServerContext.hpp
)

add_library(opentxs-consensus OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-consensus PRIVATE opentxs::messages)
target_include_directories(
  opentxs-consensus SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
set_property(TARGET opentxs-consensus PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/consensus
  COMPONENT dev
)
