// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::client::implementation
{
class PeerManager final : virtual public internal::PeerManager,
                          public opentxs::internal::StateMachine
{
public:
    static const std::map<Type, std::uint16_t> default_port_map_;
    static const std::map<Type, p2p::Protocol> protocol_map_;

    bool AddPeer(const p2p::Address& address) const noexcept final;
    void Disconnect(const int id) const noexcept final;
    void Heartbeat() const noexcept { peers_.Heartbeat(); }

    void init() noexcept final;

    ~PeerManager() final;

private:
    friend opentxs::Factory;

    struct IO {
        operator boost::asio::io_context&() noexcept { return context_; }

        void Shutdown() noexcept;

        IO() noexcept;

    private:
        boost::asio::io_context context_;
        std::unique_ptr<boost::asio::io_context::work> work_;
        boost::thread_group thread_pool_;
    };

    struct Peers {
        void Heartbeat() const noexcept;

        bool AddPeer(const p2p::Address& address) noexcept;
        void QueueDisconnect(const int id) noexcept;
        bool Run() noexcept;
        void Shutdown() noexcept;

        Peers(
            const api::internal::Core& api,
            const internal::Network& network,
            const internal::PeerManager& parent,
            const Type chain,
            const std::string& seednode,
            boost::asio::io_context& context) noexcept;

    private:
        using Endpoint = std::unique_ptr<p2p::internal::Address>;

        const api::internal::Core& api_;
        const internal::Network& network_;
        const internal::PeerManager& parent_;
        const Type chain_;
        const OTData default_peer_;
        boost::asio::io_context& context_;
        mutable std::mutex lock_;
        mutable std::mutex queue_lock_;
        std::atomic<int> next_id_;
        std::atomic<std::size_t> minimum_peers_;
        std::queue<int> disconnect_peers_;
        std::map<int, std::unique_ptr<p2p::internal::Peer>> peers_;
        std::map<OTIdentifier, int> active_;

        static OTData set_default_peer(const std::string node) noexcept;

        Endpoint get_peer() const noexcept;

        void add_peer(const Lock& lock, Endpoint endpoint) noexcept;
        p2p::internal::Peer* peer_factory(
            Endpoint endpoint,
            const int id) noexcept;
    };

    const api::internal::Core& api_;
    mutable IO io_context_;
    mutable Peers peers_;
    int heartbeat_task_;

    bool state_machine() noexcept;

    PeerManager(
        const api::internal::Core& api,
        const internal::Network& network,
        const Type chain,
        const std::string& seednode) noexcept;
    PeerManager() = delete;
    PeerManager(const PeerManager&) = delete;
    PeerManager(PeerManager&&) = delete;
    PeerManager& operator=(const PeerManager&) = delete;
    PeerManager& operator=(PeerManager&&) = delete;
};
}  // namespace opentxs::blockchain::client::implementation
