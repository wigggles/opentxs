// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLIND_PURSE_HPP
#define OPENTXS_BLIND_PURSE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/iterator/Bidirectional.hpp"
#include "opentxs/Proto.hpp"

#include <chrono>
#include <cstdint>

namespace opentxs
{
namespace blind
{
class Purse
{
public:
    using Clock = std::chrono::system_clock;
    using Time = Clock::time_point;
    using iterator = opentxs::iterator::Bidirectional<Purse, Token>;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Purse, const Token>;

    EXPORT virtual const Token& at(const std::size_t position) const = 0;
    EXPORT virtual const_iterator begin() const noexcept = 0;
    EXPORT virtual const_iterator cbegin() const noexcept = 0;
    EXPORT virtual const_iterator cend() const noexcept = 0;
    EXPORT virtual Time EarliestValidTo() const = 0;
    EXPORT virtual const_iterator end() const noexcept = 0;
    EXPORT virtual bool IsUnlocked() const = 0;
    EXPORT virtual Time LatestValidFrom() const = 0;
    EXPORT virtual const identifier::Server& Notary() const = 0;
    EXPORT virtual proto::Purse Serialize() const = 0;
    EXPORT virtual std::size_t size() const noexcept = 0;
    EXPORT virtual proto::PurseType State() const = 0;
    EXPORT virtual proto::CashType Type() const = 0;
    EXPORT virtual const identifier::UnitDefinition& Unit() const = 0;
    EXPORT virtual bool Unlock(const Nym& nym) const = 0;
    EXPORT virtual bool Verify(const api::server::Manager& server) const = 0;
    EXPORT virtual Amount Value() const = 0;

    EXPORT virtual bool AddNym(const Nym& nym) = 0;
    EXPORT virtual Token& at(const std::size_t position) = 0;
    EXPORT virtual iterator begin() noexcept = 0;
    EXPORT virtual iterator end() noexcept = 0;
    EXPORT virtual crypto::key::Symmetric& PrimaryKey() = 0;
    EXPORT virtual std::shared_ptr<Token> Pop() = 0;
    EXPORT virtual bool Process(const Nym& owner, const Mint& mint) = 0;
    EXPORT virtual bool Push(std::shared_ptr<Token> token) = 0;
    EXPORT virtual crypto::key::Symmetric& SecondaryKey(const Nym& owner) = 0;

    EXPORT virtual ~Purse() = default;

protected:
    Purse() noexcept = default;

private:
    friend OTPurse;

    EXPORT virtual Purse* clone() const noexcept = 0;

    Purse(const Purse&) = delete;
    Purse(Purse&&) = delete;
    Purse& operator=(const Purse&) = delete;
    Purse& operator=(Purse&&) = delete;
};
}  // namespace blind
}  // namespace opentxs
#endif
