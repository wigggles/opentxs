# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
  Block.cpp
  Input.cpp
  Output.cpp
  Transaction.cpp
  TransactionSection.cpp
  Witness.cpp
)

set(
  cxx-headers
  Block.hpp
  Input.hpp
  Output.hpp
  Transaction.hpp
  TransactionSection.hpp
  Witness.hpp
)

add_library(opentxs-blockchain-transaction-bitcoin OBJECT ${cxx-sources}
                                                          ${cxx-headers})
target_link_libraries(
  opentxs-blockchain-transaction-bitcoin
  PRIVATE Boost::headers
)
set_property(
  TARGET opentxs-blockchain-transaction-bitcoin
  PROPERTY POSITION_INDEPENDENT_CODE 1
)
