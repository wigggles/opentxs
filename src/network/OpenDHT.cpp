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

#include <opentxs/network/OpenDHT.hpp>

namespace opentxs
{
#if OT_DHT

OpenDHT* OpenDHT::instance_ = nullptr;

OpenDHT::OpenDHT(int port)
    : node_(new dht::DhtRunner)
{
    Init(port);
}

void OpenDHT::Init(int port)
{
    int listenPort = port;

    if ((port <= 1000) || (port >= 65535)) {
        listenPort = 4222;
    }

    node_->run(listenPort, dht::crypto::generateIdentity(), true);
    node_->bootstrap("bootstrap.ring.cx", "4222");
}

OpenDHT& OpenDHT::It(int port)
{
    if (nullptr == instance_)
    {
        instance_ = new OpenDHT(port);
    }

    return *instance_;
}

void OpenDHT::Insert(
    const std::string& key,
    const std::string& value,
    dht::Dht::DoneCallbackSimple cb)
{
    dht::InfoHash infoHash = dht::InfoHash::get(
        reinterpret_cast<const uint8_t*>(key.c_str()),
        key.size());

    std::shared_ptr<dht::Value> pValue = std::make_shared<dht::Value>(
        reinterpret_cast<const uint8_t*>(value.c_str()),
        value.size());

    pValue->user_type = key;

    if (nullptr != node_) {
        node_->put(infoHash, pValue, cb);
    }
}

void OpenDHT::Retrieve(
    const std::string& key,
    dht::Dht::GetCallback vcb,
    dht::Dht::DoneCallbackSimple dcb,
    dht::Value::Filter f)
{
    if (nullptr != node_) {
        node_->get(key, vcb, dcb, f);
    }
}

void OpenDHT::Cleanup()
{
    node_->join();
}

OpenDHT::~OpenDHT()
{
    Cleanup();
}

#endif
} // namespace opentxs
