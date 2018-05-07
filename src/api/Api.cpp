/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/stdafx.hpp"

#include "opentxs/core/Identifier.hpp"

#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/client/Sync.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/Api.hpp"
#include "opentxs/api/Activity.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/api/Settings.hpp"
//#if OT_CASH
//#include "opentxs/cash/Purse.hpp"
//#endif //OT_CASH
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
//#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
//#include "opentxs/ext/OTPayment.hpp"

#include "client/Cash.hpp"
#include "client/ServerAction.hpp"
#include "client/Workflow.hpp"

#include <set>
#include <map>

#include "Api.hpp"

namespace opentxs::api::implementation
{
Api::Api(
    const Flag& running,
    const api::Activity& activity,
    const api::Settings& config,
    const api::ContactManager& contacts,
    const api::Crypto& crypto,
    const api::Identity& identity,
    const api::storage::Storage& storage,
    const api::client::Wallet& wallet,
    const api::network::ZMQ& zmq)
    : running_(running)
    , activity_(activity)
    , config_(config)
    , contacts_(contacts)
    , crypto_(crypto)
    , identity_(identity)
    , storage_(storage)
    , wallet_(wallet)
    , zmq_(zmq)
    , ot_api_(nullptr)
    , otapi_exec_(nullptr)
    , cash_(nullptr)
    , pair_(nullptr)
    , server_action_(nullptr)
    , sync_(nullptr)
    , workflow_(nullptr)
    , lock_()
    , map_lock_()
    , context_locks_()
{
    Init();
}

void Api::Cleanup()
{
    pair_.reset();
    sync_.reset();
    cash_.reset();
    server_action_.reset();
    otapi_exec_.reset();
    ot_api_.reset();
    workflow_.reset();
}

std::recursive_mutex& Api::get_lock(const ContextID context) const
{
    std::unique_lock<std::mutex> lock(map_lock_);

    return context_locks_[context];
}

void Api::Init()
{
    otLog3 << "\n\nWelcome to Open Transactions -- version " << Log::Version()
           << "\n";

    workflow_.reset(new api::client::implementation::Workflow(
        activity_, contacts_, storage_));

    OT_ASSERT(workflow_)

    ot_api_.reset(new OT_API(
        activity_,
        config_,
        contacts_,
        crypto_,
        identity_,
        storage_,
        wallet_,
        *workflow_,
        zmq_,
        std::bind(&Api::get_lock, this, std::placeholders::_1)));

    OT_ASSERT(ot_api_);

    otapi_exec_.reset(new OTAPI_Exec(
        activity_,
        config_,
        contacts_,
        crypto_,
        identity_,
        wallet_,
        zmq_,
        *ot_api_,
        std::bind(&Api::get_lock, this, std::placeholders::_1)));

    OT_ASSERT(otapi_exec_);

    server_action_.reset(new api::client::implementation::ServerAction(
        *ot_api_,
        *otapi_exec_,
        wallet_,
        *workflow_,
        std::bind(&Api::get_lock, this, std::placeholders::_1)));

    OT_ASSERT(server_action_)

    cash_.reset(new api::client::implementation::Cash());

    OT_ASSERT(cash_);

    sync_.reset(api::client::Sync::Factory(
        running_,
        *ot_api_,
        *otapi_exec_,
        contacts_,
        config_,
        *this,
        wallet_,
        *workflow_,
        crypto_.Encode(),
        zmq_.Context(),
        std::bind(&Api::get_lock, this, std::placeholders::_1)));

    OT_ASSERT(sync_);

    pair_.reset(api::client::Pair::Factory(
        running_,
        *sync_,
        *server_action_,
        wallet_,
        *ot_api_,
        *otapi_exec_,
        zmq_.Context()));

    OT_ASSERT(pair_);
}

const OTAPI_Exec& Api::Exec(const std::string&) const
{
    OT_ASSERT(otapi_exec_);

    return *otapi_exec_;
}

std::recursive_mutex& Api::Lock(
    const Identifier& nymID,
    const Identifier& serverID) const
{
    return get_lock({nymID.str(), serverID.str()});
}

const OT_API& Api::OTAPI(const std::string&) const
{
    OT_ASSERT(ot_api_);

    return *ot_api_;
}

const api::client::Cash& Api::Cash() const
{
    OT_ASSERT(cash_);

    return *cash_;
}

const api::client::Pair& Api::Pair() const
{
    OT_ASSERT(pair_);

    return *pair_;
}

const api::client::ServerAction& Api::ServerAction() const
{
    OT_ASSERT(server_action_);

    return *server_action_;
}

const api::client::Sync& Api::Sync() const
{
    OT_ASSERT(sync_);

    return *sync_;
}

const api::client::Workflow& Api::Workflow() const
{
    OT_ASSERT(workflow_);

    return *workflow_;
}

Api::~Api() {}
}  // namespace opentxs::api::implementation
