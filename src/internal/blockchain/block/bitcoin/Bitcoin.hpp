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
#include "opentxs/blockchain/Blockchain.hpp"

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
}  // namespace opentxs

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
    const ReadView bytes,
    const bool outputScript = true,
    const bool isGeneration = false,
    const bool allowInvalidOpcodes = true) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Script>;
OPENTXS_EXPORT auto BitcoinScript(
    blockchain::block::bitcoin::ScriptElements&& elements,
    const bool outputScript = true,
    const bool isGeneration = false) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Script>;
OPENTXS_EXPORT auto BitcoinTransaction(
    const api::client::Manager& api,
    const blockchain::Type chain,
    const bool isGeneration,
    blockchain::bitcoin::EncodedTransaction&& parsed) noexcept
    -> std::shared_ptr<blockchain::block::bitcoin::Transaction>;
OPENTXS_EXPORT auto BitcoinTransaction(
    const api::client::Manager& api,
    const bool isGeneration,
    const proto::BlockchainTransaction& serialized) noexcept
    -> std::shared_ptr<blockchain::block::bitcoin::Transaction>;
OPENTXS_EXPORT auto BitcoinTransactionInput(
    const api::client::Manager& api,
    const ReadView outpoint,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    const ReadView sequence,
    const bool isGeneration,
    std::vector<Space>&& witness) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Input>;
OPENTXS_EXPORT auto BitcoinTransactionInput(
    const api::client::Manager& api,
    const proto::BlockchainTransactionInput,
    const bool isGeneration) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Input>;
auto BitcoinTransactionInputs(
    std::vector<std::unique_ptr<blockchain::block::bitcoin::Input>>&& inputs,
    std::optional<std::size_t> size = {}) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Inputs>;
OPENTXS_EXPORT auto BitcoinTransactionOutput(
    const api::client::Manager& api,
    const std::uint32_t index,
    const std::int64_t value,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Output>;
OPENTXS_EXPORT auto BitcoinTransactionOutput(
    const api::client::Manager& api,
    const proto::BlockchainTransactionOutput) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Output>;
auto BitcoinTransactionOutputs(
    std::vector<std::unique_ptr<blockchain::block::bitcoin::Output>>&& outputs,
    std::optional<std::size_t> size = {}) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Outputs>;
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::factory
