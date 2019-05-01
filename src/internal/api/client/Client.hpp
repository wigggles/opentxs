// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/UniqueQueue.hpp"

#include "internal/core/identifier/Identifier.hpp"
#include "internal/core/Core.hpp"

#include <future>
#include <string>
#include <tuple>

namespace opentxs::api::client::internal
{
struct Activity : virtual public api::client::Activity {
    virtual void MigrateLegacyThreads() const = 0;

    virtual ~Activity() = default;
};
struct Contacts : virtual public api::client::Contacts {
    virtual void start() = 0;

    virtual ~Contacts() = default;
};
struct Manager : virtual public api::client::Manager {
    virtual void StartActivity() = 0;
    virtual void StartContacts() = 0;
    virtual opentxs::OTWallet* StartWallet() = 0;

    virtual ~Manager() = default;
};

struct OTX : virtual public api::client::OTX {
    virtual void associate_message_id(
        const Identifier& messageID,
        const TaskID taskID) const = 0;
    virtual Depositability can_deposit(
        const OTPayment& payment,
        const identifier::Nym& recipient,
        const Identifier& accountIDHint,
        identifier::Server& depositServer,
        identifier::UnitDefinition& unitID,
        Identifier& depositAccount) const = 0;
    virtual bool finish_task(
        const TaskID taskID,
        const bool success,
        Result&& result) const = 0;
    virtual UniqueQueue<OTNymID>& get_nym_fetch(
        const identifier::Server& serverID) const = 0;
    virtual BackgroundTask start_task(const TaskID taskID, bool success)
        const = 0;
};
}  // namespace opentxs::api::client::internal
