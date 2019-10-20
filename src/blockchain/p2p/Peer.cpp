// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "internal/api/Api.hpp"
#include "internal/blockchain/Blockchain.hpp"

#include <functional>

#include "Peer.hpp"

#define OT_BLOCKCHAIN_PEER_PING_SECONDS 30
#define OT_BLOCKCHAIN_PEER_DISCONNECT_SECONDS 40
#define OT_BLOCKCHAIN_PEER_DOWNLOAD_ADDRESSES_MINUTES 10

#define OT_METHOD "opentxs::blockchain::p2p::implementation::Peer::"

namespace opentxs::blockchain::p2p::implementation
{
Peer::Peer(
    const api::internal::Core& api,
    const client::internal::Network& network,
    const client::internal::PeerManager& manager,
    const int id,
    const std::size_t headerSize,
    const std::size_t bodySize,
    std::unique_ptr<internal::Address> address,
    boost::asio::io_context& context) noexcept
    : StateMachine(std::bind(&Peer::state_machine, this))
    , api_(api)
    , network_(network)
    , manager_(manager)
    , id_(id)
    , send_promise_()
    , send_future_(send_promise_.get_future())
    , socket_(context)
    , address_(std::move(address))
    , download_peers_()
    , endpoint_(
          make_endpoint(address_.Type(), address_.Bytes(), address_.Port()))
    , header_(make_buffer(headerSize))
    , body_(Data::Factory())
    , body_bytes_(0)
    , outgoing_handshake_(false)
    , incoming_handshake_(false)
    , handshake_promise_()
    , handshake_(handshake_promise_.get_future())
    , shutdown_()
    , context_(context)
    , send_(api.Factory().Pipeline(
          std::bind(&Peer::transmit, this, std::placeholders::_1)))
    , process_(api.Factory().Pipeline(
          std::bind(&Peer::process_message, this, std::placeholders::_1)))
    , running_(true)
    , connection_promise_()
    , connected_(connection_promise_.get_future())
    , send_promises_()
    , activity_()
    , state_(State::Handshake)
{
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

void Peer::Activity::Bump() noexcept
{
    Lock lock(lock_);
    activity_ = Clock::now();
}

Time Peer::Activity::get() const noexcept
{
    Lock lock(lock_);

    return activity_;
}

OTData Peer::Address::Bytes() const noexcept
{
    Lock lock(lock_);

    return address_->Bytes();
}

blockchain::Type Peer::Address::Chain() const noexcept
{
    Lock lock(lock_);

    return address_->Chain();
}

std::string Peer::Address::Display() const noexcept
{
    Lock lock(lock_);

    return address_->Display();
}

OTIdentifier Peer::Address::ID() const noexcept
{
    Lock lock(lock_);

    return address_->ID();
}

std::uint16_t Peer::Address::Port() const noexcept
{
    Lock lock(lock_);

    return address_->Port();
}

Network Peer::Address::Type() const noexcept
{
    Lock lock(lock_);

    return address_->Type();
}

Peer::Address::pointer Peer::Address::UpdateServices(
    const std::set<p2p::Service>& services) noexcept
{
    Lock lock(lock_);
    address_->SetServices(services);

    return address_->clone_internal();
}

Peer::Address::pointer Peer::Address::UpdateTime(const Time& time) noexcept
{
    Lock lock(lock_);
    address_->SetLastConnected(time);

    return address_->clone_internal();
}

void Peer::DownloadPeers::Bump() noexcept
{
    Lock lock(lock_);
    downloaded_ = Clock::now();
}

Time Peer::DownloadPeers::get() const noexcept
{
    Lock lock(lock_);

    return downloaded_;
}

std::pair<std::future<bool>, int> Peer::SendPromises::NewPromise()
{
    Lock lock(lock_);
    auto [it, added] = map_.emplace(++counter_, std::promise<bool>());

    if (false == added) { return {}; }

    return {it->second.get_future(), it->first};
}

void Peer::SendPromises::SetPromise(const int promise, const bool value)
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

void Peer::check_activity() noexcept
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

void Peer::check_download_peers() noexcept
{
    const auto interval = Clock::now() - download_peers_.get();
    const bool download =
        std::chrono::minutes(OT_BLOCKCHAIN_PEER_DOWNLOAD_ADDRESSES_MINUTES) <=
        interval;

    if (download) { request_addresses(); }
}

void Peer::check_handshake() noexcept
{
    if (outgoing_handshake_ && incoming_handshake_) {
        try {
            state_.store(State::Run);
            handshake_promise_.set_value();
            update_address_activity();
            LogNormal("Connected to ")(blockchain::internal::DisplayString(
                address_.Chain()))(" peer at ")(address_.Display())
                .Flush();
        } catch (...) {
        }

        request_headers();
        request_addresses();
    }
}

void Peer::cleanup() noexcept
{
    send_->Close();
    process_->Close();
}

void Peer::connect() noexcept
{
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Connecting to ")(
        endpoint_.address().to_string())(":")(endpoint_.port())
        .Flush();
    socket_.async_connect(
        endpoint_,
        std::bind(&Peer::connect_handler, this, std::placeholders::_1));
}

void Peer::connect_handler(const boost::system::error_code& error) noexcept
{
    if (error) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error.message()).Flush();
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

void Peer::disconnect() noexcept
{
    handshake_promise_ = {};
    connection_promise_ = {};
    manager_.Disconnect(id_);
}

void Peer::handshake() noexcept
{
    start_handshake();
    auto handshakeDone{handshake_};

    try {
        handshakeDone.get();
    } catch (...) {
        disconnect();
    }
}

void Peer::init() noexcept
{
    Trigger();
    connect();
}

void Peer::init_send_promise() noexcept
{
    send_promise_ = SendPromise{};
    send_future_ = send_promise_.get_future();
}

OTData Peer::make_buffer(const std::size_t size) noexcept
{
    auto output = Data::Factory();
    output->SetSize(size);

    return output;
}

tcp::endpoint Peer::make_endpoint(
    const Network type,
    const Data& raw,
    const std::uint16_t port) noexcept
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

void Peer::read_body() noexcept
{
    asio::async_read(
        socket_,
        asio::buffer(body_->data(), body_->size()),
        std::bind(&Peer::receive_body, this, std::placeholders::_1));
}

void Peer::read_header() noexcept
{
    asio::async_read(
        socket_,
        asio::buffer(header_->data(), header_->size()),
        std::bind(&Peer::receive_header, this, std::placeholders::_1));
}

void Peer::receive_body(const boost::system::error_code& error) noexcept
{
    if (error) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error.message()).Flush();
        disconnect();
    } else {
        auto body = Data::Factory();

        if (body_->Extract(body_bytes_, body)) {
            auto message = zmq::Message::Factory();
            message->AddFrame(header_);
            message->AddFrame(body);
            process_->Push(message);
        }
    }

    body_ = Data::Factory();
    run();
}

void Peer::receive_header(const boost::system::error_code& error) noexcept
{
    if (error) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error.message()).Flush();
        disconnect();
    } else {
        activity_.Bump();
        get_body_size();

        if (0 < body_bytes_) {
            body_->SetSize(body_bytes_);
            read_body();
        } else {
            auto message = zmq::Message::Factory();
            message->AddFrame();
            message->AddFrame(header_);
            message->AddFrame();
            process_->Push(message);
            body_ = Data::Factory();
            run();
        }
    }
}

void Peer::run() noexcept
{
    Lock lock(shutdown_);

    if (running_.load()) { read_header(); }
}

Peer::SendStatus Peer::send(OTData in) noexcept
{
    try {
        if (false == connected_.get()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unable to send to disconnected peer")
                .Flush();

            return {};
        }
    } catch (...) {
        return {};
    }

    Lock lock(shutdown_);

    if (running_.load()) {
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

bool Peer::state_machine() noexcept
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

void Peer::transmit(zmq::Message& message) noexcept
{
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
        while (running_.load()) {
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

void Peer::update_address_activity() noexcept
{
    network_.Database().AddOrUpdate(address_.UpdateTime(activity_.get()));
}

void Peer::update_address_services(
    const std::set<p2p::Service>& services) noexcept
{
    network_.Database().AddOrUpdate(address_.UpdateServices(services));
}

Peer::~Peer()
{
    Stop().get();

    Lock lock(shutdown_);
    running_.store(false);
    lock.unlock();

    try {
        socket_.shutdown(tcp::socket::shutdown_receive);
    } catch (...) {
    }

    try {
        socket_.shutdown(tcp::socket::shutdown_send);
    } catch (...) {
    }

    try {
        socket_.close();
    } catch (...) {
    }

    LogNormal("Disconnected from ")(address_.Display()).Flush();
    handshake_promise_ = std::promise<void>{};

    if (State::Handshake != state_.load()) { update_address_activity(); }

    try {
        send_promise_ = SendPromise{};
        send_future_ = SendFuture{};
    } catch (...) {
    }
}
}  // namespace opentxs::blockchain::p2p::implementation
