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

#include <opentxs/core/OTData.hpp>
#include <opentxs/network/Dht.hpp>

namespace opentxs
{
#if OT_DHT

Dht* Dht::instance_ = nullptr;

Dht::Dht(int port)
    : node_(new dht::DhtRunner)
{
    Init(port);
}

void Dht::Init(int port)
{
    int listenPort = port;

    if ((port <= 1000) || (port >= 65535)) {
        listenPort = 4222;
    }

    node_->run(listenPort, dht::crypto::generateIdentity(), true);
    node_->bootstrap("bootstrap.ring.cx", "4222");
}

Dht& Dht::Node(int port)
{
    if (nullptr == instance_)
    {
        instance_ = new Dht(port);
    }

    return *instance_;
}

void Dht::Insert(
    const std::string& key,
    OTData& value,
    dht::Dht::DoneCallbackSimple cb)
{
    if (nullptr != node_) {
        node_->put(key, value.asVector(), cb);
    }
}

void Dht::Insert(
    const std::string& key,
    std::string& value,
    dht::Dht::DoneCallbackSimple cb)
{
    OTData data(value.c_str(), value.size());

    Insert(key, data, cb);
}

void Dht::Retrieve(
    const std::string& key,
    dht::Dht::GetCallback vcb,
    dht::Dht::DoneCallbackSimple dcb,
    dht::Value::Filter f)
{
    if (nullptr != node_) {
        node_->get(key, vcb, dcb, f);
    }
}

// temporary for development only
dht::DhtRunner* Dht::p()
{
    return node_;
}

void Dht::Cleanup()
{
    node_->join();
}

Dht::~Dht()
{
    Cleanup();
}

#endif
} // namespace opentxs
