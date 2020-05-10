// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "opentxs/core/PasswordPrompt.hpp"  // IWYU pragma: associated

#include "Factory.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"

namespace opentxs
{
auto Factory::PasswordPrompt(
    const api::internal::Core& api,
    const std::string& text) -> opentxs::PasswordPrompt*
{
    return new opentxs::PasswordPrompt(api, text);
}

PasswordPrompt::PasswordPrompt(
    const api::internal::Core& api,
    const std::string& display) noexcept
    : api_(api)
    , display_(display)
    , password_(nullptr)
{
}

PasswordPrompt::PasswordPrompt(const PasswordPrompt& rhs) noexcept
    : api_(rhs.api_)
    , display_(rhs.GetDisplayString())
    , password_(rhs.password_ ? new OTPassword(*rhs.password_) : nullptr)
{
}

auto PasswordPrompt::ClearPassword() -> bool
{
    password_.reset();

    return true;
}

auto PasswordPrompt::GetDisplayString() const -> const char*
{
    return display_.c_str();
}

auto PasswordPrompt::SetPassword(const OTPassword& password) -> bool
{
    password_.reset(new OTPassword(password));

    return true;
}

auto PasswordPrompt::Password() const
    -> const std::unique_ptr<const OTPassword>&
{
    return password_;
}

PasswordPrompt::~PasswordPrompt() = default;
}  // namespace opentxs
