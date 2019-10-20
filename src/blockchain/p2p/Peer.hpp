// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/blockchain/p2p/Peer.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"

#include "internal/blockchain/p2p/P2P.hpp"

#include <boost/asio.hpp>

#include <atomic>
#include <deque>
#include <map>
#include <queue>

namespace asio = boost::asio;
namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::p2p::implementation
{
using tcp = asio::ip::tcp;

class Peer : virtual public internal::Peer,
             public opentxs::internal::StateMachine
{
public:
    using SendStatus = std::future<bool>;

    OTIdentifier AddressID() const noexcept final { return address_.ID(); }
    ConnectionStatus Connected() const noexcept final { return connected_; }
    Handshake HandshakeComplete() const noexcept final { return handshake_; }
    void Heartbeat() const noexcept final { Trigger(); }

    ~Peer() override;

protected:
    using SendResult = std::pair<boost::system::error_code, std::size_t>;
    using SendPromise = std::promise<SendResult>;
    using SendFuture = std::future<SendResult>;

    struct Address {
        using pointer = std::unique_ptr<internal::Address>;

        OTData Bytes() const noexcept;
        blockchain::Type Chain() const noexcept;
        std::string Display() const noexcept;
        OTIdentifier ID() const noexcept;
        std::uint16_t Port() const noexcept;
        Network Type() const noexcept;

        pointer UpdateServices(const std::set<p2p::Service>& services) noexcept;
        pointer UpdateTime(const Time& time) noexcept;

        Address(pointer address) noexcept;

    private:
        mutable std::mutex lock_;
        pointer address_;
    };

    struct DownloadPeers {
        Time get() const noexcept;

        void Bump() noexcept;

        DownloadPeers() noexcept;

    private:
        mutable std::mutex lock_;
        Time downloaded_;
    };

    const api::internal::Core& api_;
    const client::internal::Network& network_;
    const client::internal::PeerManager& manager_;
    const int id_;
    SendPromise send_promise_;
    SendFuture send_future_;
    tcp::socket socket_;
    Address address_;
    DownloadPeers download_peers_;
    const tcp::endpoint endpoint_;
    OTData header_;
    OTData body_;
    std::size_t body_bytes_;
    bool outgoing_handshake_;
    bool incoming_handshake_;
    std::promise<void> handshake_promise_;
    Handshake handshake_;
    std::mutex shutdown_;

    void check_handshake() noexcept;
    void cleanup() noexcept;
    void disconnect() noexcept;
    // NOTE call init in every final child class constructor
    void init() noexcept;
    virtual void ping() noexcept = 0;
    virtual void pong() noexcept = 0;
    virtual void request_addresses() noexcept = 0;
    virtual void request_headers() noexcept = 0;
    SendStatus send(OTData message) noexcept;
    bool state_machine() noexcept;
    void update_address_services(
        const std::set<p2p::Service>& services) noexcept;

    Peer(
        const api::internal::Core& api,
        const client::internal::Network& network,
        const client::internal::PeerManager& manager,
        const int id,
        const std::size_t headerSize,
        const std::size_t bodySize,
        std::unique_ptr<internal::Address> address,
        boost::asio::io_context& context) noexcept;

private:
    enum class State : std::uint8_t {
        Handshake,
        Run,
    };

    struct Activity {
        Time get() const noexcept;

        void Bump() noexcept;

        Activity() noexcept;

    private:
        mutable std::mutex lock_;
        Time activity_;
    };

    struct SendPromises {
        std::pair<std::future<bool>, int> NewPromise();
        void SetPromise(const int promise, const bool value);

        SendPromises() noexcept;

    private:
        std::mutex lock_;
        int counter_;
        std::map<int, std::promise<bool>> map_;
    };

    boost::asio::io_context& context_;
    OTZMQPipeline send_;
    OTZMQPipeline process_;
    std::atomic<bool> running_;
    std::promise<bool> connection_promise_;
    std::shared_future<bool> connected_;
    SendPromises send_promises_;
    Activity activity_;
    mutable std::atomic<State> state_;

    static OTData make_buffer(const std::size_t size) noexcept;
    static tcp::endpoint make_endpoint(
        const Network type,
        const Data& bytes,
        const std::uint16_t port) noexcept;

    Time get_activity() const noexcept;

    void check_activity() noexcept;
    void check_download_peers() noexcept;
    void connect() noexcept;
    void connect_handler(const boost::system::error_code& error) noexcept;
    // This function must set body_bytes_ based on header_
    virtual void get_body_size() noexcept = 0;
    void handshake() noexcept;
    void init_send_promise() noexcept;
    virtual void process_message(const zmq::Message& message) noexcept = 0;
    void read_body() noexcept;
    void read_header() noexcept;
    void receive_body(const boost::system::error_code& error) noexcept;
    void receive_header(const boost::system::error_code& error) noexcept;
    void run() noexcept;
    virtual void start_handshake() noexcept = 0;
    void transmit(zmq::Message& message) noexcept;
    void update_address_activity() noexcept;

    Peer() = delete;
    Peer(const Peer&) = delete;
    Peer(Peer&&) = delete;
    Peer& operator=(const Peer&) = delete;
    Peer& operator=(Peer&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::implementation
