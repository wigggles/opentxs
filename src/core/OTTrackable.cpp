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

#include "opentxs/core/OTTrackable.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Instrument.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/String.hpp"

#include <stdint.h>

namespace opentxs
{

OTTrackable::OTTrackable()
    : Instrument()
    , m_lTransactionNum(0)
{
    InitTrackable();
}

OTTrackable::OTTrackable(const Identifier& NOTARY_ID,
                         const Identifier& INSTRUMENT_DEFINITION_ID)
    : Instrument(NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , m_lTransactionNum(0)
{
    InitTrackable();
}

OTTrackable::OTTrackable(const Identifier& NOTARY_ID,
                         const Identifier& INSTRUMENT_DEFINITION_ID,
                         const Identifier& ACCT_ID, const Identifier& NYM_ID)
    : Instrument(NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , m_lTransactionNum(0)
{
    InitTrackable();

    SetSenderAcctID(ACCT_ID);
    SetSenderNymID(NYM_ID);
}

OTTrackable::~OTTrackable()
{
    Release_Trackable();
}

void OTTrackable::InitTrackable()
{
    // Should never happen in practice. A child class will override it.
    m_strContractType.Set("TRACKABLE");
    m_lTransactionNum = 0;
}

bool OTTrackable::HasTransactionNum(const int64_t& lInput) const
{
    return lInput == m_lTransactionNum;
}

void OTTrackable::GetAllTransactionNumbers(NumList& numlistOutput) const
{
    if (m_lTransactionNum > 0) numlistOutput.Add(m_lTransactionNum);
}

void OTTrackable::Release_Trackable()
{
    m_SENDER_ACCT_ID.Release();
    m_SENDER_NYM_ID.Release();
}

void OTTrackable::Release()
{
    Release_Trackable();
    Instrument::Release();

    // Then I call this to re-initialize everything for myself.
    InitTrackable();
}

void OTTrackable::UpdateContents()
{
}

} // namespace opentxs
