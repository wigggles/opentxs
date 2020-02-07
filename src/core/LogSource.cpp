// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/LogSource.hpp"

#include "opentxs/api/Context.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/OT.hpp"

#include <boost/stacktrace.hpp>

#include <chrono>
#include <future>
#include <sstream>

#define LOG_SINK "inproc://opentxs/logsink/1"

namespace zmq = opentxs::network::zeromq;

namespace opentxs
{
std::string stack_trace() noexcept
{
    auto output = std::stringstream{};
    output << boost::stacktrace::stacktrace();

    return output.str();
}

std::atomic<int> LogSource::verbosity_{0};
std::atomic<bool> LogSource::running_{true};
std::mutex LogSource::buffer_lock_{};
std::map<std::thread::id, LogSource::Source> LogSource::buffer_{};

LogSource::LogSource(const int logLevel) noexcept
    : level_(logLevel)
{
}

const LogSource& LogSource::operator()() const noexcept { return *this; }

const LogSource& LogSource::operator()(char* in) const noexcept
{
    return operator()(std::string(in));
}

const LogSource& LogSource::operator()(const char* in) const noexcept
{
    if (verbosity_.load() < level_) { return *this; }

    std::string id{};

    if (running_.load()) { std::get<1>(get_buffer(id)) << in; }

    return *this;
}

const LogSource& LogSource::operator()(const std::string& in) const noexcept
{
    return operator()(in.c_str());
}

const LogSource& LogSource::operator()(const OTString& in) const noexcept
{
    return operator()(in.get());
}

const LogSource& LogSource::operator()(const OTStringXML& in) const noexcept
{
    return operator()(in.get());
}

const LogSource& LogSource::operator()(const OTArmored& in) const noexcept
{
    return operator()(in.get());
}

const LogSource& LogSource::operator()(const String& in) const noexcept
{
    return operator()(in.Get());
}

const LogSource& LogSource::operator()(const StringXML& in) const noexcept
{
    return operator()(in.Get());
}

const LogSource& LogSource::operator()(const Armored& in) const noexcept
{
    return operator()(in.Get());
}

const LogSource& LogSource::operator()(const OTIdentifier& in) const noexcept
{
    return operator()(in.get());
}

const LogSource& LogSource::operator()(const Identifier& in) const noexcept
{
    return operator()(in.str().c_str());
}

const LogSource& LogSource::operator()(const OTNymID& in) const noexcept
{
    return operator()(in.get());
}

const LogSource& LogSource::operator()(const identifier::Nym& in) const noexcept
{
    return operator()(in.str().c_str());
}

const LogSource& LogSource::operator()(const OTServerID& in) const noexcept
{
    return operator()(in.get());
}

const LogSource& LogSource::operator()(const identifier::Server& in) const
    noexcept
{
    return operator()(in.str().c_str());
}

const LogSource& LogSource::operator()(const OTUnitID& in) const noexcept
{
    return operator()(in.get());
}

const LogSource& LogSource::operator()(
    const identifier::UnitDefinition& in) const noexcept
{
    return operator()(in.str().c_str());
}

const LogSource& LogSource::operator()(const Time in) const noexcept
{
    return operator()(formatTimestamp(in));
}

void LogSource::Assert(
    const char* file,
    const std::size_t line,
    const char* message) const noexcept
{
    {
        std::string id{};
        auto& [socket, buffer] = get_buffer(id);
        buffer = std::stringstream{};
        buffer << "OT ASSERT";

        if (nullptr != file) { buffer << " in " << file << " line " << line; }

        if (nullptr != message) { buffer << ": " << message; }

        buffer << "\n" << boost::stacktrace::stacktrace();
    }

    send(true);
    abort();
}

void LogSource::Flush() const noexcept { send(false); }

LogSource::Source& LogSource::get_buffer(std::string& out) noexcept
{
    const auto id = std::this_thread::get_id();
    std::stringstream convert{};
    convert << std::hex << id;
    out = convert.str();
    auto it = buffer_.find(id);

    if (buffer_.end() == it) {
        Lock lock(buffer_lock_);
        auto inner = buffer_.emplace(
            id,
            Source{Context().ZMQ().PushSocket(
                       zmq::socket::Socket::Direction::Connect),
                   std::stringstream{}});
        auto& source = std::get<0>(inner)->second;
        auto& socket = std::get<0>(source).get();
        socket.Start(LOG_SINK);

        return source;
    }

    return it->second;
}

void LogSource::send(const bool terminate) const noexcept
{
    if (running_.load()) {
        std::string id{};
        auto& [socket, buffer] = get_buffer(id);
        auto message = zmq::Message::Factory();
        message->PrependEmptyFrame();
        message->AddFrame(std::to_string(level_));
        message->AddFrame(buffer.str());
        message->AddFrame(id);
        auto promise = std::promise<void>{};
        auto future = promise.get_future();
        const auto* pPromise = &promise;

        if (terminate) {
            message->AddFrame(&pPromise, sizeof(pPromise));
        } else {
            promise.set_value();
        }

        socket->Send(message);
        buffer = std::stringstream{};
        future.wait_for(std::chrono::seconds(10));
    }

    if (terminate) { abort(); }
}

void LogSource::SetVerbosity(const int level) noexcept
{
    verbosity_.store(level);
}

void LogSource::Shutdown() noexcept
{
    running_.store(false);
    buffer_.clear();
}

const LogSource& LogSource::StartLog(
    const LogSource& source,
    const std::string& function) noexcept
{
    return source(function);
}

void LogSource::Trace(
    const char* file,
    const std::size_t line,
    const char* message) const noexcept
{
    {
        std::string id{};
        auto& [socket, buffer] = get_buffer(id);
        buffer = std::stringstream{};
        buffer << "Stack trace requested";

        if (nullptr != file) { buffer << " in " << file << " line " << line; }

        if (nullptr != message) { buffer << ": " << message; }

        buffer << "\n" << stack_trace();
    }

    send(false);
}
}  // namespace opentxs
