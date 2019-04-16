// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blind::mint::implementation
{
class Lucre final : Mint
{
public:
    bool AddDenomination(
        const identity::Nym& theNotary,
        const std::int64_t denomination,
        const std::size_t keySize) override;

    bool SignToken(const identity::Nym& notary, blind::Token& token) override;
    bool VerifyToken(const identity::Nym& notary, const blind::Token& token)
        override;

    ~Lucre() = default;

private:
    friend opentxs::Factory;

    Lucre(const api::Core& core);
    Lucre(
        const api::Core& core,
        const String& strNotaryID,
        const String& strInstrumentDefinitionID);
    Lucre(
        const api::Core& core,
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID);
};
}  // namespace opentxs::blind::mint::implementation
