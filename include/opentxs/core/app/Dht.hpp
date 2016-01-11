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

#ifndef OPENTXS_CORE_APP_DHT_HPP
#define OPENTXS_CORE_APP_DHT_HPP

#include <string>

#include <opentxs/core/Contract.hpp>
#include <opentxs/core/Nym.hpp>
#include <opentxs/network/DhtConfig.hpp>
#include <opentxs/network/OpenDHT.hpp>

namespace opentxs
{

class App;
class OTServerContract;

//High level interface to OpenDHT. Supports opentxs types.
class Dht
{
public:
    typedef std::function<void(const OTServerContract&)> ServerContractCB;

private:
    friend class App;

    static Dht* instance_;

#if OT_DHT
    OpenDHT* node_ = nullptr;
#endif
    DhtConfig config_;

    static Dht& It(DhtConfig& config);

#if OT_DHT
    static bool ProcessPublicNym(
        const OpenDHT::Results& values);
    static bool ProcessServerContract(
        const OpenDHT::Results& values,
        ServerContractCB cb);
#endif

    Dht(DhtConfig& config);
    Dht() = delete;
    Dht(const Dht&) = delete;
    Dht& operator=(const Dht&) = delete;
    void Init();

public:
    EXPORT void Insert(
        const std::string ID,
        const Contract& contract);
    EXPORT void Insert(const Nym& nym);
    EXPORT void Insert(const serializedCredentialIndex& nym);
    EXPORT void GetPublicNym(
        const std::string& key);
    EXPORT void GetServerContract(
        const std::string& key,
        ServerContractCB cb); //function pointer for OTWallet::AddServerContract

    void Cleanup();
    ~Dht();
};

}  // namespace opentxs
#endif // OPENTXS_CORE_APP_DHT_HPP
