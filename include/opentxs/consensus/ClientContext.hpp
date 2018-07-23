// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CONSENSUS_CLIENTCONTEXT_HPP
#define OPENTXS_CONSENSUS_CLIENTCONTEXT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/consensus/Context.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <set>

namespace opentxs
{
class ClientContext : public Context
{
private:
    typedef Context ot_super;

public:
    ClientContext(
        const api::Legacy& legacy,
        const ConstNym& local,
        const ConstNym& remote,
        const Identifier& server,
        std::mutex& nymfileLock);
    ClientContext(
        const api::Legacy& legacy,
        const proto::Context& serialized,
        const ConstNym& local,
        const ConstNym& remote,
        const Identifier& server,
        std::mutex& nymfileLock);

    bool hasOpenTransactions() const;
    std::size_t IssuedNumbers(const std::set<TransactionNumber>& exclude) const;
    std::string LegacyDataFolder() const override;
    std::size_t OpenCronItems() const;
    proto::ConsensusType Type() const override;
    bool Verify(
        const TransactionStatement& statement,
        const std::set<TransactionNumber>& excluded,
        const std::set<TransactionNumber>& included) const;
    bool VerifyCronItem(const TransactionNumber number) const;
    using ot_super::VerifyIssuedNumber;
    bool VerifyIssuedNumber(
        const TransactionNumber& number,
        const std::set<TransactionNumber>& exclude) const;

    bool AcceptIssuedNumbers(std::set<TransactionNumber>& newNumbers);
    bool CloseCronItem(const TransactionNumber number) override;
    void FinishAcknowledgements(const std::set<RequestNumber>& req);
    bool IssueNumber(const TransactionNumber& number);
    bool OpenCronItem(const TransactionNumber number) override;

    ~ClientContext() = default;

private:
    std::set<TransactionNumber> open_cron_items_{};

    using ot_super::serialize;
    proto::Context serialize(const Lock& lock) const override;

    ClientContext() = delete;
    ClientContext(const ClientContext&) = delete;
    ClientContext(ClientContext&&) = delete;
    ClientContext& operator=(const ClientContext&) = delete;
    ClientContext& operator=(ClientContext&&) = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CONSENSUS_CLIENTCONTEXT_HPP
