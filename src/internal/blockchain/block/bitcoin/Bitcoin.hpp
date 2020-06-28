// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"

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
namespace bitcoin
{
class CompactSize;
class Inventory;
struct EncodedInput;
struct EncodedOutpoint;
struct EncodedOutput;
struct EncodedTransaction;
}  // namespace bitcoin

namespace block
{
namespace bitcoin
{
namespace internal
{
struct Header;
}  // namespace internal

class Block;
class Header;
class Input;
class Inputs;
class Output;
class Outputs;
class Script;
class Transaction;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace proto
{
class BlockchainBlockHeader;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::internal
{
struct Input : virtual public bitcoin::Input {
    virtual auto AssociatedLocalNyms(
        std::vector<OTNymID>& output) const noexcept -> void = 0;
    virtual auto AssociatedRemoteContacts(
        std::vector<OTIdentifier>& output) const noexcept -> void = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Input> = 0;
    virtual auto NetBalanceChange(const identifier::Nym& nym) const noexcept
        -> opentxs::Amount = 0;

    virtual auto MergeMetadata(const SerializeType& rhs) noexcept -> void = 0;

    virtual ~Input() = default;
};
struct Inputs : virtual public bitcoin::Inputs {
    virtual auto AssociatedLocalNyms(
        std::vector<OTNymID>& output) const noexcept -> void = 0;
    virtual auto AssociatedRemoteContacts(
        std::vector<OTIdentifier>& output) const noexcept -> void = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Inputs> = 0;
    virtual auto NetBalanceChange(const identifier::Nym& nym) const noexcept
        -> opentxs::Amount = 0;

    virtual auto MergeMetadata(const Input::SerializeType& rhs) noexcept(false)
        -> void = 0;

    virtual ~Inputs() = default;
};
struct Output : virtual public bitcoin::Output {
    virtual auto AssociatedLocalNyms(
        std::vector<OTNymID>& output) const noexcept -> void = 0;
    virtual auto AssociatedRemoteContacts(
        std::vector<OTIdentifier>& output) const noexcept -> void = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Output> = 0;
    virtual auto NetBalanceChange(const identifier::Nym& nym) const noexcept
        -> opentxs::Amount = 0;

    virtual auto ForTestingOnlyAddKey(const KeyID& key) noexcept -> void = 0;
    virtual auto MergeMetadata(const SerializeType& rhs) noexcept -> void = 0;

    virtual ~Output() = default;
};
struct Outputs : virtual public bitcoin::Outputs {
    virtual auto AssociatedLocalNyms(
        std::vector<OTNymID>& output) const noexcept -> void = 0;
    virtual auto AssociatedRemoteContacts(
        std::vector<OTIdentifier>& output) const noexcept -> void = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Outputs> = 0;
    virtual auto NetBalanceChange(const identifier::Nym& nym) const noexcept
        -> opentxs::Amount = 0;

    virtual auto ForTestingOnlyAddKey(
        const std::size_t index,
        const api::client::blockchain::Key& key) noexcept -> bool = 0;
    virtual auto MergeMetadata(const Output::SerializeType& rhs) noexcept(false)
        -> void = 0;

    virtual ~Outputs() = default;
};
struct Script : virtual public bitcoin::Script {
    virtual auto clone() const noexcept -> std::unique_ptr<Script> = 0;
    virtual auto LikelyPubkeyHashes(const api::client::Manager& api)
        const noexcept -> std::vector<OTData> = 0;

    virtual ~Script() = default;
};
struct Transaction : virtual public bitcoin::Transaction {
    OPENTXS_EXPORT virtual auto ForTestingOnlyAddKey(
        const std::size_t index,
        const api::client::blockchain::Key& key) noexcept -> bool = 0;
    virtual auto MergeMetadata(
        const blockchain::Type chain,
        const SerializeType& rhs) noexcept -> void = 0;
    virtual auto SetMemo(const std::string& memo) noexcept -> void = 0;

    virtual ~Transaction() = default;
};
}  // namespace opentxs::blockchain::block::bitcoin::internal

namespace opentxs::factory
{
#if OT_BLOCKCHAIN
auto BitcoinBlock(
    const api::client::Manager& api,
    const blockchain::Type chain,
    const ReadView in) noexcept
    -> std::shared_ptr<blockchain::block::bitcoin::Block>;
auto BitcoinBlockHeader(
    const api::client::Manager& api,
    const proto::BlockchainBlockHeader& serialized) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>;
auto BitcoinBlockHeader(
    const api::client::Manager& api,
    const blockchain::Type chain,
    const ReadView bytes) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>;
auto BitcoinBlockHeader(
    const api::client::Manager& api,
    const blockchain::Type chain,
    const blockchain::block::Hash& hash,
    const blockchain::block::Hash& parent,
    const blockchain::block::Height height) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>;
OPENTXS_EXPORT auto BitcoinScript(
    const blockchain::Type chain,
    const ReadView bytes,
    const bool outputScript = true,
    const bool isGeneration = false,
    const bool allowInvalidOpcodes = true) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Script>;
OPENTXS_EXPORT auto BitcoinScript(
    const blockchain::Type chain,
    blockchain::block::bitcoin::ScriptElements&& elements,
    const bool outputScript = true,
    const bool isGeneration = false) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Script>;
OPENTXS_EXPORT auto BitcoinTransaction(
    const api::client::Manager& api,
    const blockchain::Type chain,
    const bool isGeneration,
    const Time& time,
    blockchain::bitcoin::EncodedTransaction&& parsed) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Transaction>;
OPENTXS_EXPORT auto BitcoinTransaction(
    const api::client::Manager& api,
    const proto::BlockchainTransaction& serialized) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Transaction>;
OPENTXS_EXPORT auto BitcoinTransactionInput(
    const api::client::Manager& api,
    const blockchain::Type chain,
    const ReadView outpoint,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    const ReadView sequence,
    const bool isGeneration,
    std::vector<Space>&& witness) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Input>;
OPENTXS_EXPORT auto BitcoinTransactionInput(
    const api::client::Manager& api,
    const blockchain::Type chain,
    const proto::BlockchainTransactionInput,
    const bool isGeneration) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Input>;
auto BitcoinTransactionInputs(
    std::vector<std::unique_ptr<blockchain::block::bitcoin::internal::Input>>&&
        inputs,
    std::optional<std::size_t> size = {}) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Inputs>;
OPENTXS_EXPORT auto BitcoinTransactionOutput(
    const api::client::Manager& api,
    const blockchain::Type chain,
    const std::uint32_t index,
    const std::int64_t value,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Output>;
OPENTXS_EXPORT auto BitcoinTransactionOutput(
    const api::client::Manager& api,
    const blockchain::Type chain,
    const proto::BlockchainTransactionOutput) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Output>;
auto BitcoinTransactionOutputs(
    std::vector<std::unique_ptr<blockchain::block::bitcoin::internal::Output>>&&
        outputs,
    std::optional<std::size_t> size = {}) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Outputs>;
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::factory
