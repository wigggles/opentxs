// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"
#include "Internal.hpp"

#if OT_CASH
#include "opentxs/blind/Purse.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"

#if OT_CASH_USING_LUCRE
#include "token/Lucre.hpp"
#endif

#include "Token.hpp"

#define OT_METHOD "opentxs::blind::token::implementation::Token::"

namespace opentxs
{
blind::Token* Factory::Token(
    const api::Core& api,
    blind::Purse& purse,
    const proto::Token& serialized)
{
    switch (serialized.type()) {
        case proto::CASHTYPE_LUCRE: {

            return new blind::token::implementation::Lucre(
                api, purse, serialized);
        }
        default: {
            LogOutput("opentxs::Factory::")(__FUNCTION__)(
                ": Unknown token type: ")(std::to_string(serialized.type()))
                .Flush();

            return nullptr;
        }
    }
}

blind::Token* Factory::Token(
    const api::Core& api,
    const identity::Nym& owner,
    const blind::Mint& mint,
    const blind::Token::Denomination value,
    blind::Purse& purse,
    const OTPassword& primaryPassword,
    const OTPassword& secondaryPassword)
{
    switch (purse.Type()) {
        case proto::CASHTYPE_LUCRE: {

            return new blind::token::implementation::Lucre(
                api,
                owner,
                mint,
                value,
                purse,
                primaryPassword,
                secondaryPassword);
        } break;
        default: {
            LogOutput("opentxs::Factory::")(__FUNCTION__)(
                ": Unknown token type: ")(std::to_string(purse.Type()))
                .Flush();

            return nullptr;
        }
    }
}
}  // namespace opentxs

namespace opentxs::blind::token::implementation
{
const proto::SymmetricMode Token::mode_{proto::SMODE_CHACHA20POLY1305};

Token::Token(
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
    const VersionNumber version)
    : blind::Token()
    , api_(api)
    , purse_(purse)
    , state_(state)
    , notary_(notary)
    , unit_(unit)
    , series_(series)
    , denomination_(denomination)
    , valid_from_(validFrom)
    , valid_to_(validTo)
    , type_(type)
    , version_(version)
{
}

Token::Token(const Token& rhs)
    : Token(
          rhs.api_,
          rhs.purse_,
          rhs.state_,
          rhs.type_,
          rhs.notary_,
          rhs.unit_,
          rhs.series_,
          rhs.denomination_,
          rhs.valid_from_,
          rhs.valid_to_,
          rhs.version_)
{
}

Token::Token(const api::Core& api, Purse& purse, const proto::Token& in)
    : Token(
          api,
          purse,
          in.state(),
          in.type(),
          identifier::Server::Factory(in.notary()),
          identifier::UnitDefinition::Factory(in.mint()),
          in.series(),
          in.denomination(),
          Clock::from_time_t(in.validfrom()),
          Clock::from_time_t(in.validto()),
          in.version())
{
}

Token::Token(
    const api::Core& api,
    Purse& purse,
    const VersionNumber version,
    const proto::TokenState state,
    const std::uint64_t series,
    const Denomination denomination,
    const Time validFrom,
    const Time validTo)
    : Token(
          api,
          purse,
          state,
          purse.Type(),
          purse.Notary(),
          purse.Unit(),
          series,
          denomination,
          validFrom,
          validTo,
          version)
{
}

bool Token::reencrypt(
    crypto::key::Symmetric& key,
    proto::Ciphertext& ciphertext,
    const PasswordPrompt& reason)
{
    auto plaintext = Data::Factory();
    auto output = purse_.PrimaryKey().Decrypt(ciphertext, reason, plaintext);

    if (false == output) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt ciphertext.")
            .Flush();

        return false;
    }

    output = key.Encrypt(
        plaintext,
        Data::Factory(),
        reason,
        ciphertext,
        false,
        proto::SMODE_CHACHA20POLY1305);

    if (false == output) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to encrypt ciphertext.")
            .Flush();

        return false;
    }

    return output;
}

proto::Token Token::Serialize() const
{
    proto::Token output{};
    output.set_version(version_);
    output.set_type(type_);
    output.set_state(state_);
    output.set_notary(notary_->str());
    output.set_mint(unit_->str());
    output.set_series(series_);
    output.set_denomination(denomination_);
    output.set_validfrom(Clock::to_time_t(valid_from_));
    output.set_validto(Clock::to_time_t(valid_to_));

    return output;
}
}  // namespace opentxs::blind::token::implementation
#endif
