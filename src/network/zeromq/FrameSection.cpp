// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace opentxs::network::zeromq
{
FrameSection::FrameSection(const FrameSection& frameSection)
    : parent_(frameSection.parent_)
    , position_(frameSection.position_.load())
    , size_(frameSection.size_.load())
{
    OT_ASSERT(nullptr != parent_);
}

FrameSection::FrameSection(
    const Message* parent,
    std::size_t position,
    std::size_t size)
    : parent_(parent)
    , position_(position)
    , size_(size)
{
    OT_ASSERT(nullptr != parent_);
}

const Frame& FrameSection::at(const std::size_t index) const
{
    OT_ASSERT(size_ > index);

    return parent_->at(position_ + index);
}

FrameIterator FrameSection::begin() const
{
    return FrameIterator(parent_, position_);
}

FrameIterator FrameSection::end() const
{
    return FrameIterator(parent_, position_ + size_);
}

std::size_t FrameSection::size() const { return size_; }
}  // namespace opentxs::network::zeromq
