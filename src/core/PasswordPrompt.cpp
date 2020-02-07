// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/String.hpp"

#include "opentxs/core/PasswordPrompt.hpp"

namespace opentxs
{
opentxs::PasswordPrompt* Factory::PasswordPrompt(
    const api::internal::Core& api,
    const std::string& text)
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

bool PasswordPrompt::ClearPassword()
{
    password_.reset();

    return true;
}

const char* PasswordPrompt::GetDisplayString() const
{
    return display_.c_str();
}

bool PasswordPrompt::SetPassword(const OTPassword& password)
{
    password_.reset(new OTPassword(password));

    return true;
}

const std::unique_ptr<const OTPassword>& PasswordPrompt::Password() const
{
    return password_;
}

PasswordPrompt::~PasswordPrompt() = default;
}  // namespace opentxs
