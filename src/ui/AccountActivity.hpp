// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
class AccountActivity : public AccountActivityList
{
public:
    auto AccountID() const noexcept -> std::string final
    {
        return account_id_->str();
    }
    auto Balance() const noexcept -> Amount final { return balance_.load(); }
    auto BalancePolarity() const noexcept -> int final
    {
        return polarity(balance_.load());
    }
#if OT_BLOCKCHAIN
    auto DepositAddress(const blockchain::Type) const noexcept
        -> std::string override
    {
        return {};
    }
    auto DepositChains() const noexcept
        -> std::vector<blockchain::Type> override
    {
        return {};
    }
#endif  // OT_BLOCKCHAIN
    auto Type() const noexcept -> AccountType final { return type_; }

    ~AccountActivity() override;

protected:
    const ListenerDefinitions listeners_;
    mutable std::atomic<Amount> balance_;
    const OTIdentifier account_id_;
    const AccountType type_;

    AccountActivity(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const AccountType type,
#if OT_QT
        const bool qt,
#endif
        ListenerDefinitions&& listeners) noexcept;

private:
    virtual auto startup() noexcept -> void = 0;

    AccountActivity() = delete;
    AccountActivity(const AccountActivity&) = delete;
    AccountActivity(AccountActivity&&) = delete;
    AccountActivity& operator=(const AccountActivity&) = delete;
    AccountActivity& operator=(AccountActivity&&) = delete;
};
}  // namespace opentxs::ui::implementation
