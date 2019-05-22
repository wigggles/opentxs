// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_HELPERS_HPP
#define OPENTXS_CLIENT_HELPERS_HPP

#include "opentxs/Forward.hpp"

#include <cstdint>

namespace opentxs
{
// returns financial instrument (Cheque, Purse, etc.) by
// receipt ID in ledger. So if ledger contains 5 receipts,
// the transaction ID of one of those receipts might contain
// a purse or something, that the caller wants to retrieve.
//
EXPORT std::shared_ptr<OTPayment> GetInstrumentByReceiptID(
    const api::Core& api,
    const identity::Nym& theNym,
    const std::int64_t& lReceiptId,
    Ledger& ledger);

EXPORT std::shared_ptr<OTPayment> GetInstrumentByIndex(
    const api::Core& api,
    const identity::Nym& theNym,
    const std::int32_t& nIndex,
    Ledger& ledger);

// returns financial instrument inside pTransaction.
// (Cheque, Purse, etc.)
EXPORT std::shared_ptr<OTPayment> GetInstrument(
    const api::Core& api,
    const identity::Nym& theNym,
    Ledger& ledger,
    std::shared_ptr<OTTransaction> pTransaction);

EXPORT std::shared_ptr<OTPayment> extract_payment_instrument_from_notice(
    const api::Core& api,
    const identity::Nym& theNym,
    std::shared_ptr<OTTransaction> pTransaction);
}  // namespace opentxs
#endif
