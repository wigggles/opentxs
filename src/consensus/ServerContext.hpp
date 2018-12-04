// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::implementation
{
class ServerContext final : virtual public internal::ServerContext,
                            public Context
{
public:
    const std::string& AdminPassword() const override;
    bool AdminAttempted() const override;
    bool FinalizeServerCommand(Message& command) const override;
    proto::Context GetContract(const Lock& lock) const override
    {
        return contract(lock);
    }
    bool HaveAdminPassword() const override;
    TransactionNumber Highest() const override;
    bool isAdmin() const override;
    std::uint64_t Revision() const override;
    bool ShouldRename(const std::string& defaultName = "") const override;
    bool StaleNym() const override;
    std::unique_ptr<Item> Statement(const OTTransaction& owner) const override;
    std::unique_ptr<Item> Statement(
        const OTTransaction& owner,
        const std::set<TransactionNumber>& adding) const override;
    std::unique_ptr<TransactionStatement> Statement(
        const std::set<TransactionNumber>& adding,
        const std::set<TransactionNumber>& without) const override;
    proto::ConsensusType Type() const override;
    bool ValidateContext(const Lock& lock) const override
    {
        return validate(lock);
    }
    bool Verify(const TransactionStatement& statement) const override;
    bool VerifyTentativeNumber(const TransactionNumber& number) const override;

    bool AcceptIssuedNumber(const TransactionNumber& number) override;
    bool AcceptIssuedNumbers(const TransactionStatement& statement) override;
    bool AddTentativeNumber(const TransactionNumber& number) override;
    network::ServerConnection& Connection() override;
    std::mutex& GetLock() override { return lock_; }
    std::pair<RequestNumber, std::unique_ptr<Message>> InitializeServerCommand(
        const MessageType type,
        const Armored& payload,
        const Identifier& accountID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = true) override;
    std::pair<RequestNumber, std::unique_ptr<Message>> InitializeServerCommand(
        const MessageType type,
        const Identifier& recipientNymID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false) override;
    std::pair<RequestNumber, std::unique_ptr<Message>> InitializeServerCommand(
        const MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false) override;
    OTManagedNumber NextTransactionNumber(const MessageType reason) override;
    NetworkReplyMessage PingNotary() override;
    bool RemoveTentativeNumber(const TransactionNumber& number) override;
    bool Resync(const proto::Context& serialized) override;
    void SetAdminAttempted() override;
    void SetAdminPassword(const std::string& password) override;
    void SetAdminSuccess() override;
    bool SetHighest(const TransactionNumber& highest) override;
    void SetRevision(const std::uint64_t revision) override;
    TransactionNumber UpdateHighest(
        const std::set<TransactionNumber>& numbers,
        std::set<TransactionNumber>& good,
        std::set<TransactionNumber>& bad) override;
    RequestNumber UpdateRequestNumber() override;
    RequestNumber UpdateRequestNumber(bool& sendStatus) override;
    bool UpdateSignature(const Lock& lock) override
    {
        return update_signature(lock);
    }

    ~ServerContext() override = default;

private:
    friend opentxs::Factory;

    static const std::string default_node_name_;

    network::ServerConnection& connection_;
    std::mutex message_lock_{};
    std::string admin_password_{""};
    OTFlag admin_attempted_;
    OTFlag admin_success_;
    std::atomic<std::uint64_t> revision_{0};
    std::atomic<TransactionNumber> highest_transaction_number_{0};
    std::set<TransactionNumber> tentative_transaction_numbers_{};

    static void scan_number_set(
        const std::set<TransactionNumber>& input,
        TransactionNumber& highest,
        TransactionNumber& lowest);
    static void validate_number_set(
        const std::set<TransactionNumber>& input,
        const TransactionNumber limit,
        std::set<TransactionNumber>& good,
        std::set<TransactionNumber>& bad);

    const Identifier& client_nym_id(const Lock& lock) const override;
    bool finalize_server_command(Message& command) const;
    std::unique_ptr<TransactionStatement> generate_statement(
        const Lock& lock,
        const std::set<TransactionNumber>& adding,
        const std::set<TransactionNumber>& without) const;
    std::unique_ptr<Message> initialize_server_command(
        const MessageType type) const;
    std::pair<RequestNumber, std::unique_ptr<Message>>
    initialize_server_command(
        const Lock& lock,
        const MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments,
        const bool withNymboxHash);
    using implementation::Context::remove_acknowledged_number;
    bool remove_acknowledged_number(const Lock& lock, const Message& reply);
    bool remove_tentative_number(
        const Lock& lock,
        const TransactionNumber& number);
    using implementation::Context::serialize;
    proto::Context serialize(const Lock& lock) const override;
    const Identifier& server_nym_id(const Lock& lock) const override;
    TransactionNumber update_highest(
        const Lock& lock,
        const std::set<TransactionNumber>& numbers,
        std::set<TransactionNumber>& good,
        std::set<TransactionNumber>& bad);
    OTIdentifier update_remote_hash(const Lock& lock, const Message& reply);

    ServerContext(
        const api::Core& api,
        const ConstNym& local,
        const ConstNym& remote,
        const Identifier& server,
        network::ServerConnection& connection);
    ServerContext(
        const api::Core& api,
        const proto::Context& serialized,
        const ConstNym& local,
        const ConstNym& remote,
        network::ServerConnection& connection);
    ServerContext() = delete;
    ServerContext(const ServerContext&) = delete;
    ServerContext(ServerContext&&) = delete;
    ServerContext& operator=(const ServerContext&) = delete;
    ServerContext& operator=(ServerContext&&) = delete;
};
}  // namespace opentxs::implementation
