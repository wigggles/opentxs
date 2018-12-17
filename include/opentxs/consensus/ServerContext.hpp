// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CONSENSUS_SERVERCONTEXT_HPP
#define OPENTXS_CONSENSUS_SERVERCONTEXT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/consensus/Context.hpp"

#include <tuple>

namespace opentxs
{
class ServerContext : virtual public Context
{
public:
    EXPORT virtual const std::string& AdminPassword() const = 0;
    EXPORT virtual bool AdminAttempted() const = 0;
    EXPORT virtual bool FinalizeServerCommand(Message& command) const = 0;
    EXPORT virtual bool HaveAdminPassword() const = 0;
    EXPORT virtual TransactionNumber Highest() const = 0;
    EXPORT virtual bool isAdmin() const = 0;
    EXPORT virtual std::uint64_t Revision() const = 0;
    EXPORT virtual bool ShouldRename(
        const std::string& defaultName = "") const = 0;
    EXPORT virtual bool StaleNym() const = 0;
    EXPORT virtual std::unique_ptr<Item> Statement(
        const OTTransaction& owner) const = 0;
    EXPORT virtual std::unique_ptr<Item> Statement(
        const OTTransaction& owner,
        const std::set<TransactionNumber>& adding) const = 0;
    EXPORT virtual std::unique_ptr<TransactionStatement> Statement(
        const std::set<TransactionNumber>& adding,
        const std::set<TransactionNumber>& without) const = 0;
    EXPORT virtual bool Verify(const TransactionStatement& statement) const = 0;
    EXPORT virtual bool VerifyTentativeNumber(
        const TransactionNumber& number) const = 0;

    EXPORT virtual bool AcceptIssuedNumber(const TransactionNumber& number) = 0;
    EXPORT virtual bool AcceptIssuedNumbers(
        const TransactionStatement& statement) = 0;
    EXPORT virtual bool AddTentativeNumber(const TransactionNumber& number) = 0;
    EXPORT virtual network::ServerConnection& Connection() = 0;
    EXPORT virtual std::pair<RequestNumber, std::unique_ptr<Message>>
    InitializeServerCommand(
        const MessageType type,
        const Armored& payload,
        const Identifier& accountID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = true) = 0;
    EXPORT virtual std::pair<RequestNumber, std::unique_ptr<Message>>
    InitializeServerCommand(
        const MessageType type,
        const Identifier& recipientNymID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false) = 0;
    EXPORT virtual std::pair<RequestNumber, std::unique_ptr<Message>>
    InitializeServerCommand(
        const MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false) = 0;
    EXPORT virtual OTManagedNumber NextTransactionNumber(
        const MessageType reason) = 0;
    EXPORT virtual NetworkReplyMessage PingNotary() = 0;
    EXPORT virtual bool RemoveTentativeNumber(
        const TransactionNumber& number) = 0;
    EXPORT virtual bool Resync(const proto::Context& serialized) = 0;
    EXPORT virtual void SetAdminAttempted() = 0;
    EXPORT virtual void SetAdminPassword(const std::string& password) = 0;
    EXPORT virtual void SetAdminSuccess() = 0;
    EXPORT virtual bool SetHighest(const TransactionNumber& highest) = 0;
    EXPORT virtual void SetRevision(const std::uint64_t revision) = 0;
    EXPORT virtual TransactionNumber UpdateHighest(
        const std::set<TransactionNumber>& numbers,
        std::set<TransactionNumber>& good,
        std::set<TransactionNumber>& bad) = 0;
    EXPORT virtual RequestNumber UpdateRequestNumber() = 0;
    EXPORT virtual RequestNumber UpdateRequestNumber(bool& sendStatus) = 0;

    EXPORT virtual ~ServerContext() override = default;

protected:
    ServerContext() = default;

private:
    ServerContext(const ServerContext&) = delete;
    ServerContext(ServerContext&&) = delete;
    ServerContext& operator=(const ServerContext&) = delete;
    ServerContext& operator=(ServerContext&&) = delete;
};
}  // namespace opentxs
#endif
