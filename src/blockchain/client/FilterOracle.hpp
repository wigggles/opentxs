// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "core/Executor.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Forward.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
}  // namespace api

namespace blockchain
{
namespace client
{
class HeaderOracle;
}  // namespace client

namespace internal
{
struct GCS;
}  // namespace internal
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

class Factory;
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::client::implementation
{
class FilterOracle final : virtual public internal::FilterOracle,
                           public Executor<FilterOracle>
{
public:
    auto AddFilter(zmq::Message& work) const noexcept -> void final;
    auto AddHeaders(zmq::Message& work) const noexcept -> void final;
    auto CheckBlocks() const noexcept -> void final;
    auto DefaultType() const noexcept -> filter::Type final
    {
        return default_type_;
    }
    auto LoadFilter(const filter::Type type, const block::Hash& block)
        const noexcept -> std::unique_ptr<const blockchain::internal::GCS> final
    {
        return database_.LoadFilter(type, block.Bytes());
    }

    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return stop_executor();
    }
    auto Start() noexcept -> void;

    FilterOracle(
        const api::client::Manager& api,
        const internal::Network& network,
        const internal::FilterDatabase& database,
        const blockchain::Type type,
        const std::string& shutdown) noexcept;

    ~FilterOracle() final;

private:
    friend opentxs::Factory;
    friend Executor<FilterOracle>;

    enum class Work : OTZMQWorkType {
        cfilter = 0,
        cfheader = 1,
        reorg = OT_ZMQ_REORG_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        shutdown = OT_ZMQ_SHUTDOWN_SIGNAL,
    };

    struct Cleanup {
        operator bool() { return repeat_; }

        auto Off() -> void { repeat_ = false; }
        auto On() -> void { repeat_ = true; }

        Cleanup()
            : rate_limit_(10)
            , repeat_(true)
        {
        }

        ~Cleanup()
        {
            if (repeat_) { Sleep(rate_limit_); }
        }

    private:
        const std::chrono::milliseconds rate_limit_;
        bool repeat_;
    };

    struct FilterQueue {
        bool error_;

        using Filters = std::vector<internal::FilterDatabase::Filter>;

        auto AddFilter(
            const block::Height height,
            const block::Hash& hash,
            std::unique_ptr<const blockchain::internal::GCS> filter) noexcept
            -> void;
        // WARNING the lifetime of the objects added to filters ends the
        // next time Queue is executed
        auto Flush(Filters& filters) noexcept -> block::Position;
        auto IsFull() noexcept -> bool;
        auto IsRunning() noexcept -> bool;
        auto Queue(
            const block::Height startHeight,
            const block::Hash& stopHash,
            const client::HeaderOracle& headers) noexcept -> void;
        auto Reset() noexcept -> void;

        FilterQueue(const api::client::Manager& api) noexcept;

    private:
        using Pointer = std::unique_ptr<const blockchain::internal::GCS>;
        using FilterData = std::pair<block::pHash, Pointer>;

        static const std::chrono::seconds timeout_;

        const api::client::Manager& api_;
        bool running_;
        std::size_t queued_;
        std::vector<FilterData> filters_;
        Time last_received_;
        block::Position target_;
    };

    struct HeaderQueue {
        auto Finish(const block::Hash& block) noexcept -> void;
        auto IsRunning(const block::Hash& block) noexcept -> bool;
        auto Reset() noexcept -> void { hashes_.clear(); }
        auto Start(const block::Hash& hash) noexcept -> void;

        HeaderQueue(const api::client::Manager& api) noexcept;

    private:
        using Map = std::map<block::pHash, Time>;

        static const std::chrono::seconds limit_;

        auto prune() const noexcept -> void;

        mutable std::map<block::pHash, Time> hashes_;
    };

    using FilterHeaderHex = std::string;
    using FilterHeaderMap = std::map<filter::Type, FilterHeaderHex>;
    using ChainMap = std::map<block::Height, FilterHeaderMap>;
    using CheckpointMap = std::map<blockchain::Type, ChainMap>;

    static const CheckpointMap filter_checkpoints_;

    const internal::Network& network_;
    const internal::FilterDatabase& database_;
    const filter::Type default_type_;
    HeaderQueue header_requests_;
    FilterQueue outstanding_filters_;
    OTZMQPublishSocket socket_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;

    auto oldest_checkpoint_before(const block::Height height) const noexcept
        -> block::Height;

    auto check_filters(
        const filter::Type type,
        const block::Height maxRequests,
        Cleanup& repeat) noexcept -> void;
    auto check_headers(
        const filter::Type type,
        const block::Height maxRequests,
        Cleanup& repeat) noexcept -> void;
    auto compare_tips_to_checkpoint() noexcept -> void;
    auto compare_tips_to_header_chain() noexcept -> bool;
    auto pipeline(const zmq::Message& in) noexcept -> void;
    auto process_cfheader(const zmq::Message& in) noexcept -> void;
    auto process_cfilter(const zmq::Message& in) noexcept -> void;
    auto process_reorg(const zmq::Message& in) noexcept -> void;
    auto request() noexcept -> bool;
    auto reset_tips_to(const block::Position position) noexcept -> bool;
    auto shutdown(std::promise<void>& promise) noexcept -> void;

    FilterOracle() = delete;
    FilterOracle(const FilterOracle&) = delete;
    FilterOracle(FilterOracle&&) = delete;
    auto operator=(const FilterOracle&) -> FilterOracle& = delete;
    auto operator=(FilterOracle &&) -> FilterOracle& = delete;
};
}  // namespace opentxs::blockchain::client::implementation
