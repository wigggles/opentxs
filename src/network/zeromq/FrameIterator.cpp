// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "1_Internal.hpp"                            // IWYU pragma: associated
#include "opentxs/network/zeromq/FrameIterator.hpp"  // IWYU pragma: associated

#include <utility>

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace opentxs::network::zeromq
{
FrameIterator::FrameIterator()
    : position_(0)
    , parent_(nullptr)
{
}

FrameIterator::FrameIterator(const Message* parent, std::size_t position)
    : position_(position)
    , parent_(parent)
{
    OT_ASSERT(nullptr != parent_);
}

FrameIterator::FrameIterator(const FrameIterator& frameIterator)
    : position_(frameIterator.position_.load())
    , parent_(frameIterator.parent_)
{
    OT_ASSERT(nullptr != parent_);
}

FrameIterator::FrameIterator(FrameIterator&& frameIterator)
    : position_(frameIterator.position_.load())
    , parent_(std::move(frameIterator.parent_))
{
    OT_ASSERT(nullptr != parent_);
}

FrameIterator& FrameIterator::operator=(const FrameIterator& frameIterator)
{
    position_ = frameIterator.position_.load();
    parent_ = frameIterator.parent_;

    return *this;
}

const Frame& FrameIterator::operator*() const
{
    OT_ASSERT(nullptr != parent_);

    return parent_->at(position_);
}

Frame& FrameIterator::operator*()
{
    OT_ASSERT(nullptr != parent_);

    return const_cast<Message*>(parent_)->at(position_);
}

bool FrameIterator::operator==(const FrameIterator& rhs) const
{
    OT_ASSERT(nullptr != parent_);

    return (parent_ == rhs.parent_) && (position_ == rhs.position_);
}

bool FrameIterator::operator!=(const FrameIterator& rhs) const
{
    OT_ASSERT(nullptr != parent_);

    return !(*this == rhs);
}

FrameIterator& FrameIterator::operator++()
{
    OT_ASSERT(nullptr != parent_);

    position_++;

    return *this;
}

FrameIterator FrameIterator::operator++(int)
{
    OT_ASSERT(nullptr != parent_);

    FrameIterator output(*this);
    position_++;

    return output;
}
}  // namespace opentxs::network::zeromq
