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

#include "stdafx.hpp"

#include "opentxs/api/client/Issuer.hpp"
#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/AccountSummary.hpp"
#include "opentxs/ui/IssuerItem.hpp"
#include "opentxs/Types.hpp"

#include "IssuerItemBlank.hpp"
#include "AccountSummaryParent.hpp"
#include "List.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <thread>
#include <tuple>
#include <vector>

#include "AccountSummary.hpp"

#define DEFAULT_ISSUER_NAME "Stash Node Pro"

#define OT_METHOD "opentxs::ui::implementation::AccountSummary::"

namespace opentxs
{
ui::AccountSummary* Factory::AccountSummary(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::client::Wallet& wallet,
    const api::network::ZMQ& connection,
    const api::storage::Storage& storage,
    const api::ContactManager& contact,
    const Identifier& nymID,
    const proto::ContactItemType currency)
{
    return new ui::implementation::AccountSummary(
        zmq, publisher, wallet, connection, storage, contact, nymID, currency);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
const Widget::ListenerDefinitions AccountSummary::listeners_{
    {network::zeromq::Socket::ContactUpdateEndpoint,
     new MessageProcessor<AccountSummary>(&AccountSummary::process_server)},
    {network::zeromq::Socket::IssuerUpdateEndpoint,
     new MessageProcessor<AccountSummary>(&AccountSummary::process_issuer)},
    {network::zeromq::Socket::ServerUpdateEndpoint,
     new MessageProcessor<AccountSummary>(&AccountSummary::process_server)},
};

AccountSummary::AccountSummary(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::client::Wallet& wallet,
    const api::network::ZMQ& connection,
    const api::storage::Storage& storage,
    const api::ContactManager& contact,
    const Identifier& nymID,
    const proto::ContactItemType currency)
    : AccountSummaryList(nymID, zmq, publisher, contact)
    , wallet_(wallet)
    , connection_(connection)
    , storage_{storage}
    , currency_(currency)
    , threads_()
    , server_thread_map_()
{
    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&AccountSummary::startup, this));

    OT_ASSERT(startup_)
}

void AccountSummary::construct_row(
    const AccountSummaryRowID& id,
    const AccountSummarySortKey& index,
    const CustomData&) const
{
    items_[index].emplace(
        id,
        Factory::IssuerItem(
            *this,
            zmq_,
            publisher_,
            contact_manager_,
            wallet_,
            storage_,
            currency_,
            id));
    names_.emplace(id, index);
}

AccountSummarySortKey AccountSummary::extract_key(
    const Identifier& nymID,
    const Identifier& issuerID)
{
    AccountSummarySortKey output{false, DEFAULT_ISSUER_NAME};
    auto& [state, name] = output;

    const auto issuer = wallet_.Issuer(nymID, issuerID);

    if (false == bool(issuer)) { return output; }

    const auto serverID = issuer->PrimaryServer();

    if (serverID->empty()) { return output; }

    server_thread_map_.emplace(serverID, Identifier::Factory(issuerID));
    auto server = wallet_.Server(serverID);

    if (false == bool(server)) { return output; }

    switch (connection_.Status(serverID->str())) {
        case ConnectionState::ACTIVE: {
            state = true;
        }
        case ConnectionState::NOT_ESTABLISHED:
        case ConnectionState::STALLED:
        default: {
        }
    }

    name = server->Alias();

    return output;
}

void AccountSummary::process_issuer(const Identifier& issuerID)
{
    threads_.emplace(Identifier::Factory(issuerID));
    add_item(issuerID, extract_key(nym_id_, issuerID), {});
}

void AccountSummary::process_issuer(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(2 == message.Body().size());

    const auto nymID = Identifier::Factory(message.Body().at(0));
    const auto issuerID = Identifier::Factory(message.Body().at(1));

    OT_ASSERT(false == nymID->empty())
    OT_ASSERT(false == issuerID->empty())

    if (nymID.get() != nym_id_) { return; }

    auto existing = names_.count(issuerID);

    if (0 == existing) { process_issuer(issuerID); }
}

void AccountSummary::process_server(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const auto serverID =
        Identifier::Factory(std::string(*message.Body().begin()));
    sLock lock(shared_lock_);
    const auto it = server_thread_map_.find(serverID);

    if (server_thread_map_.end() == it) { return; }

    const auto issuerID = it->second;
    lock.unlock();
    add_item(issuerID, extract_key(nym_id_, issuerID), {});
}

void AccountSummary::startup()
{
    const auto issuers = wallet_.IssuerList(nym_id_);
    otWarn << OT_METHOD << __FUNCTION__ << ": Loading " << issuers.size()
           << " issuers." << std::endl;

    for (const auto& id : issuers) { process_issuer(id); }

    startup_complete_->On();
}
}  // namespace opentxs::ui::implementation
