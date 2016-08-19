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

#include <iostream>
#include <stdexcept>

namespace opentxs
{
#ifdef OT_DHT

OpenDHT* OpenDHT::instance_ = nullptr;

OpenDHT::OpenDHT(DhtConfig& config)
    : config_(config)
    , node_(new dht::DhtRunner)
{
    loaded_.store(false);
    ready_.store(false);
    Init();
}

bool OpenDHT::Init() const
{
    std::lock_guard<std::mutex> initLock(init_);

    if (!config_.enable_dht_) { return true; }

    if (ready_.load()) { return true; }

    int64_t listenPort = config_.listen_port_;

    if ((listenPort <= 1000) || (listenPort >= 65535)) {
        listenPort = config_.default_port_;
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
            config_.bootstrap_url_.c_str(),
            config_.bootstrap_port_.c_str());
            ready_.store(true);
    }
    catch (std::invalid_argument& e) {
        std::cout << e.what() << std::endl;

        return false;
    }

    return true;
}

OpenDHT& OpenDHT::It(DhtConfig& config)
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
    dht::Dht::DoneCallbackSimple cb) const
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

    pValue->user_type = key;

    node_->put(infoHash, pValue, cb);
}

void OpenDHT::Retrieve(
    const std::string& key,
    dht::Dht::GetCallback vcb,
    dht::Dht::DoneCallbackSimple dcb,
    dht::Value::Filter f) const
{
    if (!ready_.load()) {
        if (!Init()) { return; }
    }

    node_->get(key, vcb, dcb, f);
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
