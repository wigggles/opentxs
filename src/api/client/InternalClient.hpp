// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_INTERNALCLIENT_HPP
#define OPENTXS_API_CLIENT_INTERNALCLIENT_HPP

#include "Internal.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Client.hpp"
#include "opentxs/api/client/Contacts.hpp"

namespace opentxs::api::client::internal
{
struct Activity : virtual public api::client::Activity {
    virtual void MigrateLegacyThreads() const = 0;
};
struct Client : virtual public api::client::Client {
    virtual void StartActivity() = 0;
    virtual void StartContacts() = 0;
    virtual opentxs::OTWallet* StartWallet() = 0;
};
struct Contacts : virtual public api::client::Contacts {
    virtual void start() = 0;
};
}  // namespace opentxs::api::client::internal
#endif  // OPENTXS_API_CLIENT_INTERNALCLIENT_HPP
