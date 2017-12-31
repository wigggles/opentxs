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

#ifndef OPENTXS_API_IMPLEMENTATION_DHT_HPP
#define OPENTXS_API_IMPLEMENTATION_DHT_HPP

#include "opentxs/Version.hpp"

#include "opentxs/api/network/Dht.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>

namespace opentxs
{

class Credential;
class DhtConfig;
class OpenDHT;
class ServerContract;
class UnitDefinition;

namespace api
{
namespace client
{
class Wallet;
}  // namespace client

namespace implementation
{
class Native;
}  // namespace implementation

namespace network
{
namespace implementation
{

class Dht : virtual public opentxs::api::network::Dht
{
public:
    void Cleanup() override;
    void GetPublicNym(const std::string& key) override;
    void GetServerContract(const std::string& key) override;
    void GetUnitDefinition(const std::string& key) override;
    void Insert(const std::string& key, const std::string& value) override;
    void Insert(const proto::CredentialIndex& nym) override;
    void Insert(const proto::ServerContract& contract) override;
    void Insert(const proto::UnitDefinition& contract) override;
    void RegisterCallbacks(const CallbackMap& callbacks) override;

    ~Dht();

private:
    friend class api::implementation::Native;

    api::client::Wallet& wallet_;
    CallbackMap callback_map_;
    std::unique_ptr<const DhtConfig> config_;
#if OT_DHT
    OpenDHT* node_ = nullptr;
#endif

#if OT_DHT
    static bool ProcessPublicNym(
        api::client::Wallet& wallet,
        const std::string key,
        const DhtResults& values,
        NotifyCB notifyCB);
    static bool ProcessServerContract(
        api::client::Wallet& wallet,
        const std::string key,
        const DhtResults& values,
        NotifyCB notifyCB);
    static bool ProcessUnitDefinition(
        api::client::Wallet& wallet,
        const std::string key,
        const DhtResults& values,
        NotifyCB notifyCB);
#endif

    explicit Dht(DhtConfig& config, api::client::Wallet& wallet);
    Dht() = delete;
    Dht(const Dht&) = delete;
    Dht& operator=(const Dht&) = delete;
    void Init();
};
}  // namespace implementation
}  // namespace network
}  // namespace api
}  // namespace opentxs

#endif  // OPENTXS_API_IMPLEMENTATION_DHT_HPP
