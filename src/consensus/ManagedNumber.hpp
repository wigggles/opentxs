// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::implementation
{
class ManagedNumber final : virtual public opentxs::ManagedNumber
{
public:
    void SetSuccess(const bool value = true) const final;
    bool Valid() const final;
    TransactionNumber Value() const final;

    ~ManagedNumber() final;

private:
    friend opentxs::Factory;

    opentxs::ServerContext& context_;
    const TransactionNumber number_;
    mutable OTFlag success_;
    bool managed_{true};

    ManagedNumber(
        const TransactionNumber number,
        opentxs::ServerContext& context);
    ManagedNumber() = delete;
    ManagedNumber(const ManagedNumber&) = delete;
    ManagedNumber(ManagedNumber&& rhs) = delete;
    ManagedNumber& operator=(const ManagedNumber&) = delete;
    ManagedNumber& operator=(ManagedNumber&&) = delete;
};
}  // namespace opentxs::implementation
