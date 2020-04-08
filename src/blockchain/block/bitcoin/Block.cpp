// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include "blockchain/block/Block.hpp"
#include "internal/api/Api.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/block/Block.hpp"

#include <boost/endian/buffers.hpp>

#include <algorithm>
#include <cstring>
#include <map>
#include <stdexcept>
#include <vector>

#include "Block.hpp"

#define OT_METHOD "opentxs::blockchain::block::bitcoin::implementation::Block::"

namespace be = boost::endian;

namespace opentxs
{
using ReturnType = blockchain::block::bitcoin::implementation::Block;

auto Factory::BitcoinBlock(
    const api::internal::Core& api,
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
        auto expectedSize = std::size_t{80};

        if (in.size() < expectedSize) {
            throw std::runtime_error("Block size too short (header)");
        }

        auto pHeader = BitcoinBlockHeader(
            api, chain, {reinterpret_cast<const char*>(it), 80});

        if (false == bool(pHeader)) {
            throw std::runtime_error("Invalid block header");
        }

        std::advance(it, 80);
        expectedSize += 1;

        if (in.size() < expectedSize) {
            throw std::runtime_error(
                "Block size too short (transaction count)");
        }

        auto transactionCount = std::size_t{};

        if (false == bb::DecodeCompactSizeFromPayload(
                         it, expectedSize, in.size(), transactionCount)) {
            throw std::runtime_error("Failed to decode transaction count");
        }

        if (0 == transactionCount) { throw std::runtime_error("Empty block"); }

        auto counter = int{-1};
        auto index = ReturnType::TxidIndex{};
        auto transactions = ReturnType::TransactionMap{};

        while (transactions.size() < transactionCount) {
            auto data = bb::EncodedTransaction::Deserialize(
                api,
                chain,
                ReadView{reinterpret_cast<const char*>(it),
                         in.size() - expectedSize});
            const auto txBytes = data.size();
            std::advance(it, txBytes);
            expectedSize += txBytes;
            auto& txid = index.emplace_back(data.txid_);
            transactions.emplace(
                reader(txid),
                BitcoinTransaction(
                    api, chain, (0 == ++counter), std::move(data)));
        }

        return std::make_shared<ReturnType>(
            api,
            chain,
            std::move(pHeader),
            std::move(index),
            std::move(transactions));
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs

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

    if (size > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("Too many bytes");
    }

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
const Block::value_type Block::null_tx_{};

Block::Block(
    const api::internal::Core& api,
    const blockchain::Type chain,
    std::unique_ptr<const internal::Header> header,
    TxidIndex&& index,
    TransactionMap&& transactions) noexcept(false)
    : block::implementation::Block(api, *header)
    , header_p_(std::move(header))
    , header_(*header_p_)
    , index_(std::move(index))
    , transactions_(std::move(transactions))
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
    const FilterType style,
    const Patterns& outpoints,
    const Patterns& patterns) const noexcept -> Matches
{
    if (0 == (outpoints.size() + patterns.size())) { return {}; }

    auto output = Matches{};

    for (const auto& [txid, tx] : transactions_) {
        auto temp = tx->FindMatches(style, outpoints, patterns);
        output.insert(
            output.end(),
            std::make_move_iterator(temp.begin()),
            std::make_move_iterator(temp.end()));
    }

    dedup(output);

    return output;
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
