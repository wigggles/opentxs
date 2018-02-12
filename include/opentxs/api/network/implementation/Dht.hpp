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

#include "opentxs/Internal.hpp"

#include "opentxs/api/network/Dht.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>

namespace opentxs
{
namespace api
{
namespace network
{
namespace implementation
{
class Dht : virtual public opentxs::api::network::Dht
{
public:
    void GetPublicNym(const std::string& key) const override;
    void GetServerContract(const std::string& key) const override;
    void GetUnitDefinition(const std::string& key) const override;
    void Insert(const std::string& key, const std::string& value)
        const override;
    void Insert(const proto::CredentialIndex& nym) const override;
    void Insert(const proto::ServerContract& contract) const override;
    void Insert(const proto::UnitDefinition& contract) const override;
#if OT_DHT
    const opentxs::network::OpenDHT& OpenDHT() const override;
#endif
    void RegisterCallbacks(const CallbackMap& callbacks) const override;

    ~Dht();

private:
    friend class api::implementation::Native;

    const api::client::Wallet& wallet_;
    mutable CallbackMap callback_map_{};
    std::unique_ptr<const DhtConfig> config_{nullptr};
#if OT_DHT
    std::unique_ptr<opentxs::network::OpenDHT> node_{nullptr};
#endif

#if OT_DHT
    static bool ProcessPublicNym(
        const api::client::Wallet& wallet,
        const std::string key,
        const DhtResults& values,
        NotifyCB notifyCB);
    static bool ProcessServerContract(
        const api::client::Wallet& wallet,
        const std::string key,
        const DhtResults& values,
        NotifyCB notifyCB);
    static bool ProcessUnitDefinition(
        const api::client::Wallet& wallet,
        const std::string key,
        const DhtResults& values,
        NotifyCB notifyCB);
#endif

    explicit Dht(DhtConfig& config, const api::client::Wallet& wallet);
    Dht() = delete;
    Dht(const Dht&) = delete;
    Dht(Dht&&) = delete;
    Dht& operator=(const Dht&) = delete;
    Dht& operator=(Dht&&) = delete;
};
}  // namespace implementation
}  // namespace network
}  // namespace api
}  // namespace opentxs

#endif  // OPENTXS_API_IMPLEMENTATION_DHT_HPP
