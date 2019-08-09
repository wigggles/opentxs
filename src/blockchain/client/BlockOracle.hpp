// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::client::implementation
{
class BlockOracle final : public internal::BlockOracle,
                          public Executor<BlockOracle>
{
public:
    auto LoadBitcoin(const block::Hash& block) const noexcept
        -> BitcoinBlockFuture final;
    auto SubmitBlock(const zmq::Frame& in) const noexcept -> void final;

    auto Init() noexcept -> void final;
    auto Run() noexcept -> void final { Trigger(); }
    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return stop_executor();
    }

    BlockOracle(
        const api::internal::Core& api,
        const internal::Network& network,
        const blockchain::Type type,
        const std::string& shutdown) noexcept;

    ~BlockOracle() final;

private:
    friend Executor<BlockOracle>;

    using Promise = std::promise<BitcoinBlock_p>;
    using PendingData = std::tuple<Time, Promise, BitcoinBlockFuture, bool>;
    using Pending = std::map<block::pHash, PendingData>;
    using Completed = std::deque<std::pair<block::pHash, BitcoinBlockFuture>>;

    struct Cache {
        auto ReceiveBlock(const zmq::Frame& in) const noexcept -> void;
        auto Request(const block::Hash& block) const noexcept
            -> BitcoinBlockFuture;
        auto StateMachine() const noexcept -> bool;

        auto Shutdown() noexcept -> void;

        Cache(const internal::Network& network) noexcept;
        ~Cache() { Shutdown(); }

    private:
        static const std::size_t cache_limit_;
        static const std::chrono::seconds download_timeout_;

        const internal::Network& network_;
        mutable std::mutex lock_;
        mutable Pending pending_;
        mutable Completed completed_;
        bool running_;

        auto download(const block::Hash& block) const noexcept -> bool;
    };

    const internal::Network& network_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;
    Cache cache_;

    auto pipeline(const zmq::Message& in) noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    auto state_machine() noexcept -> void;

    BlockOracle() = delete;
    BlockOracle(const BlockOracle&) = delete;
    BlockOracle(BlockOracle&&) = delete;
    BlockOracle& operator=(const BlockOracle&) = delete;
    BlockOracle& operator=(BlockOracle&&) = delete;
};
}  // namespace opentxs::blockchain::client::implementation
