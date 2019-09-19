// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::implementation
{
class NullCallback final : virtual public OTCallback
{
public:
    void runOne(const char* display, OTPassword& output) const final;
    void runTwo(const char* display, OTPassword& output) const final;

    NullCallback() = default;

    ~NullCallback() final = default;

private:
    friend opentxs::Factory;

    static const std::string password_;

    NullCallback(const NullCallback&) = delete;
    NullCallback(NullCallback&&) = delete;
    NullCallback& operator=(const NullCallback&) = delete;
    NullCallback& operator=(NullCallback&&) = delete;
};
}  // namespace opentxs::implementation
