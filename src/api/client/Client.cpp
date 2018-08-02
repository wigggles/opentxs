// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/api/client/Blockchain.hpp"
#endif
#include "opentxs/api/client/Cash.hpp"
#include "opentxs/api/client/Client.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/client/Sync.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

#include "InternalClient.hpp"

#include <set>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "Client.hpp"

namespace opentxs
{
api::client::internal::Client* Factory::Client(
    const Flag& running,
    const api::Settings& config,
    const api::ContactManager& contacts,
    const api::Crypto& crypto,
    const api::Identity& identity,
    const api::Legacy& legacy,
    const api::storage::Storage& storage,
    const api::client::Wallet& wallet,
    const api::network::ZMQ& zmq)
{
    return new api::client::implementation::Client(
        running,
        config,
        contacts,
        crypto,
        identity,
        legacy,
        storage,
        wallet,
        zmq);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
Client::Client(
    const Flag& running,
    const api::Settings& config,
    const api::ContactManager& contacts,
    const api::Crypto& crypto,
    const api::Identity& identity,
    const api::Legacy& legacy,
    const api::storage::Storage& storage,
    const api::client::Wallet& wallet,
    const api::network::ZMQ& zmq)
    : running_(running)
    , wallet_(wallet)
    , zmq_(zmq)
    , storage_(storage)
    , contacts_(contacts)
    , crypto_(crypto)
    , identity_(identity)
    , legacy_(legacy)
    , config_(config)
    , activity_(nullptr)
#if OT_CRYPTO_SUPPORTED_KEY_HD
    , blockchain_(nullptr)
#endif
    , cash_(nullptr)
    , pair_(nullptr)
    , server_action_(nullptr)
    , sync_(nullptr)
    , workflow_(nullptr)
    , ot_api_(nullptr)
    , otapi_exec_(nullptr)
    , lock_()
    , map_lock_()
    , context_locks_()
{
    Init();
}

const api::client::Activity& Client::Activity() const
{
    OT_ASSERT(activity_)

    return *activity_;
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
const api::client::Blockchain& Client::Blockchain() const
{
    OT_ASSERT(blockchain_)

    return *blockchain_;
}
#endif

void Client::Cleanup()
{
    ui_.reset();
    pair_.reset();
    sync_.reset();
    cash_.reset();
    server_action_.reset();
    otapi_exec_.reset();
    ot_api_.reset();
    workflow_.reset();
#if OT_CRYPTO_SUPPORTED_KEY_HD
    blockchain_.reset();
#endif
    activity_.reset();
}

std::recursive_mutex& Client::get_lock(const ContextID context) const
{
    opentxs::Lock lock(map_lock_);

    return context_locks_[context];
}

void Client::Init()
{
    otLog3 << "\n\nWelcome to Open Transactions -- version " << Log::Version()
           << "\n";

    // FIXME

    OT_ASSERT(activity_)

#if OT_CRYPTO_SUPPORTED_KEY_HD
    Init_Blockchain();
#endif

    workflow_.reset(Factory::Workflow(
        *activity_, contacts_, legacy_, storage_, zmq_.Context()));

    OT_ASSERT(workflow_)

    ot_api_.reset(new OT_API(
        *activity_,
        *this,
        config_,
        contacts_,
        crypto_,
        identity_,
        legacy_,
        storage_,
        wallet_,
        *workflow_,
        zmq_,
        std::bind(&Client::get_lock, this, std::placeholders::_1)));

    OT_ASSERT(ot_api_);

    otapi_exec_.reset(new OTAPI_Exec(
        *activity_,
        config_,
        contacts_,
        crypto_,
        identity_,
        legacy_,
        wallet_,
        zmq_,
        *ot_api_,
        std::bind(&Client::get_lock, this, std::placeholders::_1)));

    OT_ASSERT(otapi_exec_);

    server_action_.reset(Factory::ServerAction(
        *ot_api_,
        *otapi_exec_,
        wallet_,
        *workflow_,
        legacy_,
        std::bind(&Client::get_lock, this, std::placeholders::_1)));

    OT_ASSERT(server_action_)

    cash_.reset(Factory::Cash(legacy_));

    OT_ASSERT(cash_);

    sync_.reset(Factory::Sync(
        running_,
        *ot_api_,
        *otapi_exec_,
        contacts_,
        config_,
        *this,
        legacy_,
        wallet_,
        *workflow_,
        crypto_.Encode(),
        storage_,
        zmq_.Context(),
        std::bind(&Client::get_lock, this, std::placeholders::_1)));

    OT_ASSERT(sync_);

    pair_.reset(Factory::Pair(
        running_,
        *sync_,
        *server_action_,
        wallet_,
        legacy_,
        *ot_api_,
        *otapi_exec_,
        zmq_.Context()));

    OT_ASSERT(pair_);

    Init_UI();  // Requires activity_, sync_, workflow_
}

void Client::Init_Activity()
{
    activity_.reset(Factory::Activity(
        legacy_, contacts_, storage_, wallet_, zmq_.Context()));
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
void Client::Init_Blockchain()
{
    OT_ASSERT(activity_)

    blockchain_.reset(
        Factory::Blockchain(*activity_, crypto_, storage_, wallet_));
}
#endif

void Client::Init_UI()
{
    OT_ASSERT(activity_)
    OT_ASSERT(sync_)
    OT_ASSERT(workflow_)

    ui_.reset(Factory::UI(
        *sync_,
        wallet_,
        *workflow_,
        zmq_,
        storage_,
        *activity_,
        contacts_,
        legacy_,
        zmq_.Context(),
        running_));

    OT_ASSERT(ui_);
}

const OTAPI_Exec& Client::Exec(const std::string&) const
{
    OT_ASSERT(otapi_exec_);

    return *otapi_exec_;
}

std::recursive_mutex& Client::Lock(
    const Identifier& nymID,
    const Identifier& serverID) const
{
    return get_lock({nymID.str(), serverID.str()});
}

const OT_API& Client::OTAPI(const std::string&) const
{
    OT_ASSERT(ot_api_);

    return *ot_api_;
}

const api::client::Cash& Client::Cash() const
{
    OT_ASSERT(cash_);

    return *cash_;
}

const api::client::Pair& Client::Pair() const
{
    OT_ASSERT(pair_);

    return *pair_;
}

const api::client::ServerAction& Client::ServerAction() const
{
    OT_ASSERT(server_action_);

    return *server_action_;
}

void Client::StartActivity()
{
    OT_ASSERT(activity_)

    activity_->MigrateLegacyThreads();
}

opentxs::OTWallet* Client::StartWallet()
{
    OT_ASSERT(ot_api_)

    const bool loaded = ot_api_->LoadWallet();

    OT_ASSERT(loaded);

    return ot_api_->GetWallet(nullptr);
}

const api::client::Sync& Client::Sync() const
{
    OT_ASSERT(sync_);

    return *sync_;
}

const api::client::UI& Client::UI() const
{
    OT_ASSERT(ui_)

    return *ui_;
}

const api::client::Workflow& Client::Workflow() const
{
    OT_ASSERT(workflow_);

    return *workflow_;
}

Client::~Client() { Cleanup(); }
}  // namespace opentxs::api::client::implementation
