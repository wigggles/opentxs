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

    if ((1000 >= port) || (65535 <= port)) {
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
