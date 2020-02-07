// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::client::implementation
{
class FilterOracle final : virtual public internal::FilterOracle,
                           public opentxs::internal::StateMachine
{
public:
    void AddFilter(
        const filter::Type type,
        const block::Hash& block,
        const Data& filter) const noexcept final;
    void AddHeaders(
        const filter::Type type,
        const ReadView stopBlock,
        const ReadView previousHeader,
        const std::vector<ReadView> hashes) const noexcept final;
    void CheckBlocks() const noexcept final;

    void Start() noexcept;
    std::shared_future<void> Shutdown() noexcept final;

    ~FilterOracle() final;

private:
    friend opentxs::Factory;

    enum class Work : std::uint8_t {
        cfilter = 0,
        cfheader = 1,
    };

    struct Cleanup {
        operator bool() { return repeat_; }

        void Off() { repeat_ = false; }
        void On() { repeat_ = true; }

        Cleanup(std::mutex& lock)
            : rate_limit_(10)
            , lock_(lock)
            , repeat_(true)
        {
        }

        ~Cleanup()
        {
            lock_.unlock();

            if (repeat_) { Sleep(rate_limit_); }
        }

    private:
        const std::chrono::milliseconds rate_limit_;
        Lock lock_;
        bool repeat_;
    };

    struct FilterQueue {
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

        FilterQueue(const api::Core& api) noexcept;

    private:
        using Pointer = std::unique_ptr<const blockchain::internal::GCS>;
        using FilterData = std::pair<block::pHash, Pointer>;

        static const std::chrono::seconds timeout_;

        bool running_;
        std::size_t queued_;
        std::vector<FilterData> filters_;
        Time last_received_;
        block::Position target_;
    };

    struct RequestQueue {
        bool IsRunning(const block::Hash& block) const noexcept;
        std::size_t size() const noexcept;

        void Finish(const block::Hash& block) noexcept;
        void Start(const block::Hash& hash) noexcept;

        RequestQueue(const api::Core& api) noexcept;

    private:
        using Map = std::map<block::pHash, Time>;

        static const std::chrono::seconds limit_;

        void prune(const Lock& lock) const noexcept;

        mutable std::mutex lock_;
        mutable block::Position highest_;
        mutable std::map<block::pHash, Time> hashes_;
    };

    const api::internal::Core& api_;
    const internal::Network& network_;
    const internal::FilterDatabase& database_;
    OTFlag running_;
    mutable std::mutex lock_;
    mutable RequestQueue header_requests_;
    FilterQueue outstanding_filters_;
    OTZMQPipeline pipeline_;
    opentxs::internal::ShutdownReceiver shutdown_;

    void check_filters(
        const filter::Type type,
        const block::Height maxRequests,
        Cleanup& repeat) noexcept;
    void check_headers(
        const filter::Type type,
        const block::Height maxRequests,
        Cleanup& repeat) noexcept;
    void pipeline(const zmq::Message& in) noexcept;
    void process_cfheader(const zmq::Message& in) noexcept;
    void process_cfilter(const zmq::Message& in) noexcept;
    void shutdown(std::promise<void>& promise) noexcept;
    bool state_machine() noexcept;

    FilterOracle(
        const api::internal::Core& api,
        const internal::Network& network,
        const internal::FilterDatabase& database,
        const std::string& shutdown) noexcept;
    FilterOracle() = delete;
    FilterOracle(const FilterOracle&) = delete;
    FilterOracle(FilterOracle&&) = delete;
    FilterOracle& operator=(const FilterOracle&) = delete;
    FilterOracle& operator=(FilterOracle&&) = delete;
};
}  // namespace opentxs::blockchain::client::implementation
