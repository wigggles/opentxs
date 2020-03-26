// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Output final : public bitcoin::Output
{
public:
    auto CalculateSize() const noexcept -> std::size_t final;
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t>;
    auto Serialize(
        const std::uint32_t index,
        proto::BlockchainTransactionOutput& destination) const noexcept -> bool;
    auto Script() const noexcept -> const bitcoin::Script& final
    {
        return *script_;
    }
    auto Value() const noexcept -> std::int64_t final { return value_; }

    Output(const std::int64_t value, ScriptElements&& script) noexcept(false);
    Output(
        const std::int64_t value,
        const std::size_t size,
        const ReadView script,
        const VersionNumber version = default_version_) noexcept(false);
    ~Output() final = default;

private:
    static const VersionNumber default_version_;

    const VersionNumber serialize_version_;
    const std::int64_t value_;
    const std::unique_ptr<const bitcoin::Script> script_;
    mutable std::optional<std::size_t> size_;

    Output(
        const VersionNumber version,
        const std::int64_t value,
        std::unique_ptr<const bitcoin::Script> script,
        std::optional<std::size_t> size = {}) noexcept(false);
    Output() = delete;
    Output(const Output&) = delete;
    Output(Output&&) = delete;
    Output& operator=(const Output&) = delete;
    Output& operator=(Output&&) = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
