// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "internal/api/Api.hpp"
#include "internal/blockchain/Blockchain.hpp"

#include <functional>

#include "Peer.hpp"

#define OT_BLOCKCHAIN_PEER_PING_SECONDS 30
#define OT_BLOCKCHAIN_PEER_DISCONNECT_SECONDS 40
#define OT_BLOCKCHAIN_PEER_DOWNLOAD_ADDRESSES_MINUTES 10

#define OT_METHOD "opentxs::blockchain::p2p::implementation::Peer::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::p2p::implementation
{
Peer::Peer(
    const api::internal::Core& api,
    const client::internal::Network& network,
    const client::internal::PeerManager& manager,
    const std::string& shutdown,
    const int id,
    const std::size_t headerSize,
    const std::size_t bodySize,
    std::unique_ptr<internal::Address> address,
    boost::asio::io_context& context) noexcept
    : StateMachine(std::bind(&Peer::state_machine, this))
    , api_(api)
    , network_(network)
    , manager_(manager)
    , endpoint_(
          make_endpoint(address->Type(), address->Bytes(), address->Port()))
    , running_(Flag::Factory(true))
    , send_promise_()
    , send_future_(send_promise_.get_future())
    , address_(std::move(address))
    , download_peers_()
    , header_(make_buffer(headerSize))
    , body_bytes_(0)
    , outgoing_handshake_(false)
    , incoming_handshake_(false)
    , header_worker_(api, [this](auto& in) { request_headers(); })
    , cfheader_worker_(api, [this](auto& in) { request_cfheaders(in); })
    , cfilter_worker_(api, [this](auto& in) { request_cfilter(in); })
    , id_(id)
    , context_(context)
    , socket_(context_)
    , send_(api.Factory().Pipeline(
          std::bind(&Peer::transmit, this, std::placeholders::_1)))
    , process_(api.Factory().Pipeline(
          std::bind(&Peer::process_message, this, std::placeholders::_1)))
    , incoming_body_(Data::Factory())
    , outgoing_message_(Data::Factory())
    , connection_promise_()
    , connected_(connection_promise_.get_future())
    , handshake_promise_()
    , handshake_(handshake_promise_.get_future())
    , send_promises_()
    , activity_()
    , state_(State::Handshake)
    , heartbeat_callback_(
          zmq::ListenCallback::Factory([this](auto&) { Trigger(); }))
    , heartbeat_(api.ZeroMQ().SubscribeSocket(heartbeat_callback_))
    , shutdown_(
          api.ZeroMQ(),
          {api.Endpoints().Shutdown(), shutdown},
          [this](auto& promise) { this->shutdown(promise); })
{
    auto listening = heartbeat_->Start(manager.Endpoint(Task::Heartbeat));

    OT_ASSERT(listening);
}

Peer::Activity::Activity() noexcept
    : lock_()
    , activity_(Clock::now())
{
}

Peer::Address::Address(std::unique_ptr<internal::Address> address) noexcept
    : lock_()
    , address_(std::move(address))
{
    OT_ASSERT(address_);
}

Peer::DownloadPeers::DownloadPeers() noexcept
    : lock_()
    , downloaded_(Clock::now())
{
}

Peer::SendPromises::SendPromises() noexcept
    : lock_()
    , counter_(0)
    , map_()
{
}

Peer::Worker::Worker(
    const api::internal::Core& api,
    zmq::ListenCallback::ReceiveCallback callback) noexcept
    : callback_(zmq::ListenCallback::Factory(callback))
    , socket_(api.ZeroMQ().PullSocket(
          callback_,
          zmq::socket::Socket::Direction::Connect))
{
}

auto Peer::Activity::Bump() noexcept -> void
{
    Lock lock(lock_);
    activity_ = Clock::now();
}

auto Peer::Activity::get() const noexcept -> Time
{
    Lock lock(lock_);

    return activity_;
}

auto Peer::Address::Bytes() const noexcept -> OTData
{
    Lock lock(lock_);

    return address_->Bytes();
}

auto Peer::Address::Chain() const noexcept -> blockchain::Type
{
    Lock lock(lock_);

    return address_->Chain();
}

auto Peer::Address::Display() const noexcept -> std::string
{
    Lock lock(lock_);

    return address_->Display();
}

auto Peer::Address::ID() const noexcept -> OTIdentifier
{
    Lock lock(lock_);

    return address_->ID();
}

auto Peer::Address::Port() const noexcept -> std::uint16_t
{
    Lock lock(lock_);

    return address_->Port();
}

auto Peer::Address::Services() const noexcept -> std::set<Service>
{
    Lock lock(lock_);

    return address_->Services();
}

auto Peer::Address::Type() const noexcept -> Network
{
    Lock lock(lock_);

    return address_->Type();
}

auto Peer::Address::UpdateServices(
    const std::set<p2p::Service>& services) noexcept -> pointer
{
    Lock lock(lock_);
    address_->SetServices(services);

    return address_->clone_internal();
}

auto Peer::Address::UpdateTime(const Time& time) noexcept -> pointer
{
    Lock lock(lock_);
    address_->SetLastConnected(time);

    return address_->clone_internal();
}

auto Peer::DownloadPeers::Bump() noexcept -> void
{
    Lock lock(lock_);
    downloaded_ = Clock::now();
}

auto Peer::DownloadPeers::get() const noexcept -> Time
{
    Lock lock(lock_);

    return downloaded_;
}

auto Peer::SendPromises::Break() -> void
{
    Lock lock(lock_);

    for (auto& [id, promise] : map_) { promise = {}; }
}

auto Peer::SendPromises::NewPromise() -> std::pair<std::future<bool>, int>
{
    Lock lock(lock_);
    auto [it, added] = map_.emplace(++counter_, std::promise<bool>());

    if (false == added) { return {}; }

    return {it->second.get_future(), it->first};
}

auto Peer::SendPromises::SetPromise(const int promise, const bool value) -> void
{
    Lock lock(lock_);

    auto it = map_.find(promise);

    if (map_.end() != it) {
        try {
            it->second.set_value(value);
        } catch (...) {
        }

        map_.erase(it);
    }
}

auto Peer::break_promises() noexcept -> void
{
    handshake_promise_ = {};
    connection_promise_ = {};
    send_promise_ = {};
    send_promises_.Break();
}

auto Peer::check_activity() noexcept -> void
{
    const auto interval = Clock::now() - activity_.get();
    const bool disconnect =
        std::chrono::seconds(OT_BLOCKCHAIN_PEER_DISCONNECT_SECONDS) <= interval;
    const bool ping =
        std::chrono::seconds(OT_BLOCKCHAIN_PEER_PING_SECONDS) <= interval;

    if (disconnect) {
        this->disconnect();
    } else if (ping) {
        this->ping();
    }
}

auto Peer::check_download_peers() noexcept -> void
{
    const auto interval = Clock::now() - download_peers_.get();
    const bool download =
        std::chrono::minutes(OT_BLOCKCHAIN_PEER_DOWNLOAD_ADDRESSES_MINUTES) <=
        interval;

    if (download) { request_addresses(); }
}

auto Peer::check_handshake() noexcept -> void
{
    if (outgoing_handshake_ && incoming_handshake_) {
        try {
            state_.store(State::Run);
            handshake_promise_.set_value();
            update_address_activity();
            LogNormal("Connected to ")(blockchain::internal::DisplayString(
                address_.Chain()))(" peer at ")(address_.Display())
                .Flush();
            LogNormal("Advertised services: ").Flush();

            for (const auto& service : address_.Services()) {
                LogNormal(" * ")(p2p::DisplayService(service)).Flush();
            }

        } catch (...) {
        }

        request_headers();
        request_addresses();
    }
}

auto Peer::connect() noexcept -> void
{
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Connecting to ")(
        endpoint_.address().to_string())(":")(endpoint_.port())
        .Flush();
    socket_.async_connect(
        endpoint_,
        std::bind(&Peer::connect_handler, this, std::placeholders::_1));
}

auto Peer::connect_handler(const boost::system::error_code& error) noexcept
    -> void
{
    if (error) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(error.message()).Flush();
        connection_promise_.set_value(false);
        disconnect();
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Connect to ")(
            endpoint_.address().to_string())(":")(endpoint_.port())(
            " successful")
            .Flush();
        connection_promise_.set_value(true);
        run();
    }
}

auto Peer::disconnect() noexcept -> void { manager_.Disconnect(id_); }

auto Peer::handshake() noexcept -> void
{
    static const auto limit = std::chrono::seconds(15);
    static const auto wait = std::chrono::milliseconds(10);
    start_handshake();
    auto disconnect{true};
    auto done{handshake_};
    const auto start = Clock::now();

    try {
        while (running_.get() && (limit > (Clock::now() - start))) {
            if (std::future_status::ready == done.wait_for(wait)) {
                disconnect = false;
                break;
            }
        }
    } catch (...) {
    }

    if (disconnect) { this->disconnect(); }
}

auto Peer::init() noexcept -> void
{
    Trigger();
    connect();
}

auto Peer::init_send_promise() noexcept -> void
{
    send_promise_ = SendPromise{};
    send_future_ = send_promise_.get_future();
}

auto Peer::make_buffer(const std::size_t size) noexcept -> OTData
{
    auto output = Data::Factory();
    output->SetSize(size);

    return output;
}

auto Peer::make_endpoint(
    const Network type,
    const Data& raw,
    const std::uint16_t port) noexcept -> tcp::endpoint
{
    ip::address_v6 output{};

    switch (type) {
        case p2p::Network::ipv6: {
            ip::address_v6::bytes_type bytes{};

            OT_ASSERT(bytes.size() == raw.size());

            std::memcpy(&bytes, raw.data(), bytes.size());
            output = ip::make_address_v6(bytes);
        } break;
        case p2p::Network::ipv4: {
            ip::address_v4 output4{};
            ip::address_v4::bytes_type bytes{};

            OT_ASSERT(bytes.size() == raw.size());

            std::memcpy(&bytes, raw.data(), bytes.size());
            output4 = ip::make_address_v4(bytes);
            output = ip::make_address_v6(
                std::string("::ffff:") + output4.to_string());
        } break;
        default: {
            OT_FAIL;
        }
    }

    return ip::tcp::endpoint(output, port);
}

auto Peer::read_body() noexcept -> void
{
    if (running_.get()) {
        asio::async_read(
            socket_,
            asio::buffer(incoming_body_->data(), incoming_body_->size()),
            std::bind(&Peer::receive_body, this, std::placeholders::_1));
    }
}

auto Peer::read_header() noexcept -> void
{
    if (running_.get()) {
        asio::async_read(
            socket_,
            asio::buffer(header_->data(), header_->size()),
            std::bind(&Peer::receive_header, this, std::placeholders::_1));
    }
}

auto Peer::receive_body(const boost::system::error_code& error) noexcept -> void
{
    if (error) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(error.message()).Flush();
        disconnect();
    } else {
        auto body = Data::Factory();

        if (incoming_body_->Extract(body_bytes_, body)) {
            auto message = zmq::Message::Factory();
            message->AddFrame(header_);
            message->AddFrame(body);
            process_->Push(message);
        }
    }

    incoming_body_ = Data::Factory();
    run();
}

auto Peer::receive_header(const boost::system::error_code& error) noexcept
    -> void
{
    if (error) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(error.message()).Flush();
        disconnect();
    } else {
        activity_.Bump();
        get_body_size();

        if (0 < body_bytes_) {
            incoming_body_->SetSize(body_bytes_);
            read_body();
        } else {
            auto message = zmq::Message::Factory();
            message->AddFrame();
            message->AddFrame(header_);
            message->AddFrame();
            process_->Push(message);
            incoming_body_ = Data::Factory();
            run();
        }
    }
}

auto Peer::run() noexcept -> void
{
    if (running_.get()) { read_header(); }
}

auto Peer::send(OTData in) noexcept -> SendStatus
{
    try {
        if (false == connected_.get()) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(
                ": Unable to send to disconnected peer")
                .Flush();

            return {};
        }
    } catch (...) {
        return {};
    }

    if (running_.get()) {
        auto [future, promise] = send_promises_.NewPromise();
        auto message = zmq::Message::Factory();
        message->AddFrame(in);
        message->AddFrame(Data::Factory(&promise, sizeof(promise)));
        send_->Push(message);

        return std::move(future);
    } else {
        return {};
    }
}

auto Peer::Shutdown() noexcept -> std::shared_future<void>
{
    shutdown_.Close();

    if (running_.get()) { shutdown(shutdown_.promise_); }

    return shutdown_.future_;
}

auto Peer::shutdown(std::promise<void>& promise) noexcept -> void
{
    running_->Off();
    Stop().get();

    try {
        socket_.shutdown(tcp::socket::shutdown_both);
    } catch (...) {
    }

    heartbeat_->Close();
    cfilter_worker_.Stop();
    header_worker_.Stop();
    send_->Close();
    process_->Close();
    break_promises();

    try {
        socket_.close();
    } catch (...) {
    }

    Sleep(std::chrono::milliseconds(100));

    if (State::Handshake != state_.load()) { update_address_activity(); }

    LogVerbose("Disconnected from ")(address_.Display()).Flush();

    try {
        promise.set_value();
    } catch (...) {
    }
}

auto Peer::state_machine() noexcept -> bool
{
    switch (state_.load()) {
        case State::Handshake: {
            handshake();
        } break;
        case State::Run: {
            check_activity();
            check_download_peers();
            [[fallthrough]];
        }
        default: {
        }
    }

    return false;
}

auto Peer::transmit(zmq::Message& message) noexcept -> void
{
    if (false == running_.get()) { return; }

    struct Cleanup {
        Cleanup(const bool& sucess, const zmq::Frame& frame, Peer& parent)
            : success_(sucess)
            , parent_(parent)
            , index_(0)
        {
            OT_ASSERT(sizeof(index_) == frame.size());

            std::memcpy(&index_, frame.data(), frame.size());
        }

        ~Cleanup() { parent_.send_promises_.SetPromise(index_, success_); }

    private:
        const bool& success_;
        Peer& parent_;
        int index_;
    };

    if (2 < message.Body().size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        return;
    }

    const auto& payload = message.Body_at(0);
    const auto& promiseFrame = message.Body_at(1);
    auto success = bool{false};
    const Cleanup cleanup{success, promiseFrame, *this};
    LogTrace(OT_METHOD)(__FUNCTION__)(": Sending ")(payload.size())(
        " byte message:")
        .Flush();
    LogTrace(Data::Factory(message.at(0))->asHex()).Flush();
    init_send_promise();
    auto work = [this, &payload]() -> void {
        auto cb = [this](auto& error, auto bytes) -> void {
            try {
                this->send_promise_.set_value({error, bytes});
            } catch (...) {
            }
        };
        asio::async_write(
            this->socket_, asio::buffer(payload.data(), payload.size()), cb);
    };

    context_.post(work);
    auto result = SendResult{};

    try {
        while (running_.get()) {
            const auto status =
                send_future_.wait_for(std::chrono::milliseconds(5));

            if (std::future_status::ready == status) {
                result = send_future_.get();
                break;
            }
        }
    } catch (...) {
        return;
    }

    const auto& [error, bytes] = result;

    if (error) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error.message()).Flush();
        success = false;
        disconnect();
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Sent ")(bytes)(" bytes").Flush();
        success = true;
    }
}

auto Peer::update_address_activity() noexcept -> void
{
    manager_.Database().AddOrUpdate(address_.UpdateTime(activity_.get()));
}

auto Peer::update_address_services(
    const std::set<p2p::Service>& services) noexcept -> void
{
    manager_.Database().AddOrUpdate(address_.UpdateServices(services));
}

Peer::~Peer()
{
    Shutdown().wait_for(std::chrono::seconds(1));
    shutdown_.Close();
}
}  // namespace opentxs::blockchain::p2p::implementation
