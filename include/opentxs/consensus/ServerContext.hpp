// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CONSENSUS_SERVERCONTEXT_HPP
#define OPENTXS_CONSENSUS_SERVERCONTEXT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/consensus/Context.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <atomic>
#include <set>
#include <tuple>

namespace opentxs
{
class ServerContext : public Context
{
public:
    class ManagedNumber
    {
    public:
        ManagedNumber(ManagedNumber&& rhs);

        operator TransactionNumber() const;

        void SetSuccess(const bool value = true) const;
        bool Valid() const;

        ~ManagedNumber();

    private:
        friend ServerContext;

        ServerContext& context_;
        const TransactionNumber number_;
        mutable OTFlag success_;
        bool managed_{true};

        ManagedNumber(const TransactionNumber number, ServerContext& context);
        ManagedNumber() = delete;
        ManagedNumber(const ManagedNumber&) = delete;
        ManagedNumber& operator=(const ManagedNumber&) = delete;
        ManagedNumber& operator=(ManagedNumber&&) = delete;
    };

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

    const std::string& AdminPassword() const;
    bool AdminAttempted() const;
    bool FinalizeServerCommand(Message& command) const;
    bool HaveAdminPassword() const;
    TransactionNumber Highest() const;
    bool isAdmin() const;
    std::uint64_t Revision() const;
    bool ShouldRename(const std::string& defaultName = "") const;
    bool StaleNym() const;
    std::unique_ptr<Item> Statement(const OTTransaction& owner) const;
    std::unique_ptr<Item> Statement(
        const OTTransaction& owner,
        const std::set<TransactionNumber>& adding) const;
    std::unique_ptr<TransactionStatement> Statement(
        const std::set<TransactionNumber>& adding,
        const std::set<TransactionNumber>& without) const;
    bool Verify(const TransactionStatement& statement) const;
    bool VerifyTentativeNumber(const TransactionNumber& number) const;

    bool AcceptIssuedNumber(const TransactionNumber& number);
    bool AcceptIssuedNumbers(const TransactionStatement& statement);
    bool AddTentativeNumber(const TransactionNumber& number);
    network::ServerConnection& Connection();
    std::pair<RequestNumber, std::unique_ptr<Message>> InitializeServerCommand(
        const MessageType type,
        const Armored& payload,
        const Identifier& accountID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = true);
    std::pair<RequestNumber, std::unique_ptr<Message>> InitializeServerCommand(
        const MessageType type,
        const Identifier& recipientNymID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false);
    std::pair<RequestNumber, std::unique_ptr<Message>> InitializeServerCommand(
        const MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false);
    ManagedNumber NextTransactionNumber(const MessageType reason);
    NetworkReplyMessage PingNotary();
    bool RemoveTentativeNumber(const TransactionNumber& number);
    bool Resync(const proto::Context& serialized);
    void SetAdminAttempted();
    void SetAdminPassword(const std::string& password);
    void SetAdminSuccess();
    bool SetHighest(const TransactionNumber& highest);
    void SetRevision(const std::uint64_t revision);
    TransactionNumber UpdateHighest(
        const std::set<TransactionNumber>& numbers,
        std::set<TransactionNumber>& good,
        std::set<TransactionNumber>& bad);
    RequestNumber UpdateRequestNumber();
    RequestNumber UpdateRequestNumber(bool& sendStatus);

    proto::ConsensusType Type() const override;

    ~ServerContext() = default;

private:
    typedef Context ot_super;

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
    using ot_super::remove_acknowledged_number;
    bool remove_acknowledged_number(const Lock& lock, const Message& reply);
    bool remove_tentative_number(
        const Lock& lock,
        const TransactionNumber& number);
    using ot_super::serialize;
    proto::Context serialize(const Lock& lock) const override;
    const Identifier& server_nym_id(const Lock& lock) const override;
    TransactionNumber update_highest(
        const Lock& lock,
        const std::set<TransactionNumber>& numbers,
        std::set<TransactionNumber>& good,
        std::set<TransactionNumber>& bad);
    OTIdentifier update_remote_hash(const Lock& lock, const Message& reply);

    ServerContext() = delete;
    ServerContext(const ServerContext&) = delete;
    ServerContext(ServerContext&&) = delete;
    ServerContext& operator=(const ServerContext&) = delete;
    ServerContext& operator=(ServerContext&&) = delete;
};
}  // namespace opentxs

#endif
