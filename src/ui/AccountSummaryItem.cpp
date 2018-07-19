// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Issuer.hpp"
#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/AccountSummaryItem.hpp"

#include "IssuerItemParent.hpp"
#include "Row.hpp"

#include <atomic>

#include "AccountSummaryItem.hpp"

template class opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem>;

namespace opentxs
{
ui::AccountSummaryItem* Factory::AccountSummaryItem(
    const ui::implementation::IssuerItemParent& parent,
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::client::Wallet& wallet,
    const api::storage::Storage& storage,
    const api::ContactManager& contact,
    const ui::implementation::IssuerItemRowID& id)
{
    return new ui::implementation::AccountSummaryItem(
        parent, zmq, publisher, wallet, storage, contact, id);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
const Widget::ListenerDefinitions AccountSummaryItem::listeners_{
    {network::zeromq::Socket::AccountUpdateEndpoint,
     new MessageProcessor<AccountSummaryItem>(
         &AccountSummaryItem::process_account)}};

AccountSummaryItem::AccountSummaryItem(
    const IssuerItemParent& parent,
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::client::Wallet& wallet,
    const api::storage::Storage& storage,
    const api::ContactManager& contact,
    const IssuerItemRowID& id)
    : AccountSummaryItemRow(parent, zmq, publisher, contact, id, true)
    , wallet_{wallet}
    , storage_{storage}
    , account_id_{std::get<0>(id_).get()}
    , currency_{std::get<1>(id_)}
    , balance_{0}
    , name_{""}
    , contract_{nullptr}
{
    setup_listeners(listeners_);
    update();
}

std::string AccountSummaryItem::DisplayBalance() const
{
    sLock lock(shared_lock_);

    if (contract_) {
        const auto amount = balance_.load();
        std::string output{};
        const auto formatted =
            contract_->FormatAmountLocale(amount, output, ",", ".");

        if (formatted) { return output; }

        return std::to_string(amount);
    }

    return {};
}

std::string AccountSummaryItem::Name() const
{
    sLock lock(shared_lock_);

    return name_;
}

void AccountSummaryItem::process_account(
    const network::zeromq::Message& message)
{
    OT_ASSERT(2 == message.Body().size())

    const auto accountID = Identifier::Factory(message.Body().at(0));

    if (account_id_ == accountID) { update(); }
}

void AccountSummaryItem::update()
{
    auto account = wallet_.Account(account_id_);

    if (account) {
        String name{""};
        account.get().GetName(name);
        balance_.store(account.get().GetBalance());
        eLock lock(shared_lock_);

        if (false == bool(contract_)) {
            contract_ =
                wallet_.UnitDefinition(storage_.AccountContract(account_id_));
        }

        name_ = std::string(name.Get());
        lock.unlock();
        UpdateNotify();
    }
}
}  // namespace opentxs::ui::implementation
