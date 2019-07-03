// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_FRAME_HPP
#define OPENTXS_NETWORK_ZEROMQ_FRAME_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#include <string>

struct zmq_msg_t;

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::Frame::data;
%ignore opentxs::network::zeromq::Frame::operator zmq_msg_t*;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Frame>::Pimpl(opentxs::network::zeromq::Frame const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::Frame>::operator opentxs::network::zeromq::Frame&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Frame>::operator const opentxs::network::zeromq::Frame &;
%rename(string) opentxs::network::zeromq::Frame::operator std::string() const;
%rename(assign) operator=(const opentxs::network::zeromq::Frame&);
%rename(ZMQFrame) opentxs::network::zeromq::Frame;
%template(OTZMQFrame) opentxs::Pimpl<opentxs::network::zeromq::Frame>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Frame
{
public:
    EXPORT static Pimpl<opentxs::network::zeromq::Frame> Factory();
    EXPORT static Pimpl<opentxs::network::zeromq::Frame> Factory(
        const opentxs::Data& input);
    EXPORT static Pimpl<opentxs::network::zeromq::Frame> Factory(
        const std::string& input);
    EXPORT static Pimpl<opentxs::network::zeromq::Frame> Factory(
        const ProtobufType& input);

    EXPORT virtual operator std::string() const = 0;

    EXPORT virtual const void* data() const = 0;
    EXPORT virtual std::size_t size() const = 0;

    EXPORT virtual operator zmq_msg_t*() = 0;

    EXPORT virtual ~Frame() = default;

protected:
    Frame() = default;

private:
    friend OTZMQFrame;

    EXPORT virtual Frame* clone() const = 0;

    Frame(const Frame&) = delete;
    Frame(Frame&&) = delete;
    Frame& operator=(Frame&&) = delete;
    Frame& operator=(const Frame&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
