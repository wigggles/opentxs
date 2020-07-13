// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/client/FilterOracle.hpp"  // IWYU pragma: associated

#include "internal/blockchain/Blockchain.hpp"  // IWYU pragma: keep
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::client::implementation::FilterOracle::BlockQueue::"  \
    "IndexBlockJob::"

namespace opentxs::blockchain::client::implementation
{
FilterOracle::BlockQueue::IndexBlockJob::IndexBlockJob(
    const Parent parent,
    const block::Position& position,
    const HeaderFuture previousHeader,
    const Future block) noexcept
    : position_(position)
    , running_(false)
    , complete_(false)
    , parent_(parent)
    , block_(block)
    , previous_header_(previousHeader)
    , filter_promise_(std::make_shared<std::promise<Filter>>())
    , filter_future_(filter_promise_->get_future())
    , header_promise_(std::make_shared<std::promise<block::pHash>>())
    , header_future_(header_promise_->get_future())
{
    OT_ASSERT(nullptr != parent_);
    OT_ASSERT(filter_promise_);
    OT_ASSERT(header_promise_);
}

FilterOracle::BlockQueue::IndexBlockJob::IndexBlockJob(
    const IndexBlockJob& rhs) noexcept
    : position_(rhs.position_)
    , running_(rhs.running_.load())
    , complete_(rhs.complete_.load())
    , parent_(rhs.parent_)
    , block_(rhs.block_)
    , previous_header_(rhs.previous_header_)
    , filter_promise_(rhs.filter_promise_)
    , filter_future_(rhs.filter_future_)
    , header_promise_(rhs.header_promise_)
    , header_future_(rhs.header_future_)
{
    OT_ASSERT(nullptr != parent_);
    OT_ASSERT(header_promise_);
}

auto FilterOracle::BlockQueue::IndexBlockJob::operator=(
    const IndexBlockJob& rhs) noexcept -> IndexBlockJob&
{
    if (this == &rhs) { return *this; }

    const_cast<block::Position&>(position_) = rhs.position_;
    running_.store(rhs.running_.load());
    complete_.store(rhs.complete_.load());
    const_cast<Parent&>(parent_) = rhs.parent_;
    block_ = rhs.block_;
    previous_header_ = rhs.previous_header_;
    filter_promise_ = rhs.filter_promise_;
    filter_future_ = rhs.filter_future_;
    header_promise_ = rhs.header_promise_;
    header_future_ = rhs.header_future_;

    OT_ASSERT(nullptr != parent_);
    OT_ASSERT(filter_promise_);
    OT_ASSERT(header_promise_);

    return *this;
}

auto FilterOracle::BlockQueue::IndexBlockJob::CalculateFilter() noexcept -> void
{
    if (false == ready(block_)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Block is not downloaded yet")
            .Flush();

        return;
    }

    const auto& blockP = block_.get();

    OT_ASSERT(blockP);

    const auto& block = *blockP;
    auto gcsP = factory::GCS(parent_->api_, parent_->type_, block);

    OT_ASSERT(gcsP);

    filter_promise_->set_value(std::move(gcsP));
}

auto FilterOracle::BlockQueue::IndexBlockJob::CalculateHeader() noexcept -> void
{
    if (false == ready(filter_future_)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Filter is not calculated yet")
            .Flush();

        return;
    }

    if (false == ready(previous_header_)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(
            ": Previous header is not calculated yet")
            .Flush();

        return;
    }

    const auto& gcs = *filter_future_.get();
    const auto filterHash = gcs.Hash();
    const auto& previousHeader = previous_header_.get().get();
    auto header = blockchain::internal::FilterHashToHeader(
        parent_->api_, filterHash->Bytes(), previousHeader.Bytes());

    OT_ASSERT(false == header->empty());

    header_promise_->set_value(std::move(header));
    complete_ = true;
}

auto FilterOracle::BlockQueue::IndexBlockJob::Evaluate(
    std::atomic<std::size_t>& completed) noexcept -> bool
{
    if (complete_) { return true; }

    if (hasBlock() && (false == hasFilter())) { CalculateFilter(); }

    if (hasPreviousHeader() && (false == hasHeader())) { CalculateHeader(); }

    if (complete_) { ++completed; }

    return complete_;
}
}  // namespace opentxs::blockchain::client::implementation
