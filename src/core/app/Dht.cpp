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

#include <opentxs/core/app/Dht.hpp>

#include <opentxs/core/Log.hpp>
#include <opentxs/core/OTServerContract.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/app/App.hpp>

namespace opentxs
{

Dht* Dht::instance_ = nullptr;

Dht::Dht(DhtConfig& config)
    : config_(config)
{
    Init();
}

void Dht::Init()
{
#if OT_DHT
    node_ = &OpenDHT::It(config_);
#endif
}

Dht& Dht::It(DhtConfig& config)
{
    if (nullptr == instance_)
    {
        instance_ = new Dht(config);
    }

    return *instance_;
}

void Dht::Insert(
    const std::string ID,
    const Contract& contract)
{
#if OT_DHT
    OT_ASSERT(nullptr != node_);

    String data(contract);
    node_->Insert(ID, std::string(data.Get()));
#endif
}

void Dht::GetServerContract(
    const std::string& key,
    std::function<void(const OTServerContract&)> cb)
{
#if OT_DHT
    OT_ASSERT(nullptr != node_);

    dht::Dht::GetCallback gcb(
        [cb](const OpenDHT::Results& values) -> bool {
            return ProcessServerContract(values, cb);});

    node_->Retrieve(key, gcb);
#endif
}

#if OT_DHT
bool Dht::ProcessServerContract(
    const OpenDHT::Results& values,
    ServerContractCB cb)
{
    std::string theresult;
    bool foundData = false;
    bool foundValid = false;

    for (const auto & it: values)
    {
        auto & ptr = *it;
        std::string data(ptr.data.begin(), ptr.data.end());
        foundData = data.size() > 0;

        if (0 == ptr.user_type.size()) { continue; }

        Identifier contractID(ptr.user_type);

        if (0 == data.size()) { continue; }

        OTServerContract* newContract = new OTServerContract;
        bool loaded = newContract->LoadContractFromString(data);

        if (!loaded) { continue; }

        Identifier serverID;
        newContract->SetIdentifier(contractID);

        if (!newContract->VerifyContract()) { continue; }

        if (cb) {
            cb(*newContract);
            otLog3 << "Saved contract: " << ptr.user_type
                << std::endl;
            foundValid = true;
            break; // We only need the first valid result
        }
    }

    if (!foundValid) {
        otErr << "Found results, but none are valid." << std::endl;
    }

    if (!foundData) {
        otErr << "All results are empty" << std::endl;
    }

    return foundData;
}
#endif

void Dht::Cleanup()
{
#if OT_DHT
    delete node_;
    node_ = nullptr;
#endif
}

Dht::~Dht()
{
    Cleanup();
}

} // namespace opentxs
