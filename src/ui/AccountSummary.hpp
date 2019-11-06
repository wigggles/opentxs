// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using AccountSummaryList = List<
    AccountSummaryExternalInterface,
    AccountSummaryInternalInterface,
    AccountSummaryRowID,
    AccountSummaryRowInterface,
    AccountSummaryRowInternal,
    AccountSummaryRowBlank,
    AccountSummarySortKey,
    AccountSummaryPrimaryID>;

class AccountSummary final : public AccountSummaryList
{
public:
    proto::ContactItemType Currency() const noexcept final { return currency_; }
#if OT_QT
    int FindRow(const AccountSummaryRowID& id, const AccountSummarySortKey& key)
        const noexcept final
    {
        return find_row(id, key);
    }
#endif
    const identifier::Nym& NymID() const noexcept final { return primary_id_; }

    ~AccountSummary();

private:
    friend opentxs::Factory;

    const ListenerDefinitions listeners_;
    const proto::ContactItemType currency_;
    std::set<OTNymID> issuers_;
    std::map<OTServerID, OTNymID> server_issuer_map_;
    std::map<OTNymID, OTServerID> nym_server_map_;

    void* construct_row(
        const AccountSummaryRowID& id,
        const AccountSummarySortKey& index,
        const CustomData& custom) const noexcept final;

    AccountSummarySortKey extract_key(
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID) noexcept;
    void process_connection(const network::zeromq::Message& message) noexcept;
    void process_issuer(const identifier::Nym& issuerID) noexcept;
    void process_issuer(const network::zeromq::Message& message) noexcept;
    void process_nym(const network::zeromq::Message& message) noexcept;
    void process_server(const network::zeromq::Message& message) noexcept;
    void process_server(const identifier::Server& serverID) noexcept;
    void startup() noexcept;

    AccountSummary(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const proto::ContactItemType currency
#if OT_QT
        ,
        const bool qt
#endif
        ) noexcept;
    AccountSummary() = delete;
    AccountSummary(const AccountSummary&) = delete;
    AccountSummary(AccountSummary&&) = delete;
    AccountSummary& operator=(const AccountSummary&) = delete;
    AccountSummary& operator=(AccountSummary&&) = delete;
};
}  // namespace opentxs::ui::implementation
