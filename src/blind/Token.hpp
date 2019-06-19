// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/blind/Token.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

#define OT_TOKEN_VERSION 1

namespace opentxs::blind::token::implementation
{
class Token : virtual public blind::Token
{
public:
    const identifier::Server& Notary() const override { return notary_; }
    MintSeries Series() const override { return series_; }
    proto::TokenState State() const override { return state_; }
    proto::CashType Type() const override { return type_; }
    const identifier::UnitDefinition& Unit() const override { return unit_; }
    Time ValidFrom() const override { return valid_from_; }
    Time ValidTo() const override { return valid_to_; }
    Denomination Value() const override { return denomination_; }

    virtual bool GenerateTokenRequest(
        const identity::Nym& owner,
        const OTPassword& primaryPassword,
        const OTPassword& secondaryPassword,
        const Mint& mint) = 0;

    ~Token() override = default;

protected:
    static const proto::SymmetricMode mode_;

    const api::Core& api_;
    Purse& purse_;
    proto::TokenState state_;
    const OTServerID notary_;
    const OTUnitID unit_;
    const std::uint64_t series_;
    const Denomination denomination_;
    const Time valid_from_;
    const Time valid_to_;

    bool reencrypt(
        crypto::key::Symmetric& key,
        proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason);

    proto::Token Serialize() const override;

    Token(const api::Core& api, Purse& purse, const proto::Token& serialized);
    Token(
        const api::Core& api,
        Purse& purse,
        const VersionNumber version,
        const proto::TokenState state,
        const std::uint64_t series,
        const Denomination denomination,
        const Time validFrom,
        const Time validTo);
    Token(const Token&);

private:
    friend blind::implementation::Purse;

    const proto::CashType type_;
    const VersionNumber version_;

    virtual Token* clone() const noexcept override = 0;

    Token(
        const api::Core& api,
        Purse& purse,
        const proto::TokenState state,
        const proto::CashType type,
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const Denomination denomination,
        const Time validFrom,
        const Time validTo,
        const VersionNumber version);
    Token() = delete;
    Token(Token&&) = delete;
    Token& operator=(const Token&) = delete;
    Token& operator=(Token&&) = delete;
};
}  // namespace opentxs::blind::token::implementation
