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

#include "opentxs/Internal.hpp"

#include "opentxs/api/Api.hpp"
#include "opentxs/core/Flag.hpp"

#include <memory>
#include <mutex>
#include <string>

namespace opentxs
{
namespace api
{
namespace implementation
{

class Api : virtual public opentxs::api::Api
{
public:
    std::recursive_mutex& Lock() const override;

    const OTAPI_Exec& Exec(const std::string& wallet = "") const override;
    const OT_API& OTAPI(const std::string& wallet = "") const override;
    const OT_ME& OTME(const std::string& wallet = "") const override;
    const api::client::Pair& Pair() const override;
    const client::ServerAction& ServerAction() const override;
    const client::Sync& Sync() const override;

    ~Api();

private:
    friend class implementation::Native;

    const Flag& running_;
    const Activity& activity_;
    const Settings& config_;
    const ContactManager& contacts_;
    const api::Crypto& crypto_;
    const Identity& identity_;
    const storage::Storage& storage_;
    const api::client::Wallet& wallet_;
    const api::network::ZMQ& zmq_;

    std::unique_ptr<OT_API> ot_api_{nullptr};
    std::unique_ptr<OTAPI_Exec> otapi_exec_{nullptr};
    std::unique_ptr<OT_ME> ot_me_{nullptr};
    std::unique_ptr<api::client::Pair> pair_{nullptr};
    std::unique_ptr<api::client::ServerAction> server_action_{nullptr};
    std::unique_ptr<api::client::Sync> sync_{nullptr};

    mutable std::recursive_mutex lock_;

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
}  // namespace implementation
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_IMPLEMENTATION_API_HPP
