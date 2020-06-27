// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/client/FilterOracle.hpp"  // IWYU pragma: associated

#include <iterator>
#include <type_traits>

#include "internal/blockchain/Blockchain.hpp"  // IWYU pragma: keep
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/client/HeaderOracle.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::client::implementation::FilterOracle::FilterQueue::"

namespace opentxs::blockchain::client::implementation
{
const std::chrono::seconds FilterOracle::FilterQueue::timeout_{20};

FilterOracle::FilterQueue::FilterQueue(const api::client::Manager& api) noexcept
    : error_(false)
    , api_(api)
    , running_(false)
    , queued_(0)
    , filters_()
    , last_received_()
    , target_(make_blank<block::Position>::value(api))
{
    filters_.reserve(1000);
}

auto FilterOracle::FilterQueue::AddFilter(
    const block::Height height,
    const block::Hash& hash,
    std::unique_ptr<const blockchain::internal::GCS> filter) noexcept -> void
{
    OT_ASSERT(filter);

    const auto& target = target_.first;

    if (height > target) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Filter height (")(height)(
            ") is after requested range")
            .Flush();

        return;
    }

    const auto offset = static_cast<std::size_t>(target - height);

    if (offset >= filters_.size()) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Filter height (")(height)(
            ") is before requested range")
            .Flush();

        return;
    }

    auto it{filters_.begin()};
    std::advance(it, offset);

    if (hash != it->first) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Wrong block for filter at height ")(height)
            .Flush();

        return;
    }

    auto& cachedFilter = it->second;

    if (false == bool(cachedFilter)) {
        cachedFilter.reset(filter.release());
        ++queued_;
    }

    OT_ASSERT(cachedFilter);

    last_received_ = Clock::now();

    if ((height == target) && (queued_ < filters_.size())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Last filter in range received, however only ")(queued_)(" of ")(
            filters_.size())(" filters are queued.")
            .Flush();

        if (filters_.rbegin()->second) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Accepting partial filter batch.")
                .Flush();
            queued_ = filters_.size();
        } else {
            error_ = true;
        }
    }
}

auto FilterOracle::FilterQueue::Flush(Filters& filters) noexcept
    -> block::Position
{
    auto counter = target_.first - filters_.size();

    for (auto it{filters_.rbegin()}; it != filters_.rend(); ++it) {
        ++counter;

        if (it->second) {
            filters.emplace_back(internal::FilterDatabase::Filter{
                it->first->Bytes(), std::move(it->second)});
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Filter for block ")(counter)(
                " is missing.")
                .Flush();

            OT_ASSERT(filters_.rbegin() != it);

            std::advance(it, -1);

            return {counter - 1, it->first};
        }
    }

    return target_;
}

auto FilterOracle::FilterQueue::IsFull() noexcept -> bool
{
    return filters_.size() == queued_;
}

auto FilterOracle::FilterQueue::IsRunning() noexcept -> bool
{
    if (error_) { Reset(); }

    if (running_) {
        if ((Clock::now() - last_received_) > timeout_) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(
                ": Filter request timed out after ")(timeout_.count())(
                " seconds ")
                .Flush();
            Reset();
        }
    }

    return running_;
}

auto FilterOracle::FilterQueue::Queue(
    const block::Height startHeight,
    const block::Hash& stopHash,
    const client::HeaderOracle& headers) noexcept -> void
{
    OT_ASSERT(false == running_);
    OT_ASSERT(0 == queued_);
    OT_ASSERT(0 == filters_.size());

    auto header = headers.LoadHeader(stopHash);

    OT_ASSERT(header);

    target_ = header->Position();
    filters_.emplace_back(FilterData{header->Hash(), nullptr});

    while (header->Height() > startHeight) {
        header = headers.LoadHeader(header->ParentHash());

        OT_ASSERT(header);

        filters_.emplace_back(FilterData{header->Hash(), nullptr});
    }

    running_ = true;
    last_received_ = Clock::now();
}

auto FilterOracle::FilterQueue::Reset() noexcept -> void
{
    running_ = false;
    queued_ = 0;
    filters_.clear();
    last_received_ = Time{};
    target_ = make_blank<block::Position>::value(api_);
}
}  // namespace opentxs::blockchain::client::implementation
