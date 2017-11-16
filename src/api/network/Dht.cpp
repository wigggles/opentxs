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

#include "opentxs/api/network/implementation/Dht.hpp"

#include "opentxs/api/Native.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/network/DhtConfig.hpp"
#if OT_DHT
#include "opentxs/network/OpenDHT.hpp"
#endif

#include <string>

namespace opentxs::api::network::implementation
{
Dht::Dht(DhtConfig& config, api::Wallet& wallet)
    : wallet_(wallet)
    , config_(new DhtConfig(config))
{
    Init();
}

void Dht::Init()
{
#if OT_DHT
    node_ = &OpenDHT::It(*config_);
#endif
}

void Dht::Insert(
    __attribute__((unused)) const std::string& key,
    __attribute__((unused)) const std::string& value)
{
#if OT_DHT
    OT_ASSERT(nullptr != node_);

    node_->Insert(key, value);
#endif
}

void Dht::Insert(__attribute__((unused)) const serializedCredentialIndex& nym)
{
#if OT_DHT
    OT_ASSERT(nullptr != node_);

    node_->Insert(nym.nymid(), proto::ProtoAsString(nym));
#endif
}

void Dht::Insert(__attribute__((unused)) const proto::ServerContract& contract)
{
#if OT_DHT
    OT_ASSERT(nullptr != node_);

    node_->Insert(contract.id(), proto::ProtoAsString(contract));
#endif
}

void Dht::Insert(__attribute__((unused)) const proto::UnitDefinition& contract)
{
#if OT_DHT
    OT_ASSERT(nullptr != node_);

    node_->Insert(contract.id(), proto::ProtoAsString(contract));
#endif
}

void Dht::GetPublicNym(__attribute__((unused)) const std::string& key)
{
#if OT_DHT
    OT_ASSERT(nullptr != node_);

    auto it = callback_map_.find(Dht::Callback::PUBLIC_NYM);
    bool haveCB = (it != callback_map_.end());
    NotifyCB notifyCB;

    if (haveCB) {
        notifyCB = it->second;
    }

    DhtResultsCallback gcb(
        [this, notifyCB, key](const DhtResults& values) -> bool {
            return ProcessPublicNym(wallet_, key, values, notifyCB);
        });

    node_->Retrieve(key, gcb);
#endif
}

void Dht::GetServerContract(__attribute__((unused)) const std::string& key)
{
#if OT_DHT
    OT_ASSERT(nullptr != node_);

    auto it = callback_map_.find(Dht::Callback::SERVER_CONTRACT);
    bool haveCB = (it != callback_map_.end());
    NotifyCB notifyCB;

    if (haveCB) {
        notifyCB = it->second;
    }

    DhtResultsCallback gcb(
        [this, notifyCB, key](const DhtResults& values) -> bool {
            return ProcessServerContract(wallet_, key, values, notifyCB);
        });

    node_->Retrieve(key, gcb);
#endif
}

void Dht::GetUnitDefinition(__attribute__((unused)) const std::string& key)
{
#if OT_DHT
    OT_ASSERT(nullptr != node_);

    auto it = callback_map_.find(Dht::Callback::ASSET_CONTRACT);
    bool haveCB = (it != callback_map_.end());
    NotifyCB notifyCB;

    if (haveCB) {
        notifyCB = it->second;
    }

    DhtResultsCallback gcb(
        [this, notifyCB, key](const DhtResults& values) -> bool {
            return ProcessUnitDefinition(wallet_, key, values, notifyCB);
        });

    node_->Retrieve(key, gcb);
#endif
}

#if OT_DHT
bool Dht::ProcessPublicNym(
    api::Wallet& wallet,
    const std::string key,
    const DhtResults& values,
    NotifyCB notifyCB)
{
    std::string theresult;
    bool foundData = false;
    bool foundValid = false;

    if (key.empty()) {
        return false;
    }

    for (const auto& it : values) {
        if (nullptr == it) {
            continue;
        }

        auto& data = *it;
        foundData = data.size() > 0;

        if (0 == data.size()) {
            continue;
        }

        auto publicNym = proto::DataToProto<proto::CredentialIndex>(
            Data(data.c_str(), data.size()));

        if (key != publicNym.nymid()) {
            continue;
        }

        auto existing = wallet.Nym(Identifier(key));

        if (existing) {
            if (existing->Revision() >= publicNym.revision()) {
                continue;
            }
        }

        auto saved = wallet.Nym(publicNym);

        if (!saved) {
            continue;
        }

        foundValid = true;
        otLog3 << "Saved nym: " << key << std::endl;

        if (notifyCB) {
            notifyCB(key);
        }
    }

    if (!foundValid) {
        otErr << __FUNCTION__ << "Found results, but none are valid."
              << std::endl;
    }

    if (!foundData) {
        otErr << __FUNCTION__ << "All results are empty" << std::endl;
    }

    return foundData;
}

bool Dht::ProcessServerContract(
    api::Wallet& wallet,
    const std::string key,
    const DhtResults& values,
    NotifyCB notifyCB)
{
    std::string theresult;
    bool foundData = false;
    bool foundValid = false;

    if (key.empty()) {
        return false;
    }

    for (const auto& it : values) {
        if (nullptr == it) {
            continue;
        }

        auto& data = *it;
        foundData = data.size() > 0;

        if (0 == data.size()) {
            continue;
        }

        auto contract = proto::DataToProto<proto::ServerContract>(
            Data(data.c_str(), data.size()));

        if (key != contract.id()) {
            continue;
        }

        auto saved = wallet.Server(contract);

        if (!saved) {
            continue;
        }

        otLog3 << "Saved contract: " << key << std::endl;
        foundValid = true;

        if (notifyCB) {
            notifyCB(key);
        }

        break;  // We only need the first valid result
    }

    if (!foundValid) {
        otErr << __FUNCTION__ << "Found results, but none are valid."
              << std::endl;
    }

    if (!foundData) {
        otErr << __FUNCTION__ << "All results are empty" << std::endl;
    }

    return foundData;
}

bool Dht::ProcessUnitDefinition(
    api::Wallet& wallet,
    const std::string key,
    const DhtResults& values,
    NotifyCB notifyCB)
{
    std::string theresult;
    bool foundData = false;
    bool foundValid = false;

    if (key.empty()) {
        return false;
    }

    for (const auto& it : values) {
        if (nullptr == it) {
            continue;
        }

        auto& data = *it;
        foundData = data.size() > 0;

        if (0 == data.size()) {
            continue;
        }

        auto contract = proto::DataToProto<proto::UnitDefinition>(
            Data(data.c_str(), data.size()));

        if (key != contract.id()) {
            continue;
        }

        auto saved = wallet.UnitDefinition(contract);

        if (!saved) {
            continue;
        }

        otLog3 << "Saved unit definition: " << key << std::endl;
        foundValid = true;

        if (notifyCB) {
            notifyCB(key);
        }

        break;  // We only need the first valid result
    }

    if (!foundValid) {
        otErr << __FUNCTION__ << "Found results, but none are valid."
              << std::endl;
    }

    if (!foundData) {
        otErr << __FUNCTION__ << "All results are empty" << std::endl;
    }

    return foundData;
}
#endif

void Dht::Cleanup()
{
#if OT_DHT
    if (nullptr != node_) delete node_;
    node_ = nullptr;
#endif
}

void Dht::RegisterCallbacks(const CallbackMap& callbacks)
{
    callback_map_ = callbacks;
}

Dht::~Dht() { Cleanup(); }
}  // opentxs::api::network::implementation
