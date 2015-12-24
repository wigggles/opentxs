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

#include <opentxs/core/Log.hpp>
#include <opentxs/core/OTData.hpp>
#include <opentxs/core/OTServerContract.hpp>
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
    dht::InfoHash infoHash = dht::InfoHash::get(
        reinterpret_cast<const uint8_t*>(key.c_str()),
        key.size());

    otErr << "Inserting key: " << infoHash.toString() << "\n";

    std::shared_ptr<dht::Value> pValue = std::make_shared<dht::Value>(
        static_cast<const uint8_t*>(value.GetPointer()),
        value.GetSize());

    if (nullptr != node_) {
        node_->put(infoHash, pValue, cb);
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

// static
bool Dht::ProcessServerContract(
    const std::vector<std::shared_ptr<dht::Value>>& values)
{
    std::string theresult;
    bool foundData = false;

    for (const auto & it: values)
    {
        auto & ptr = *it;
        std::string data(ptr.data.begin(), ptr.data.end());
        foundData = data.size() > 0;

        if (data.size() > 0) {
            OTServerContract newContract;
            bool loaded = newContract.LoadContractFromString(data);

            if (loaded) {
                otErr << "Loaded contract." << std::endl;
            } else {
                otErr << "Something went wrong." << std::endl;
                otErr << contract << std::endl;
            }
        }
    }

    if (!foundData) {
        otErr << "empty result." << std::endl;
    }

    return foundData;
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
