// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                 // IWYU pragma: associated
#include "1_Internal.hpp"               // IWYU pragma: associated
#include "consensus/ManagedNumber.hpp"  // IWYU pragma: associated

#include "Factory.hpp"
#include "opentxs/consensus/ManagedNumber.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/Flag.hpp"

namespace opentxs
{
auto Factory::ManagedNumber(
    const TransactionNumber number,
    opentxs::ServerContext& context) -> opentxs::ManagedNumber*
{
    return new implementation::ManagedNumber(number, context);
}

auto operator<(const OTManagedNumber& lhs, const OTManagedNumber& rhs) -> bool
{
    return lhs->Value() < rhs->Value();
}
}  // namespace opentxs

namespace opentxs::implementation
{
ManagedNumber::ManagedNumber(
    const TransactionNumber number,
    opentxs::ServerContext& context)
    : context_(context)
    , number_(number)
    , success_(Flag::Factory(false))
    , managed_(0 != number)
{
}

void ManagedNumber::SetSuccess(const bool value) const { success_->Set(value); }

auto ManagedNumber::Valid() const -> bool { return managed_; }

auto ManagedNumber::Value() const -> TransactionNumber { return number_; }

ManagedNumber::~ManagedNumber()
{
    if (false == managed_) { return; }

    if (success_.get()) { return; }

    context_.RecoverAvailableNumber(number_);
}
}  // namespace opentxs::implementation
