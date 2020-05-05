// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/consensus/ClientContext.cpp"

#pragma once

#include <iosfwd>
#include <mutex>
#include <set>
#include <string>

#include "consensus/Context.hpp"
#include "internal/consensus/Consensus.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/consensus/TransactionStatement.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/protobuf/ConsensusEnums.pb.h"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace identifier
{
class Nym;
class Server;
}  // namespace identifier

class Factory;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::implementation
{
class ClientContext final : virtual public internal::ClientContext,
                            public Context
{
public:
    proto::Context GetContract(const Lock& lock) const final
    {
        return contract(lock);
    }
    bool hasOpenTransactions() const final;
    using implementation::Context::IssuedNumbers;
    std::size_t IssuedNumbers(
        const std::set<TransactionNumber>& exclude) const final;
    std::size_t OpenCronItems() const final;
    proto::ConsensusType Type() const final;
    bool ValidateContext(const Lock& lock) const final
    {
        return validate(lock);
    }
    bool Verify(
        const TransactionStatement& statement,
        const std::set<TransactionNumber>& excluded,
        const std::set<TransactionNumber>& included) const final;
    bool VerifyCronItem(const TransactionNumber number) const final;
    using implementation::Context::VerifyIssuedNumber;
    bool VerifyIssuedNumber(
        const TransactionNumber& number,
        const std::set<TransactionNumber>& exclude) const final;

    bool AcceptIssuedNumbers(std::set<TransactionNumber>& newNumbers) final;
    bool CloseCronItem(const TransactionNumber number) final;
    void FinishAcknowledgements(const std::set<RequestNumber>& req) final;
    std::mutex& GetLock() final { return lock_; }
    bool IssueNumber(const TransactionNumber& number) final;
    bool OpenCronItem(const TransactionNumber number) final;
    bool UpdateSignature(const Lock& lock, const PasswordPrompt& reason) final
    {
        return update_signature(lock, reason);
    }

    ~ClientContext() final = default;

private:
    friend opentxs::Factory;

    std::set<TransactionNumber> open_cron_items_{};

    const identifier::Nym& client_nym_id(const Lock& lock) const final;
    using implementation::Context::serialize;
    proto::Context serialize(const Lock& lock) const final;
    const identifier::Nym& server_nym_id(const Lock& lock) const final;
    std::string type() const final { return "client"; }

    ClientContext(
        const api::internal::Core& api,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Server& server);
    ClientContext(
        const api::internal::Core& api,
        const proto::Context& serialized,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Server& server);
    ClientContext() = delete;
    ClientContext(const ClientContext&) = delete;
    ClientContext(ClientContext&&) = delete;
    ClientContext& operator=(const ClientContext&) = delete;
    ClientContext& operator=(ClientContext&&) = delete;
};
}  // namespace opentxs::implementation
