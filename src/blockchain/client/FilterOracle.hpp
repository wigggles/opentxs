// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/circular_buffer.hpp>
#include <atomic>
#include <chrono>
#include <deque>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
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
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
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
namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

class Core;
}  // namespace api

namespace blockchain
{
namespace client
{
class HeaderOracle;
struct GCS;
}  // namespace client
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
                           public Worker<FilterOracle, api::Core>
{
public:
    static const std::size_t max_filter_requests_;
    static const std::size_t max_header_requests_;

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
        const noexcept -> std::unique_ptr<const GCS> final
    {
        return database_.LoadFilter(type, block.Bytes());
    }
    auto LoadFilterHeader(const filter::Type type, const block::Hash& block)
        const noexcept -> Header final
    {
        return database_.LoadFilterHeader(type, block.Bytes());
    }
    auto LoadFilterOrResetTip(
        const filter::Type type,
        const block::Position& position) const noexcept
        -> std::unique_ptr<const GCS> final;
    auto PreviousHeader(const filter::Type type, const block::Height& block)
        const noexcept -> Header final;

    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return stop_worker();
    }
    auto Start() noexcept -> void;

    FilterOracle(
        const api::Core& api,
        const api::client::internal::Blockchain& blockchain,
        const internal::Network& network,
        const internal::HeaderOracle& header,
        const internal::FilterDatabase& database,
        const blockchain::Type type,
        const std::string& shutdown) noexcept;

    ~FilterOracle() final;

private:
    friend Worker<FilterOracle, api::Core>;
    friend internal::FilterOracle;

    enum class Work : OTZMQWorkType {
        cfilter = OT_ZMQ_INTERNAL_SIGNAL + 0,
        cfheader = OT_ZMQ_INTERNAL_SIGNAL + 1,
        reset_filter_tip = OT_ZMQ_INTERNAL_SIGNAL + 2,
        index_block = OT_ZMQ_INTERNAL_SIGNAL + 3,
        calculate_headers = OT_ZMQ_INTERNAL_SIGNAL + 4,
        peer = value(WorkType::BlockchainPeerAdded),
        block = value(WorkType::BlockchainNewHeader),
        reorg = value(WorkType::BlockchainReorg),
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        shutdown = value(WorkType::Shutdown),
    };

    struct BlockQueue {
        using Future = client::BlockOracle::BitcoinBlockFuture;

        static constexpr auto download_batch_{250u};

        std::atomic_bool have_received_blocks_;
        std::atomic<std::size_t> jobs_;
        std::atomic<bool> calculate_headers_job_;

        auto DownloadCapacity() const noexcept -> std::size_t;
        auto HighestRequested() const noexcept -> block::Height
        {
            return highest_requested_;
        }

        auto Add(
            std::vector<block::pHash>& hashes,
            BlockOracle::BitcoinBlockFutures&& futures,
            block::Height start) noexcept -> void;
        auto CalculateHeaders(const zmq::Message& task) noexcept -> void;
        auto IndexBlock(const std::size_t index) noexcept -> void;
        auto Reset() noexcept -> void;
        auto Run() noexcept -> void;

        BlockQueue(
            const api::Core& api,
            const internal::FilterDatabase& db,
            const internal::HeaderOracle& header,
            const FilterOracle& parent,
            const blockchain::Type chain,
            const filter::Type type) noexcept;
        ~BlockQueue();

    private:
        using Block = client::BlockOracle::BitcoinBlock_p;
        using Element = std::pair<block::Position, Future>;
        using FilterHeader = OTData;

        template <typename FutureType>
        static auto ready(const FutureType& future) -> bool
        {
            constexpr auto zero = std::chrono::seconds{0};

            return std::future_status::ready == future.wait_for(zero);
        }

        struct IndexBlockJob {
            using HeaderPromise = std::shared_ptr<std::promise<block::pHash>>;
            using HeaderFuture = std::shared_future<block::pHash>;
            using Filter = std::unique_ptr<GCS>;
            using FilterPromise = std::shared_ptr<std::promise<Filter>>;
            using FilterFuture = std::shared_future<Filter>;
            using Parent = const BlockQueue*;

            block::Position position_;
            std::atomic_bool running_;
            std::atomic_bool complete_;

            auto future() const noexcept { return header_future_; }
            auto GetHeader() noexcept -> const block::Hash&
            {
                return header_future_.get().get();
            }
            auto hasBlock() const noexcept { return ready(block_); }
            auto hasFilter() const noexcept { return ready(filter_future_); }
            auto hasHeader() const noexcept { return ready(header_future_); }
            auto hasPreviousHeader() const noexcept
            {
                return ready(previous_header_);
            }

            auto CalculateFilter() noexcept -> void;
            auto CalculateHeader() noexcept -> void;
            auto Evaluate(std::atomic<std::size_t>& completed) noexcept -> bool;
            auto GetFilter() noexcept -> Filter&
            {
                return const_cast<Filter&>(filter_future_.get());
            }

            IndexBlockJob(
                const Parent parent,
                const block::Position& position,
                const HeaderFuture previousHeader,
                const Future block) noexcept;
            // NOTE due to limitations of boost::circular_buffer,
            // IndexBlockJob must be copy assignable. Perhaps someday this
            // ticket will be closed and that won't be necessary any more
            // https://svn.boost.org/trac10/ticket/9299
            IndexBlockJob(const IndexBlockJob&) noexcept;
            auto operator=(const IndexBlockJob&) noexcept -> IndexBlockJob&;

        private:
            const Parent parent_;
            Future block_;
            HeaderFuture previous_header_;
            FilterPromise filter_promise_;
            FilterFuture filter_future_;
            HeaderPromise header_promise_;
            HeaderFuture header_future_;
        };

        using Buffer = boost::circular_buffer<IndexBlockJob>;

        const api::Core& api_;
        const internal::FilterDatabase& db_;
        const internal::HeaderOracle& header_;
        const FilterOracle& parent_;
        const blockchain::Type chain_;
        const filter::Type type_;
        const std::size_t download_limit_;
        mutable std::mutex buffer_lock_;
        bool running_;
        std::atomic<std::size_t> downloading_;
        std::size_t downloaded_;
        std::atomic<std::size_t> completed_;
        block::Height highest_requested_;
        block::Position highest_completed_;
        Buffer buffer_;

        auto ready() const noexcept -> std::pair<std::size_t, block::Position>;
        auto wait_for_jobs() const noexcept -> void;

        auto begin(const std::size_t index = 0) noexcept -> Buffer::iterator;
        auto begin(const Lock& lock, const std::size_t index = 0) noexcept
            -> Buffer::iterator;
        auto calculate_headers(
            const Lock& lock,
            Buffer::iterator it,
            std::optional<Buffer::iterator> lastGood) noexcept -> void;
        auto check_block_futures() noexcept -> void;
        auto end(const Buffer::iterator& it) noexcept -> bool;
        auto end(const Lock& lock, const Buffer::iterator& it) noexcept -> bool;
        auto flush(const std::size_t items) noexcept -> block::Position;
        auto queue_calculate_headers() noexcept -> void;
        auto queue_process_block(const std::size_t index) noexcept -> void;
        auto queue_work(const Work type, const std::size_t index = 0) noexcept
            -> void;
        auto write_buffer() noexcept -> void;
    };

    struct FilterQueue {
        bool error_;
        filter::Type type_;

        using Filters = std::vector<internal::FilterDatabase::Filter>;

        auto AddFilter(
            const filter::Type type,
            const block::Height height,
            const block::Hash& hash,
            std::unique_ptr<const GCS> filter) noexcept -> void;
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

        FilterQueue(const api::Core& api) noexcept;

    private:
        using Pointer = std::unique_ptr<const GCS>;
        using FilterData = std::pair<block::pHash, Pointer>;

        static const std::chrono::seconds timeout_;

        const api::Core& api_;
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

        HeaderQueue(const api::Core& api) noexcept;

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
    OTZMQPushSocket thread_pool_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;

    auto notify_new_filter(
        const filter::Type type,
        const block::Position& position) const noexcept -> void;
    auto oldest_checkpoint_before(const block::Height height) const noexcept
        -> block::Height;
    auto thread_pool() const noexcept -> const zmq::socket::Push&
    {
        return thread_pool_.get();
    }

    auto check_blocks(
        const filter::Type type,
        const block::Height maxRequests) noexcept -> void;
    auto check_filters(
        const filter::Type type,
        const block::Height maxRequests) noexcept -> void;
    auto check_headers(
        const filter::Type type,
        const block::Height maxRequests) noexcept -> void;
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
