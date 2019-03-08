// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

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
class OTX : virtual public client::OTX, Lockable
{
public:
    BackgroundTask AcknowledgeBailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const Identifier& requestID,
        const std::string& instructions,
        const SetID setID) const override;
    BackgroundTask AcknowledgeNotice(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& recipientID,
        const Identifier& requestID,
        const bool ack,
        const SetID setID) const override;
    BackgroundTask AcknowledgeOutbailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& recipientID,
        const Identifier& requestID,
        const std::string& details,
        const SetID setID) const override;
    BackgroundTask AcknowledgeConnection(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& recipientID,
        const Identifier& requestID,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key,
        const SetID setID) const override;
    bool AutoProcessInboxEnabled() const override
    {
        return auto_process_inbox_.get();
    };
    Depositability CanDeposit(
        const Identifier& recipientNymID,
        const OTPayment& payment) const override;
    Depositability CanDeposit(
        const Identifier& recipientNymID,
        const Identifier& accountID,
        const OTPayment& payment) const override;
    Messagability CanMessage(
        const Identifier& senderNymID,
        const Identifier& recipientContactID,
        const bool startIntroductionServer) const override;
    bool CheckTransactionNumbers(
        const identifier::Nym& nym,
        const Identifier& serverID,
        const std::size_t quantity) const override;
    Finished ContextIdle(
        const identifier::Nym& nym,
        const identifier::Server& server) const override;
    std::size_t DepositCheques(const Identifier& nymID) const override;
    std::size_t DepositCheques(
        const Identifier& nymID,
        const std::set<OTIdentifier>& chequeIDs) const override;
    BackgroundTask DepositPayment(
        const Identifier& recipientNymID,
        const std::shared_ptr<const OTPayment>& payment) const override;
    BackgroundTask DepositPayment(
        const Identifier& recipientNymID,
        const Identifier& accountID,
        const std::shared_ptr<const OTPayment>& payment) const override;
    void DisableAutoaccept() const override;
    BackgroundTask DownloadContract(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& contractID) const override;
#if OT_CASH
    BackgroundTask DownloadMint(
        const identifier::Nym& nym,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit) const override;
#endif
    BackgroundTask DownloadNym(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID) const override;
    BackgroundTask DownloadNymbox(
        const Identifier& localNymID,
        const Identifier& serverID) const override;
    BackgroundTask FindNym(const Identifier& nymID) const override;
    BackgroundTask FindNym(
        const Identifier& nymID,
        const Identifier& serverIDHint) const override;
    BackgroundTask FindServer(const Identifier& serverID) const override;
    BackgroundTask InitiateBailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const SetID setID) const override;
    BackgroundTask InitiateOutbailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Amount amount,
        const std::string& message,
        const SetID setID) const override;
    BackgroundTask InitiateRequestConnection(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const proto::ConnectionInfoType& type,
        const SetID setID) const override;
    BackgroundTask InitiateStoreSecret(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const proto::SecretType& type,
        const std::string& primary,
        const std::string& secondary,
        const SetID setID) const override;
    const Identifier& IntroductionServer() const override;
    BackgroundTask IssueUnitDefinition(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& unitID,
        const std::string& label) const override;
    BackgroundTask MessageContact(
        const Identifier& senderNymID,
        const Identifier& contactID,
        const std::string& message,
        const SetID setID) const override;
    std::pair<ThreadStatus, MessageID> MessageStatus(
        const TaskID taskID) const override;
    BackgroundTask NotifyBailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Identifier& requestID,
        const std::string& txid,
        const Amount amount,
        const SetID setID) const override;
    BackgroundTask PayContact(
        const Identifier& senderNymID,
        const Identifier& contactID,
        std::shared_ptr<const OTPayment> payment) const override;
#if OT_CASH
    BackgroundTask PayContactCash(
        const Identifier& senderNymID,
        const Identifier& contactID,
        const Identifier& workflowID) const override;
#endif  // OT_CASH
    BackgroundTask ProcessInbox(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID) const override;
    BackgroundTask PublishServerContract(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& contractID) const override;
    void Refresh() const override;
    std::uint64_t RefreshCount() const override;
    BackgroundTask RegisterAccount(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& unitID,
        const std::string& label) const override;
    BackgroundTask RegisterNym(
        const Identifier& localNymID,
        const Identifier& serverID,
        const bool resync) const override;
    BackgroundTask RegisterNymPublic(
        const Identifier& nymID,
        const Identifier& server,
        const bool setContactData,
        const bool forcePrimary,
        const bool resync) const override;
    OTIdentifier SetIntroductionServer(
        const ServerContract& contract) const override;
    BackgroundTask SendCheque(
        const Identifier& localNymID,
        const Identifier& sourceAccountID,
        const Identifier& recipientContactID,
        const Amount value,
        const std::string& memo,
        const Time validFrom,
        const Time validTo) const override;
    BackgroundTask SendExternalTransfer(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& sourceAccountID,
        const Identifier& targetAccountID,
        const Amount value,
        const std::string& memo) const override;
    BackgroundTask SendTransfer(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& sourceAccountID,
        const Identifier& targetAccountID,
        const Amount value,
        const std::string& memo) const override;
    void StartIntroductionServer(const Identifier& localNymID) const override;
    ThreadStatus Status(const TaskID taskID) const override;
#if OT_CASH
    BackgroundTask WithdrawCash(
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const Identifier& account,
        const Amount value) const override;
#endif  // OT_CASH

    ~OTX();

private:
    static const std::string DEFAULT_INTRODUCTION_SERVER;

    friend opentxs::Factory;

    using TaskStatusMap =
        std::map<TaskID, std::pair<ThreadStatus, std::promise<Result>>>;
    /** ContextID: localNymID, serverID */
    using ContextID = std::pair<OTIdentifier, OTIdentifier>;

    struct OperationQueue {
        using RefreshTask = std::pair<int, std::promise<void>>;
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

        std::unique_ptr<api::client::internal::Operation> op_;
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

        Lock lock() const { return Lock{lock_, std::defer_lock}; }
        int counter() const { return counter_.load(); }

        std::future<void> add_task(const Lock& lock);
        void check_thread(Thread task, std::unique_ptr<std::thread>& thread);
        std::future<void> check_future(
            Thread task,
            std::unique_ptr<std::thread>& thread);
        void cleanup(Lock& lock);
        template <typename T>
        T& get_param()
        {
            throw;
        }
        template <typename T>
        UniqueQueue<T>& get_task()
        {
            throw;
        }
        void increment_counter(const bool missing, Lock& lock, bool& run);
        template <typename T>
        void run_task(
            UniqueQueue<T>& queue,
            std::function<
                bool(const TaskID, api::client::internal::Operation&, const T&)>
                func);
        template <typename T>
        void run_task(
            std::function<
                bool(const TaskID, api::client::internal::Operation&, const T&)>
                func);

        OperationQueue(
            const Flag& running,
            const api::client::Manager& api,
            const ContextID& id);
        OperationQueue() = delete;

    private:
        const Flag& running_;
        Params param_;
        TaskID task_id_{};
        std::atomic<int> counter_;
        mutable std::mutex lock_;
        bool thread_;
        bool continue_;
        std::vector<RefreshTask> tasks_;

        void check_thread(
            const Lock& lock,
            Thread task,
            std::unique_ptr<std::thread>& thread);
    };

    enum class TaskDone : int { no, yes, retry };

    TaskDone task_done(bool done) const
    {
        return done ? TaskDone::yes : TaskDone::no;
    }

    ContextLockCallback lock_callback_;
    const Flag& running_;
    const api::client::Manager& client_;
    OTClient& ot_client_;
    mutable std::mutex introduction_server_lock_{};
    mutable std::mutex nym_fetch_lock_{};
    mutable std::mutex task_status_lock_{};
    mutable std::atomic<std::uint64_t> refresh_counter_{0};
    mutable std::map<ContextID, OperationQueue> operations_;
    mutable std::map<OTIdentifier, UniqueQueue<OTNymID>> server_nym_fetch_;
    UniqueQueue<CheckNymTask> missing_nyms_;
    UniqueQueue<OTIdentifier> missing_servers_;
    mutable std::map<ContextID, std::unique_ptr<std::thread>> state_machines_;
    mutable std::unique_ptr<OTIdentifier> introduction_server_id_;
    mutable TaskStatusMap task_status_;
    mutable std::map<TaskID, MessageID> task_message_id_;
    OTZMQListenCallback account_subscriber_callback_;
    OTZMQSubscribeSocket account_subscriber_;
    OTZMQListenCallback notification_listener_callback_;
    OTZMQPullSocket notification_listener_;
    OTZMQPublishSocket task_finished_;
    mutable OTFlag auto_process_inbox_;
    mutable std::atomic<TaskID> next_task_id_;

    static Result error_result();
    static BackgroundTask error_task();

    BackgroundTask add_task(const TaskID taskID, const ThreadStatus status)
        const;
    void associate_message_id(const Identifier& messageID, const TaskID taskID)
        const;
    Depositability can_deposit(
        const OTPayment& payment,
        const Identifier& recipient,
        const Identifier& accountIDHint,
        OTIdentifier& depositServer,
        OTIdentifier& depositAccount) const;
    Messagability can_message(
        const Identifier& senderID,
        const Identifier& recipientID,
        OTNymID& recipientNymID,
        OTIdentifier& serverID) const;
    void check_nym_revision(const ServerContext& context, OperationQueue& queue)
        const;
    bool check_registration(
        const Identifier& nymID,
        const Identifier& serverID,
        api::client::internal::Operation& op,
        std::shared_ptr<const ServerContext>& context) const;
    bool check_server_contract(const Identifier& serverID) const;
    bool check_server_name(OperationQueue& queue, const ServerContext& context)
        const;
    bool deposit_cheque(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const DepositPaymentTask& task) const;
    bool download_contract(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const Identifier& contractID) const;
#if OT_CASH
    bool download_mint(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const DownloadMintTask& task) const;
#endif
    bool download_nym(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const identifier::Nym& targetNymID) const;
    bool download_nymbox(
        const TaskID taskID,
        api::client::internal::Operation& op) const;
    bool extract_payment_data(
        const OTPayment& payment,
        OTIdentifier& nymID,
        OTIdentifier& serverID,
        identifier::UnitDefinition& unitID) const;
    bool find_nym(
        api::client::internal::Operation& op,
        const identifier::Nym& targetNymID) const;
    bool find_server(
        api::client::internal::Operation& op,
        const Identifier& targetNymID) const;
    bool finish_task(const TaskID taskID, const bool success, Result&& result)
        const;
    bool get_admin(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const OTPassword& password) const;
    std::future<void> get_future(const ContextID& id) const;
    OTIdentifier get_introduction_server(const Lock& lock) const;
    UniqueQueue<OTNymID>& get_nym_fetch(const Identifier& serverID) const;
    OperationQueue& get_operations(const ContextID& id) const;
    OperationQueue& get_task(const ContextID& id) const;
    bool get_transaction_numbers(
        const TaskID taskID,
        api::client::internal::Operation& op) const;
    OTIdentifier import_default_introduction_server(const Lock& lock) const;
    bool initiate_peer_reply(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const PeerReplyTask& task) const;
    bool initiate_peer_request(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const PeerRequestTask& task) const;
    bool issue_unit_definition(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const IssueUnitDefinitionTask& task) const;
    void load_introduction_server(const Lock& lock) const;
    bool message_nym(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const MessageTask& task) const;
    TaskID next_task_id() const { return ++next_task_id_; }
    bool pay_nym(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const PaymentTask& task) const;
#if OT_CASH
    bool pay_nym_cash(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const PayCashTask& Task) const;
#endif  // OT_CASH
    void process_account(
        const opentxs::network::zeromq::Message& message) const;
    bool process_inbox(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const Identifier& accountID) const;
    void process_notification(
        const opentxs::network::zeromq::Message& message) const;
    bool publish_server_contract(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const identifier::Server& serverID) const;
    bool publish_server_registration(
        const Identifier& nymID,
        const Identifier& serverID,
        const bool forcePrimary) const;
    bool queue_cheque_deposit(const Identifier& nymID, const Cheque& cheque)
        const;
    void refresh_accounts() const;
    void refresh_contacts() const;
    bool register_account(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const RegisterAccountTask& task) const;
    bool register_nym(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const bool resync) const;
    BackgroundTask schedule_download_nymbox(
        const Identifier& localNymID,
        const Identifier& serverID) const;
    BackgroundTask schedule_register_account(
        const Identifier& localNymID,
        const Identifier& serverID,
        const identifier::UnitDefinition& unitID,
        const std::string& label) const;
    bool send_transfer(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const SendTransferTask& task) const;
    void set_contact(const Identifier& nymID, const Identifier& serverID) const;
    OTIdentifier set_introduction_server(
        const Lock& lock,
        const ServerContract& contract) const;
    BackgroundTask start_task(const TaskID taskID, bool success) const;
    void state_machine(const ContextID id, OperationQueue& queue) const;
    ThreadStatus status(const Lock& lock, const TaskID taskID) const;
    void update_task(
        const TaskID taskID,
        const ThreadStatus status,
        Result&& result) const;
    void start_introduction_server(const Identifier& nymID) const;
    Depositability valid_account(
        const OTPayment& payment,
        const Identifier& recipient,
        const Identifier& serverID,
        const Identifier& unitID,
        const Identifier& accountIDHint,
        OTIdentifier& depositAccount) const;
    bool valid_context(const Identifier& nymID, const Identifier& serverID)
        const;
    Depositability valid_recipient(
        const OTPayment& payment,
        const Identifier& specifiedNymID,
        const Identifier& recipient) const;
#if OT_CASH
    bool withdraw_cash(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const WithdrawCashTask& task) const;
#endif  // OT_CASH
    TaskDone write_and_send_cheque(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const SendChequeTask& task) const;

    OTX(const Flag& running,
        const api::client::Manager& client,
        OTClient& otclient,
        const ContextLockCallback& lockCallback);
    OTX() = delete;
    OTX(const OTX&) = delete;
    OTX(OTX&&) = delete;
    OTX& operator=(const OTX&) = delete;
    OTX& operator=(OTX&&) = delete;
};

template <>
CheckNymTask& OTX::OperationQueue::get_param()
{
    return param_.check_nym_;
}
template <>
DepositPaymentTask& OTX::OperationQueue::get_param()
{
    return param_.deposit_payment_;
}
template <>
DownloadContractTask& OTX::OperationQueue::get_param()
{
    return param_.download_contract_;
}
template <>
DownloadMintTask& OTX::OperationQueue::get_param()
{
    return param_.download_mint_;
}
template <>
DownloadNymboxTask& OTX::OperationQueue::get_param()
{
    return param_.download_nymbox_;
}
template <>
GetTransactionNumbersTask& OTX::OperationQueue::get_param()
{
    return param_.get_transaction_numbers_;
}
template <>
IssueUnitDefinitionTask& OTX::OperationQueue::get_param()
{
    return param_.issue_unit_definition_;
}
template <>
RegisterAccountTask& OTX::OperationQueue::get_param()
{
    return param_.register_account_;
}
template <>
RegisterNymTask& OTX::OperationQueue::get_param()
{
    return param_.register_nym_;
}
template <>
MessageTask& OTX::OperationQueue::get_param()
{
    return param_.send_message_;
}
template <>
PaymentTask& OTX::OperationQueue::get_param()
{
    return param_.send_payment_;
}
#if OT_CASH
template <>
PayCashTask& OTX::OperationQueue::get_param()
{
    return param_.send_cash_;
}
template <>
WithdrawCashTask& OTX::OperationQueue::get_param()
{
    return param_.withdraw_cash_;
}
#endif
template <>
SendTransferTask& OTX::OperationQueue::get_param()
{
    return param_.send_transfer_;
}
template <>
PublishServerContractTask& OTX::OperationQueue::get_param()
{
    return param_.publish_server_contract_;
}
template <>
SendChequeTask& OTX::OperationQueue::get_param()
{
    return param_.send_cheque_;
}
template <>
PeerReplyTask& OTX::OperationQueue::get_param()
{
    return param_.peer_reply_;
}
template <>
PeerRequestTask& OTX::OperationQueue::get_param()
{
    return param_.peer_request_;
}
template <>
UniqueQueue<CheckNymTask>& OTX::OperationQueue::get_task()
{
    return check_nym_;
}
template <>
UniqueQueue<DepositPaymentTask>& OTX::OperationQueue::get_task()
{
    return deposit_payment_;
}
template <>
UniqueQueue<DownloadContractTask>& OTX::OperationQueue::get_task()
{
    return download_contract_;
}
template <>
UniqueQueue<DownloadMintTask>& OTX::OperationQueue::get_task()
{
    return download_mint_;
}
template <>
UniqueQueue<DownloadNymboxTask>& OTX::OperationQueue::get_task()
{
    return download_nymbox_;
}
template <>
UniqueQueue<GetTransactionNumbersTask>& OTX::OperationQueue::get_task()
{
    return get_transaction_numbers_;
}
template <>
UniqueQueue<IssueUnitDefinitionTask>& OTX::OperationQueue::get_task()
{
    return issue_unit_definition_;
}
template <>
UniqueQueue<RegisterAccountTask>& OTX::OperationQueue::get_task()
{
    return register_account_;
}
template <>
UniqueQueue<RegisterNymTask>& OTX::OperationQueue::get_task()
{
    return register_nym_;
}
template <>
UniqueQueue<MessageTask>& OTX::OperationQueue::get_task()
{
    return send_message_;
}
template <>
UniqueQueue<PaymentTask>& OTX::OperationQueue::get_task()
{
    return send_payment_;
}
#if OT_CASH
template <>
UniqueQueue<PayCashTask>& OTX::OperationQueue::get_task()
{
    return send_cash_;
}
template <>
UniqueQueue<WithdrawCashTask>& OTX::OperationQueue::get_task()
{
    return withdraw_cash_;
}
#endif
template <>
UniqueQueue<SendTransferTask>& OTX::OperationQueue::get_task()
{
    return send_transfer_;
}
template <>
UniqueQueue<PublishServerContractTask>& OTX::OperationQueue::get_task()
{
    return publish_server_contract_;
}
template <>
UniqueQueue<SendChequeTask>& OTX::OperationQueue::get_task()
{
    return send_cheque_;
}
template <>
UniqueQueue<PeerReplyTask>& OTX::OperationQueue::get_task()
{
    return peer_reply_;
}
template <>
UniqueQueue<PeerRequestTask>& OTX::OperationQueue::get_task()
{
    return peer_request_;
}

}  // namespace opentxs::api::client::implementation
