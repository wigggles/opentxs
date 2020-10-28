# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources base58.cpp)
set(cxx-install-headers "")
set(cxx-headers ${cxx-install-headers} base58.h)

add_library(opentxs-api-crypto-base58 OBJECT ${cxx-sources} ${cxx-headers})
