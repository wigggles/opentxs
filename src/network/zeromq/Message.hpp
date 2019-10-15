// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::network::zeromq::implementation
{
class Message : virtual public zeromq::Message
{
public:
    const Frame& at(const std::size_t index) const final;
    FrameIterator begin() const final;
    const FrameSection Body() const final;
    const Frame& Body_at(const std::size_t index) const final;
    FrameIterator Body_begin() const final;
    FrameIterator Body_end() const final;
    FrameIterator end() const final;
    const FrameSection Header() const final;
    const Frame& Header_at(const std::size_t index) const final;
    FrameIterator Header_begin() const final;
    FrameIterator Header_end() const final;
    std::size_t size() const final;

    Frame& AddFrame() final;
    Frame& AddFrame(const ProtobufType& input) final;
    Frame& AddFrame(const void* input, const std::size_t size) final;
    Frame& at(const std::size_t index) final;

    void EnsureDelimiter() final;
    void PrependEmptyFrame() final;

    ~Message() override = default;

protected:
    std::vector<OTZMQFrame> messages_{};

    std::size_t body_position() const;

    bool set_field(const std::size_t position, const zeromq::Frame& input);

    Message();
    Message(const Message& rhs);

private:
    friend opentxs::Factory;
    friend network::zeromq::Message;

    Message* clone() const override { return new Message(*this); }
    bool hasDivider() const;
    std::size_t findDivider() const;

    Message(Message&&) = delete;
    Message& operator=(const Message&) = delete;
    Message& operator=(Message&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
