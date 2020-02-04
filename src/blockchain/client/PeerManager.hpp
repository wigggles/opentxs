// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::client::implementation
{
class PeerManager final : virtual public internal::PeerManager,
                          public Executor<PeerManager>
{
public:
    static const std::map<Type, std::uint16_t> default_port_map_;
    static const std::map<Type, std::vector<std::string>> dns_seeds_;
    static const std::map<Type, p2p::Protocol> protocol_map_;

    bool AddPeer(const p2p::Address& address) const noexcept final;
    const internal::PeerDatabase& Database() const noexcept final
    {
        return database_;
    }
    bool Connect() noexcept;
    void Disconnect(const int id) const noexcept final;
    std::string Endpoint(const Task type) const noexcept final
    {
        return jobs_.Endpoint(type);
    }
    std::size_t GetPeerCount() const noexcept final { return peers_.Count(); }
    void Heartbeat() const noexcept { jobs_.Dispatch(Task::Heartbeat); }
    void RequestFilterHeaders(
        const filter::Type type,
        const block::Height start,
        const block::Hash& stop) const noexcept final;
    void RequestFilters(
        const filter::Type type,
        const block::Height start,
        const block::Hash& stop) const noexcept final;
    void RequestHeaders() const noexcept final;
    std::shared_future<void> Shutdown() noexcept final
    {
        return stop_executor();
    }

    void init() noexcept final;
    void Run() noexcept final { Trigger(); }

    PeerManager(
        const api::internal::Core& api,
        const internal::Network& network,
        const internal::PeerDatabase& database,
        const blockchain::client::internal::IO& io,
        const Type chain,
        const std::string& seednode,
        const std::string& shutdown) noexcept;

    ~PeerManager() final;

private:
    friend opentxs::Factory;
    friend Executor<PeerManager>;

    struct Jobs {
        auto Endpoint(const Task type) const noexcept -> std::string;
        auto Work(const Task task, std::promise<void>* promise = nullptr) const
            noexcept -> OTZMQMessage;

        auto Dispatch(const Task type) noexcept -> void;
        auto Dispatch(zmq::Message& work) noexcept -> void;
        auto Shutdown() noexcept -> void;

        Jobs(const api::internal::Core& api) noexcept;

    private:
        using EndpointMap = std::map<Task, std::string>;
        using SocketMap = std::map<Task, zmq::socket::Sender*>;

        const zmq::Context& zmq_;
        OTZMQPushSocket getheaders_;
        OTZMQPushSocket getcfheaders_;
        OTZMQPushSocket getcfilters_;
        OTZMQPublishSocket heartbeat_;
        const EndpointMap endpoint_map_;
        const SocketMap socket_map_;

        auto listen(const Task type, const zmq::socket::Sender& socket) noexcept
            -> void;

        Jobs() = delete;
    };

    struct Peers {
        auto Count() const noexcept -> std::size_t { return count_.load(); }

        auto AddPeer(
            const p2p::Address& address,
            std::promise<bool>& promise) noexcept -> void;
        auto Disconnect(const int id) noexcept -> void;
        auto Run(std::promise<bool>& promise) noexcept -> void;
        auto Shutdown() noexcept -> void;

        Peers(
            const api::internal::Core& api,
            const internal::Network& network,
            const internal::PeerDatabase& database,
            const internal::PeerManager& parent,
            const Flag& running,
            const std::string& shutdown,
            const Type chain,
            const std::string& seednode,
            const blockchain::client::internal::IO& context) noexcept;

    private:
        using Endpoint = std::unique_ptr<p2p::internal::Address>;
        using Peer = std::unique_ptr<p2p::internal::Peer>;
        using Resolver = boost::asio::ip::tcp::resolver;

        const api::internal::Core& api_;
        const internal::Network& network_;
        const internal::PeerDatabase& database_;
        const internal::PeerManager& parent_;
        const Flag& running_;
        const std::string& shutdown_endpoint_;
        const Type chain_;
        const bool invalid_peer_;
        const OTData localhost_peer_;
        const OTData default_peer_;
        const blockchain::client::internal::IO& context_;
        mutable Resolver resolver_;
        std::atomic<int> next_id_;
        std::atomic<std::size_t> minimum_peers_;
        std::map<int, Peer> peers_;
        std::map<OTIdentifier, int> active_;
        std::atomic<std::size_t> count_;

        static auto set_default_peer(
            const std::string node,
            const Data& localhost,
            bool& invalidPeer) noexcept -> OTData;

        auto get_default_peer() const noexcept -> Endpoint;
        auto get_dns_peer() const noexcept -> Endpoint;
        auto get_fallback_peer(const p2p::Protocol protocol) const noexcept
            -> Endpoint;
        auto get_peer() const noexcept -> Endpoint;
        auto get_preferred_peer(const p2p::Protocol protocol) const noexcept
            -> Endpoint;

        auto add_peer(Endpoint endpoint) noexcept -> void;
        auto peer_factory(Endpoint endpoint, const int id) noexcept
            -> p2p::internal::Peer*;
    };

    enum class Work : OTZMQWorkType {
        Disconnect = 0,
        AddPeer = 1,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        Shutdown = OT_ZMQ_SHUTDOWN_SIGNAL,
    };

    static const unsigned int peer_target_{2};

    const internal::PeerDatabase& database_;
    const internal::IO& io_context_;
    mutable Jobs jobs_;
    mutable Peers peers_;
    int heartbeat_task_;

    auto pipeline(zmq::Message& message) noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;

    PeerManager() = delete;
    PeerManager(const PeerManager&) = delete;
    PeerManager(PeerManager&&) = delete;
    PeerManager& operator=(const PeerManager&) = delete;
    PeerManager& operator=(PeerManager&&) = delete;
};
}  // namespace opentxs::blockchain::client::implementation
