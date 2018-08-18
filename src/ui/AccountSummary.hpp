// Copyright (c) 2018 The Open-Transactions developers
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
    AccountSummarySortKey>;

class AccountSummary final : public AccountSummaryList
{
public:
    proto::ContactItemType Currency() const override { return currency_; }
    const Identifier& NymID() const override { return nym_id_.get(); }

    ~AccountSummary() = default;

private:
    friend opentxs::Factory;

    static const ListenerDefinitions listeners_;

    const api::network::ZMQ& connection_;
    const api::storage::Storage& storage_;
    const api::Core& api_;
    const proto::ContactItemType currency_;
    std::set<OTIdentifier> issuers_;
    std::map<OTIdentifier, OTIdentifier> server_issuer_map_;
    std::map<OTIdentifier, OTIdentifier> nym_server_map_;

    void construct_row(
        const AccountSummaryRowID& id,
        const AccountSummarySortKey& index,
        const CustomData& custom) const override;

    AccountSummarySortKey extract_key(
        const Identifier& nymID,
        const Identifier& issuerID);
    void process_connection(const network::zeromq::Message& message);
    void process_issuer(const Identifier& issuerID);
    void process_issuer(const network::zeromq::Message& message);
    void process_nym(const network::zeromq::Message& message);
    void process_server(const network::zeromq::Message& message);
    void process_server(const OTIdentifier& serverID);
    void startup();

    AccountSummary(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::network::ZMQ& connection,
        const api::storage::Storage& storage,
        const api::client::Contacts& contact,
        const api::Core& core,
        const Identifier& nymID,
        const proto::ContactItemType currency);
    AccountSummary() = delete;
    AccountSummary(const AccountSummary&) = delete;
    AccountSummary(AccountSummary&&) = delete;
    AccountSummary& operator=(const AccountSummary&) = delete;
    AccountSummary& operator=(AccountSummary&&) = delete;
};
}  // namespace opentxs::ui::implementation
