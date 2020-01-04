// Copyright (c) 2010-2019 The Open-Transactions developers
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
    void CheckBlocks() const noexcept final;

    void Start() noexcept;
    std::shared_future<void> Shutdown() noexcept final;

    ~FilterOracle() final;

private:
    friend opentxs::Factory;

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
    mutable RequestQueue requests_;
    OTZMQPipeline new_filters_;
    opentxs::internal::ShutdownReceiver shutdown_;

    bool have_all_filters(
        const filter::Type type,
        const block::Position checkpoint,
        const block::Hash& block) const noexcept;

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
