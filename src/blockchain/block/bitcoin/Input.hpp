// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <optional>
#include <set>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
}  // namespace api

namespace proto
{
class BlockchainTransactionInput;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Input final : public bitcoin::Input
{
public:
    static const VersionNumber default_version_;

    auto CalculateSize(const bool normalized) const noexcept
        -> std::size_t final;
    auto ExtractElements(const filter::Type style) const noexcept
        -> std::vector<Space> final;
    auto FindMatches(
        const ReadView txid,
        const FilterType type,
        const Patterns& txos,
        const Patterns& elements) const noexcept -> Matches final;
    auto Keys() const noexcept -> std::vector<KeyID> final;
    auto PreviousOutput() const noexcept -> const Outpoint& final
    {
        return previous_;
    }
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto SerializeNormalized(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize(
        const std::uint32_t index,
        proto::BlockchainTransactionInput& destination) const noexcept
        -> bool final;
    auto Script() const noexcept -> const bitcoin::Script& final
    {
        return *script_;
    }
    auto Sequence() const noexcept -> std::uint32_t final { return sequence_; }
    auto Witness() const noexcept -> const std::vector<Space>& final
    {
        return witness_;
    }

    Input(
        const api::client::Manager& api,
        const std::uint32_t sequence,
        Outpoint&& previous,
        std::vector<Space>&& witness,
        std::unique_ptr<const bitcoin::Script> script,
        const VersionNumber version,
        std::optional<std::size_t> size = {}) noexcept(false);
    Input(
        const api::client::Manager& api,
        const std::uint32_t sequence,
        Outpoint&& previous,
        std::vector<Space>&& witness,
        const ReadView coinbase,
        const VersionNumber version,
        std::optional<std::size_t> size = {}) noexcept(false);
    ~Input() final = default;

private:
    static const VersionNumber outpoint_version_;

    const api::client::Manager& api_;
    const VersionNumber serialize_version_;
    const Outpoint previous_;
    const std::vector<Space> witness_;
    const std::unique_ptr<const bitcoin::Script> script_;
    const Space coinbase_;
    const std::uint32_t sequence_;
    mutable std::optional<std::size_t> size_;
    mutable std::optional<std::size_t> normalized_size_;
    mutable std::set<KeyID> keys_;

    auto serialize(const AllocateOutput destination, const bool normalized)
        const noexcept -> std::optional<std::size_t>;

    Input(
        const api::client::Manager& api,
        const std::uint32_t sequence,
        Outpoint&& previous,
        std::vector<Space>&& witness,
        std::unique_ptr<const bitcoin::Script> script,
        Space&& coinbase,
        const VersionNumber version,
        std::optional<std::size_t> size) noexcept(false);
    Input() = delete;
    Input(const Input&) = delete;
    Input(Input&&) = delete;
    auto operator=(const Input&) -> Input& = delete;
    auto operator=(Input &&) -> Input& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
