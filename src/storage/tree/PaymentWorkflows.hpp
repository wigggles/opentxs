// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/Proto.hpp"

#include "Node.hpp"

namespace opentxs::storage
{
class PaymentWorkflows final : public Node
{
public:
    using State =
        std::pair<proto::PaymentWorkflowType, proto::PaymentWorkflowState>;
    using Workflows = std::set<std::string>;

    State GetState(const std::string& workflowID) const;
    Workflows ListByAccount(const std::string& accountID) const;
    Workflows ListByState(
        proto::PaymentWorkflowType type,
        proto::PaymentWorkflowState state) const;
    Workflows ListByUnit(const std::string& unitID) const;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::PaymentWorkflow>& output,
        const bool checking) const;
    std::string LookupBySource(const std::string& sourceID) const;

    bool Delete(const std::string& id);
    bool Store(const proto::PaymentWorkflow& data, std::string& plaintext);

    ~PaymentWorkflows() final = default;

private:
    friend Nym;

    Workflows archived_;
    std::map<std::string, std::string> item_workflow_map_;
    std::map<std::string, Workflows> account_workflow_map_;
    std::map<std::string, Workflows> unit_workflow_map_;
    std::map<std::string, State> workflow_state_map_;
    std::map<proto::PaymentWorkflowType, Workflows> type_workflow_map_;
    std::map<State, Workflows> state_workflow_map_;

    bool save(const Lock& lock) const final;
    proto::StoragePaymentWorkflows serialize() const;

    void add_state_index(
        const Lock& lock,
        const std::string& workflowID,
        proto::PaymentWorkflowType type,
        proto::PaymentWorkflowState state);
    void delete_by_value(const std::string& value);
    void init(const std::string& hash) final;
    void reindex(
        const Lock& lock,
        const std::string& workflowID,
        const proto::PaymentWorkflowType type,
        const proto::PaymentWorkflowState newState,
        proto::PaymentWorkflowState& state);

    PaymentWorkflows(
        const opentxs::api::storage::Driver& storage,
        const std::string& key);
    PaymentWorkflows() = delete;
    PaymentWorkflows(const PaymentWorkflows&) = delete;
    PaymentWorkflows(PaymentWorkflows&&) = delete;
    PaymentWorkflows operator=(const PaymentWorkflows&) = delete;
    PaymentWorkflows operator=(PaymentWorkflows&&) = delete;
};
}  // namespace opentxs::storage
