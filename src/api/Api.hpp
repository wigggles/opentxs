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

#ifndef OPENTXS_API_IMPLEMENTATION_API_HPP
#define OPENTXS_API_IMPLEMENTATION_API_HPP

#include "Internal.hpp"

namespace opentxs::api::implementation
{
class Api : virtual public opentxs::api::Api
{
public:
    std::recursive_mutex& Lock(
        const Identifier& nymID,
        const Identifier& serverID) const override;

    const OTAPI_Exec& Exec(const std::string& wallet = "") const override;
    const OT_API& OTAPI(const std::string& wallet = "") const override;
    const api::client::Cash& Cash() const override;
    const api::client::Pair& Pair() const override;
    const client::ServerAction& ServerAction() const override;
    const client::Sync& Sync() const override;
    const client::Workflow& Workflow() const override;

    ~Api();

private:
    friend Factory;

    const Flag& running_;
    const Activity& activity_;
    const Settings& config_;
    const ContactManager& contacts_;
    const api::Crypto& crypto_;
    const Identity& identity_;
    const storage::Storage& storage_;
    const api::client::Wallet& wallet_;
    const api::network::ZMQ& zmq_;

    std::unique_ptr<OT_API> ot_api_;
    std::unique_ptr<OTAPI_Exec> otapi_exec_;
    std::unique_ptr<api::client::Cash> cash_;
    std::unique_ptr<api::client::Pair> pair_;
    std::unique_ptr<api::client::ServerAction> server_action_;
    std::unique_ptr<api::client::Sync> sync_;
    std::unique_ptr<api::client::Workflow> workflow_;

    mutable std::recursive_mutex lock_;
    mutable std::mutex map_lock_;
    mutable std::map<ContextID, std::recursive_mutex> context_locks_;

    std::recursive_mutex& get_lock(const ContextID context) const;

    void Cleanup();
    void Init();

    Api(const Flag& running,
        const api::Activity& activity,
        const api::Settings& config,
        const api::ContactManager& contacts,
        const api::Crypto& crypto,
        const api::Identity& identity,
        const api::storage::Storage& storage,
        const api::client::Wallet& wallet,
        const api::network::ZMQ& zmq);
    Api() = delete;
    Api(const Api&) = delete;
    Api(Api&&) = delete;
    Api& operator=(const Api&) = delete;
    Api& operator=(Api&&) = delete;
};
}  // namespace opentxs::api::implementation
#endif  // OPENTXS_API_IMPLEMENTATION_API_HPP
