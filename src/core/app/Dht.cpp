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
#include <opentxs/core/String.hpp>


namespace opentxs
{

Dht* Dht::instance_ = nullptr;

Dht::Dht(int port)
{
    Init(port);
}

void Dht::Init(int port)
{
#if OT_DHT
    node_ = &OpenDHT::It(port);
#endif
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
    const std::string ID,
    const Contract& contract)
{
#if OT_DHT
    OT_ASSERT(nullptr != node_);

    String data(contract);
    node_->Insert(ID, std::string(data.Get()));
#endif
}

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
