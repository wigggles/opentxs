// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client

class Core;
}  // namespace api
}  // namespace opentxs

namespace opentxs::blockchain::block::implementation
{
class Header : virtual public block::Header
{
public:
    using GenesisBlock = OTData;
    using GenesisBlockMap = std::map<blockchain::Type, GenesisBlock>;

    static const GenesisBlockMap genesis_blocks_;

    auto Difficulty() const noexcept -> OTWork final { return work_; }
    auto EffectiveState() const noexcept -> Status final;
    auto Hash() const noexcept -> const block::Hash& final;
    auto Height() const noexcept -> block::Height final;
    auto InheritedState() const noexcept -> Status final;
    auto IsBlacklisted() const noexcept -> bool final;
    auto IsDisconnected() const noexcept -> bool final;
    auto LocalState() const noexcept -> Status final;
    auto NumericHash() const noexcept -> OTNumericHash final;
    auto ParentHash() const noexcept -> const block::Hash& final;
    auto ParentWork() const noexcept -> OTWork final { return inherit_work_; }
    auto Position() const noexcept -> block::Position final;
    auto Serialize() const noexcept -> SerializedType override;
    auto Type() const noexcept -> blockchain::Type final { return type_; }
    auto Valid() const noexcept -> bool final;
    auto Work() const noexcept -> OTWork final;

    void CompareToCheckpoint(const block::Position& checkpoint) noexcept final;
    void InheritHeight(const block::Header& parent) final;
    void InheritState(const block::Header& parent) final;
    void InheritWork(const blockchain::Work& work) noexcept final;
    void RemoveBlacklistState() noexcept final;
    void RemoveCheckpointState() noexcept final;
    void SetDisconnectedState() noexcept final;

    ~Header() override = default;

protected:
    const api::Core& api_;
    const OTData hash_;
    const OTData pow_;
    const OTData parent_hash_;

    static auto minimum_work() -> OTWork;

    Header(
        const api::Core& api,
        const blockchain::Type type,
        const block::Hash& hash,
        const block::Hash& pow,
        const block::Hash& parentHash,
        const block::Height height,
        const blockchain::Work& work) noexcept;
    Header(
        const api::Core& api,
        const block::Hash& hash,
        const block::Hash& pow,
        const block::Hash& parentHash,
        const SerializedType& serialized) noexcept;
    Header(const Header& rhs) noexcept;

private:
    static const VersionNumber default_version_{1};
    static const VersionNumber local_data_version_{1};

    const VersionNumber version_;
    const blockchain::Type type_;
    const OTWork work_;
    block::Height height_;
    Status status_;
    Status inherit_status_;
    OTWork inherit_work_;

    Header(
        const api::Core& api,
        const VersionNumber version,
        const blockchain::Type type,
        const block::Hash& hash,
        const block::Hash& pow,
        const block::Hash& parentHash,
        const block::Height height,
        const Status status,
        const Status inheritStatus,
        const blockchain::Work& work,
        const blockchain::Work& inheritWork) noexcept;
    Header() = delete;
    Header(Header&&) = delete;
    auto operator=(const Header&) -> Header& = delete;
    auto operator=(Header &&) -> Header& = delete;
};
}  // namespace opentxs::blockchain::block::implementation
