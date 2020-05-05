// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/AccountActivity.cpp"

#pragma once

#include <atomic>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/protobuf/PaymentWorkflowEnums.pb.h"
#include "opentxs/ui/AccountActivity.hpp"
#include "ui/List.hpp"
#include "ui/Widget.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace client
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class PaymentEvent;
class PaymentWorkflow;
}  // namespace proto

class Factory;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using AccountActivityList = List<
    AccountActivityExternalInterface,
    AccountActivityInternalInterface,
    AccountActivityRowID,
    AccountActivityRowInterface,
    AccountActivityRowInternal,
    AccountActivityRowBlank,
    AccountActivitySortKey,
    AccountActivityPrimaryID>;

/** Show the list of Workflows applicable to this account

    Each row is a BalanceItem which is associated with a Workflow state.

    Some Workflows will only have one entry in the AccountActivity based on
    their type, but others may have multiple entries corresponding to different
    states.
 */
class AccountActivity final : public AccountActivityList
{
public:
    const Identifier& AccountID() const noexcept final
    {
        return account_id_.get();
    }
    int BalancePolarity() const noexcept final
    {
        return polarity(balance_.load());
    }
    Amount Balance() const noexcept final { return balance_.load(); }
    std::string DisplayBalance() const noexcept final;

    AccountActivity(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const Identifier& accountID
#if OT_QT
        ,
        const bool qt
#endif
        ) noexcept;

    ~AccountActivity() final;

private:
    friend opentxs::Factory;

    using EventRow =
        std::pair<AccountActivitySortKey, const proto::PaymentEvent*>;
    using RowKey = std::pair<proto::PaymentEventType, EventRow>;

    const ListenerDefinitions listeners_;
    mutable std::atomic<Amount> balance_;
    const OTIdentifier account_id_;
    OTUnitDefinition contract_;

    static EventRow extract_event(
        const proto::PaymentEventType event,
        const proto::PaymentWorkflow& workflow) noexcept;
    static std::vector<RowKey> extract_rows(
        const proto::PaymentWorkflow& workflow) noexcept;

    void* construct_row(
        const AccountActivityRowID& id,
        const AccountActivitySortKey& index,
        const CustomData& custom) const noexcept final;

    void process_balance(const network::zeromq::Message& message) noexcept;
    void process_workflow(
        const Identifier& workflowID,
        std::set<AccountActivityRowID>& active) noexcept;
    void process_workflow(const network::zeromq::Message& message) noexcept;
    void startup() noexcept;

    AccountActivity() = delete;
    AccountActivity(const AccountActivity&) = delete;
    AccountActivity(AccountActivity&&) = delete;
    AccountActivity& operator=(const AccountActivity&) = delete;
    AccountActivity& operator=(AccountActivity&&) = delete;
};
}  // namespace opentxs::ui::implementation
