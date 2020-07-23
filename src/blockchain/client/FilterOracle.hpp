// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <deque>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Forward.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/client/BlockOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/util/WorkType.hpp"
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
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::client::implementation
{
class FilterOracle final : virtual public internal::FilterOracle,
                           public Worker<FilterOracle>
{
public:
    auto AddFilter(zmq::Message& work) const noexcept -> void final;
    auto AddHeaders(zmq::Message& work) const noexcept -> void final;
    auto DefaultType() const noexcept -> filter::Type final
    {
        return default_type_;
    }
    auto FilterTip(const filter::Type type) const noexcept
        -> block::Position final
    {
        return database_.FilterTip(type);
    }
    auto Heartbeat() const noexcept -> void final { trigger(); }
    auto LoadFilter(const filter::Type type, const block::Hash& block)
        const noexcept -> std::unique_ptr<const blockchain::internal::GCS> final
    {
        return database_.LoadFilter(type, block.Bytes());
    }
    auto LoadFilterOrResetTip(
        const filter::Type type,
        const block::Position& position) const noexcept
        -> std::unique_ptr<const blockchain::internal::GCS> final;

    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return stop_worker();
    }
    auto Start() noexcept -> void;

    FilterOracle(
        const api::client::Manager& api,
        const internal::Network& network,
        const internal::HeaderOracle& header,
        const internal::FilterDatabase& database,
        const blockchain::Type type,
        const std::string& shutdown) noexcept;

    ~FilterOracle() final;

private:
    friend Worker<FilterOracle>;

    enum class Work : OTZMQWorkType {
        cfilter = OT_ZMQ_INTERNAL_SIGNAL + 0,
        cfheader = OT_ZMQ_INTERNAL_SIGNAL + 1,
        reset_filter_tip = OT_ZMQ_INTERNAL_SIGNAL + 2,
        peer = OT_ZMQ_NEW_PEER_SIGNAL,
        block = OT_ZMQ_NEW_BLOCK_HEADER_SIGNAL,
        reorg = OT_ZMQ_REORG_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        shutdown = value(WorkType::Shutdown),
    };

    struct BlockQueue {
        using Future = client::BlockOracle::BitcoinBlockFuture;

        auto Capacity() const noexcept -> std::size_t;
        auto HighestRequested() const noexcept -> block::Height
        {
            return highest_;
        }

        auto Add(const block::Position& position, Future block) noexcept
            -> void;
        auto Reset() noexcept -> void;
        auto Run(bool& repeat) noexcept -> void;

        BlockQueue(
            const api::client::Manager& api,
            const internal::FilterDatabase& db,
            const internal::HeaderOracle& header,
            const FilterOracle& parent,
            const blockchain::Type chain,
            const filter::Type type,
            const std::size_t limit) noexcept;

    private:
        using Block = client::BlockOracle::BitcoinBlock_p;
        using Element = std::pair<block::Position, Future>;
        using Queue = std::queue<Element>;
        using FilterHeader = OTData;
        using Cached = std::pair<block::Height, FilterHeader>;

        const api::client::Manager& api_;
        const internal::FilterDatabase& db_;
        const internal::HeaderOracle& header_;
        const FilterOracle& parent_;
        const blockchain::Type chain_;
        const filter::Type type_;
        const std::size_t limit_;
        mutable std::optional<Cached> cached_;
        block::Height highest_;
        Queue queue_;

        auto previous_filter_header(
            const block::Position& current) const noexcept -> const Data&;
        auto process(const block::Position& position, Block block)
            const noexcept -> void;
    };

    struct FilterQueue {
        bool error_;
        filter::Type type_;

        using Filters = std::vector<internal::FilterDatabase::Filter>;

        auto AddFilter(
            const filter::Type type,
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
            const filter::Type type,
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
        // filters are stored in newest-to-oldest order
        std::vector<FilterData> filters_;
        Time last_received_;
        // height of oldest filter to fetch
        block::Height start_height_;
        // position of the newest fetch to fetch
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
    const internal::HeaderOracle& header_;
    const internal::FilterDatabase& database_;
    const blockchain::Type chain_;
    const bool full_mode_;
    const filter::Type default_type_;
    HeaderQueue header_requests_;
    FilterQueue outstanding_filters_;
    BlockQueue block_requests_;
    OTZMQPublishSocket socket_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;

    auto notify_new_filter(
        const filter::Type type,
        const block::Position& position) const noexcept -> void;
    auto oldest_checkpoint_before(const block::Height height) const noexcept
        -> block::Height;

    auto check_blocks(
        const filter::Type type,
        const block::Height maxRequests,
        bool& repeat) noexcept -> void;
    auto check_filters(
        const filter::Type type,
        const block::Height maxRequests,
        bool& repeat) noexcept -> void;
    auto check_headers(
        const filter::Type type,
        const block::Height maxRequests,
        bool& repeat) noexcept -> void;
    auto compare_tips_to_checkpoint() noexcept -> void;
    auto compare_tips_to_header_chain() noexcept -> bool;
    auto flush_filters() noexcept -> void;
    auto pipeline(const zmq::Message& in) noexcept -> void;
    auto process_cfheader(const zmq::Message& in) noexcept -> void;
    auto process_cfilter(const zmq::Message& in) noexcept -> bool;
    auto process_reorg(const zmq::Message& in) noexcept -> void;
    auto process_reorg(const block::Position& parent) noexcept -> void;
    auto process_reset_filter_tip(const zmq::Message& in) noexcept -> void;
    auto reset_tips_to(
        const filter::Type type,
        const block::Position& position,
        const std::optional<bool> resetHeader = std::nullopt,
        const std::optional<bool> resetfilter = std::nullopt) noexcept -> bool;
    auto reset_tips_to(
        const filter::Type type,
        const block::Position& headerTip,
        const block::Position& position,
        const std::optional<bool> resetHeader = std::nullopt) noexcept -> bool;
    auto reset_tips_to(
        const filter::Type type,
        const block::Position& headerTip,
        const block::Position& filterTip,
        const block::Position& position,
        std::optional<bool> resetHeader = std::nullopt,
        std::optional<bool> resetfilter = std::nullopt) noexcept -> bool;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    auto state_machine() noexcept -> bool;

    FilterOracle() = delete;
    FilterOracle(const FilterOracle&) = delete;
    FilterOracle(FilterOracle&&) = delete;
    auto operator=(const FilterOracle&) -> FilterOracle& = delete;
    auto operator=(FilterOracle &&) -> FilterOracle& = delete;
};
}  // namespace opentxs::blockchain::client::implementation
