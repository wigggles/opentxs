// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_PIPELINE_HPP
#define OPENTXS_NETWORK_ZEROMQ_PIPELINE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/Socket.hpp"

#ifdef SWIG
// clang-format off
%rename(ZMQPipeline) opentxs::network::zeromq::Pipeline;
%template(OTZMQPipeline) opentxs::Pimpl<opentxs::network::zeromq::Pipeline>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Pipeline
{
public:
    EXPORT virtual bool Close() const = 0;
    EXPORT virtual bool Push(const std::string& data) const = 0;
    EXPORT virtual bool Push(const opentxs::Data& data) const = 0;
    EXPORT virtual bool Push(network::zeromq::Message& data) const = 0;

    EXPORT virtual ~Pipeline() = default;

protected:
    Pipeline() = default;

private:
    friend OTZMQPipeline;

    virtual Pipeline* clone() const = 0;

    Pipeline(const Pipeline&) = delete;
    Pipeline(Pipeline&&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline& operator=(Pipeline&&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
