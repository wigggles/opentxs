// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class Workflow : virtual public opentxs::api::client::Workflow, Lockable
{
public:
    bool CancelCheque(
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const override;
    bool ClearCheque(
        const Identifier& recipientNymID,
        const OTTransaction& receipt) const override;
    bool DepositCheque(
        const Identifier& nymID,
        const Identifier& accountID,
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const override;
    bool ExpireCheque(const Identifier& nymID, const opentxs::Cheque& cheque)
        const override;
    bool ExportCheque(const opentxs::Cheque& cheque) const override;
    bool FinishCheque(
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const override;
    OTIdentifier ImportCheque(
        const Identifier& nymID,
        const opentxs::Cheque& cheque) const override;
    std::set<OTIdentifier> List(
        const Identifier& nymID,
        const proto::PaymentWorkflowType type,
        const proto::PaymentWorkflowState state) const override;
    Cheque LoadCheque(const Identifier& nymID, const Identifier& chequeID)
        const override;
    Cheque LoadChequeByWorkflow(
        const Identifier& nymID,
        const Identifier& workflowID) const override;
    std::shared_ptr<proto::PaymentWorkflow> LoadWorkflow(
        const Identifier& nymID,
        const Identifier& workflowID) const override;
    bool SendCheque(
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const override;
    OTIdentifier ReceiveCheque(
        const Identifier& nymID,
        const opentxs::Cheque& cheque,
        const Message& message) const override;
    std::vector<OTIdentifier> WorkflowsByAccount(
        const Identifier& nymID,
        const Identifier& accountID) const override;
    OTIdentifier WriteCheque(const opentxs::Cheque& cheque) const override;

    ~Workflow() = default;

private:
    friend opentxs::Factory;

    const api::Core& api_;
    const Activity& activity_;
    const Contacts& contact_;
    const OTZMQPublishSocket account_publisher_;
    const OTZMQPushSocket rpc_publisher_;

    static bool can_accept_cheque(const proto::PaymentWorkflow& workflow);
    static bool can_cancel_cheque(const proto::PaymentWorkflow& workflow);
    static bool can_convey_cheque(const proto::PaymentWorkflow& workflow);
    static bool can_deposit_cheque(const proto::PaymentWorkflow& workflow);
    static bool can_expire_cheque(
        const opentxs::Cheque& cheque,
        const proto::PaymentWorkflow& workflow);
    static bool can_finish_cheque(const proto::PaymentWorkflow& workflow);
    static bool cheque_deposit_success(const Message* message);
    static std::chrono::time_point<std::chrono::system_clock>
    extract_conveyed_time(const proto::PaymentWorkflow& workflow);
    static bool isCheque(const opentxs::Cheque& cheque);
    static std::int64_t now();
    static bool validate_recipient(
        const Identifier& nymID,
        const opentxs::Cheque& cheque);

    bool add_cheque_event(
        const std::string& nymID,
        const std::string& eventNym,
        proto::PaymentWorkflow& workflow,
        const proto::PaymentWorkflowState newState,
        const proto::PaymentEventType newEventType,
        const std::uint32_t version,
        const Message& request,
        const Message* reply,
        const std::string& account = "") const;
    bool add_cheque_event(
        const std::string& nymID,
        const std::string& accountID,
        proto::PaymentWorkflow& workflow,
        const proto::PaymentWorkflowState newState,
        const proto::PaymentEventType newEventType,
        const std::uint32_t version,
        const Identifier& recipientNymID,
        const OTTransaction& receipt,
        const std::chrono::time_point<std::chrono::system_clock> time =
            std::chrono::system_clock::from_time_t(now())) const;
    std::pair<OTIdentifier, proto::PaymentWorkflow> create_cheque(
        const eLock& lock,
        const std::string& nymID,
        const opentxs::Cheque& cheque,
        const proto::PaymentWorkflowType workflowType,
        const proto::PaymentWorkflowState workflowState,
        const std::uint32_t workflowVersion,
        const std::uint32_t sourceVersion,
        const std::uint32_t eventVersion,
        const std::string& party,
        const std::string& account,
        const Message* message = nullptr) const;
    std::shared_ptr<proto::PaymentWorkflow> get_workflow(
        const std::set<proto::PaymentWorkflowType>& types,
        const std::string& nymID,
        const opentxs::Cheque& source) const;
    std::shared_ptr<proto::PaymentWorkflow> get_workflow_by_id(
        const std::set<proto::PaymentWorkflowType>& types,
        const std::string& nymID,
        const std::string& workflowID) const;
    std::shared_ptr<proto::PaymentWorkflow> get_workflow_by_id(
        const std::string& nymID,
        const std::string& workflowID) const;
    std::shared_ptr<proto::PaymentWorkflow> get_workflow_by_source(
        const std::set<proto::PaymentWorkflowType>& types,
        const std::string& nymID,
        const std::string& sourceID) const;
    bool save_workflow(
        const std::string& nymID,
        const std::string& accountID,
        const proto::PaymentWorkflow& workflow) const;
    OTIdentifier save_workflow(
        OTIdentifier&& workflowID,
        const std::string& nymID,
        const std::string& accountID,
        const proto::PaymentWorkflow& workflow) const;
    std::pair<OTIdentifier, proto::PaymentWorkflow> save_workflow(
        std::pair<OTIdentifier, proto::PaymentWorkflow>&& workflowID,
        const std::string& nymID,
        const std::string& accountID,
        const proto::PaymentWorkflow& workflow) const;
    void update_activity(
        const Identifier& localNymID,
        const Identifier& remoteNymID,
        const Identifier& sourceID,
        const Identifier& workflowID,
        const StorageBox type,
        std::chrono::time_point<std::chrono::system_clock> time) const;
    void update_rpc(
        const std::string& localNymID,
        const std::string& remoteNymID,
        const std::string& accountID,
        const proto::AccountEventType type,
        const TransactionNumber number,
        const Amount amount,
        const Amount pending,
        const std::chrono::time_point<std::chrono::system_clock> time,
        const std::string& memo) const;

    Workflow(
        const api::Core& api,
        const Activity& activity,
        const Contacts& contact);
    Workflow() = delete;
    Workflow(const Workflow&) = delete;
    Workflow(Workflow&&) = delete;
    Workflow& operator=(const Workflow&) = delete;
    Workflow& operator=(Workflow&&) = delete;
};
}  // namespace opentxs::api::client::implementation
