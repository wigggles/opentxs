// Copyright (c) 2010-2019 The Open-Transactions developers
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
        const std::size_t keySize,
        const PasswordPrompt& reason) final;

    bool SignToken(
        const identity::Nym& notary,
        blind::Token& token,
        const PasswordPrompt& reason) final;
    bool VerifyToken(
        const identity::Nym& notary,
        const blind::Token& token,
        const PasswordPrompt& reason) final;

    ~Lucre() final = default;

private:
    friend opentxs::Factory;

    Lucre(const api::internal::Core& core);
    Lucre(
        const api::internal::Core& core,
        const String& strNotaryID,
        const String& strInstrumentDefinitionID);
    Lucre(
        const api::internal::Core& core,
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID);
};
}  // namespace opentxs::blind::mint::implementation
