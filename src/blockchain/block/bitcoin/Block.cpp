// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/block/bitcoin/Block.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "blockchain/block/Block.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "util/Container.hpp"

#define OT_METHOD "opentxs::blockchain::block::bitcoin::implementation::Block::"

namespace be = boost::endian;

namespace opentxs::factory
{
using ReturnType = blockchain::block::bitcoin::implementation::Block;

auto BitcoinBlock(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const ReadView in) noexcept
    -> std::shared_ptr<blockchain::block::bitcoin::Block>
{
    auto data = api.Factory().Data(in);

    try {
        if ((nullptr == in.data()) || (0 == in.size())) {
            throw std::runtime_error("Invalid block input");
        }

        auto it = reinterpret_cast<const std::byte*>(in.data());
        auto expectedSize = std::size_t{ReturnType::header_bytes_};

        if (in.size() < expectedSize) {
            throw std::runtime_error("Block size too short (header)");
        }

        auto pHeader = BitcoinBlockHeader(
            api,
            chain,
            {reinterpret_cast<const char*>(it), ReturnType::header_bytes_});

        if (false == bool(pHeader)) {
            throw std::runtime_error("Invalid block header");
        }

        const auto& header = *pHeader;
        std::advance(it, ReturnType::header_bytes_);
        expectedSize += 1;

        if (in.size() < expectedSize) {
            throw std::runtime_error(
                "Block size too short (transaction count)");
        }

        auto sizeData =
            ReturnType::CalculatedSize{in.size(), bb::CompactSize{}};
        auto& [size, txCount] = sizeData;

        if (false == bb::DecodeCompactSizeFromPayload(
                         it, expectedSize, in.size(), txCount)) {
            throw std::runtime_error("Failed to decode transaction count");
        }

        const auto& transactionCount = txCount.Value();

        if (0 == transactionCount) { throw std::runtime_error("Empty block"); }

        auto counter = int{-1};
        auto index = ReturnType::TxidIndex{};
        auto transactions = ReturnType::TransactionMap{};

        while (transactions.size() < transactionCount) {
            auto data = bb::EncodedTransaction::Deserialize(
                api,
                chain,
                ReadView{
                    reinterpret_cast<const char*>(it),
                    in.size() - expectedSize});
            const auto txBytes = data.size();
            std::advance(it, txBytes);
            expectedSize += txBytes;
            auto& txid = index.emplace_back(data.txid_);
            transactions.emplace(
                reader(txid),
                BitcoinTransaction(
                    api,
                    blockchain,
                    chain,
                    (0 == ++counter),
                    header.Timestamp(),
                    std::move(data)));
        }

        return std::make_shared<ReturnType>(
            api,
            chain,
            std::move(pHeader),
            std::move(index),
            std::move(transactions),
            std::move(sizeData));
    } catch (const std::exception& e) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block
{
auto SetIntersection(
    const api::Core& api,
    const ReadView txid,
    const Block::Patterns& patterns,
    const std::vector<Space>& compare) noexcept -> Block::Matches
{
    auto test = std::vector<Space>{};
    auto map = std::map<ReadView, Block::Patterns::const_iterator>{};

    for (auto i{patterns.cbegin()}; i != patterns.cend(); std::advance(i, 1)) {
        const auto& [elementID, data] = *i;
        map.emplace(reader(data), i);
        test.emplace_back(data);
    }

    auto matches = std::vector<Space>{};
    auto output = Block::Matches{};
    std::set_intersection(
        std::begin(test),
        std::end(test),
        std::begin(compare),
        std::end(compare),
        std::back_inserter(matches));
    std::transform(
        std::begin(matches),
        std::end(matches),
        std::back_inserter(output),
        [&](const auto& match) -> Block::Match {
            return {api.Factory().Data(txid), map.at(reader(match))->first};
        });

    return output;
}
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block::bitcoin::internal
{
auto Opcode(const OP opcode) noexcept(false) -> ScriptElement
{
    return {opcode, {}, {}, {}};
}

auto PushData(const ReadView in) noexcept(false) -> ScriptElement
{
    const auto size = in.size();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
    // std::size_t might be 32 bit
    if (size > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("Too many bytes");
    }
#pragma GCC diagnostic pop

    if ((nullptr == in.data()) || (0 == size)) {
        return {OP::PUSHDATA1, {}, Space{std::byte{0x0}}, Space{}};
    }

    auto output = ScriptElement{};
    auto& [opcode, invalid, bytes, data] = output;

    if (75 >= size) {
        opcode = static_cast<OP>(static_cast<std::uint8_t>(size));
    } else if (std::numeric_limits<std::uint8_t>::max() >= size) {
        opcode = OP::PUSHDATA1;
        bytes = Space{std::byte{static_cast<std::uint8_t>(size)}};
    } else if (std::numeric_limits<std::uint16_t>::max() >= size) {
        opcode = OP::PUSHDATA2;
        const auto buf =
            be::little_uint16_buf_t{static_cast<std::uint16_t>(size)};
        bytes = space(sizeof(buf));
        std::memcpy(bytes.value().data(), &buf, sizeof(buf));
    } else {
        opcode = OP::PUSHDATA4;
        const auto buf =
            be::little_uint32_buf_t{static_cast<std::uint32_t>(size)};
        bytes = space(sizeof(buf));
        std::memcpy(bytes.value().data(), &buf, sizeof(buf));
    }

    data = space(size);
    std::memcpy(data.value().data(), in.data(), in.size());

    return output;
}
}  // namespace opentxs::blockchain::block::bitcoin::internal

namespace opentxs::blockchain::block::bitcoin::implementation
{
const std::size_t Block::header_bytes_{80};
const Block::value_type Block::null_tx_{};

Block::Block(
    const api::Core& api,
    const blockchain::Type chain,
    std::unique_ptr<const internal::Header> header,
    TxidIndex&& index,
    TransactionMap&& transactions,
    std::optional<CalculatedSize>&& size) noexcept(false)
    : block::implementation::Block(api, *header)
    , header_p_(std::move(header))
    , header_(*header_p_)
    , index_(std::move(index))
    , transactions_(std::move(transactions))
    , size_(std::move(size))
{
    if (false == bool(header_p_)) {
        throw std::runtime_error("Invalid header");
    }

    for (const auto& [txid, tx] : transactions_) {
        if (false == bool(tx)) {
            throw std::runtime_error("Invalid transaction");
        }
    }
}

auto Block::at(const std::size_t index) const noexcept -> const value_type&
{
    try {
        if (index_.size() <= index) {
            throw std::out_of_range("invalid index " + std::to_string(index));
        }

        return at(reader(index_.at(index)));
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

        return null_tx_;
    }
}

auto Block::at(const ReadView txid) const noexcept -> const value_type&
{
    try {

        return transactions_.at(txid);
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": transaction ")(
            api_.Factory().Data(txid)->asHex())(" not found in block ")(
            header_.Hash().asHex())
            .Flush();

        return null_tx_;
    }
}

auto Block::calculate_size() const noexcept -> CalculatedSize
{
    if (false == size_.has_value()) {
        size_ = CalculatedSize{0, bb::CompactSize(transactions_.size())};
        auto& [bytes, cs] = size_.value();
        auto cb = [](const auto& previous, const auto& in) -> std::size_t {
            return previous + in.second->CalculateSize();
        };
        bytes = std::accumulate(
            std::begin(transactions_),
            std::end(transactions_),
            header_bytes_ + cs.Size(),
            cb);
    }

    return size_.value();
}

auto Block::ExtractElements(const FilterType style) const noexcept
    -> std::vector<Space>
{
    auto output = std::vector<Space>{};
    LogTrace(OT_METHOD)(__FUNCTION__)(": processing ")(transactions_.size())(
        " transactions")
        .Flush();

    for (const auto& [txid, tx] : transactions_) {
        auto temp = tx->ExtractElements(style);
        output.insert(
            output.end(),
            std::make_move_iterator(temp.begin()),
            std::make_move_iterator(temp.end()));
    }

    LogTrace(OT_METHOD)(__FUNCTION__)(": extracted ")(output.size())(
        " elements")
        .Flush();

    return output;
}

auto Block::FindMatches(
    const api::client::Blockchain& blockchain,
    const FilterType style,
    const Patterns& outpoints,
    const Patterns& patterns) const noexcept -> Matches
{
    if (0 == (outpoints.size() + patterns.size())) { return {}; }

    auto output = Matches{};

    for (const auto& [txid, tx] : transactions_) {
        auto temp = tx->FindMatches(blockchain, style, outpoints, patterns);
        output.insert(
            output.end(),
            std::make_move_iterator(temp.begin()),
            std::make_move_iterator(temp.end()));
    }

    dedup(output);

    return output;
}

auto Block::Serialize(AllocateOutput bytes) const noexcept -> bool
{
    if (false == bool(bytes)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator")
            .Flush();

        return false;
    }

    const auto [size, txCount] = calculate_size();
    const auto out = bytes(size);

    if (false == out.valid(size)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to allocate output")
            .Flush();

        return false;
    }

    LogInsane(OT_METHOD)(__FUNCTION__)(": Serializing ")(txCount.Value())(
        " transactions into ")(size)(" bytes.")
        .Flush();
    auto remaining{size};
    auto it = static_cast<std::byte*>(out.data());

    if (false == header_.Serialize(preallocated(remaining, it))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to serialize header")
            .Flush();

        return false;
    }

    remaining -= header_bytes_;
    std::advance(it, header_bytes_);

    if (false == txCount.Encode(preallocated(remaining, it))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to serialize transaction count")
            .Flush();

        return false;
    }

    remaining -= txCount.Size();
    std::advance(it, txCount.Size());

    for (const auto& txid : index_) {
        try {
            const auto& pTX = transactions_.at(reader(txid));

            OT_ASSERT(pTX);

            const auto& tx = *pTX;
            const auto encoded = tx.Serialize(preallocated(remaining, it));

            if (false == encoded.has_value()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": failed to serialize transaction ")(tx.ID().asHex())
                    .Flush();

                return false;
            }

            remaining -= encoded.value();
            std::advance(it, encoded.value());
        } catch (...) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": missing transaction").Flush();

            return false;
        }
    }

    if (0 != remaining) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Extra bytes: ")(remaining)
            .Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
