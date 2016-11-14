/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_CORE_OTTRACKABLE_HPP
#define OPENTXS_CORE_OTTRACKABLE_HPP

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Instrument.hpp"

#include <stdint.h>

namespace opentxs
{

class NumList;

// OTTrackable is very similar to OTInstrument.
// The difference is, it may have identifying info on it:
// TRANSACTION NUMBER, SENDER USER ID (NYM ID), AND SENDER ACCOUNT ID.
//
class OTTrackable : public Instrument
{
public:
    OTTrackable();
    OTTrackable(const Identifier& NOTARY_ID,
                const Identifier& INSTRUMENT_DEFINITION_ID);
    OTTrackable(const Identifier& NOTARY_ID,
                const Identifier& INSTRUMENT_DEFINITION_ID,
                const Identifier& ACCT_ID, const Identifier& NYM_ID);

    virtual ~OTTrackable();

    void InitTrackable();
    void Release_Trackable();

    void Release() override;
    void UpdateContents() override;

    virtual bool HasTransactionNum(const int64_t& lInput) const;
    virtual void GetAllTransactionNumbers(NumList& numlistOutput) const;

    inline int64_t GetTransactionNum() const
    {
        return m_lTransactionNum;
    }

    inline void SetTransactionNum(int64_t lTransactionNum)
    {
        m_lTransactionNum = lTransactionNum;
    }

    inline const Identifier& GetSenderAcctID() const
    {
        return m_SENDER_ACCT_ID;
    }

    inline const Identifier& GetSenderNymID() const
    {
        return m_SENDER_NYM_ID;
    }

protected:
    inline void SetSenderAcctID(const Identifier& ACCT_ID)
    {
        m_SENDER_ACCT_ID = ACCT_ID;
    }

    inline void SetSenderNymID(const Identifier& NYM_ID)
    {
        m_SENDER_NYM_ID = NYM_ID;
    }

protected:
    int64_t m_lTransactionNum{0};
    // The asset account the instrument is drawn on.
    Identifier m_SENDER_ACCT_ID;
    // This ID must match the user ID on that asset account,
    // AND must verify the instrument's signature with that user's key.
    Identifier m_SENDER_NYM_ID;
};

} // namespace opentxs

#endif // OPENTXS_CORE_OTTRACKABLE_HPP
