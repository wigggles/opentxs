// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLIND_TOKEN_HPP
#define OPENTXS_BLIND_TOKEN_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#include <chrono>
#include <cstdint>

namespace opentxs
{
namespace blind
{
class Token
{
public:
    using Clock = std::chrono::system_clock;
    using Time = Clock::time_point;
    using Denomination = std::uint64_t;
    using MintSeries = std::uint64_t;

    EXPORT virtual std::string ID() const = 0;
    EXPORT virtual bool IsSpent() const = 0;
    EXPORT virtual const identifier::Server& Notary() const = 0;
    EXPORT virtual proto::Token Serialize() const = 0;
    EXPORT virtual MintSeries Series() const = 0;
    EXPORT virtual proto::TokenState State() const = 0;
    EXPORT virtual proto::CashType Type() const = 0;
    EXPORT virtual const identifier::UnitDefinition& Unit() const = 0;
    EXPORT virtual Time ValidFrom() const = 0;
    EXPORT virtual Time ValidTo() const = 0;
    EXPORT virtual Denomination Value() const = 0;

    EXPORT virtual bool ChangeOwner(crypto::key::Symmetric& key) = 0;
    EXPORT virtual bool MarkSpent() = 0;
    EXPORT virtual bool Process(const Nym& owner, const Mint& mint) = 0;

    EXPORT virtual ~Token() = default;

protected:
    Token() = default;

private:
    friend OTToken;

    EXPORT virtual Token* clone() const noexcept = 0;

    Token(const Token&) = delete;
    Token(Token&&) = delete;
    Token& operator=(const Token&) = delete;
    Token& operator=(Token&&) = delete;
};
}  // namespace blind
}  // namespace opentxs
#endif
