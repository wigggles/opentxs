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

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Activity.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OT_ME.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/Log.hpp"

#include "client/Pair.hpp"
#include "client/ServerAction.hpp"
#include "client/Sync.hpp"

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
    , ot_me_(nullptr)
    , pair_(nullptr)
    , server_action_(nullptr)
    , sync_(nullptr)
    , lock_()
{
    Init();
}

void Api::Cleanup()
{
    pair_.reset();
    sync_.reset();
    ot_me_.reset();
    server_action_.reset();
    otapi_exec_.reset();
    ot_api_.reset();
}

void Api::Init()
{
    // Changed this to otErr (stderr) so it doesn't muddy the output.
    otLog3 << "\n\nWelcome to Open Transactions -- version " << Log::Version()
           << "\n";

    otLog4 << "(transport build: OTMessage -> OTEnvelope -> ZMQ )\n";

    // TODO in the case of Windows, figure err into this return val somehow.
    // (Or log it or something.)

    ot_api_.reset(new OT_API(
        activity_,
        config_,
        contacts_,
        crypto_,
        identity_,
        storage_,
        wallet_,
        zmq_,
        lock_));

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
        lock_));

    OT_ASSERT(otapi_exec_);

    server_action_.reset(new api::client::implementation::ServerAction(
        lock_, *ot_api_, *otapi_exec_, wallet_));

    OT_ASSERT(server_action_)

    ot_me_.reset(
        new OT_ME(lock_, *otapi_exec_, *ot_api_, *server_action_, wallet_));

    OT_ASSERT(ot_me_);

    sync_.reset(new api::client::implementation::Sync(
        lock_,
        running_,
        *ot_api_,
        *otapi_exec_,
        contacts_,
        config_,
        *this,
        wallet_,
        crypto_.Encode()));

    OT_ASSERT(sync_);

    pair_.reset(new api::client::implementation::Pair(
        running_,
        lock_,
        *sync_,
        *server_action_,
        wallet_,
        *ot_api_,
        *otapi_exec_));

    OT_ASSERT(pair_);
}

const OTAPI_Exec& Api::Exec(const std::string&) const
{
    OT_ASSERT(otapi_exec_);

    return *otapi_exec_;
}

std::recursive_mutex& Api::Lock() const { return lock_; }

const OT_API& Api::OTAPI(const std::string&) const
{
    OT_ASSERT(ot_api_);

    return *ot_api_;
}

const OT_ME& Api::OTME(const std::string&) const
{
    OT_ASSERT(ot_me_);

    return *ot_me_;
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

Api::~Api() {}
}  // namespace opentxs::api::implementation
