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

#ifndef OPENTXS_CASH_TOKENLUCRE_HPP
#define OPENTXS_CASH_TOKENLUCRE_HPP

#include "Token.hpp"

namespace opentxs
{

class Identifier;
class Mint;
class Nym;
class Purse;

/*
 Here's a rough sketch of the protocol:

 Client requests Mint for withdrawal of 100 ithica work hours.

1) Client blinds and sends N tokens to the server, each worth 100 hours. Client
retains the keys.
2) Server responds with a single index, the one the server has chosen for
signing.
3) Client replies with 99 keys.
4) Server unblinds 99 tokens (or some randomly-chosen % of those) and verifies
them.
   He signs the last one and returns it.
5) Client receives signed token, unblinds it, stores it for later.
6) When token is redeemed, it has already been unblinded. So Server simply
verifies it.

 LAST NAGGING QUESTION:  Should the server sign the other 99 tokens before
unblinding them and verifying?
                        In fact, what is it verifying at all?? Certainly not the
amount, which is not even in
                        the Lucre token. If all it does is verify its signature,
then why sign it just to
                        verify it?  Why exactly am I sending 99 tokens? What is
the server unblinding them
                        to look for??  Just to make sure all the IDs are random?
That they aren't spent
                        already?
                        I think that's it.  The client has assurance he chose
his own random IDs, the server
                        verifies they are random and not spent already, and the
ID portion is the only part
                        that has to be randomized.

 UPDATE:
 Ben Laurie has confirmed that the Chaumian 99 token requirement does not exist
with Lucre. All I have to
 do is send a single blinded token. The server signs it and sends it back, and
the client unblinds it. Only the
 ID itself is blinded -- the server can clearly see the amount and only the Mint
key for that denomination will work.
 */

// SUBCLASSES OF OTTOKEN FOR EACH DIGITAL CASH ALGORITHM.
#if defined(OT_CASH_USING_MAGIC_MONEY)
// Todo:  Someday...
#endif // Magic Money

#if defined(OT_CASH_USING_LUCRE)

class Token_Lucre : public Token
{
private: // Private prevents erroneous use by other classes.
    typedef Token ot_super;
    friend class Token; // for the factory.

protected:
    EXPORT Token_Lucre();
    EXPORT Token_Lucre(const Identifier& NOTARY_ID,
                       const Identifier& INSTRUMENT_DEFINITION_ID);
    EXPORT Token_Lucre(const Purse& thePurse);

    EXPORT virtual bool GenerateTokenRequest(
        const Nym& theNym, Mint& theMint, int64_t lDenomination,
        int32_t nTokenCount = Token::GetMinimumPrototokenCount());

public:
    EXPORT virtual bool ProcessToken(const Nym& theNym, Mint& theMint,
                                     Token& theRequest);

    EXPORT virtual ~Token_Lucre();
};

#endif // Lucre

} // namespace opentxs

#endif // OPENTXS_CASH_TOKENLUCRE_HPP
