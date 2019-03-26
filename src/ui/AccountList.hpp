// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using AccountListList = List<
    AccountListExternalInterface,
    AccountListInternalInterface,
    AccountListRowID,
    AccountListRowInterface,
    AccountListRowInternal,
    AccountListRowBlank,
    AccountListSortKey,
    AccountListPrimaryID>;

class AccountList final : public AccountListList
{
public:
    ~AccountList();

private:
    friend opentxs::Factory;

    const ListenerDefinitions listeners_;

    void construct_row(
        const AccountListRowID& id,
        const AccountListSortKey& index,
        const CustomData& custom) const override;

    void process_account(const Identifier& id);
    void process_account(const Identifier& id, const Amount balance);
    void process_account(
        const Identifier& id,
        const Amount balance,
        const std::string& name);
    void process_account(const network::zeromq::Message& message);
    void startup();

    AccountList(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const identifier::Nym& nymID);
    AccountList() = delete;
    AccountList(const AccountList&) = delete;
    AccountList(AccountList&&) = delete;
    AccountList& operator=(const AccountList&) = delete;
    AccountList& operator=(AccountList&&) = delete;
};
}  // namespace opentxs::ui::implementation
