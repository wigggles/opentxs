// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTCaller.hpp"

#include "opentxs/core/crypto/OTCallback.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/PasswordPrompt.hpp"

// #define OT_METHOD "opentxs::OTCaller"

namespace opentxs
{
OTCaller::OTCaller()
    : callback_(nullptr)
{
}

void OTCaller::AskOnce(const PasswordPrompt& prompt, OTPassword& output)
{
    OT_ASSERT(callback_);

    callback_->runOne(prompt.GetDisplayString(), output);
}

void OTCaller::AskTwice(const PasswordPrompt& prompt, OTPassword& output)
{
    OT_ASSERT(callback_);

    callback_->runTwo(prompt.GetDisplayString(), output);
}

bool OTCaller::HaveCallback() const { return nullptr != callback_; }

void OTCaller::SetCallback(OTCallback* callback) { callback_ = callback; }

OTCaller::~OTCaller() { callback_ = nullptr; }
}  // namespace opentxs
