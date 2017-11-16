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

#ifndef OPENTXS_API_DHT_HPP
#define OPENTXS_API_DHT_HPP

#include "opentxs/Version.hpp"

#include "opentxs/Proto.hpp"

#include <functional>
#include <map>
#include <string>

namespace opentxs
{
namespace api
{

class Dht
{
public:
    enum class Callback : std::uint8_t {
        SERVER_CONTRACT = 0,
        ASSET_CONTRACT = 1,
        PUBLIC_NYM = 2
    };

    typedef std::function<void(const std::string)> NotifyCB;
    typedef std::map<Callback, NotifyCB> CallbackMap;

    EXPORT virtual void Cleanup() = 0;
    EXPORT virtual void GetPublicNym(const std::string& key) = 0;
    EXPORT virtual void GetServerContract(const std::string& key) = 0;
    EXPORT virtual void GetUnitDefinition(const std::string& key) = 0;
    EXPORT virtual void Insert(
        const std::string& key,
        const std::string& value) = 0;
    EXPORT virtual void Insert(const proto::CredentialIndex& nym) = 0;
    EXPORT virtual void Insert(const proto::ServerContract& contract) = 0;
    EXPORT virtual void Insert(const proto::UnitDefinition& contract) = 0;
    EXPORT virtual void RegisterCallbacks(const CallbackMap& callbacks) = 0;

    EXPORT virtual ~Dht() = default;

protected:
    Dht() = default;

private:
    Dht(const Dht&) = delete;
    Dht(Dht&&) = delete;
    Dht& operator=(const Dht&) = delete;
    Dht& operator=(Dht&&) = delete;
};
}  // namespace api
}  // namespace opentxs

#endif  // OPENTXS_API_DHT_HPP
