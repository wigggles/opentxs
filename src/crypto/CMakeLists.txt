# Copyright (c) 2010-2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(key)
add_subdirectory(library)

set(cxx-sources Bip32.cpp)
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/Bip32.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/Bip39.hpp"
)
set(cxx-headers ${cxx-install-headers} Bip32.hpp)

add_library(opentxs-crypto OBJECT ${cxx-sources} ${cxx-headers})
set_property(TARGET opentxs-crypto PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/crypto
  COMPONENT dev
)
