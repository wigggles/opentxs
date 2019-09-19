// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "internal/api/Api.hpp"

#ifdef ANDROID
#include <android/log.h>
#endif

#include <future>

#include "Log.hpp"

#define LOG_SINK "inproc://opentxs/logsink/1"

namespace zmq = opentxs::network::zeromq;

namespace opentxs
{
api::internal::Log* Factory::Log(
    const zmq::Context& zmq,
    const std::string& endpoint)
{
    return new api::implementation::Log(zmq, endpoint);
}
}  // namespace opentxs

namespace opentxs::api::implementation
{
Log::Log(const zmq::Context& zmq, const std::string& endpoint)
    : callback_(zmq::ListenCallback::Factory(
          std::bind(&Log::callback, this, std::placeholders::_1)))
    , socket_(zmq.PullSocket(callback_, zmq::socket::Socket::Direction::Bind))
    , publish_socket_(zmq.PublishSocket())
    , publish_{!endpoint.empty()}
{
    const auto started = socket_->Start(LOG_SINK);

    if (false == started) { abort(); }

    if (publish_) {
        const auto publishStarted = publish_socket_->Start(endpoint);
        if (false == publishStarted) { abort(); }
    }
}

void Log::callback(zmq::Message& message)
{
    if (message.Body().size() < 3) { return; }

    int level{-1};
    const auto& levelFrame = message.Body_at(0);
    const auto& messageFrame = message.Body_at(1);
    const auto& id = message.Body_at(2);
    OTPassword::safe_memcpy(
        &level, sizeof(level), levelFrame.data(), levelFrame.size());
#ifdef ANDROID
    print_android(level, messageFrame, id);
#else
    print(level, messageFrame, id);
#endif

    if (publish_) { publish_socket_->Send(message); }

    if (message.Body().size() >= 4) {
        const auto& promiseFrame = message.Body_at(3);
        auto* pPromise = promiseFrame.as<std::promise<void>*>();

        if (nullptr != pPromise) { pPromise->set_value(); }
    }
}

void Log::print(
    const int level,
    const std::string& text,
    const std::string& thread)
{
    if (false == text.empty()) {
        std::cerr << "(" << thread << ") ";
        std::cerr << text << std::endl;
        std::cerr.flush();
    }
}

#ifdef ANDROID
void Log::print_android(
    const int level,
    const std::string& text,
    const std::string& thread)
{
    switch (level) {
        case 0:
        case 1: {
            __android_log_write(ANDROID_LOG_INFO, "OT Output", text.c_str());
        } break;
        case 2:
        case 3: {
            __android_log_write(ANDROID_LOG_DEBUG, "OT Debug", text.c_str());
        } break;
        case 4:
        case 5: {
            __android_log_write(
                ANDROID_LOG_VERBOSE, "OT Verbose", text.c_str());
        } break;
        default: {
            __android_log_write(
                ANDROID_LOG_UNKNOWN, "OT Unknown", text.c_str());
        } break;
    }
}
#endif
}  // namespace opentxs::api::implementation
