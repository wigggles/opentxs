// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::block::implementation
{
class Header : virtual public block::Header
{
public:
    using GenesisBlock = OTData;
    using GenesisBlockMap = std::map<blockchain::Type, GenesisBlock>;

    static const GenesisBlockMap genesis_blocks_;

    OTWork Difficulty() const noexcept final { return work_; }
    Status EffectiveState() const noexcept final;
    const block::Hash& Hash() const noexcept final;
    block::Height Height() const noexcept final;
    Status InheritedState() const noexcept final;
    bool IsBlacklisted() const noexcept final;
    bool IsDisconnected() const noexcept final;
    Status LocalState() const noexcept final;
    OTNumericHash NumericHash() const noexcept final;
    const block::Hash& ParentHash() const noexcept final;
    OTWork ParentWork() const noexcept final { return inherit_work_; }
    block::Position Position() const noexcept final;
    SerializedType Serialize() const noexcept override;
    blockchain::Type Type() const noexcept final { return type_; }
    bool Valid() const noexcept final;
    OTWork Work() const noexcept final;

    void CompareToCheckpoint(const block::Position& checkpoint) noexcept final;
    void InheritHeight(const block::Header& parent) final;
    void InheritState(const block::Header& parent) final;
    void InheritWork(const blockchain::Work& work) noexcept final;
    void RemoveBlacklistState() noexcept final;
    void RemoveCheckpointState() noexcept final;
    void SetDisconnectedState() noexcept final;

    ~Header() override = default;

protected:
    const api::internal::Core& api_;
    const OTData hash_;
    const OTData parent_hash_;

    static OTWork minimum_work();

    Header(
        const api::internal::Core& api,
        const blockchain::Type type,
        const block::Hash& hash,
        const block::Hash& parentHash,
        const block::Height height,
        const blockchain::Work& work) noexcept;
    Header(
        const api::internal::Core& api,
        const block::Hash& hash,
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
        const api::internal::Core& api,
        const VersionNumber version,
        const blockchain::Type type,
        const block::Hash& hash,
        const block::Hash& parentHash,
        const block::Height height,
        const Status status,
        const Status inheritStatus,
        const blockchain::Work& work,
        const blockchain::Work& inheritWork) noexcept;
    Header() = delete;
    Header(Header&&) = delete;
    Header& operator=(const Header&) = delete;
    Header& operator=(Header&&) = delete;
};
}  // namespace opentxs::blockchain::block::implementation
