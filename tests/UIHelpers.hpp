// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"

namespace
{
struct Counter {
    std::atomic_int expected_{};
    std::atomic_int updated_{};
};

[[maybe_unused]] auto make_cb(Counter& counter, const std::string name) noexcept
{
    return [&counter, name]() {
        auto& [expected, value] = counter;

        if (++value > expected) { std::cout << name << ": " << value << '\n'; }
    };
}

[[maybe_unused]] auto wait_for_counter(const Counter& data) noexcept -> bool
{
    constexpr auto limit = std::chrono::minutes(5);
    auto start = ot::Clock::now();
    const auto& [expected, updated] = data;

    while ((updated < expected) && ((ot::Clock::now() - start) < limit)) {
        ot::Sleep(std::chrono::milliseconds(100));
    }

    return updated >= expected;
}
}  // namespace
