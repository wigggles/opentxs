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

#ifndef OPENTXS_CASH_MINTLUCRE_HPP
#define OPENTXS_CASH_MINTLUCRE_HPP

#include "opentxs/cash/Mint.hpp"
#include "opentxs/core/String.hpp"

#include <stdint.h>

namespace opentxs
{

class Nym;
class Token;

// SUBCLASSES OF OTMINT FOR EACH DIGITAL CASH ALGORITHM.

#if defined(OT_CASH_USING_MAGIC_MONEY)
// Todo:  Someday...
#endif // Magic Money

#if defined(OT_CASH_USING_LUCRE)

class MintLucre : public Mint
{
private: // Private prevents erroneous use by other classes.
    typedef Mint ot_super;
    friend class Mint; // for the factory.
protected:
    MintLucre();
    EXPORT MintLucre(const String& strNotaryID,
                     const String& strInstrumentDefinitionID);
    EXPORT MintLucre(const String& strNotaryID, const String& strServerNymID,
                     const String& strInstrumentDefinitionID);

public:
    bool AddDenomination(Nym& theNotary, int64_t lDenomination,
                                 int32_t nPrimeLength = 1024) override;

    EXPORT bool SignToken(Nym& theNotary, Token& theToken,
                                  String& theOutput, int32_t nTokenIndex) override;
    EXPORT bool VerifyToken(Nym& theNotary, String& theCleartextToken,
                                    int64_t lDenomination) override;

    EXPORT virtual ~MintLucre();
};

#endif // Lucre

} // namespace opentxs

#endif // OPENTXS_CASH_MINTLUCRE_HPP
