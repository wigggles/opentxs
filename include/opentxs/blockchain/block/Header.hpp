// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_HEADER_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_HEADER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/Proto.hpp"

#include <cstdint>

namespace opentxs
{
namespace blockchain
{
namespace block
{
class Header
{
public:
    using SerializedType = proto::BlockchainBlockHeader;

    enum class Status : std::uint32_t {
        Error,
        Normal,
        Disconnected,
        CheckpointBanned,
        Checkpoint
    };

    EXPORT virtual std::unique_ptr<Header> clone() const noexcept = 0;
    EXPORT virtual OTWork Difficulty() const noexcept = 0;
    EXPORT virtual Status EffectiveState() const noexcept = 0;
    EXPORT virtual const block::Hash& Hash() const noexcept = 0;
    EXPORT virtual block::Height Height() const noexcept = 0;
    EXPORT virtual Status InheritedState() const noexcept = 0;
    EXPORT virtual bool IsBlacklisted() const noexcept = 0;
    EXPORT virtual bool IsDisconnected() const noexcept = 0;
    EXPORT virtual Status LocalState() const noexcept = 0;
    EXPORT virtual OTNumericHash NumericHash() const noexcept = 0;
    EXPORT virtual const block::Hash& ParentHash() const noexcept = 0;
    EXPORT virtual OTWork ParentWork() const noexcept = 0;
    EXPORT virtual block::Position Position() const noexcept = 0;
    EXPORT virtual SerializedType Serialize() const noexcept = 0;
    EXPORT virtual OTNumericHash Target() const noexcept = 0;
    EXPORT virtual blockchain::Type Type() const noexcept = 0;
    EXPORT virtual bool Valid() const noexcept = 0;
    EXPORT virtual OTWork Work() const noexcept = 0;

    EXPORT virtual void CompareToCheckpoint(
        const block::Position& checkpoint) noexcept = 0;
    /// Throws std::runtime_error if parent hash incorrect
    EXPORT virtual void InheritHeight(const Header& parent) = 0;
    /// Throws std::runtime_error if parent hash incorrect
    EXPORT virtual void InheritState(const Header& parent) = 0;
    EXPORT virtual void InheritWork(
        const blockchain::Work& parent) noexcept = 0;
    EXPORT virtual void RemoveBlacklistState() noexcept = 0;
    EXPORT virtual void RemoveCheckpointState() noexcept = 0;
    EXPORT virtual void SetDisconnectedState() noexcept = 0;

    EXPORT virtual ~Header() = default;

protected:
    Header() noexcept = default;

private:
    Header(const Header&) = delete;
    Header(Header&&) = delete;
    Header& operator=(const Header&) = delete;
    Header& operator=(Header&&) = delete;
};
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
