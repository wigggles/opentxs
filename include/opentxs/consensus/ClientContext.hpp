// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CONSENSUS_CLIENTCONTEXT_HPP
#define OPENTXS_CONSENSUS_CLIENTCONTEXT_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/consensus/Context.hpp"

namespace opentxs
{
class ClientContext : virtual public Context
{
public:
    OPENTXS_EXPORT virtual bool hasOpenTransactions() const = 0;
    using Context::IssuedNumbers;
    OPENTXS_EXPORT virtual std::size_t IssuedNumbers(
        const TransactionNumbers& exclude) const = 0;
    OPENTXS_EXPORT virtual std::size_t OpenCronItems() const = 0;
    OPENTXS_EXPORT virtual bool Verify(
        const TransactionStatement& statement,
        const TransactionNumbers& excluded,
        const TransactionNumbers& included) const = 0;
    OPENTXS_EXPORT virtual bool VerifyCronItem(
        const TransactionNumber number) const = 0;
    using Context::VerifyIssuedNumber;
    OPENTXS_EXPORT virtual bool VerifyIssuedNumber(
        const TransactionNumber& number,
        const TransactionNumbers& exclude) const = 0;

    OPENTXS_EXPORT virtual bool AcceptIssuedNumbers(
        TransactionNumbers& newNumbers) = 0;
    OPENTXS_EXPORT virtual void FinishAcknowledgements(
        const RequestNumbers& req) = 0;
    OPENTXS_EXPORT virtual bool IssueNumber(
        const TransactionNumber& number) = 0;

    OPENTXS_EXPORT ~ClientContext() override = default;

protected:
    ClientContext() = default;

private:
    ClientContext(const ClientContext&) = delete;
    ClientContext(ClientContext&&) = delete;
    ClientContext& operator=(const ClientContext&) = delete;
    ClientContext& operator=(ClientContext&&) = delete;
};
}  // namespace opentxs
#endif
