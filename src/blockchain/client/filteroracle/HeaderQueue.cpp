// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/client/FilterOracle.hpp"  // IWYU pragma: associated

#include "internal/blockchain/Blockchain.hpp"  // IWYU pragma: keep

// #define OT_METHOD
// "opentxs::blockchain::client::implementation::FilterOracle::HeaderQueue::"

namespace opentxs::blockchain::client::implementation
{
const std::chrono::seconds FilterOracle::HeaderQueue::limit_{15};

FilterOracle::HeaderQueue::HeaderQueue(const api::Core& api) noexcept
    : hashes_()
{
}

auto FilterOracle::HeaderQueue::Finish(const block::Hash& block) noexcept
    -> void
{
    prune();
    hashes_.erase(block);
}

auto FilterOracle::HeaderQueue::IsRunning(const block::Hash& block) noexcept
    -> bool
{
    prune();

    return 0 < hashes_.count(block);
}

auto FilterOracle::HeaderQueue::prune() const noexcept -> void
{
    for (auto i = hashes_.begin(); i != hashes_.end();) {
        const auto duration = Clock::now() - i->second;

        if ((duration > limit_) || (std::chrono::seconds(0) > duration)) {
            i = hashes_.erase(i);
        } else {
            ++i;
        }
    }
}

auto FilterOracle::HeaderQueue::Start(const block::Hash& block) noexcept -> void
{
    prune();
    hashes_.emplace(block, Clock::now());
}
}  // namespace opentxs::blockchain::client::implementation
