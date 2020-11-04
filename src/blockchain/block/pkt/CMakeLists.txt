# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources Block.cpp)

set(cxx-install-headers "")

set(cxx-headers ${cxx-install-headers} Block.hpp)

add_library(opentxs-blockchain-block-pkt OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(
  opentxs-blockchain-block-pkt PRIVATE Boost::headers opentxs::messages
)
