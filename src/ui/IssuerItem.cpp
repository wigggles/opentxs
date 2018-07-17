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
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/IssuerItem.hpp"

#include "AccountSummaryItemBlank.hpp"
#include "AccountSummaryParent.hpp"
#include "IssuerItemParent.hpp"
#include "List.hpp"
#include "Row.hpp"

#include <atomic>

#include "IssuerItem.hpp"

template class opentxs::SharedPimpl<opentxs::ui::IssuerItem>;

#define DEFAULT_NAME "Stash Node Pro"

#define OT_METHOD "opentxs::ui::implementation::IssuerItem::"

namespace opentxs
{
ui::IssuerItem* Factory::IssuerItem(
    const ui::implementation::AccountSummaryParent& parent,
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const api::client::Wallet& wallet,
    const api::storage::Storage& storage,
    const proto::ContactItemType currency,
    const Identifier& id)
{
    return new ui::implementation::IssuerItem(
        parent, zmq, publisher, contact, wallet, storage, currency, id);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
const Widget::ListenerDefinitions IssuerItem::listeners_{
    {network::zeromq::Socket::ConnectionStatusEndpoint,
     new MessageProcessor<IssuerItem>(&IssuerItem::process_connection)},
    {network::zeromq::Socket::IssuerUpdateEndpoint,
     new MessageProcessor<IssuerItem>(&IssuerItem::process_issuer)},
    {network::zeromq::Socket::ServerUpdateEndpoint,
     new MessageProcessor<IssuerItem>(&IssuerItem::process_server)},
    {network::zeromq::Socket::NymDownloadEndpoint,
     new MessageProcessor<IssuerItem>(&IssuerItem::process_nym)},
};

IssuerItem::IssuerItem(
    const AccountSummaryParent& parent,
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const api::client::Wallet& wallet,
    const api::storage::Storage& storage,
    const proto::ContactItemType currency,
    const Identifier& id)
    : IssuerItemList(parent.NymID(), zmq, publisher, contact)
    , IssuerItemRow(parent, Identifier::Factory(id), true)
    , wallet_{wallet}
    , storage_{storage}
    , connection_{false}
    , issuer_{wallet.Issuer(parent.NymID(), id)}
    , currency_{currency}
{
    OT_ASSERT(issuer_)

    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&IssuerItem::startup, this));

    OT_ASSERT(startup_)
}

bool IssuerItem::ConnectionState() const { return connection_.load(); }

void IssuerItem::construct_row(
    const IssuerItemRowID& id,
    const IssuerItemSortKey& index,
    const CustomData&) const
{
    items_[index].emplace(
        id,
        Factory::AccountSummaryItem(
            *this, zmq_, publisher_, wallet_, storage_, contact_manager_, id));
    names_.emplace(id, index);
}

std::string IssuerItem::Debug() const { return *issuer_; }

std::string IssuerItem::Name() const
{
    const auto id = issuer_->PrimaryServer();

    if (id->empty()) { return DEFAULT_NAME; }

    auto server = wallet_.Server(id);

    if (!server) { return DEFAULT_NAME; }

    return server->Alias();
}

bool IssuerItem::Trusted() const { return issuer_->Paired(); }

void IssuerItem::process_connection(const network::zeromq::Message& message)
{
    OT_ASSERT(2 == message.Body().size());

    const auto serverID = Identifier::Factory(message.Body().at(0));

    if (issuer_->PrimaryServer() != serverID) { return; }

    const auto& raw = message.Body().at(1);
    const bool state(*static_cast<const bool*>(raw.data()));
    const auto previous = connection_.exchange(state);

    if (previous != state) { UpdateNotify(); }
}

void IssuerItem::process_issuer(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(2 == message.Body().size());

    const auto nymID = Identifier::Factory(message.Body().at(0));
    const auto issuerID = Identifier::Factory(message.Body().at(1));

    OT_ASSERT(false == nymID->empty())
    OT_ASSERT(false == issuerID->empty())

    if (nymID.get() != nym_id_) { return; }
    if (id_ != issuerID) { return; }

    refresh_accounts();
}

void IssuerItem::process_nym(const network::zeromq::Message& message)
{
    OT_ASSERT(1 == message.Body().size());

    const auto nymID =
        Identifier::Factory(std::string(*message.Body().begin()));

    const auto serverID = issuer_->PrimaryServer();

    if (serverID->empty()) { return; }

    auto server = wallet_.Server(serverID);

    if (false == bool(server)) { return; }

    if (server->Nym()->ID() == nymID.get()) {
        UpdateNotify();
        // TODO call reindex_item on parent AccountSummary object
    }
}

void IssuerItem::process_server(const network::zeromq::Message& message)
{
    OT_ASSERT(1 == message.Body().size());

    const auto serverID =
        Identifier::Factory(std::string(*message.Body().begin()));

    if (issuer_->PrimaryServer() != serverID) { return; }

    UpdateNotify();
}

void IssuerItem::refresh_accounts()
{
    const auto blank = Identifier::Factory();
    const auto accounts = issuer_->AccountList(currency_, blank);
    otWarn << OT_METHOD << __FUNCTION__ << ": Loading " << accounts.size()
           << " accounts." << std::endl;

    for (const auto& id : accounts) {
        const auto account = wallet_.Account(id);

        if (false == bool(account)) { continue; }

        String name{""};
        account.get().GetName(name);
        add_item({id, currency_}, name.Get(), {});
    }

    std::set<IssuerItemRowID> active{};
    std::transform(
        accounts.begin(),
        accounts.end(),
        std::inserter(active, active.end()),
        [&](const OTIdentifier& in) -> IssuerItemRowID {
            return {in, currency_};
        });
    delete_inactive(active);
}

void IssuerItem::startup()
{
    refresh_accounts();
    startup_complete_->On();
}
}  // namespace opentxs::ui::implementation
