# Copyright (c) 2010-2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_CRYPTO_SUPPORTED_KEY_HD)
  add_subdirectory(blockchain)
endif()

if(OT_CASH_EXPORT)
  add_subdirectory(blind)
endif()

add_subdirectory(client)
add_subdirectory(core)
add_subdirectory(contact)
add_subdirectory(crypto)
add_subdirectory(identity)
add_subdirectory(integration)
add_subdirectory(network/zeromq)
add_subdirectory(otx)
add_subdirectory(rpc)
add_subdirectory(ui)
