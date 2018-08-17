// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CASH_MINTLUCRE_HPP
#define OPENTXS_CASH_MINTLUCRE_HPP

#include "opentxs/Forward.hpp"

#if OT_CASH_USING_LUCRE

#include "opentxs/cash/Mint.hpp"
#include "opentxs/core/String.hpp"

#include <cstdint>

namespace opentxs
{

class Nym;
class Token;

namespace api
{
namespace implementation
{

class Factory;

}  // namespace implementation
}  // namespace api

// SUBCLASSES OF OTMINT FOR EACH DIGITAL CASH ALGORITHM.

class MintLucre : public Mint
{
public:
    bool AddDenomination(
        const Nym& theNotary,
        std::int64_t lDenomination,
        std::int32_t nPrimeLength = 1024) override;

    EXPORT bool SignToken(
        const Nym& theNotary,
        Token& theToken,
        String& theOutput,
        std::int32_t nTokenIndex) override;
    EXPORT bool VerifyToken(
        const Nym& theNotary,
        String& theCleartextToken,
        std::int64_t lDenomination) override;

    EXPORT ~MintLucre() = default;

private:
    friend api::implementation::Factory;

    typedef Mint ot_super;

    MintLucre(const api::Core& core);
    EXPORT MintLucre(
        const api::Core& core,
        const String& strNotaryID,
        const String& strInstrumentDefinitionID);
    EXPORT MintLucre(
        const api::Core& core,
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID);
};
}  // namespace opentxs
#endif  // OT_CASH_USING_LUCRE
#endif
