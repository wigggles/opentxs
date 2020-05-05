# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
  Accounts.cpp
  Bip47Channels.cpp
  BlockchainTransactions.cpp
  Contacts.cpp
  Contexts.cpp
  Credentials.cpp
  Issuers.cpp
  Node.cpp
  Mailbox.cpp
  Notary.cpp
  Nym.cpp
  Nyms.cpp
  PaymentWorkflows.cpp
  PeerReplies.cpp
  PeerRequests.cpp
  Root.cpp
  Seeds.cpp
  Servers.cpp
  Thread.cpp
  Threads.cpp
  Tree.cpp
  Txos.cpp
  Units.cpp
)
set(cxx-install-headers "")
set(
  cxx-headers
  ${cxx-install-headers}
  Accounts.hpp
  Bip47Channels.hpp
  BlockchainTransactions.hpp
  Contacts.hpp
  Contexts.hpp
  Credentials.hpp
  Issuers.hpp
  Node.hpp
  Mailbox.hpp
  Notary.hpp
  Nym.hpp
  Nyms.hpp
  PaymentWorkflows.hpp
  PeerReplies.hpp
  PeerRequests.hpp
  Root.hpp
  Seeds.hpp
  Servers.hpp
  Thread.hpp
  Threads.hpp
  Tree.hpp
  Txos.hpp
  Units.hpp
)

add_library(opentxs-storage-tree OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-storage-tree PRIVATE opentxs::messages)
set_property(TARGET opentxs-storage-tree PROPERTY POSITION_INDEPENDENT_CODE 1)
