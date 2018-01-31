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

#include "opentxs/Version.hpp"

#include "opentxs/api/Api.hpp"

#include <memory>
#include <mutex>
#include <string>

namespace opentxs
{
class MadeEasy;
class OT_API;
class OT_ME;
class OTAPI_Exec;
class OTME_too;

namespace api
{
class Activity;
class ContactManager;
class Crypto;
class Identity;
class Settings;

namespace client
{
class Pair;
class Wallet;
}  // namespace client

namespace network
{
class ZMQ;
}  // namespace network

namespace storage
{
class Storage;
}  // namespace storage

namespace implementation
{
class Native;

class Api : virtual public opentxs::api::Api
{
public:
    const std::recursive_mutex& Lock() const override;

    const OTAPI_Exec& Exec(const std::string& wallet = "") const override;
    const MadeEasy& ME(const std::string& wallet = "") const override;
    const OT_API& OTAPI(const std::string& wallet = "") const override;
    const OT_ME& OTME(const std::string& wallet = "") const override;
    const OTME_too& OTME_TOO(const std::string& wallet = "") const override;
    const api::client::Pair& Pair() const override;

    ~Api();

private:
    friend class implementation::Native;

    const std::atomic<bool>& shutdown_;
    const Activity& activity_;
    const Settings& config_;
    const ContactManager& contacts_;
    const Crypto& crypto_;
    const Identity& identity_;
    const storage::Storage& storage_;
    const api::client::Wallet& wallet_;
    const api::network::ZMQ& zmq_;

    std::unique_ptr<OT_API> ot_api_;
    std::unique_ptr<OTAPI_Exec> otapi_exec_;
    std::unique_ptr<MadeEasy> made_easy_;
    std::unique_ptr<OT_ME> ot_me_;
    std::unique_ptr<OTME_too> otme_too_;
    std::unique_ptr<api::client::Pair> pair_;

    mutable std::recursive_mutex lock_;

    void Cleanup();
    void Init();

    Api(const std::atomic<bool>& shutdown,
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
