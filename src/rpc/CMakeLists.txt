# Copyright (c) 2010-2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources RPC.cpp)
set(cxx-install-headers "")
set(
  cxx-headers
  ${cxx-install-headers}
  ${opentxs_SOURCE_DIR}/src/internal/rpc/RPC.hpp
  RPC.hpp
  RPC.tpp
)

add_library(opentxs-rpc OBJECT ${cxx-sources} ${cxx-headers})
set_property(TARGET opentxs-rpc PROPERTY POSITION_INDEPENDENT_CODE 1)
