// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

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

    Input(
        const api::Core& api,
        const std::uint32_t sequence,
        Outpoint&& previous,
        std::unique_ptr<const bitcoin::Script> script,
        const VersionNumber version,
        std::optional<std::size_t> size = {}) noexcept(false);
    Input(
        const api::Core& api,
        const std::uint32_t sequence,
        Outpoint&& previous,
        const ReadView coinbase,
        const VersionNumber version,
        std::optional<std::size_t> size = {}) noexcept(false);
    ~Input() final = default;

private:
    static const VersionNumber outpoint_version_;

    const api::Core& api_;
    const VersionNumber serialize_version_;
    const Outpoint previous_;
    const std::unique_ptr<const bitcoin::Script> script_;
    const Space coinbase_;
    const std::uint32_t sequence_;
    mutable std::optional<std::size_t> size_;
    mutable std::optional<std::size_t> normalized_size_;

    auto serialize(const AllocateOutput destination, const bool normalized)
        const noexcept -> std::optional<std::size_t>;

    Input(
        const api::Core& api,
        const std::uint32_t sequence,
        Outpoint&& previous,
        std::unique_ptr<const bitcoin::Script> script,
        Space&& coinbase,
        const VersionNumber version,
        std::optional<std::size_t> size) noexcept(false);
    Input() = delete;
    Input(const Input&) = delete;
    Input(Input&&) = delete;
    Input& operator=(const Input&) = delete;
    Input& operator=(Input&&) = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
