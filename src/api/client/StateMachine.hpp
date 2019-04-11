// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/client/OTX.hpp"
#include "opentxs/core/UniqueQueue.hpp"

#include "internal/api/client/Client.hpp"

#include <functional>
#include <future>
#include <tuple>

#define SHUTDOWN()                                                             \
    {                                                                          \
        YIELD(50);                                                             \
    }

#define YIELD(a)                                                               \
    {                                                                          \
        if (!running_) { return; }                                             \
                                                                               \
        Log::Sleep(std::chrono::milliseconds(a));                              \
    }

namespace std
{
using MESSAGETASK = tuple<opentxs::OTNymID, string, opentxs::SetID>;
using PAYMENTTASK =
    pair<opentxs::OTIdentifier, shared_ptr<const opentxs::OTPayment>>;

template <>
struct less<PAYMENTTASK> {
    bool operator()(const PAYMENTTASK& lhs, const PAYMENTTASK& rhs) const
    {
        /* TODO: use structured bindings */
        const auto& lID = std::get<0>(lhs);
        const auto& lPayment = std::get<1>(lhs);
        const auto& rID = std::get<0>(rhs);
        const auto& rPayment = std::get<1>(rhs);

        if (lID->str() < rID->str()) { return true; }

        if (rID->str() < lID->str()) { return false; }

        if (lPayment.get() < rPayment.get()) { return true; }

        return false;
    }
};

template <>
struct less<MESSAGETASK> {
    bool operator()(const MESSAGETASK& lhs, const MESSAGETASK& rhs) const
    {
        /* TODO: use structured bindings */
        const auto& lID = std::get<0>(lhs);
        const auto& lMessage = std::get<1>(lhs);
        const auto& lFunction = std::get<2>(lhs);
        const auto& rID = std::get<0>(rhs);
        const auto& rMessage = std::get<1>(rhs);
        const auto& rFunction = std::get<2>(rhs);

        if (lID->str() < rID->str()) { return true; }

        if (rID->str() < lID->str()) { return false; }

        if (lMessage < rMessage) { return true; }

        if (rMessage < lMessage) { return false; }

        if (&lFunction < &rFunction) { return true; }

        return false;
    }
};
}  // namespace std

namespace opentxs::api::client::implementation
{
class StateMachine
{
public:
    using BackgroundTask = api::client::OTX::BackgroundTask;
    using ContextID = std::pair<OTNymID, OTServerID>;
    using RefreshTask = std::pair<int, std::promise<void>>;
    using Result = api::client::OTX::Result;
    using TaskID = api::client::OTX::TaskID;
    using Thread = std::function<void()>;

    union Params {
        CheckNymTask check_nym_;
        DepositPaymentTask deposit_payment_;
        DownloadContractTask download_contract_;
        DownloadMintTask download_mint_;
        DownloadNymboxTask download_nymbox_;
        GetTransactionNumbersTask get_transaction_numbers_;
        IssueUnitDefinitionTask issue_unit_definition_;
        RegisterAccountTask register_account_;
        RegisterNymTask register_nym_;
        MessageTask send_message_;
        PaymentTask send_payment_;
#if OT_CASH
        PayCashTask send_cash_;
        WithdrawCashTask withdraw_cash_;
#endif
        SendTransferTask send_transfer_;
        PublishServerContractTask publish_server_contract_;
        ProcessInboxTask process_inbox_;
        SendChequeTask send_cheque_;
        PeerReplyTask peer_reply_;
        PeerRequestTask peer_request_;

        Params() { memset(this, 0, sizeof(Params)); }
        ~Params() {}
    };

    static Result error_result();

    template <typename T>
    const UniqueQueue<T>& Queue() const;

    void check_thread(Thread task, std::unique_ptr<std::thread>& thread);
    std::future<void> check_future(
        Thread task,
        std::unique_ptr<std::thread>& thread);
    void Process();
    void Shutdown() { op_.Shutdown(); }

    StateMachine(
        const api::client::Manager& client,
        const api::client::internal::OTX& parent,
        const Flag& running,
        const api::client::Manager& api,
        const ContextID& id,
        std::atomic<TaskID>& nextTaskID,
        const UniqueQueue<CheckNymTask>& missingNyms,
        const UniqueQueue<CheckNymTask>& outdatedNyms,
        const UniqueQueue<OTServerID>& missingServers,
        const UniqueQueue<OTUnitID>& missingUnitDefinitions);
    StateMachine() = delete;

private:
    enum class TaskDone : int { no, yes, retry };

    const api::client::Manager& client_;
    const api::client::internal::OTX& parent_;
    std::atomic<TaskID>& next_task_id_;
    const UniqueQueue<CheckNymTask>& missing_nyms_;
    const UniqueQueue<CheckNymTask>& outdated_nyms_;
    const UniqueQueue<OTServerID>& missing_servers_;
    const UniqueQueue<OTUnitID>& missing_unit_definitions_;
    std::unique_ptr<api::client::internal::Operation> pOp_;
    api::client::internal::Operation& op_;
    UniqueQueue<CheckNymTask> check_nym_;
    UniqueQueue<DepositPaymentTask> deposit_payment_;
    UniqueQueue<DownloadContractTask> download_contract_;
    UniqueQueue<DownloadMintTask> download_mint_;
    UniqueQueue<DownloadNymboxTask> download_nymbox_;
    UniqueQueue<GetTransactionNumbersTask> get_transaction_numbers_;
    UniqueQueue<IssueUnitDefinitionTask> issue_unit_definition_;
    UniqueQueue<RegisterAccountTask> register_account_;
    UniqueQueue<RegisterNymTask> register_nym_;
    UniqueQueue<MessageTask> send_message_;
    UniqueQueue<PaymentTask> send_payment_;
#if OT_CASH
    UniqueQueue<PayCashTask> send_cash_;
    UniqueQueue<WithdrawCashTask> withdraw_cash_;
#endif  // OT_CASH
    UniqueQueue<SendTransferTask> send_transfer_;
    UniqueQueue<PublishServerContractTask> publish_server_contract_;
    UniqueQueue<ProcessInboxTask> process_inbox_;
    UniqueQueue<SendChequeTask> send_cheque_;
    UniqueQueue<PeerReplyTask> peer_reply_;
    UniqueQueue<PeerRequestTask> peer_request_;
    const Flag& running_;
    Params param_;
    TaskID task_id_{};
    std::atomic<int> counter_;
    mutable std::mutex lock_;
    bool thread_;
    bool continue_;
    std::vector<RefreshTask> tasks_;

    static TaskDone task_done(bool done)
    {
        return done ? TaskDone::yes : TaskDone::no;
    }

    void associate_message_id(const Identifier& messageID, const TaskID taskID)
        const
    {
        return parent_.associate_message_id(messageID, taskID);
    }
    Depositability can_deposit(
        const OTPayment& payment,
        const identifier::Nym& recipient,
        const Identifier& accountIDHint,
        identifier::Server& depositServer,
        Identifier& depositAccount) const
    {
        return parent_.can_deposit(
            payment, recipient, accountIDHint, depositServer, depositAccount);
    }
    void check_nym_revision(const ServerContext& context) const;
    bool check_registration(
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        std::shared_ptr<const ServerContext>& context) const;
    bool check_server_contract(const identifier::Server& serverID) const;
    bool check_server_name(const ServerContext& context) const;
    void check_transaction_numbers(const ServerContext& context) const;
    int counter() const { return counter_.load(); }
    bool deposit_cheque(const TaskID taskID, const DepositPaymentTask& task)
        const;
    bool download_contract(const TaskID taskID, const Identifier& contractID)
        const;
#if OT_CASH
    bool download_mint(const TaskID taskID, const DownloadMintTask& task) const;
#endif
    bool download_nym(const TaskID taskID, const identifier::Nym& targetNymID)
        const;
    bool download_nymbox(const TaskID taskID) const;
    bool find_nym(const identifier::Nym& targetNymID, const bool skipExisting)
        const;
    bool find_server(const identifier::Server& notary) const;
    bool finish_task(const TaskID taskID, const bool success, Result&& result)
        const
    {
        return parent_.finish_task(taskID, success, std::move(result));
    }
    bool get_admin(const TaskID taskID, const OTPassword& password) const;
    UniqueQueue<OTNymID>& get_nym_fetch(
        const identifier::Server& serverID) const
    {
        return parent_.get_nym_fetch(serverID);
    }
    template <typename T>
    const UniqueQueue<T>& get_task() const
    {
        throw;
    }
    bool get_transaction_numbers(const TaskID taskID) const;
    bool initiate_peer_reply(const TaskID taskID, const PeerReplyTask& task)
        const;
    bool initiate_peer_request(const TaskID taskID, const PeerRequestTask& task)
        const;
    bool issue_unit_definition(
        const TaskID taskID,
        const IssueUnitDefinitionTask& task) const;
    bool message_nym(const TaskID taskID, const MessageTask& task) const;
    TaskID next_task_id() const { return ++next_task_id_; }
    bool pay_nym(const TaskID taskID, const PaymentTask& task) const;
#if OT_CASH
    bool pay_nym_cash(const TaskID taskID, const PayCashTask& Task) const;
#endif  // OT_CASH
    bool process_inbox(const TaskID taskID, const Identifier& accountID) const;
    bool publish_server_contract(
        const TaskID taskID,
        const identifier::Server& serverID) const;
    bool register_account(const TaskID taskID, const RegisterAccountTask& task)
        const;
    template <typename T>
    void run_task(
        const UniqueQueue<T>& queue,
        std::function<bool(const TaskID, const T&)> func);
    template <typename T>
    void run_task(std::function<bool(const TaskID, const T&)> func);
    bool register_nym(const TaskID taskID, const bool resync) const;
    bool send_transfer(const TaskID taskID, const SendTransferTask& task) const;
    BackgroundTask start_task(const TaskID taskID, bool success) const
    {
        return parent_.start_task(taskID, success);
    }
#if OT_CASH
    bool withdraw_cash(const TaskID taskID, const WithdrawCashTask& task) const;
#endif  // OT_CASH
    TaskDone write_and_send_cheque(
        const TaskID taskID,
        const SendChequeTask& task) const;

    std::future<void> add_task(const Lock& lock);
    void check_thread(
        const Lock& lock,
        Thread task,
        std::unique_ptr<std::thread>& thread);
    void cleanup(Lock& lock);
    template <typename T>
    T& get_param()
    {
        throw;
    }
    void increment_counter(const bool missing, Lock& lock, bool& run);
};
}  // namespace opentxs::api::client::implementation
