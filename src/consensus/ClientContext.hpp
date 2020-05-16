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
    auto GetContract(const Lock& lock) const -> proto::Context final
    {
        return contract(lock);
    }
    auto hasOpenTransactions() const -> bool final;
    using implementation::Context::IssuedNumbers;
    auto IssuedNumbers(const std::set<TransactionNumber>& exclude) const
        -> std::size_t final;
    auto OpenCronItems() const -> std::size_t final;
    auto Type() const -> proto::ConsensusType final;
    auto ValidateContext(const Lock& lock) const -> bool final
    {
        return validate(lock);
    }
    auto Verify(
        const TransactionStatement& statement,
        const std::set<TransactionNumber>& excluded,
        const std::set<TransactionNumber>& included) const -> bool final;
    auto VerifyCronItem(const TransactionNumber number) const -> bool final;
    using implementation::Context::VerifyIssuedNumber;
    auto VerifyIssuedNumber(
        const TransactionNumber& number,
        const std::set<TransactionNumber>& exclude) const -> bool final;

    auto AcceptIssuedNumbers(std::set<TransactionNumber>& newNumbers)
        -> bool final;
    auto CloseCronItem(const TransactionNumber number) -> bool final;
    void FinishAcknowledgements(const std::set<RequestNumber>& req) final;
    auto GetLock() -> std::mutex& final { return lock_; }
    auto IssueNumber(const TransactionNumber& number) -> bool final;
    auto OpenCronItem(const TransactionNumber number) -> bool final;
    auto UpdateSignature(const Lock& lock, const PasswordPrompt& reason)
        -> bool final
    {
        return update_signature(lock, reason);
    }

    ~ClientContext() final = default;

private:
    friend opentxs::Factory;

    std::set<TransactionNumber> open_cron_items_{};

    auto client_nym_id(const Lock& lock) const -> const identifier::Nym& final;
    using implementation::Context::serialize;
    auto serialize(const Lock& lock) const -> proto::Context final;
    auto server_nym_id(const Lock& lock) const -> const identifier::Nym& final;
    auto type() const -> std::string final { return "client"; }

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
    auto operator=(const ClientContext&) -> ClientContext& = delete;
    auto operator=(ClientContext &&) -> ClientContext& = delete;
};
}  // namespace opentxs::implementation
