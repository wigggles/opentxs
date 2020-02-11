// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_FRAMESECTION_HPP
#define OPENTXS_NETWORK_ZEROMQ_FRAMESECTION_HPP

#include "opentxs/Forward.hpp"

#include <atomic>

#ifdef SWIG
// clang-format off
%rename(ZMQFrameSection) opentxs::network::zeromq::FrameSection;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class FrameSection
{
public:
    using difference_type = std::size_t;
    using value_type = Frame;
    using pointer = Frame*;
    using reference = Frame&;
    using iterator_category = std::forward_iterator_tag;

    OPENTXS_EXPORT const Frame& at(const std::size_t index) const;
    OPENTXS_EXPORT FrameIterator begin() const;
    OPENTXS_EXPORT FrameIterator end() const;
    OPENTXS_EXPORT std::size_t size() const;

    OPENTXS_EXPORT FrameSection(
        const Message* parent,
        std::size_t position,
        std::size_t size);
    OPENTXS_EXPORT FrameSection(const FrameSection&);

    OPENTXS_EXPORT virtual ~FrameSection() = default;

protected:
    FrameSection() = default;

private:
    const Message* parent_{nullptr};
    std::atomic<std::size_t> position_{0};
    std::atomic<std::size_t> size_{0};

    FrameSection(FrameSection&&) = delete;
    FrameSection& operator=(const FrameSection&) = delete;
    FrameSection& operator=(FrameSection&&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
