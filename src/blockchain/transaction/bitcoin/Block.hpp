// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::transaction::bitcoin
{
class Block
{
public:
    block::Version Version() const noexcept { return block_version_; }
    const Data& previousBlockHash() const noexcept
    {
        return previous_block_hash_;
    }
    const Data& merkleRootHash() const noexcept { return merkle_root_hash_; }
    Time getTimestamp() const noexcept { return timestamp_; }
    std::uint32_t difficultyTarget() const noexcept
    {
        return difficulty_target_;
    }
    std::uint32_t nonce() const noexcept { return nonce_; }
    const std::vector<std::shared_ptr<Transaction>> transactions() const
        noexcept
    {
        return transactions_;
    }

    OTData Encode() const noexcept;

    static Block* Factory(
        const opentxs::api::internal::Core& api,
        const opentxs::blockchain::Type network,
        const void* payload,
        const std::size_t size);

    static Block* Factory(
        const opentxs::api::internal::Core& api,
        const opentxs::blockchain::Type network,
        const block::Version block_version,
        const Data& previous_block_hash,
        const Data& merkle_root_hash,
        const Time timestamp,
        const std::uint32_t difficulty_target,
        const std::uint32_t nonce,
        const std::vector<std::shared_ptr<Transaction>>& transactions);

    static bool DecodeFromPayload(
        const opentxs::api::internal::Core& api,
        const opentxs::blockchain::Type network,
        const std::byte*& it,
        std::size_t& expectedSize,
        const std::size_t size,
        block::Version& block_version,
        Data& previous_block_hash,
        Data& merkle_root_hash,
        Time& timestamp,
        std::uint32_t& difficulty_target,
        std::uint32_t& nonce,
        std::vector<std::shared_ptr<Transaction>>& transactions) noexcept;

protected:
    const api::internal::Core& api_;
    const blockchain::Type network_;

private:
    const block::Version block_version_;
    const OTData previous_block_hash_;
    const OTData merkle_root_hash_;
    const Time timestamp_;
    const std::uint32_t difficulty_target_;
    const std::uint32_t nonce_;
    const std::vector<std::shared_ptr<bitcoin::Transaction>> transactions_;

    Block(
        const api::internal::Core& api,
        const blockchain::Type network,
        const block::Version block_version,
        const Data& previous_block_hash,
        const Data& merkle_root_hash,
        const Time timestamp,
        const std::uint32_t difficulty_target,
        const std::uint32_t nonce,
        const std::vector<std::shared_ptr<Transaction>>& transactions) noexcept;
    Block() = delete;
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    Block& operator=(const Block&) = delete;
    Block& operator=(Block&&) = delete;
};
}  // namespace opentxs::blockchain::transaction::bitcoin
