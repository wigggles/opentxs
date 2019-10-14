// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"
#include "Internal.hpp"

#if OT_CASH
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blind/Mint.hpp"
#include "opentxs/blind/Purse.hpp"
#include "opentxs/blind/Token.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/identity/Nym.hpp"

#include "Token.hpp"

#include <limits>
#include <vector>

#include "Purse.hpp"

#define OT_PURSE_VERSION 1

#define OT_METHOD "opentxs::blind::implementation::Purse::"

namespace opentxs
{
blind::Purse* Factory::Purse(
    const api::Core& api,
    const opentxs::ServerContext& context,
    const proto::CashType type,
    const blind::Mint& mint,
    const Amount totalValue,
    const opentxs::PasswordPrompt& reason)
{
    return Purse(
        api,
        *context.Nym(),
        context.Server(),
        context.RemoteNym(),
        type,
        mint,
        totalValue,
        reason);
}

blind::Purse* Factory::Purse(
    const api::Core& api,
    const identity::Nym& nym,
    const identifier::Server& server,
    const identity::Nym& serverNym,
    const proto::CashType type,
    const blind::Mint& mint,
    const Amount totalValue,
    const opentxs::PasswordPrompt& reason)
{
    auto* output =
        new blind::implementation::Purse(api, nym.ID(), server, type, mint);

    if (nullptr == output) { return nullptr; }

    auto& purse = *output;
    auto locked = nym.Lock(
        purse.secondary_key_password_,
        purse.secondary_->get(),
        *purse.secondary_password_);

    if (false == locked) { return nullptr; }

    locked = purse.AddNym(serverNym, reason);
    locked = purse.AddNym(nym, reason);

    if (false == locked) { return nullptr; }

    const auto generated =
        purse.GeneratePrototokens(nym, mint, totalValue, reason);

    if (false == generated) { return nullptr; }

    return output;
}

blind::Purse* Factory::Purse(
    const api::Core& api,
    const proto::Purse& serialized)
{
    return new blind::implementation::Purse(api, serialized);
}

blind::Purse* Factory::Purse(
    const api::Core& api,
    const blind::Purse& request,
    const identity::Nym& requester,
    const opentxs::PasswordPrompt& reason)
{
    auto* output = new blind::implementation::Purse(
        api, dynamic_cast<const blind::implementation::Purse&>(request));

    if (nullptr == output) { return nullptr; }

    auto& purse = *output;
    auto locked = purse.AddNym(requester, reason);

    if (false == locked) { return nullptr; }

    return output;
}

blind::Purse* Factory::Purse(
    const api::Core& api,
    const identity::Nym& owner,
    const identifier::Server& server,
    const identifier::UnitDefinition& unit,
    const proto::CashType type,
    const opentxs::PasswordPrompt& reason)
{
    auto* output = new blind::implementation::Purse(api, server, unit, type);

    if (nullptr == output) { return nullptr; }

    const auto added = output->AddNym(owner, reason);

    if (false == added) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to encrypt purse").Flush();

        return nullptr;
    }

    return output;
}
}  // namespace opentxs

namespace opentxs::blind::implementation
{
const proto::SymmetricMode Purse::mode_{proto::SMODE_CHACHA20POLY1305};

Purse::Purse(
    const api::Core& api,
    const VersionNumber version,
    const proto::CashType type,
    const identifier::Server& notary,
    const identifier::UnitDefinition& unit,
    const proto::PurseType state,
    const Amount totalValue,
    const Time validFrom,
    const Time validTo,
    const std::vector<OTToken>& tokens,
    const std::shared_ptr<OTSymmetricKey> primary,
    const std::vector<proto::SessionKey>& primaryPasswords,
    const std::shared_ptr<OTSymmetricKey> secondary,
    const std::shared_ptr<proto::Ciphertext> secondaryPassword)
    : blind::Purse()
    , api_(api)
    , version_(version)
    , type_(type)
    , notary_(notary)
    , unit_(unit)
    , state_(state)
    , total_value_(totalValue)
    , latest_valid_from_(validFrom)
    , earliest_valid_to_(validTo)
    , tokens_(tokens)
    , unlocked_(false)
    , primary_key_password_()
    , primary_(primary)
    , primary_passwords_(primaryPasswords)
    , secondary_key_password_()
    , secondary_(secondary)
    , secondary_password_(secondaryPassword)
{
}

Purse::Purse(const Purse& rhs)
    : Purse(
          rhs.api_,
          rhs.version_,
          rhs.type_,
          rhs.notary_,
          rhs.unit_,
          rhs.state_,
          rhs.total_value_,
          rhs.latest_valid_from_,
          rhs.earliest_valid_to_,
          rhs.tokens_,
          rhs.primary_,
          rhs.primary_passwords_,
          rhs.secondary_,
          rhs.secondary_password_)
{
}

Purse::Purse(
    const api::Core& api,
    const identifier::Nym& owner,
    const identifier::Server& server,
    const proto::CashType type,
    const Mint& mint)
    : Purse(
          api,
          OT_PURSE_VERSION,
          type,
          server,
          mint.InstrumentDefinitionID(),
          proto::PURSETYPE_REQUEST,
          0,
          Time::min(),
          Time::max(),
          {},
          nullptr,
          {},
          nullptr,
          std::make_shared<proto::Ciphertext>())
{
    auto primary = generate_key(primary_key_password_);
    auto secondary = generate_key(secondary_key_password_);
    primary_.reset(new OTSymmetricKey(std::move(primary)));
    secondary_.reset(new OTSymmetricKey(std::move(secondary)));
    unlocked_ = true;

    OT_ASSERT(primary_);
    OT_ASSERT(secondary_);
    OT_ASSERT(secondary_password_);
}

Purse::Purse(
    const api::Core& api,
    const identifier::Server& server,
    const identifier::UnitDefinition& unit,
    const proto::CashType type)
    : Purse(
          api,
          OT_PURSE_VERSION,
          type,
          server,
          unit,
          proto::PURSETYPE_NORMAL,
          0,
          Time::min(),
          Time::max(),
          {},
          nullptr,
          {},
          nullptr,
          nullptr)
{
    auto primary = generate_key(primary_key_password_);
    primary_.reset(new OTSymmetricKey(std::move(primary)));
    unlocked_ = true;

    OT_ASSERT(primary_);
}

Purse::Purse(const api::Core& api, const proto::Purse& in)
    : Purse(
          api,
          in.version(),
          in.type(),
          identifier::Server::Factory(in.notary()),
          identifier::UnitDefinition::Factory(in.mint()),
          in.state(),
          in.totalvalue(),
          Clock::from_time_t(in.latestvalidfrom()),
          Clock::from_time_t(in.earliestvalidto()),
          {},
          nullptr,
          get_passwords(in),
          nullptr,
          nullptr)
{
    auto primary =
        api.Symmetric().Key(in.primarykey(), proto::SMODE_CHACHA20POLY1305);
    primary_.reset(new OTSymmetricKey(std::move(primary)));

    OT_ASSERT(primary_);

    for (const auto& serialized : in.token()) {
        tokens_.emplace_back(Factory::Token(api_, *this, serialized));
    }

    switch (state_) {
        case proto::PURSETYPE_REQUEST:
        case proto::PURSETYPE_ISSUE: {
            auto secondary = api.Symmetric().Key(
                in.secondarykey(), proto::SMODE_CHACHA20POLY1305);
            secondary_.reset(new OTSymmetricKey(std::move(secondary)));

            OT_ASSERT(secondary_);

            secondary_password_ =
                std::make_shared<proto::Ciphertext>(in.secondarypassword());

            OT_ASSERT(secondary_password_);
        } break;
        case proto::PURSETYPE_NORMAL: {
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid purse state").Flush();

            throw std::runtime_error("invalid purse state");
        }
    }
}

Purse::Purse(const api::Core& api, const Purse& owner)
    : Purse(
          api,
          owner.version_,
          owner.type_,
          owner.notary_,
          owner.unit_,
          proto::PURSETYPE_ISSUE,
          0,
          Time::min(),
          Time::max(),
          {},
          nullptr,
          {},
          owner.secondary_,
          owner.secondary_password_)
{
    auto primary = generate_key(primary_key_password_);
    primary_.reset(new OTSymmetricKey(std::move(primary)));
    unlocked_ = true;

    OT_ASSERT(primary_);
}

bool Purse::AddNym(const identity::Nym& nym, const PasswordPrompt& reason)
{
    if (false == unlocked_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Purse is locked").Flush();

        return false;
    }

    primary_passwords_.emplace_back();
    auto& sessionKey = *primary_passwords_.rbegin();

    if (false == bool(primary_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing primary key").Flush();

        return false;
    }

    const auto sealed =
        nym.Seal(primary_key_password_, primary_->get(), sessionKey, reason);

    if (false == sealed) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to add nym").Flush();
        primary_passwords_.pop_back();

        return false;
    }

    return true;
}

void Purse::apply_times(const Token& token)
{
    latest_valid_from_ = std::max(latest_valid_from_, token.ValidFrom());
    earliest_valid_to_ = std::min(earliest_valid_to_, token.ValidTo());
}

OTSymmetricKey Purse::generate_key(OTPassword& password) const
{
    password.randomizeMemory(32);
    auto keyPassword = api_.Factory().PasswordPrompt("");
    keyPassword->SetPassword(password);

    return api_.Symmetric().Key(keyPassword, mode_);
}

// TODO replace this algorithm with one that will ensure all spends up to and
// including the specified amount are possible
bool Purse::GeneratePrototokens(
    const identity::Nym& owner,
    const Mint& mint,
    const Amount amount,
    const opentxs::PasswordPrompt& reason)
{
    Amount workingAmount(amount);
    Amount tokenAmount{mint.GetLargestDenomination(workingAmount)};

    while (tokenAmount > 0) {
        workingAmount -= tokenAmount;
        std::shared_ptr<Token> pToken{Factory::Token(
            api_,
            owner,
            mint,
            tokenAmount,
            *this,
            primary_key_password_,
            secondary_key_password_)};

        if (false == bool(pToken)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to generate prototoken")
                .Flush();

            return {};
        }

        if (false == Push(pToken, reason)) { return false; }

        tokenAmount = mint.GetLargestDenomination(workingAmount);
    }

    return total_value_ == amount;
}

std::vector<proto::SessionKey> Purse::get_passwords(const proto::Purse& in)
{
    std::vector<proto::SessionKey> output;

    for (const auto& password : in.primarypassword()) {
        output.emplace_back(password);
    }

    return output;
}

crypto::key::Symmetric& Purse::PrimaryKey()
{
    if (false == bool(primary_)) { throw std::out_of_range("No primary key"); }

    if (primary_passwords_.empty()) {
        throw std::out_of_range("No session keys");
    }

    if (false == unlocked_) { throw std::out_of_range("Purse is locked"); }

    return primary_->get();
}

std::shared_ptr<Token> Purse::Pop()
{
    if (0 == tokens_.size()) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Purse is empty").Flush();

        return {};
    }

    const auto& existing = dynamic_cast<const token::implementation::Token&>(
        tokens_.crbegin()->get());
    std::shared_ptr<Token> pToken{existing.clone()};

    if (false == bool(pToken)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to get token").Flush();

        return {};
    }

    auto& token = *pToken;
    tokens_.pop_back();
    total_value_ -= token.Value();
    recalculate_times();

    return pToken;
}

bool Purse::Process(
    const identity::Nym& owner,
    const Mint& mint,
    const opentxs::PasswordPrompt& reason)
{
    if (proto::PURSETYPE_ISSUE != state_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect purse state").Flush();

        return false;
    }

    bool processed{true};

    for (auto& token : tokens_) {
        processed &= token->Process(owner, mint, reason);
    }

    if (processed) {
        state_ = proto::PURSETYPE_NORMAL;
        secondary_password_.reset();
        secondary_.reset();
        secondary_key_password_ = OTPassword{};
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to process token").Flush();
    }

    return processed;
}

bool Purse::Push(
    std::shared_ptr<Token> pToken,
    const opentxs::PasswordPrompt& reason)
{
    if (false == bool(pToken)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid token").Flush();

        return false;
    }

    if (false == bool(primary_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing primary key").Flush();

        return false;
    }

    if (false == unlocked_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Purse is locked").Flush();

        return false;
    }

    auto& token = *pToken;

    switch (token.State()) {
        case proto::TOKENSTATE_BLINDED:
        case proto::TOKENSTATE_SIGNED:
        case proto::TOKENSTATE_READY: {
            total_value_ += token.Value();
            apply_times(token);
        } break;
        default: {
        }
    }

    if (false == token.ChangeOwner(primary_->get(), reason)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to encrypt token").Flush();

        return false;
    }

    tokens_.emplace(tokens_.begin(), token);

    return true;
}

// TODO let's do this in constant time someday
void Purse::recalculate_times()
{
    latest_valid_from_ = Time::min();
    earliest_valid_to_ = Time::max();

    for (const auto& token : tokens_) { apply_times(token); }
}

crypto::key::Symmetric& Purse::SecondaryKey(const identity::Nym& owner)
{
    if (false == bool(secondary_)) {
        throw std::out_of_range("No secondary key");
    }

    if (false == bool(secondary_password_)) {
        throw std::out_of_range("No secondary key password");
    }

    const auto unlocked = owner.Unlock(
        *secondary_password_, *secondary_, secondary_key_password_);

    if (false == unlocked) { throw std::out_of_range("Failed to unlock key"); }

    return secondary_->get();
}

proto::Purse Purse::Serialize() const
{
    proto::Purse output{};
    output.set_version(version_);
    output.set_type(type_);
    output.set_state(state_);
    output.set_notary(notary_->str());
    output.set_mint(unit_->str());
    output.set_totalvalue(total_value_);
    output.set_latestvalidfrom(Clock::to_time_t(latest_valid_from_));
    output.set_earliestvalidto(Clock::to_time_t(earliest_valid_to_));

    for (const auto& token : tokens_) {
        *output.add_token() = token->Serialize();
    }

    if (false == bool(primary_)) {
        throw std::runtime_error("missing primary key");
    }

    if (false == primary_->get().Serialize(*output.mutable_primarykey())) {
        throw std::runtime_error("failed to serialize primary key");
    }

    for (const auto& password : primary_passwords_) {
        *output.add_primarypassword() = password;
    }

    switch (state_) {
        case proto::PURSETYPE_REQUEST:
        case proto::PURSETYPE_ISSUE: {
            if (false == bool(secondary_)) {
                throw std::runtime_error("missing secondary key");
            }

            if (!secondary_->get().Serialize(*output.mutable_secondarykey())) {
                throw std::runtime_error("failed to serialize secondary key");
            }

            if (false == bool(secondary_password_)) {
                throw std::runtime_error("missing secondary password");
            }

            *output.mutable_secondarypassword() = *secondary_password_;
        } break;
        case proto::PURSETYPE_NORMAL: {
        } break;
        default: {
            throw std::runtime_error("invalid purse state");
        }
    }

    return output;
}

bool Purse::Unlock(
    const identity::Nym& nym,
    const opentxs::PasswordPrompt& reason) const
{
    if (primary_passwords_.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No session keys found").Flush();

        return false;
    }

    if (false == bool(primary_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing primary key").Flush();

        return false;
    }

    OTPassword password{};

    for (const auto& sessionKey : primary_passwords_) {
        unlocked_ = nym.Open(sessionKey, primary_->get(), password, reason);

        if (unlocked_) {
            primary_key_password_ = password;
            break;
        }
    }

    if (false == unlocked_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Nym ")(nym.ID())(
            " can not decrypt any session key in the purse.")
            .Flush();
    }

    return unlocked_;
}

bool Purse::Verify(const api::server::Manager& server) const
{
    Amount total{0};
    auto validFrom{Time::min()};
    auto validTo{Time::max()};
    std::set<proto::TokenState> allowedStates{};

    switch (state_) {
        case proto::PURSETYPE_REQUEST: {
            allowedStates.insert(proto::TOKENSTATE_BLINDED);
        } break;
        case proto::PURSETYPE_ISSUE: {
            allowedStates.insert(proto::TOKENSTATE_SIGNED);
        } break;
        case proto::PURSETYPE_NORMAL: {
            allowedStates.insert(proto::TOKENSTATE_READY);
            allowedStates.insert(proto::TOKENSTATE_SPENT);
            allowedStates.insert(proto::TOKENSTATE_EXPIRED);
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid purse state.")
                .Flush();

            return false;
        }
    }

    for (const auto& pToken : tokens_) {
        const auto& token = pToken.get();

        if (type_ != token.Type()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Token type does not match purse type.")
                .Flush();

            return false;
        }

        if (notary_ != token.Notary()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Token notary does not match purse notary.")
                .Flush();

            return false;
        }

        if (unit_ != token.Unit()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Token unit does not match purse unit.")
                .Flush();

            return false;
        }

        if (0 == allowedStates.count(token.State())) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect token state")
                .Flush();

            return false;
        }

        auto pMint = server.GetPrivateMint(unit_, token.Series());

        if (false == bool(pMint)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect token series")
                .Flush();

            return false;
        }

        auto& mint = *pMint;

        if (mint.Expired() && (token.State() != proto::TOKENSTATE_EXPIRED)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Token is expired").Flush();

            return false;
        }

        if (token.ValidFrom() != mint.GetValidFrom()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect token valid from")
                .Flush();

            return false;
        }

        if (token.ValidTo() != mint.GetValidTo()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect token valid to")
                .Flush();

            return false;
        }

        validFrom = std::max(validFrom, token.ValidFrom());
        validTo = std::min(validTo, token.ValidFrom());

        switch (token.State()) {
            case proto::TOKENSTATE_BLINDED:
            case proto::TOKENSTATE_SIGNED:
            case proto::TOKENSTATE_READY: {
                total += token.Value();
                [[fallthrough]];
            }
            case proto::TOKENSTATE_SPENT:
            case proto::TOKENSTATE_EXPIRED: {
            } break;
            default: {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid token state")
                    .Flush();

                return false;
            }
        }
    }

    if (total_value_ != total) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect purse value").Flush();

        return false;
    }

    if (latest_valid_from_ != validFrom) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Incorrect purse latest valid from")
            .Flush();

        return false;
    }

    if (earliest_valid_to_ != validTo) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Incorrect purse earliest valid to")
            .Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs::blind::implementation
#endif
