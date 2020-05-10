// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <optional>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/core/Identifier.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Output final : public bitcoin::Output
{
public:
    auto CalculateSize() const noexcept -> std::size_t final;
    auto ExtractElements(const filter::Type style) const noexcept
        -> std::vector<Space> final;
    auto FindMatches(
        const ReadView txid,
        const FilterType type,
        const Patterns& patterns) const noexcept -> Matches final;
    auto Keys() const noexcept -> std::vector<KeyID> final { return keys_; }
    auto MergeMetadata(const SerializeType& rhs) const noexcept -> void final;
    auto Payee() const noexcept -> ContactID final { return payee_; }
    auto Payer() const noexcept -> ContactID final { return payer_; }
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t>;
    auto Serialize(SerializeType& destination) const noexcept -> bool;
    auto Script() const noexcept -> const bitcoin::Script& final
    {
        return *script_;
    }
    auto Value() const noexcept -> std::int64_t final { return value_; }

    Output(
        const api::Core& api,
        const std::uint32_t index,
        const std::int64_t value,
        const std::size_t size,
        const ReadView script,
        const VersionNumber version = default_version_) noexcept(false);
    ~Output() final = default;

private:
    static const VersionNumber default_version_;

    const api::Core& api_;
    const VersionNumber serialize_version_;
    const std::uint32_t index_;
    const std::int64_t value_;
    const std::unique_ptr<const bitcoin::Script> script_;
    mutable std::optional<std::size_t> size_;
    mutable OTIdentifier payee_;
    mutable OTIdentifier payer_;
    mutable std::vector<KeyID> keys_;

    Output(
        const api::Core& api,
        const VersionNumber version,
        const std::uint32_t index,
        const std::int64_t value,
        std::unique_ptr<const bitcoin::Script> script,
        std::optional<std::size_t> size = {}) noexcept(false);
    Output() = delete;
    Output(const Output&) = delete;
    Output(Output&&) = delete;
    auto operator=(const Output&) -> Output& = delete;
    auto operator=(Output &&) -> Output& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
