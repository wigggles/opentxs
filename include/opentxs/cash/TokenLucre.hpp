// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CASH_TOKENLUCRE_HPP
#define OPENTXS_CASH_TOKENLUCRE_HPP

#include "opentxs/Forward.hpp"

#if OT_CASH_USING_LUCRE

#include "opentxs/cash/Token.hpp"

#include <cstdint>

namespace opentxs
{
namespace api
{
namespace implementation
{

class Factory;

}  // namespace implementation
}  // namespace api

/*
Here's a rough sketch of the protocol:

Client requests Mint for withdrawal of 100 ithica work hours.

1) Client blinds and sends N tokens to the server, each worth 100 hours. Client
retains the keys.
2) Server responds with a single index, the one the server has chosen for
signing.
3) Client replies with 99 keys.
4) Server unblinds 99 tokens (or some randomly-chosen % of those) and verifies
them. He signs the last one and returns it.
5) Client receives signed token, unblinds it, stores it for later.
6) When token is redeemed, it has already been unblinded. So Server simply
verifies it.

LAST NAGGING QUESTION: Should the server sign the other 99 tokens before
unblinding them and verifying? In fact, what is it verifying at all?? Certainly
not the amount, which is not even in the Lucre token. If all it does is verify
its signature, then why sign it just to verify it?  Why exactly am I sending 99
tokens? What is the server unblinding them to look for??  Just to make sure all
the IDs are random? That they aren't spent already? I think that's it.  The
client has assurance he chose his own random IDs, the server verifies they are
random and not spent already, and the ID portion is the only part that has to be
randomized.

UPDATE: Ben Laurie has confirmed that the Chaumian 99 token requirement does not
exist with Lucre. All I have to do is send a single blinded token. The server
signs it and sends it back, and the client unblinds it. Only the ID itself is
blinded -- the server can clearly see the amount and only the Mint key for that
denomination will work. */

// SUBCLASSES OF OTTOKEN FOR EACH DIGITAL CASH ALGORITHM.

class Token_Lucre : public Token
{
public:
    EXPORT bool ProcessToken(
        const Nym& theNym,
        Mint& theMint,
        Token& theRequest) override;

    EXPORT ~Token_Lucre() = default;

private:
    friend api::implementation::Factory;

    Token_Lucre(const api::Core& core);
    Token_Lucre(
        const api::Core& core,
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID);
    Token_Lucre(const api::Core& core, const Purse& thePurse);

    bool GenerateTokenRequest(
        const Nym& theNym,
        Mint& theMint,
        std::int64_t lDenomination,
        std::int32_t nTokenCount = Token::GetMinimumPrototokenCount()) override;

    typedef Token ot_super;

    Token_Lucre() = delete;
};
}  // namespace opentxs
#endif  // OT_CASH_USING_LUCRE
#endif
