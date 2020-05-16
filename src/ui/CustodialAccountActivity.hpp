// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/CustodialAccountActivity.cpp"

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
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/protobuf/PaymentWorkflowEnums.pb.h"
#include "opentxs/ui/AccountActivity.hpp"
#include "ui/AccountActivity.hpp"
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

class Core;
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

class Identifier;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
class CustodialAccountActivity final : public AccountActivity
{
public:
    auto ContractID() const noexcept -> std::string final
    {
        return contract_->ID()->str();
    }
    auto DisplayBalance() const noexcept -> std::string final;
    auto DisplayUnit() const noexcept -> std::string final
    {
        return contract_->TLA();
    }
    auto Name() const noexcept -> std::string final;
    auto NotaryID() const noexcept -> std::string final
    {
        return notary_->ID()->str();
    }
    auto NotaryName() const noexcept -> std::string final
    {
        return notary_->EffectiveName();
    }
    auto Unit() const noexcept -> proto::ContactItemType final
    {
        return contract_->UnitOfAccount();
    }

    CustodialAccountActivity(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const Identifier& accountID
#if OT_QT
        ,
        const bool qt
#endif
        ) noexcept;

    ~CustodialAccountActivity() final = default;

private:
    using EventRow =
        std::pair<AccountActivitySortKey, const proto::PaymentEvent*>;
    using RowKey = std::pair<proto::PaymentEventType, EventRow>;

    const OTUnitDefinition contract_;
    const OTServerContract notary_;
    std::string alias_;

    static auto extract_event(
        const proto::PaymentEventType event,
        const proto::PaymentWorkflow& workflow) noexcept -> EventRow;
    static auto extract_rows(const proto::PaymentWorkflow& workflow) noexcept
        -> std::vector<RowKey>;
    static auto load_server(const api::Core& api, const Identifier& account)
        -> OTServerContract;
    static auto load_unit(const api::Core& api, const Identifier& account)
        -> OTUnitDefinition;

    auto construct_row(
        const AccountActivityRowID& id,
        const AccountActivitySortKey& index,
        const CustomData& custom) const noexcept -> void* final;

    void process_balance(const network::zeromq::Message& message) noexcept;
    void process_workflow(
        const Identifier& workflowID,
        std::set<AccountActivityRowID>& active) noexcept;
    void process_workflow(const network::zeromq::Message& message) noexcept;
    void startup() noexcept final;

    CustodialAccountActivity() = delete;
    CustodialAccountActivity(const CustodialAccountActivity&) = delete;
    CustodialAccountActivity(CustodialAccountActivity&&) = delete;
    auto operator=(const CustodialAccountActivity&)
        -> CustodialAccountActivity& = delete;
    auto operator=(CustodialAccountActivity &&)
        -> CustodialAccountActivity& = delete;
};
}  // namespace opentxs::ui::implementation
