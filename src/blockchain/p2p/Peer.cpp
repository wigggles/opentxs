// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "1_Internal.hpp"           // IWYU pragma: associated
#include "blockchain/p2p/Peer.hpp"  // IWYU pragma: associated

#include <boost/asio.hpp>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <type_traits>

#include "internal/api/client/Client.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

#define OT_BLOCKCHAIN_PEER_PING_SECONDS 30
#define OT_BLOCKCHAIN_PEER_DISCONNECT_SECONDS 40
#define OT_BLOCKCHAIN_PEER_DOWNLOAD_ADDRESSES_MINUTES 10

#define OT_METHOD "opentxs::blockchain::p2p::implementation::Peer::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::p2p::implementation
{
Peer::Peer(
    const api::client::Manager& api,
    const client::internal::Network& network,
    const client::internal::PeerManager& manager,
    const blockchain::client::internal::IO& context,
    const int id,
    const std::string& shutdown,
    const std::size_t headerSize,
    const std::size_t bodySize,
    std::unique_ptr<internal::Address> address) noexcept
    : Worker(api, std::chrono::milliseconds(10))
    , network_(network)
    , manager_(manager)
    , chain_(address->Chain())
    , endpoint_(
          make_endpoint(address->Type(), address->Bytes(), address->Port()))
    , send_promise_()
    , send_future_(send_promise_.get_future())
    , address_(std::move(address))
    , download_peers_()
    , header_(make_buffer(headerSize))
    , state_()
    , verify_filter_checkpoint_(
          api::client::blockchain::BlockStorage::All !=
          static_cast<const blockchain::client::internal::BlockDatabase&>(
              network.DB())
              .BlockPolicy())
    , header_bytes_(headerSize)
    , id_(id)
    , connection_id_()
    , shutdown_endpoint_(shutdown)
    , context_(context)
    , socket_(context_.operator boost::asio::io_context &())
    , outgoing_message_(Data::Factory())
    , connection_id_promise_()
    , send_promises_()
    , activity_()
    , init_promise_()
    , init_(init_promise_.get_future())
    , cb_(zmq::ListenCallback::Factory([&](auto& in) { pipeline_d(in); }))
    , dealer_(api.ZeroMQ().DealerSocket(
          cb_,
          zmq::socket::Socket::Direction::Connect))
{
    auto future = connection_id_promise_.get_future();
    auto zmq = dealer_->Start(api_.Endpoints().InternalBlockchainAsioContext());

    OT_ASSERT(zmq);

    auto message = MakeWork(OT_ZMQ_REGISTER_SIGNAL);
    message->AddFrame(id_);
    zmq = dealer_->Send(message);

    OT_ASSERT(zmq);

    const auto status = future.wait_for(std::chrono::seconds(10));

    if (std::future_status::ready != status) {
        LogNormal("Connection to peer ")(address_.Display())(
            " timed out during connect")
            .Flush();
        disconnect();
    }
}

auto Peer::break_promises() noexcept -> void
{
    state_.break_promises();
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
    auto& state = state_.handshake_;

    if (state.first_action_ && state.second_action_ &&
        (false == state.done())) {
        LogNormal("Connected to ")(blockchain::internal::DisplayString(
            address_.Chain()))(" peer at ")(address_.Display())
            .Flush();
        LogNormal("Advertised services: ").Flush();

        for (const auto& service : address_.Services()) {
            LogNormal(" * ")(p2p::DisplayService(service)).Flush();
        }

        update_address_activity();
        state.promise_.set_value();

        OT_ASSERT(state.done());
    }

    trigger();
}

auto Peer::check_verify() noexcept -> void
{
    auto& state = state_.verify_;

    if (state.first_action_ &&
        (state.second_action_ || (false == verify_filter_checkpoint_)) &&
        (false == state.done())) {
        state.promise_.set_value();
        network_.Blockchain().UpdatePeer(chain_, address_.Display());
    }

    trigger();
}

auto Peer::connect() noexcept -> void
{
    if (0 < connection_id_.size()) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Connecting to ")(
            endpoint_.address().to_string())(":")(endpoint_.port())
            .Flush();
        context_.Connect(connection_id_, endpoint_, socket_);
    }
}

auto Peer::disconnect() noexcept -> void
{
    try {
        state_.connect_.promise_.set_value(false);
    } catch (...) {
    }

    LogNormal("Disconnecting from ")(endpoint_.address().to_string())(":")(
        endpoint_.port())
        .Flush();
    manager_.Disconnect(id_);
}

auto Peer::init() noexcept -> void
{
    connect();
    init_promise_.set_value();
}

auto Peer::init_send_promise() noexcept -> void
{
    send_promise_ = SendPromise{};
    send_future_ = send_promise_.get_future();
}

auto Peer::local_endpoint() noexcept -> tcp::socket::endpoint_type
{
    try {
        return socket_.local_endpoint();
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();
        OT_FAIL;
    }
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
    auto output = ip::address_v6{};

    switch (type) {
        case p2p::Network::ipv6: {
            auto bytes = ip::address_v6::bytes_type{};

            OT_ASSERT(bytes.size() == raw.size());

            std::memcpy(&bytes, raw.data(), bytes.size());
            output = ip::make_address_v6(bytes);
        } break;
        case p2p::Network::ipv4: {
            auto output4 = ip::address_v4{};
            auto bytes = ip::address_v4::bytes_type{};

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

    return ip::tcp::endpoint{output, port};
}

auto Peer::pipeline(zmq::Message& message) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto header = message.Header();

    if (0 < header.size()) {
        auto* promise = reinterpret_cast<std::promise<void>*>(
            header.at(0).as<std::uintptr_t>());

        OT_ASSERT(nullptr != promise);

        promise->set_value();
    }

    const auto body = message.Body();

    OT_ASSERT(0 < body.size());

    switch (body.at(0).as<Task>()) {
        case Task::Getheaders: {
            if (State::Run == state_.value_.load()) { request_headers(); }
        } break;
        case Task::Getcfheaders: {
            if (State::Run == state_.value_.load()) {
                request_cfheaders(message);
            }
        } break;
        case Task::Getcfilters: {
            if (State::Run == state_.value_.load()) {
                request_cfilter(message);
            }
        } break;
        case Task::Getblock: {
            request_block(message);
        } break;
        case Task::BroadcastTransaction: {
            broadcast_transaction(message);
        } break;
        case Task::SendMessage: {
            transmit(message);
        } break;
        case Task::ReceiveMessage: {
            process_message(message);
        } break;
        case Task::Heartbeat:
        case Task::StateMachine: {
            do_work();
        } break;
        case Task::Shutdown: {
            shutdown(shutdown_promise_);
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Peer::pipeline_d(zmq::Message& message) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto body = message.Body();

    OT_ASSERT(0 < body.size());

    switch (body.at(0).as<Task>()) {
        case Task::Register: {
            OT_ASSERT(1 < body.size());

            const auto& id = body.at(1);

            OT_ASSERT(0 < id.size());

            const auto start = static_cast<const std::byte*>(id.data());
            const_cast<Space&>(connection_id_).assign(start, start + id.size());

            OT_ASSERT(0 < connection_id_.size());

            try {
                connection_id_promise_.set_value();
            } catch (...) {
            }
        } break;
        case Task::Connect: {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Connect to ")(
                endpoint_.address().to_string())(":")(endpoint_.port())(
                " successful")
                .Flush();

            try {
                state_.connect_.promise_.set_value(true);
            } catch (...) {
            }

            state_.value_.store(State::Handshake);
            init_executor(
                {manager_.Endpoint(Task::Heartbeat), shutdown_endpoint_});
            trigger();
            run();
        } break;
        case Task::Disconnect: {
            disconnect();
        } break;
        case Task::Header: {
            OT_ASSERT(1 < body.size());

            const auto& messageHeader = body.at(1);
            activity_.Bump();
            const auto size = get_body_size(messageHeader);

            if (0 < size) {
                header_->Assign(messageHeader.Bytes());
                context_.Receive(
                    connection_id_,
                    static_cast<OTZMQWorkType>(Task::Body),
                    size,
                    socket_);
            } else {
                auto message = MakeWork(Task::ReceiveMessage);
                message->AddFrame(messageHeader);
                message->AddFrame();
                pipeline_->Push(message);
                run();
            }
        } break;
        case Task::Body: {
            OT_ASSERT(1 < body.size());

            auto message = MakeWork(Task::ReceiveMessage);
            message->AddFrame(header_);
            message->AddFrame(body.at(1));
            pipeline_->Push(message);
            run();
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Peer::process_state_machine() noexcept -> void
{
    switch (state_.value_.load()) {
        case State::Run: {
            check_activity();
            check_download_peers();
        } break;
        default: {
        }
    }
}

auto Peer::run() noexcept -> void
{
    if (running_.get()) {
        context_.Receive(
            connection_id_,
            static_cast<OTZMQWorkType>(Task::Header),
            header_bytes_,
            socket_);
    }
}

auto Peer::send(OTData in) noexcept -> SendStatus
{
    try {
        if (false == state_.connect_.future_.get()) {
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
        auto message = MakeWork(Task::SendMessage);
        message->AddFrame(in);
        message->AddFrame(Data::Factory(&promise, sizeof(promise)));
        pipeline_->Push(message);

        return std::move(future);
    } else {
        return {};
    }
}

auto Peer::Shutdown() noexcept -> std::shared_future<void>
{
    dealer_->Close();

    return stop_worker();
}

auto Peer::shutdown(std::promise<void>& promise) noexcept -> void
{
    init_.get();

    if (running_->Off()) {
        const auto state = state_.value_.exchange(State::Shutdown);

        try {
            socket_.shutdown(tcp::socket::shutdown_both);
        } catch (...) {
        }

        break_promises();

        try {
            socket_.close();
        } catch (...) {
        }

        if (State::Handshake != state) { update_address_activity(); }

        LogVerbose("Disconnected from ")(address_.Display()).Flush();

        try {
            promise.set_value();
        } catch (...) {
        }
    }
}

auto Peer::start_verify() noexcept -> void
{
    request_checkpoint_block_header();

    if (verify_filter_checkpoint_) { request_checkpoint_filter_header(); }
}

auto Peer::state_machine() noexcept -> bool
{
    LogTrace(OT_METHOD)(__FUNCTION__).Flush();

    if (false == running_.get()) { return false; }

    auto disconnect{false};

    switch (state_.value_.load()) {
        case State::Handshake: {
            disconnect = state_.handshake_.run(
                std::chrono::seconds(15),
                [this] { start_handshake(); },
                State::Verify);
        } break;
        case State::Verify: {
            disconnect = state_.verify_.run(
                std::chrono::seconds(15),
                [this] { start_verify(); },
                State::Subscribe);
        } break;
        case State::Subscribe: {
            subscribe();
            state_.value_.store(State::Run);
            [[fallthrough]];
        }
        case State::Run: {
            process_state_machine();
        } break;
        case State::Shutdown: {
            disconnect = true;
        } break;
        default: {
        }
    }

    if (disconnect) { this->disconnect(); }

    return false;
}

auto Peer::subscribe() noexcept -> void
{
    const auto network =
        (1 == address_.Services().count(p2p::Service::Network));
    const auto limited =
        (1 == address_.Services().count(p2p::Service::Limited));
    const auto cfilter =
        (1 == address_.Services().count(p2p::Service::CompactFilters));

    if (network || limited) {
        pipeline_->Start(manager_.Endpoint(Task::Getheaders));
        pipeline_->Start(manager_.Endpoint(Task::Getblock));
        pipeline_->Start(manager_.Endpoint(Task::BroadcastTransaction));
    }

    if (cfilter) {
        pipeline_->Start(manager_.Endpoint(Task::Getcfheaders));
        pipeline_->Start(manager_.Endpoint(Task::Getcfilters));
    }

    request_headers();
    request_addresses();
}

auto Peer::transmit(zmq::Message& message) noexcept -> void
{
    if (false == running_.get()) { return; }

    if (3 < message.Body().size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        return;
    }

    const auto& payload = message.Body_at(1);
    const auto& promiseFrame = message.Body_at(2);
    const auto index = promiseFrame.as<int>();
    auto success = bool{false};
    auto postcondition =
        ScopeGuard{[&] { send_promises_.SetPromise(index, success); }};
    LogTrace(OT_METHOD)(__FUNCTION__)(": Sending ")(payload.size())(
        " byte message:")
        .Flush();
    LogTrace(Data::Factory(payload)->asHex()).Flush();
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

    auto& asio = context_.operator boost::asio::io_context &();
    asio.post(work);
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

Peer::~Peer() { Shutdown().get(); }
}  // namespace opentxs::blockchain::p2p::implementation
