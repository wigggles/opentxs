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

#ifndef OPENTXS_FACTORY_HPP
#define OPENTXS_FACTORY_HPP

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
class Factory
{
public:
    static api::Api* Api(
        const Flag& running,
        const api::Activity& activity,
        const api::Settings& config,
        const api::ContactManager& contacts,
        const api::Crypto& crypto,
        const api::Identity& identity,
        const api::storage::Storage& storage,
        const api::client::Wallet& wallet,
        const api::network::ZMQ& zmq);
    static api::client::Cash* Cash();
    static api::client::Issuer* Issuer(
        const api::client::Wallet& wallet,
        const Identifier& nymID,
        const proto::Issuer& serialized);
    static api::client::Issuer* Issuer(
        const api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& issuerID);
    static api::client::Pair* Pair(
        const Flag& running,
        const api::client::Sync& sync,
        const api::client::ServerAction& action,
        const api::client::Wallet& wallet,
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const network::zeromq::Context& context);
    static api::client::ServerAction* ServerAction(
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const ContextLockCallback& lockCallback);
    static api::client::Sync* Sync(
        const Flag& running,
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const api::ContactManager& contacts,
        const api::Settings& config,
        const api::Api& api,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const api::crypto::Encode& encoding,
        const network::zeromq::Context& zmq,
        const ContextLockCallback& lockCallback);
    static api::client::Wallet* Wallet(api::Native& ot);
    static api::client::Workflow* Workflow(
        const api::Activity& activity,
        const api::ContactManager& contact,
        const api::storage::Storage& storage);
};
}  // namespace opentxs
#endif  // OPENTXS_FACTORY_HPP
