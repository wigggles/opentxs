// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "core/Worker.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Forward.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/p2p/Peer.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"

namespace boost
{
namespace system
{
class error_code;
}  // namespace system
}  // namespace boost

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
}  // namespace api

namespace network
{
namespace zeromq
{
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace asio = boost::asio;
namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::p2p::implementation
{
using tcp = asio::ip::tcp;

class Peer : virtual public internal::Peer, public Worker<Peer>
{
public:
    using SendStatus = std::future<bool>;

    auto AddressID() const noexcept -> OTIdentifier final
    {
        return address_.ID();
    }
    auto Connected() const noexcept -> ConnectionStatus final
    {
        return state_.connect_.future_;
    }
    auto HandshakeComplete() const noexcept -> Handshake final
    {
        return state_.handshake_.future_;
    }
    auto Shutdown() noexcept -> std::shared_future<void> final;

    ~Peer() override;

protected:
    using SendResult = std::pair<boost::system::error_code, std::size_t>;
    using SendPromise = std::promise<SendResult>;
    using SendFuture = std::future<SendResult>;
    using Task = client::internal::PeerManager::Task;

    enum class State : std::uint8_t {
        Connect,
        Handshake,
        Verify,
        Subscribe,
        Run,
        Shutdown,
    };

    struct Address {
        using pointer = std::unique_ptr<internal::Address>;

        auto Bytes() const noexcept -> OTData;
        auto Chain() const noexcept -> blockchain::Type;
        auto Display() const noexcept -> std::string;
        auto ID() const noexcept -> OTIdentifier;
        auto Port() const noexcept -> std::uint16_t;
        auto Services() const noexcept -> std::set<Service>;
        auto Type() const noexcept -> Network;

        auto UpdateServices(const std::set<p2p::Service>& services) noexcept
            -> pointer;
        auto UpdateTime(const Time& time) noexcept -> pointer;

        Address(pointer address) noexcept;

    private:
        mutable std::mutex lock_;
        pointer address_;
    };

    struct DownloadPeers {
        auto get() const noexcept -> Time;

        void Bump() noexcept;

        DownloadPeers() noexcept;

    private:
        Time downloaded_;
    };

    template <typename ValueType>
    struct StateData {
        std::atomic<State>& state_;
        bool first_run_;
        Time started_;
        std::atomic_bool first_action_;
        std::atomic_bool second_action_;
        std::promise<ValueType> promise_;
        std::shared_future<ValueType> future_;

        auto done() const noexcept -> bool
        {
            try {
                const auto status =
                    future_.wait_for(std::chrono::microseconds(5));

                return std::future_status::ready == status;
            } catch (...) {

                return false;
            }
        }

        auto break_promise() noexcept -> void { promise_ = {}; }
        auto run(
            const std::chrono::seconds limit,
            SimpleCallback firstAction,
            const State nextState) noexcept -> bool
        {
            if ((false == first_run_) && firstAction) {
                first_run_ = true;
                started_ = Clock::now();
                firstAction();

                return false;
            }

            auto disconnect{true};

            if (done()) {
                disconnect = false;
                state_.store(nextState);
            } else {
                disconnect = ((Clock::now() - started_) > limit);
            }

            if (disconnect) {
                LogVerbose("State transition timeout exceeded.").Flush();
            }

            return disconnect;
        }

        StateData(std::atomic<State>& state) noexcept
            : state_(state)
            , first_run_(false)
            , started_()
            , first_action_(false)
            , second_action_(false)
            , promise_()
            , future_(promise_.get_future())
        {
        }
    };

    struct States {
        std::atomic<State> value_;
        StateData<bool> connect_;
        StateData<void> handshake_;
        StateData<void> verify_;

        auto break_promises() noexcept -> void
        {
            connect_.break_promise();
            handshake_.break_promise();
            verify_.break_promise();
        }

        States() noexcept
            : value_(State::Connect)
            , connect_(value_)
            , handshake_(value_)
            , verify_(value_)
        {
        }
    };

    const client::internal::Network& network_;
    const client::internal::PeerManager& manager_;
    const blockchain::Type chain_;
    const tcp::endpoint endpoint_;
    SendPromise send_promise_;
    SendFuture send_future_;
    Address address_;
    DownloadPeers download_peers_;
    OTData header_;
    States state_;

    virtual auto broadcast_transaction(zmq::Message& message) noexcept
        -> void = 0;
    auto check_handshake() noexcept -> void;
    auto check_verify() noexcept -> void;
    auto disconnect() noexcept -> void;
    // NOTE call init in every final child class constructor
    auto init() noexcept -> void;
    auto local_endpoint() noexcept -> tcp::socket::endpoint_type;
    virtual auto ping() noexcept -> void = 0;
    virtual auto pong() noexcept -> void = 0;
    virtual auto request_addresses() noexcept -> void = 0;
    virtual auto request_block(zmq::Message& message) noexcept -> void = 0;
    virtual auto request_headers() noexcept -> void = 0;
    auto send(OTData message) noexcept -> SendStatus;
    auto update_address_services(
        const std::set<p2p::Service>& services) noexcept -> void;
    auto verifying() noexcept -> bool
    {
        return (State::Verify == state_.value_.load());
    }

    Peer(
        const api::client::Manager& api,
        const client::internal::Network& network,
        const client::internal::PeerManager& manager,
        const blockchain::client::internal::IO& io,
        const int id,
        const std::string& shutdown,
        const std::size_t headerSize,
        const std::size_t bodySize,
        std::unique_ptr<internal::Address> address) noexcept;

private:
    friend Worker<Peer>;

    struct Activity {
        auto get() const noexcept -> Time;

        void Bump() noexcept;

        Activity() noexcept;

    private:
        mutable std::mutex lock_;
        Time activity_;
    };

    struct SendPromises {
        void Break();
        auto NewPromise() -> std::pair<std::future<bool>, int>;
        void SetPromise(const int promise, const bool value);

        SendPromises() noexcept;

    private:
        std::mutex lock_;
        int counter_;
        std::map<int, std::promise<bool>> map_;
    };

    const bool verify_filter_checkpoint_;
    const std::size_t header_bytes_;
    const int id_;
    const Space connection_id_;
    const std::string shutdown_endpoint_;
    const blockchain::client::internal::IO& context_;
    tcp::socket socket_;
    OTData outgoing_message_;
    std::promise<void> connection_id_promise_;
    SendPromises send_promises_;
    Activity activity_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;
    OTZMQListenCallback cb_;
    OTZMQDealerSocket dealer_;

    static auto make_buffer(const std::size_t size) noexcept -> OTData;
    static auto make_endpoint(
        const Network type,
        const Data& bytes,
        const std::uint16_t port) noexcept -> tcp::endpoint;

    auto get_activity() const noexcept -> Time;
    virtual auto get_body_size(const zmq::Frame& header) const noexcept
        -> std::size_t = 0;

    auto break_promises() noexcept -> void;
    auto check_activity() noexcept -> void;
    auto check_download_peers() noexcept -> void;
    auto connect() noexcept -> void;
    auto init_send_promise() noexcept -> void;
    auto pipeline(zmq::Message& message) noexcept -> void;
    auto pipeline_d(zmq::Message& message) noexcept -> void;
    virtual auto process_message(const zmq::Message& message) noexcept
        -> void = 0;
    auto process_state_machine() noexcept -> void;
    virtual auto request_cfheaders(zmq::Message& message) noexcept -> void = 0;
    virtual auto request_cfilter(zmq::Message& message) noexcept -> void = 0;
    virtual auto request_checkpoint_block_header() noexcept -> void = 0;
    virtual auto request_checkpoint_filter_header() noexcept -> void = 0;
    auto run() noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    virtual auto start_handshake() noexcept -> void = 0;
    auto state_machine() noexcept -> bool;
    auto start_verify() noexcept -> void;
    auto subscribe() noexcept -> void;
    auto transmit(zmq::Message& message) noexcept -> void;
    auto update_address_activity() noexcept -> void;
    auto verify() noexcept -> void;

    Peer() = delete;
    Peer(const Peer&) = delete;
    Peer(Peer&&) = delete;
    auto operator=(const Peer&) -> Peer& = delete;
    auto operator=(Peer &&) -> Peer& = delete;
};
}  // namespace opentxs::blockchain::p2p::implementation
