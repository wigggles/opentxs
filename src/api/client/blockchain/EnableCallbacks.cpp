// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "api/client/Blockchain.hpp"  // IWYU pragma: associated

#include <functional>
#include <map>
#include <vector>

#include "internal/api/client/Client.hpp"
#include "opentxs/core/Log.hpp"

// #define OT_METHOD
// "opentxs::api::client::implementation::Blockchain::EnableCallbacks::"

namespace opentxs::api::client::implementation
{
auto Blockchain::EnableCallbacks::Add(
    const Chain type,
    EnabledCallback cb) noexcept -> std::size_t
{
    Lock lock(lock_);
    auto& vector = map_[type];
    vector.emplace_back(cb);

    OT_ASSERT(0u < vector.size());

    return vector.size() - 1u;
}

auto Blockchain::EnableCallbacks::Delete(
    const Chain type,
    const std::size_t index) noexcept -> void
{
    Lock lock(lock_);
    auto& vector = map_[type];

    if (index >= vector.size()) { return; }

    vector.erase(vector.begin() + index);
}

auto Blockchain::EnableCallbacks::Execute(
    const Chain type,
    const bool value) noexcept -> void
{
    Lock lock(lock_);

    try {
        auto& vector = map_.at(type);

        for (auto i{vector.begin()}; i < vector.end();) {
            const auto& cb = *i;

            if (false == bool(cb)) {
                // Null callback
                i = vector.erase(i);
            } else if (cb(value)) {
                // Callback can be executed again
                ++i;
            } else {
                // Callback is no longer valid
                i = vector.erase(i);
            }
        }
    } catch (...) {
    }
}
}  // namespace opentxs::api::client::implementation
