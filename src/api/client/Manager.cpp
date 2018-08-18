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
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/client/Sync.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/network/Dht.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Factory.hpp"
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/HDSeed.hpp"
#endif
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/client/SwigWrap.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

#include "api/storage/StorageInternal.hpp"
#include "api/Scheduler.hpp"
#include "api/StorageParent.hpp"
#include "internal/api/Internal.hpp"
#include "InternalClient.hpp"

#include <set>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "Manager.hpp"

namespace opentxs
{
api::client::internal::Manager* Factory::ClientManager(
    const Flag& running,
    const ArgList& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const network::zeromq::Context& context,
    const std::string& dataFolder,
    const int instance)
{
    return new api::client::implementation::Manager(
        running, args, config, crypto, context, dataFolder, instance);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
Manager::Manager(
    const Flag& running,
    const ArgList& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const opentxs::network::zeromq::Context& context,
    const std::string& dataFolder,
    const int instance)
    : Scheduler(running)
    , StorageParent(running, args, crypto, config, dataFolder)
    , zmq_context_{context}
    , instance_
{
    instance
}
#if OT_CRYPTO_WITH_BIP39
, seeds_(opentxs::Factory::HDSeed(
      crypto_.Symmetric(),
      *storage_,
      crypto_.BIP32(),
      crypto_.BIP39(),
      crypto_.AES()))
#endif
      ,
    factory_(opentxs::Factory::FactoryAPI(
#if OT_CRYPTO_WITH_BIP39
        *seeds_
#endif
        )),
    wallet_(opentxs::Factory::Wallet(*this)),
    zeromq_(opentxs::Factory::ZMQ(*this, running_)),
    identity_(opentxs::Factory::Identity(*wallet_)),
    contacts_(opentxs::Factory::Contacts(
        *storage_,
        *factory_,
        *wallet_,
        zmq_context_)),
    activity_(
        opentxs::Factory::Activity(*storage_, *contacts_, *this, zmq_context_))
#if OT_CRYPTO_SUPPORTED_KEY_HD
        ,
    blockchain_(opentxs::Factory::Blockchain(
        *activity_,
        crypto_,
        *seeds_,
        *storage_,
        *wallet_))
#endif
        ,
    workflow_(opentxs::Factory::Workflow(
        *activity_,
        *contacts_,
        *this,
        *storage_,
        zmq_context_)),
    ot_api_(new OT_API(
        *this,
        std::bind(&Manager::get_lock, this, std::placeholders::_1))),
    otapi_exec_(new OTAPI_Exec(
        *activity_,
        config_,
        *contacts_,
        crypto_,
        this->Factory(),
        *identity_,
        *this,
        *zeromq_,
        *ot_api_,
        std::bind(&Manager::get_lock, this, std::placeholders::_1))),
    cash_(opentxs::Factory::Cash(*this)),
    server_action_(opentxs::Factory::ServerAction(
        *ot_api_,
        *otapi_exec_,
        *workflow_,
        *this,
        std::bind(&Manager::get_lock, this, std::placeholders::_1))),
    sync_(opentxs::Factory::Sync(
        running_,
        *this,
        std::bind(&Manager::get_lock, this, std::placeholders::_1))),
    ui_(opentxs::Factory::UI(
        *sync_,
        *wallet_,
        *workflow_,
        *zeromq_,
        *storage_,
        *activity_,
        *contacts_,
        *this,
        zmq_context_,
        running_)),
    pair_(opentxs::Factory::Pair(running_, *this)),
    dht_(opentxs::Factory::Dht(
        instance_,
        false,
        config_,
        *wallet_,
        context,
        nym_publish_interval_,
        nym_refresh_interval_,
        server_publish_interval_,
        server_refresh_interval_,
        unit_publish_interval_,
        unit_refresh_interval_)),
    lock_(), map_lock_(), context_locks_()
{
    OT_ASSERT(seeds_);
    OT_ASSERT(factory_);
    OT_ASSERT(wallet_);
    OT_ASSERT(zeromq_);
    OT_ASSERT(identity_);
    OT_ASSERT(contacts_);
    OT_ASSERT(activity_);
#if OT_CRYPTO_SUPPORTED_KEY_HD
    OT_ASSERT(blockchain_);
#endif
    OT_ASSERT(workflow_);
    OT_ASSERT(ot_api_);
    OT_ASSERT(otapi_exec_);
    OT_ASSERT(cash_);
    OT_ASSERT(server_action_);
    OT_ASSERT(sync_);
    OT_ASSERT(ui_);
    OT_ASSERT(pair_);
    OT_ASSERT(dht_);

    if (0 == instance_) {
        SwigWrap::client_ = this;

        OT_ASSERT(nullptr != SwigWrap::client_)
    }

    Init();
}

const api::client::Activity& Manager::Activity() const
{
    OT_ASSERT(activity_)

    return *activity_;
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
const api::client::Blockchain& Manager::Blockchain() const
{
    OT_ASSERT(blockchain_)

    return *blockchain_;
}
#endif

void Manager::Cleanup()
{
    if (0 == instance_) { SwigWrap::client_ = nullptr; }

    dht_.reset();
    pair_.reset();
    ui_.reset();
    sync_.reset();
    server_action_.reset();
    cash_.reset();
    otapi_exec_.reset();
    ot_api_.reset();
    workflow_.reset();
#if OT_CRYPTO_SUPPORTED_KEY_HD
    blockchain_.reset();
#endif
    activity_.reset();
    contacts_.reset();
    identity_.reset();
    zeromq_.reset();
    wallet_.reset();
    factory_.reset();
}

const api::client::Contacts& Manager::Contacts() const
{
    OT_ASSERT(contacts_)

    return *contacts_;
}

const api::network::Dht& Manager::DHT() const
{
    OT_ASSERT(dht_)

    return *dht_;
}

const api::Factory& Manager::Factory() const
{
    OT_ASSERT(factory_)

    return *factory_;
}

std::recursive_mutex& Manager::get_lock(const ContextID context) const
{
    opentxs::Lock lock(map_lock_);

    return context_locks_[context];
}

const OTAPI_Exec& Manager::Exec(const std::string&) const
{
    OT_ASSERT(otapi_exec_);

    return *otapi_exec_;
}

void Manager::Init()
{
    StartWallet();

    OT_ASSERT(seeds_)

    StorageParent::init(*seeds_);
    StartContacts();
    StartActivity();
}

std::recursive_mutex& Manager::Lock(
    const Identifier& nymID,
    const Identifier& serverID) const
{
    return get_lock({nymID.str(), serverID.str()});
}

const OT_API& Manager::OTAPI(const std::string&) const
{
    OT_ASSERT(ot_api_);

    return *ot_api_;
}

const api::client::Cash& Manager::Cash() const
{
    OT_ASSERT(cash_);

    return *cash_;
}

const api::client::Pair& Manager::Pair() const
{
    OT_ASSERT(pair_);

    return *pair_;
}

const api::HDSeed& Manager::Seeds() const
{
    OT_ASSERT(seeds_);

    return *seeds_;
}

const api::client::ServerAction& Manager::ServerAction() const
{
    OT_ASSERT(server_action_);

    return *server_action_;
}

void Manager::StartActivity()
{
    OT_ASSERT(activity_)

    activity_->MigrateLegacyThreads();

    OT_ASSERT(dht_)

    Scheduler::Start(storage_.get(), dht_.get());
}

void Manager::StartContacts()
{
    OT_ASSERT(contacts_);

    contacts_->start();
}

opentxs::OTWallet* Manager::StartWallet()
{
    OT_ASSERT(ot_api_)

    const bool loaded = ot_api_->LoadWallet();

    OT_ASSERT(loaded);

    auto wallet = ot_api_->GetWallet(nullptr);

    OT_ASSERT(nullptr != wallet);

    wallet->SaveWallet();

    return wallet;
}

const api::storage::Storage& Manager::Storage() const
{
    OT_ASSERT(storage_)

    return *storage_;
}

void Manager::storage_gc_hook()
{
    if (storage_) { storage_->RunGC(); }
}

const api::client::Sync& Manager::Sync() const
{
    OT_ASSERT(sync_);

    return *sync_;
}

const api::client::UI& Manager::UI() const
{
    OT_ASSERT(ui_)

    return *ui_;
}

const api::Wallet& Manager::Wallet() const
{
    OT_ASSERT(wallet_);

    return *wallet_;
}

const api::client::Workflow& Manager::Workflow() const
{
    OT_ASSERT(workflow_);

    return *workflow_;
}

const api::network::ZMQ& Manager::ZMQ() const
{
    OT_ASSERT(zeromq_);

    return *zeromq_;
}

Manager::~Manager() { Cleanup(); }
}  // namespace opentxs::api::client::implementation
