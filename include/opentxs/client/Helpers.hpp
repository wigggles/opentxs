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

class Ledger;
class Nym;
class OTPayment;
class OTTransaction;

// returns financial instrument (Cheque, Purse, etc.) by
// receipt ID in ledger. So if ledger contains 5 receipts,
// the transaction ID of one of those receipts might contain
// a purse or something, that the caller wants to retrieve.
//
// Caller is responsible to delete.
EXPORT OTPayment* GetInstrumentByReceiptID(
    const Nym& theNym,
    const std::int64_t& lReceiptId,
    Ledger& ledger);

EXPORT OTPayment* GetInstrumentByIndex(
    const Nym& theNym,
    const std::int32_t& nIndex,
    Ledger& ledger);

// returns financial instrument inside pTransaction.
// (Cheque, Purse, etc.)
// Caller is responsible to delete.
EXPORT OTPayment* GetInstrument(
    const Nym& theNym,
    Ledger& ledger,
    OTTransaction*& pTransaction);

EXPORT OTPayment* extract_payment_instrument_from_notice(
    const Nym& theNym,
    OTTransaction*& pTransaction);

EXPORT std::int32_t GetOutpaymentsIndexByTransNum(
    const Nym& nym,
    std::int64_t lTransNum);

}  // namespace opentxs

#endif  // OPENTXS_CLIENT_HELPERS_HPP
