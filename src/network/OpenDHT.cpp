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

#include "opentxs/network/OpenDHT.hpp"

#include "opentxs/network/DhtConfig.hpp"

#ifdef OT_DHT
#include <opendht.h>
#endif

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace opentxs
{
#ifdef OT_DHT

OpenDHT* OpenDHT::instance_ = nullptr;

OpenDHT::OpenDHT(const DhtConfig& config)
    : config_(new DhtConfig(config))
    , node_(new dht::DhtRunner)
{
    loaded_.store(false);
    ready_.store(false);
    Init();
}

bool OpenDHT::Init() const
{
    std::lock_guard<std::mutex> initLock(init_);

    if (!config_->enable_dht_) { return true; }

    if (ready_.load()) { return true; }

    if (!node_) { return false; }

    int64_t listenPort = config_->listen_port_;

    if ((listenPort <= 1000) || (listenPort >= 65535)) {
        listenPort = config_->default_port_;
    }

    if (!loaded_.load()) {
        try {
            node_->run(listenPort, dht::crypto::generateIdentity(), true);
        }
        catch (dht::DhtException& e) {
            std::cout << e.what() << std::endl;

            return false;
        }

        loaded_.store(true);
    }

    try {
        node_->bootstrap(
            config_->bootstrap_url_.c_str(),
            config_->bootstrap_port_.c_str());
            ready_.store(true);
    }
    catch (std::invalid_argument& e) {
        std::cout << e.what() << std::endl;

        return false;
    }

    return true;
}

OpenDHT& OpenDHT::It(const DhtConfig& config)
{
    if (nullptr == instance_) {
        instance_ = new OpenDHT(config);
    }

    assert(nullptr != instance_);

    return *instance_;
}

void OpenDHT::Insert(
    const std::string& key,
    const std::string& value,
    DhtDoneCallback cb) const
{
    if (!ready_.load()) {
        if (!Init()) { return; }
    }

    dht::InfoHash infoHash = dht::InfoHash::get(
        reinterpret_cast<const uint8_t*>(key.c_str()),
        key.size());

    std::shared_ptr<dht::Value> pValue = std::make_shared<dht::Value>(
        reinterpret_cast<const uint8_t*>(value.c_str()),
        value.size());

    if (!pValue) { return; }

    node_->put(infoHash, pValue, cb);
}

void OpenDHT::Retrieve(
    const std::string& key,
    DhtResultsCallback vcb,
    DhtDoneCallback dcb) const
{
    if (!ready_.load()) {
        if (!Init()) { return; }
    }

    // The OpenDHT get method wants a lambda function that accepts an
    // OpenDHT-specific type as an argument. I don't consumers of this class
    // to need to include OpenDHT headers. Solution: this lambda performs
    // the translation
    dht::GetCallback cb(
        [vcb](const std::vector<std::shared_ptr<dht::Value>> results) -> bool {
            DhtResults input;

            for (const auto& it : results) {
                if (nullptr == it) { continue; }

                input.emplace(input.end(), new std::string(it->toString()));
            }

            return vcb(input);
        });

    node_->get(dht::InfoHash(key), cb, dcb, dht::Value::AllFilter());
}

void OpenDHT::Cleanup()
{
    if (node_) {
        node_->join();
    }

    instance_ = nullptr;
}

OpenDHT::~OpenDHT()
{
    Cleanup();
}

#endif
} // namespace opentxs
