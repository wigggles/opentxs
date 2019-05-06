// Copyright (c) 2018 The Open-Transactions developers
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
    const Frame& at(const std::size_t index) const override;
    FrameIterator begin() const override;
    const FrameSection Body() const override;
    const Frame& Body_at(const std::size_t index) const override;
    FrameIterator Body_begin() const override;
    FrameIterator Body_end() const override;
    FrameIterator end() const override;
    const FrameSection Header() const override;
    const Frame& Header_at(const std::size_t index) const override;
    FrameIterator Header_begin() const override;
    FrameIterator Header_end() const override;
    std::size_t size() const override;

    Frame& AddFrame() override;
    Frame& AddFrame(const opentxs::Data& input) override;
    Frame& AddFrame(const std::string& input) override;
    Frame& at(const std::size_t index) override;

    void EnsureDelimiter() override;
    void PrependEmptyFrame() override;

    virtual ~Message() = default;

protected:
    std::vector<OTZMQFrame> messages_{};

    std::size_t body_position() const;

    template <typename I>
    bool set_field(const std::size_t position, const I& input)
    {
        const auto effectivePosition = body_position() + position;

        if (effectivePosition >= messages_.size()) { return false; }

        messages_[effectivePosition] = Frame::Factory(input);

        return true;
    }

    Message();
    Message(const Message& rhs);

private:
    friend network::zeromq::Message;

    Message* clone() const override { return new Message(*this); }
    bool hasDivider() const;
    std::size_t findDivider() const;

    Message(Message&&) = delete;
    Message& operator=(const Message&) = delete;
    Message& operator=(Message&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
