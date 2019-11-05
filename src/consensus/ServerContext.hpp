// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace zmq = opentxs::network::zeromq;

namespace opentxs::implementation
{
class ServerContext final : virtual public internal::ServerContext,
                            public Context,
                            public opentxs::internal::StateMachine
{
public:
    std::vector<OTIdentifier> Accounts() const final;
    const std::string& AdminPassword() const final;
    bool AdminAttempted() const final;
    bool FinalizeServerCommand(Message& command, const PasswordPrompt& reason)
        const final;
    proto::Context GetContract(const Lock& lock) const final
    {
        return contract(lock);
    }
    bool HaveAdminPassword() const final;
    bool HaveSufficientNumbers(const MessageType reason) const final;
    TransactionNumber Highest() const final;
    bool isAdmin() const final;
#if OT_CASH
    std::shared_ptr<const blind::Purse> Purse(
        const identifier::UnitDefinition& id) const final;
#endif
    std::uint64_t Revision() const final;
    bool ShouldRename(
        const PasswordPrompt& reason,
        const std::string& defaultName = "") const final;
    bool StaleNym() const final;
    std::unique_ptr<Item> Statement(
        const OTTransaction& owner,
        const PasswordPrompt& reason) const final;
    std::unique_ptr<Item> Statement(
        const OTTransaction& owner,
        const TransactionNumbers& adding,
        const PasswordPrompt& reason) const final;
    std::unique_ptr<TransactionStatement> Statement(
        const TransactionNumbers& adding,
        const TransactionNumbers& without,
        const PasswordPrompt& reason) const final;
    proto::ConsensusType Type() const final;
    bool ValidateContext(const Lock& lock, const PasswordPrompt& reason)
        const final
    {
        return validate(lock, reason);
    }
    bool Verify(const TransactionStatement& statement) const final;
    bool VerifyTentativeNumber(const TransactionNumber& number) const final;

    bool AcceptIssuedNumber(const TransactionNumber& number) final;
    bool AcceptIssuedNumbers(const TransactionStatement& statement) final;
    bool AddTentativeNumber(const TransactionNumber& number) final;
    network::ServerConnection& Connection() final;
    std::mutex& GetLock() final { return lock_; }
    std::pair<RequestNumber, std::unique_ptr<Message>> InitializeServerCommand(
        const MessageType type,
        const Armored& payload,
        const Identifier& accountID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = true) final;
    std::pair<RequestNumber, std::unique_ptr<Message>> InitializeServerCommand(
        const MessageType type,
        const identifier::Nym& recipientNymID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false) final;
    std::pair<RequestNumber, std::unique_ptr<Message>> InitializeServerCommand(
        const MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false) final;
    void Join() const final;
#if OT_CASH
    Editor<blind::Purse> mutable_Purse(
        const identifier::UnitDefinition& id,
        const PasswordPrompt& reason) final;
#endif
    OTManagedNumber NextTransactionNumber(const MessageType reason) final;
    NetworkReplyMessage PingNotary(const PasswordPrompt& reason) final;
    bool ProcessNotification(
        const api::client::internal::Manager& client,
        const otx::Reply& notification,
        const PasswordPrompt& reason) final;
    QueueResult Queue(
        const api::client::internal::Manager& client,
        std::shared_ptr<Message> message,
        const PasswordPrompt& reason,
        const ExtraArgs& args) final;
    QueueResult Queue(
        const api::client::internal::Manager& client,
        std::shared_ptr<Message> message,
        std::shared_ptr<Ledger> inbox,
        std::shared_ptr<Ledger> outbox,
        std::set<OTManagedNumber>* numbers,
        const PasswordPrompt& reason,
        const ExtraArgs& args) final;
    QueueResult RefreshNymbox(
        const api::client::internal::Manager& client,
        const PasswordPrompt& reason) final;
    bool RemoveTentativeNumber(const TransactionNumber& number) final;
    void ResetThread() final;
    bool Resync(const proto::Context& serialized) final;
    NetworkReplyMessage SendMessage(
        const api::client::internal::Manager& client,
        const std::set<OTManagedNumber>& pending,
        opentxs::ServerContext& context,
        const Message& message,
        const PasswordPrompt& reason,
        const std::string& label,
        const bool resync) final;
    void SetAdminAttempted() final;
    void SetAdminPassword(const std::string& password) final;
    void SetAdminSuccess() final;
    bool SetHighest(const TransactionNumber& highest) final;
    void SetPush(const bool on) final { enable_otx_push_.store(on); }
    void SetRevision(const std::uint64_t revision) final;
    TransactionNumber UpdateHighest(
        const TransactionNumbers& numbers,
        TransactionNumbers& good,
        TransactionNumbers& bad) final;
    RequestNumber UpdateRequestNumber(const PasswordPrompt& reason) final;
    RequestNumber UpdateRequestNumber(
        bool& sendStatus,
        const PasswordPrompt& reason) final;
    bool UpdateRequestNumber(Message& command, const PasswordPrompt& reason)
        final;
    bool UpdateSignature(const Lock& lock, const PasswordPrompt& reason) final
    {
        return update_signature(lock, reason);
    }

    ~ServerContext() final;

private:
    friend opentxs::Factory;
    using ReplyNoticeOutcome =
        std::pair<RequestNumber, ServerContext::DeliveryResult>;
    using ReplyNoticeOutcomes = std::vector<ReplyNoticeOutcome>;

    enum class Exit : bool { Yes = true, Continue = false };
    enum class UpdateHash : bool { Remote = false, Both = true };
    enum class BoxType : std::int64_t {
        Invalid = -1,
        Nymbox = 0,
        Inbox = 1,
        Outbox = 2
    };
    enum class ActionType : bool { ProcessNymbox = true, Normal = false };
    enum class TransactionAttempt : bool { Accepted = true, Rejected = false };

    static const std::string default_node_name_;
    static const std::set<MessageType> do_not_need_request_number_;

    const network::zeromq::socket::Publish& request_sent_;
    const network::zeromq::socket::Publish& reply_received_;
    // WARNING the lifetime of the object pointed to by this member variable
    // has a shorter lifetime than this ServerContext object. Call Join()
    // on all ServerContext objects before allowing the client api to shut down.
    std::atomic<const api::client::internal::Manager*> client_;
    network::ServerConnection& connection_;
    std::mutex message_lock_{};
    std::string admin_password_{""};
    OTFlag admin_attempted_;
    OTFlag admin_success_;
    std::atomic<std::uint64_t> revision_{0};
    std::atomic<TransactionNumber> highest_transaction_number_{0};
    TransactionNumbers tentative_transaction_numbers_{};
    std::atomic<proto::DeliveryState> state_;
    std::atomic<proto::LastReplyStatus> last_status_;
    std::shared_ptr<opentxs::Message> pending_message_;
    ExtraArgs pending_args_;
    std::promise<DeliveryResult> pending_result_;
    std::atomic<bool> pending_result_set_;
    std::atomic<bool> process_nymbox_;
    std::atomic<bool> enable_otx_push_;
    std::atomic<int> failure_counter_;
    std::shared_ptr<Ledger> inbox_;
    std::shared_ptr<Ledger> outbox_;
    std::set<OTManagedNumber>* numbers_;
    OTZMQPushSocket find_nym_;
    OTZMQPushSocket find_server_;
    OTZMQPushSocket find_unit_definition_;

    static const api::client::internal::Manager& client(
        const api::internal::Core& api);
    static TransactionNumbers extract_numbers(OTTransaction& input);
    static Exit get_item_type(OTTransaction& input, itemType& output);
    static BoxType get_type(const std::int64_t depth);
    static std::unique_ptr<opentxs::Message> instantiate_message(
        const api::internal::Core& api,
        const std::string& serialized);
    static bool need_request_number(const MessageType type);
    static void scan_number_set(
        const TransactionNumbers& input,
        TransactionNumber& highest,
        TransactionNumber& lowest);
    static void validate_number_set(
        const TransactionNumbers& input,
        const TransactionNumber limit,
        TransactionNumbers& good,
        TransactionNumbers& bad);

    bool add_item_to_payment_inbox(
        const TransactionNumber number,
        const std::string& payment,
        const PasswordPrompt& reason) const;
    bool add_item_to_workflow(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Message& transportItem,
        const std::string& item,
        const PasswordPrompt& reason) const;
    bool add_transaction_to_ledger(
        const TransactionNumber number,
        std::shared_ptr<OTTransaction> transaction,
        Ledger& ledger,
        const PasswordPrompt& reason) const;
    const identifier::Nym& client_nym_id(const Lock& lock) const final;
    bool create_instrument_notice_from_peer_object(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Message& message,
        const PeerObject& peerObject,
        const TransactionNumber number,
        const PasswordPrompt& reason) const;
    std::shared_ptr<OTTransaction> extract_box_receipt(
        const String& serialized,
        const identity::Nym& signer,
        const identifier::Nym& owner,
        const TransactionNumber target,
        const PasswordPrompt& reason);
    std::unique_ptr<Ledger> extract_ledger(
        const Armored& armored,
        const Identifier& accountID,
        const identity::Nym& signer,
        const PasswordPrompt& reason) const;
    std::unique_ptr<Message> extract_message(
        const Armored& armored,
        const identity::Nym& signer,
        const PasswordPrompt& reason) const;
    std::unique_ptr<Item> extract_original_item(
        const itemType type,
        OTTransaction& response) const;
    std::unique_ptr<Item> extract_original_item(
        const Item& response,
        const PasswordPrompt& reason) const;
    std::shared_ptr<OTPayment> extract_payment_instrument_from_notice(
        const api::internal::Core& api,
        const identity::Nym& theNym,
        std::shared_ptr<OTTransaction> pTransaction,
        const PasswordPrompt& reason);
    std::unique_ptr<Item> extract_transfer(
        const OTTransaction& receipt,
        const PasswordPrompt& reason) const;
    std::unique_ptr<Item> extract_transfer_pending(
        const OTTransaction& receipt,
        const PasswordPrompt& reason) const;
    std::unique_ptr<Item> extract_transfer_receipt(
        const OTTransaction& receipt,
        const PasswordPrompt& reason) const;
    bool finalize_server_command(Message& command, const PasswordPrompt& reason)
        const;
    std::unique_ptr<TransactionStatement> generate_statement(
        const Lock& lock,
        const TransactionNumbers& adding,
        const TransactionNumbers& without) const;
    std::shared_ptr<OTPayment> get_instrument(
        const api::internal::Core& api,
        const identity::Nym& theNym,
        Ledger& ledger,
        std::shared_ptr<OTTransaction> pTransaction,
        const PasswordPrompt& reason);
    std::shared_ptr<OTPayment> get_instrument_by_receipt_id(
        const api::internal::Core& api,
        const identity::Nym& theNym,
        const TransactionNumber lReceiptId,
        Ledger& ledger,
        const PasswordPrompt& reason);
    bool init_new_account(
        const Identifier& accountID,
        const PasswordPrompt& reason);
    std::unique_ptr<Message> initialize_server_command(
        const MessageType type) const;
    void initialize_server_command(const MessageType type, Message& output)
        const;
    bool is_internal_transfer(const Item& item) const;
    std::unique_ptr<Ledger> load_account_inbox(
        const Identifier& accountID,
        const PasswordPrompt& reason) const;
    std::unique_ptr<Ledger> load_or_create_account_recordbox(
        const Identifier& accountID,
        const PasswordPrompt& reason) const;
    std::unique_ptr<Ledger> load_or_create_payment_inbox(
        const PasswordPrompt& reason) const;
    void process_accept_pending_reply(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Identifier& accountID,
        const Item& acceptItemReceipt,
        const Message& reply,
        const PasswordPrompt& reason) const;
#if OT_CASH
    bool process_incoming_cash(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const TransactionNumber number,
        const PeerObject& incoming,
        const Message& message,
        const PasswordPrompt& reason) const;
    void process_incoming_cash_withdrawal(
        const Item& item,
        const PasswordPrompt& reason) const;
#endif
    void process_incoming_instrument(
        const std::shared_ptr<OTTransaction> receipt,
        const PasswordPrompt& reason) const;
    void process_incoming_message(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const OTTransaction& receipt,
        const PasswordPrompt& reason) const;
    std::string type() const final { return "server"; }
    void verify_blank(
        const Lock& lock,
        OTTransaction& blank,
        TransactionNumbers& output) const;
    void verify_success(
        const Lock& lock,
        OTTransaction& blank,
        TransactionNumbers& output) const;

    bool accept_entire_nymbox(
        const Lock& lock,
        const api::client::internal::Manager& client,
        Ledger& theNymbox,
        Message& output,
        ReplyNoticeOutcomes& notices,
        std::size_t& alreadySeenNotices,
        const PasswordPrompt& reason);
    bool accept_issued_number(
        const Lock& lock,
        const TransactionNumber& number);
    bool accept_issued_number(
        const Lock& lock,
        const TransactionStatement& statement);
    void accept_numbers(
        const Lock& lock,
        OTTransaction& transaction,
        OTTransaction& replyTransaction);
    bool add_tentative_number(
        const Lock& lock,
        const TransactionNumber& number);
    NetworkReplyMessage attempt_delivery(
        const Lock& contextLock,
        const Lock& messageLock,
        const api::client::internal::Manager& client,
        Message& message,
        const PasswordPrompt& reason);
    bool harvest_unused(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const PasswordPrompt& reason);
    void init_sockets();
    RequestNumber initialize_server_command(
        const Lock& lock,
        const MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments,
        const bool withNymboxHash,
        Message& output);
    std::pair<RequestNumber, std::unique_ptr<Message>>
    initialize_server_command(
        const Lock& lock,
        const MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments,
        const bool withNymboxHash);
    const Item& make_accept_item(
        const PasswordPrompt& reason,
        const itemType type,
        const OTTransaction& input,
        OTTransaction& acceptTransaction,
        const TransactionNumbers& accept = {});
    void need_box_items(
        const api::client::internal::Manager& client,
        const PasswordPrompt& reason);
    void need_nymbox(
        const api::client::internal::Manager& client,
        const PasswordPrompt& reason);
    void need_process_nymbox(
        const api::client::internal::Manager& client,
        const PasswordPrompt& reason);
    OTManagedNumber next_transaction_number(
        const Lock& lock,
        const MessageType reason);
    void pending_send(
        const api::client::internal::Manager& client,
        const PasswordPrompt& reason);
    void process_accept_basket_receipt_reply(
        const Lock& lock,
        const OTTransaction& inboxTransaction);
    void process_accept_cron_receipt_reply(
        const Lock& lock,
        const Identifier& accountID,
        OTTransaction& inboxTransaction,
        const PasswordPrompt& reason);
    void process_accept_final_receipt_reply(
        const Lock& lock,
        const OTTransaction& inboxTransaction);
    void process_accept_item_receipt_reply(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Identifier& accountID,
        const Message& reply,
        const OTTransaction& inboxTransaction,
        const PasswordPrompt& reason);
    bool process_account_data(
        const Lock& lock,
        const Identifier& accountID,
        const String& account,
        const Identifier& inboxHash,
        const String& inbox,
        const Identifier& outboxHash,
        const String& outbox,
        const PasswordPrompt& reason);
    bool process_account_push(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const proto::OTXPush& push,
        const PasswordPrompt& reason);
    bool process_box_item(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Identifier& accountID,
        const proto::OTXPush& push,
        const PasswordPrompt& reason);
    bool process_check_nym_response(
        const Lock& lock,
        const PasswordPrompt& reason,
        const api::client::internal::Manager& client,
        const Message& reply);
    bool process_get_account_data(
        const Lock& lock,
        const Message& reply,
        const PasswordPrompt& reason);
    bool process_get_box_receipt_response(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Message& reply,
        const PasswordPrompt& reason);
    bool process_get_box_receipt_response(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Identifier& accountID,
        const std::shared_ptr<OTTransaction> receipt,
        const String& serialized,
        const BoxType type,
        const PasswordPrompt& reason);
    bool process_get_market_list_response(
        const Lock& lock,
        const Message& reply);
    bool process_get_market_offers_response(
        const Lock& lock,
        const Message& reply);
    bool process_get_market_recent_trades_response(
        const Lock& lock,
        const Message& reply);
#if OT_CASH
    bool process_get_mint_response(
        const Lock& lock,
        const Message& reply,
        const PasswordPrompt& reason);
#endif
    bool process_get_nym_market_offers_response(
        const Lock& lock,
        const Message& reply);
    bool process_get_unit_definition_response(
        const Lock& lock,
        const PasswordPrompt& reason,
        const Message& reply);
    bool process_issue_unit_definition_response(
        const Lock& lock,
        const Message& reply,
        const PasswordPrompt& reason);
    bool process_get_nymbox_response(
        const Lock& lock,
        const Message& reply,
        const PasswordPrompt& reason);
    bool process_notarize_transaction_response(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Message& reply,
        const PasswordPrompt& reason);
    bool process_process_box_response(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Message& reply,
        const BoxType inbox,
        const Identifier& accountID,
        const PasswordPrompt& reason);
    bool process_process_inbox_response(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Message& reply,
        Ledger& ledger,
        Ledger& responseLedger,
        std::shared_ptr<OTTransaction>& transaction,
        std::shared_ptr<OTTransaction>& replyTransaction,
        const PasswordPrompt& reason);
    bool process_process_nymbox_response(
        const Lock& lock,
        const Message& reply,
        Ledger& ledger,
        Ledger& responseLedger,
        std::shared_ptr<OTTransaction>& transaction,
        std::shared_ptr<OTTransaction>& replyTransaction,
        const PasswordPrompt& reason);
    bool process_register_account_response(
        const Lock& lock,
        const Message& reply,
        const PasswordPrompt& reason);
    bool process_request_admin_response(const Lock& lock, const Message& reply);
    bool process_register_nym_response(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Message& reply,
        const PasswordPrompt& reason);
    bool process_reply(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const std::set<OTManagedNumber>& managed,
        const Message& reply,
        const PasswordPrompt& reason);
    void process_response_transaction(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Message& reply,
        OTTransaction& responseTransaction,
        const PasswordPrompt& reason);
    void process_response_transaction_cancel(
        const Lock& lock,
        const Message& reply,
        const itemType type,
        OTTransaction& response,
        const PasswordPrompt& reason);
#if OT_CASH
    void process_response_transaction_cash_deposit(
        Item& replyItem,
        const PasswordPrompt& reason);
#endif
    void process_response_transaction_cheque_deposit(
        const api::client::internal::Manager& client,
        const Identifier& accountID,
        const Message* reply,
        const Item& replyItem,
        const PasswordPrompt& reason);
    void process_response_transaction_cron(
        const Lock& lock,
        const Message& reply,
        const itemType type,
        OTTransaction& response,
        const PasswordPrompt& reason);
    void process_response_transaction_deposit(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Message& reply,
        const itemType type,
        OTTransaction& response,
        const PasswordPrompt& reason);
    void process_response_transaction_exchange_basket(
        const Lock& lock,
        const Message& reply,
        const itemType type,
        OTTransaction& response,
        const PasswordPrompt& reason);
    void process_response_transaction_pay_dividend(
        const Lock& lock,
        const Message& reply,
        const itemType type,
        OTTransaction& response);
    void process_response_transaction_transfer(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Message& reply,
        const itemType type,
        OTTransaction& response,
        const PasswordPrompt& reason);
#if OT_CASH
    void process_response_transaction_withdrawal(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Message& reply,
        const itemType type,
        OTTransaction& response,
        const PasswordPrompt& reason);
#endif
    bool process_unregister_nym_response(
        const Lock& lock,
        const Message& reply,
        const PasswordPrompt& reason);
    bool process_unregister_account_response(
        const Lock& lock,
        const Message& reply,
        const PasswordPrompt& reason);
    void process_unseen_reply(
        const Lock& lock,
        const api::client::internal::Manager& client,
        const Item& input,
        ReplyNoticeOutcomes& notices,
        const PasswordPrompt& reason);
    using implementation::Context::remove_acknowledged_number;
    bool remove_acknowledged_number(const Lock& lock, const Message& reply);
    bool remove_nymbox_item(
        const Lock& lock,
        const Item& replyItem,
        Ledger& nymbox,
        OTTransaction& transaction,
        const PasswordPrompt& reason);
    bool remove_tentative_number(
        const Lock& lock,
        const TransactionNumber& number);
    void resolve_queue(
        const Lock& contextLock,
        DeliveryResult&& result,
        const PasswordPrompt& reason,
        const proto::DeliveryState state = proto::DELIVERTYSTATE_ERROR);
    bool resync(const Lock& lock, const proto::Context& serialized);
    using implementation::Context::serialize;
    proto::Context serialize(const Lock& lock) const final;
    const identifier::Nym& server_nym_id(const Lock& lock) const final;
    QueueResult start(
        const Lock& decisionLock,
        const PasswordPrompt& reason,
        const api::client::internal::Manager& client,
        std::shared_ptr<Message> message,
        const ExtraArgs& args,
        const proto::DeliveryState state = proto::DELIVERTYSTATE_PENDINGSEND,
        const ActionType type = ActionType::Normal,
        std::shared_ptr<Ledger> inbox = {},
        std::shared_ptr<Ledger> outbox = {},
        std::set<OTManagedNumber>* numbers = nullptr);
    bool state_machine() noexcept;
    std::unique_ptr<Item> statement(
        const Lock& lock,
        const OTTransaction& owner,
        const TransactionNumbers& adding,
        const PasswordPrompt& reason) const;
    TransactionNumber update_highest(
        const Lock& lock,
        const TransactionNumbers& numbers,
        TransactionNumbers& good,
        TransactionNumbers& bad);
    bool update_nymbox_hash(
        const Lock& lock,
        const Message& reply,
        const UpdateHash which = UpdateHash::Remote);
    OTIdentifier update_remote_hash(const Lock& lock, const Message& reply);
    RequestNumber update_request_number(
        const PasswordPrompt& reason,
        const Lock& contextLock,
        const Lock& messageLock,
        bool& sendStatus);
    bool update_request_number(
        const PasswordPrompt& reason,
        const Lock& lock,
        Message& command);
    void update_state(
        const Lock& contextLock,
        const proto::DeliveryState state,
        const PasswordPrompt& reason,
        const proto::LastReplyStatus status = proto::LASTREPLYSTATUS_INVALID);
    bool verify_tentative_number(
        const Lock& lock,
        const TransactionNumber& number) const;

    ServerContext(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& requestSent,
        const network::zeromq::socket::Publish& replyReceived,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Server& server,
        network::ServerConnection& connection);
    ServerContext(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& requestSent,
        const network::zeromq::socket::Publish& replyReceived,
        const proto::Context& serialized,
        const Nym_p& local,
        const Nym_p& remote,
        network::ServerConnection& connection);
    ServerContext() = delete;
    ServerContext(const ServerContext&) = delete;
    ServerContext(ServerContext&&) = delete;
    ServerContext& operator=(const ServerContext&) = delete;
    ServerContext& operator=(ServerContext&&) = delete;
};
}  // namespace opentxs::implementation
