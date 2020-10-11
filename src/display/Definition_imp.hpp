// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "display/Definition.hpp"  // IWYU pragma: associated

namespace opentxs::display
{
struct Definition::Imp {
    using Scales = std::vector<NamedScale>;

    const Scales scales_;

    auto Import(const std::string& in, const Index index) const noexcept(false)
        -> Amount
    {
        try {
            const auto& scale = scales_.at(static_cast<std::size_t>(index));

            return scale.second.Import(in);
        } catch (...) {
            throw std::out_of_range("Invalid scale index");
        }
    }

    Imp(Scales&& scales) noexcept
        : scales_(std::move(scales))
    {
    }
};
}  // namespace opentxs::display
