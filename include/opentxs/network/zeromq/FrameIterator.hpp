// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_FRAMEITERATOR_HPP
#define OPENTXS_NETWORK_ZEROMQ_FRAMEITERATOR_HPP

#include "opentxs/Forward.hpp"

#include <atomic>

#ifdef SWIG
// clang-format off
%rename(ZMQFrameIterator) opentxs::network::zeromq::FrameIterator;
%rename(assign) opentxs::network::zeromq::FrameIterator::operator=(const FrameIterator&);
%rename(toMessageConst) opentxs::network::zeromq::FrameIterator::operator*() const;
%rename(toMessage) opentxs::network::zeromq::FrameIterator::operator*();
%rename(compareEqual) opentxs::network::zeromq::FrameIterator::operator==(const FrameIterator&) const;
%rename(compareNotEqual) opentxs::network::zeromq::FrameIterator::operator!=(const FrameIterator&) const;
%rename(preIncrement) opentxs::network::zeromq::FrameIterator::operator++();
%rename(postIncrement) opentxs::network::zeromq::FrameIterator::operator++(int);
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class FrameIterator
{
public:
    using difference_type = std::size_t;
    using value_type = Frame;
    using pointer = Frame*;
    using reference = Frame&;
    using iterator_category = std::forward_iterator_tag;

    EXPORT FrameIterator() = default;
    EXPORT FrameIterator(const FrameIterator&);
    EXPORT FrameIterator(const Message* parent, std::size_t position = 0);
    EXPORT FrameIterator& operator=(const FrameIterator&);

    EXPORT const opentxs::network::zeromq::Frame& operator*() const;
    EXPORT bool operator==(const FrameIterator&) const;
    EXPORT bool operator!=(const FrameIterator&) const;

    EXPORT opentxs::network::zeromq::Frame& operator*();
    EXPORT FrameIterator& operator++();
    EXPORT FrameIterator operator++(int);

    EXPORT ~FrameIterator() = default;

private:
    std::atomic<std::size_t> position_{0};
    const Message* parent_{nullptr};

    FrameIterator(FrameIterator&&) = delete;
    FrameIterator& operator=(FrameIterator&&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
