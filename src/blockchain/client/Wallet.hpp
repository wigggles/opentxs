// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstddef>
#include <deque>
#include <functional>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "blockchain/bitcoin/CompactSize.hpp"
#include "blockchain/client/wallet/HDStateData.hpp"
#include "core/Worker.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"
#include "opentxs/protobuf/BlockchainTransactionProposal.pb.h"
#include "opentxs/protobuf/Enums.pb.h"
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

class Manager;
}  // namespace client
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
struct Bip143Hashes;
struct SigHash;
}  // namespace bitcoin

namespace block
{
namespace bitcoin
{
namespace internal
{
struct Input;
struct Output;
struct Transaction;
}  // namespace internal

struct Outpoint;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class BlockchainTransactionOutput;
class BlockchainTransactionProposal;
}  // namespace proto
}  // namespace opentxs

namespace be = boost::endian;

namespace opentxs::blockchain::client::implementation
{
class Wallet final : virtual public internal::Wallet, Worker<Wallet>
{
public:
    struct Account {
        using Subchain = internal::WalletDatabase::Subchain;
        using Task = internal::Wallet::Task;
        using BalanceTree = api::client::blockchain::internal::BalanceTree;

        auto queue_work(const Task task, const HDStateData& data) noexcept
            -> void;
        auto reorg(const block::Position& parent) noexcept -> bool;
        auto state_machine() noexcept -> bool;

        Account(
            const api::client::Manager& api,
            const BalanceTree& ref,
            const internal::Network& network,
            const internal::WalletDatabase& db,
            const zmq::socket::Push& socket) noexcept;
        Account(Account&&) noexcept;

    private:
        const api::client::Manager& api_;
        const BalanceTree& ref_;
        const internal::Network& network_;
        const internal::WalletDatabase& db_;
        const filter::Type filter_type_;
        const zmq::socket::Push& socket_;
        std::map<OTIdentifier, HDStateData> internal_;
        std::map<OTIdentifier, HDStateData> external_;

        auto state_machine_hd(HDStateData& data) noexcept -> bool;
        auto reorg_hd(HDStateData& data, const block::Position& parent) noexcept
            -> bool;

        Account(const Account&) = delete;
    };

    auto ConstructTransaction(const proto::BlockchainTransactionProposal& tx)
        const noexcept -> std::future<block::pTxid> final;

    auto Init() noexcept -> void final;
    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return stop_worker();
    }

    Wallet(
        const api::client::Manager& api,
        const api::client::internal::Blockchain& blockchain,
        const internal::Network& parent,
        const Type chain,
        const std::string& shutdown) noexcept;

    ~Wallet() final;

private:
    friend Worker<Wallet>;

    enum class Work : OTZMQWorkType {
        key = OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL,
        block = OT_ZMQ_NEW_BLOCK_HEADER_SIGNAL,
        filter = OT_ZMQ_NEW_FILTER_SIGNAL,
        nym = OT_ZMQ_NEW_NYM_SIGNAL,
        reorg = OT_ZMQ_REORG_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        shutdown = OT_ZMQ_SHUTDOWN_SIGNAL,
    };

    struct Accounts {
        auto Add(const identifier::Nym& nym) noexcept -> bool;
        auto Add(const zmq::Frame& message) noexcept -> bool;
        auto Reorg(const block::Position& parent) noexcept -> bool;

        auto state_machine() noexcept -> bool;

        Accounts(
            const api::client::Manager& api,
            const api::client::internal::Blockchain& blockchain,
            const internal::Network& network,
            const internal::WalletDatabase& db,
            const zmq::socket::Push& socket,
            const Type chain) noexcept;

    private:
        using AccountMap = std::map<OTNymID, Account>;

        const api::client::Manager& api_;
        const api::client::internal::Blockchain& blockchain_api_;
        const internal::Network& network_;
        const internal::WalletDatabase& db_;
        const zmq::socket::Push& socket_;
        const Type chain_;
        AccountMap map_;

        static auto init(
            const api::client::Manager& api,
            const api::client::internal::Blockchain& blockchain,
            const internal::Network& network,
            const internal::WalletDatabase& db,
            const zmq::socket::Push& socket,
            const Type chain) noexcept -> AccountMap;
    };

    struct Proposals {
        using Proposal = proto::BlockchainTransactionProposal;

        auto Add(const Proposal& tx) const noexcept
            -> std::future<block::pTxid>;

        auto Run() noexcept -> bool;

        Proposals(
            const api::client::Manager& api,
            const internal::Network& network,
            const internal::WalletDatabase& db,
            const Type chain) noexcept;

    private:
        enum class BuildResult : std::int8_t {
            PermanentFailure = -1,
            Success = 0,
            TemporaryFailure = 1,
        };

        struct BitcoinTransactionBuilder {
            using UTXO = std::pair<
                blockchain::block::bitcoin::Outpoint,
                proto::BlockchainTransactionOutput>;
            using Transaction =
                std::unique_ptr<block::bitcoin::internal::Transaction>;

            auto IsFunded() const noexcept -> bool;

            auto AddChange() noexcept -> bool;
            auto AddInput(const UTXO& utxo) noexcept -> bool;
            auto CreateOutputs(const Proposal& proposal) noexcept -> bool;
            auto FinalizeOutputs() noexcept -> void;
            auto FinalizeTransaction() noexcept -> Transaction;
            auto SignInputs() noexcept -> bool;

            BitcoinTransactionBuilder(
                const api::client::Manager& api,
                const internal::WalletDatabase& db,
                const Identifier& proposal,
                const Type chain,
                const Amount feeRate) noexcept;

        private:
            using Input = std::unique_ptr<block::bitcoin::internal::Input>;
            using Output = std::unique_ptr<block::bitcoin::internal::Output>;
            using Bip143 = std::optional<bitcoin::Bip143Hashes>;
            using Hash = std::array<std::byte, 32>;

            const api::client::Manager& api_;
            const internal::WalletDatabase& db_;
            const Identifier& proposal_;
            const Type chain_;
            const Amount fee_rate_;
            const be::little_int32_buf_t version_;
            const be::little_uint32_buf_t lock_time_;
            std::vector<Output> outputs_;
            std::vector<Output> change_;
            std::vector<Input> inputs_;
            const std::size_t fixed_overhead_;
            bitcoin::CompactSize input_count_;
            bitcoin::CompactSize output_count_;
            std::size_t input_total_;
            std::size_t output_total_;
            Amount input_value_;
            Amount output_value_;

            static auto is_segwit(
                const block::bitcoin::internal::Input& input) noexcept -> bool;

            auto add_signatures(
                const ReadView preimage,
                const blockchain::bitcoin::SigHash& sigHash,
                block::bitcoin::internal::Input& input) const noexcept -> bool;
            auto bytes() const noexcept -> std::size_t;
            auto dust() const noexcept -> std::size_t;
            auto get_single(
                const std::size_t index,
                const blockchain::bitcoin::SigHash& sigHash) const noexcept
                -> std::unique_ptr<Hash>;
            auto hash_type() const noexcept -> proto::HashType;
            auto init_bip143(Bip143& bip143) const noexcept -> bool;
            auto init_txcopy(Transaction& txcopy) const noexcept -> bool;
            auto required_fee() const noexcept -> Amount;
            auto sign_input(
                const int index,
                block::bitcoin::internal::Input& input,
                Transaction& txcopy,
                Bip143& bip143) const noexcept -> bool;
            auto sign_input_bch(
                const int index,
                block::bitcoin::internal::Input& input,
                Bip143& bip143) const noexcept -> bool;
            auto sign_input_btc(
                const int index,
                block::bitcoin::internal::Input& input,
                Transaction& txcopy) const noexcept -> bool;

            auto bip_69() noexcept -> void;

            BitcoinTransactionBuilder() = delete;
        };

        using Builder = std::function<BuildResult(
            const Identifier& id,
            Proposal&,
            std::promise<block::pTxid>&)>;

        const api::client::Manager& api_;
        const internal::Network& network_;
        const internal::WalletDatabase& db_;
        const Type chain_;
        mutable std::mutex lock_;
        mutable std::map<OTIdentifier, std::promise<block::pTxid>> pending_;
        mutable std::map<OTIdentifier, Time> confirming_;

        static auto is_expired(const Proposal& tx) noexcept -> bool;

        auto build_transaction_bitcoin(
            const Identifier& id,
            Proposal& proposal,
            std::promise<block::pTxid>& promise) const noexcept -> BuildResult;
        auto cleanup(const Lock& lock) noexcept -> void;
        auto get_builder() const noexcept -> Builder;
        auto rebroadcast(const Lock& lock) noexcept -> void;
        auto send(const Lock& lock) noexcept -> void;
    };

    const internal::Network& parent_;
    const internal::WalletDatabase& db_;
    const api::client::internal::Blockchain& blockchain_api_;
    const Type chain_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;
    OTZMQPushSocket socket_;
    Accounts accounts_;
    Proposals proposals_;

    auto pipeline(const zmq::Message& in) noexcept -> void;
    auto process_reorg(const zmq::Message& in) noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    auto state_machine() noexcept -> bool;

    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet &&) -> Wallet& = delete;
};
}  // namespace opentxs::blockchain::client::implementation
