// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                 // IWYU pragma: associated
#include "1_Internal.hpp"               // IWYU pragma: associated
#include "blockchain/block/Header.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/BlockchainBlockHeader.pb.h"
#include "opentxs/protobuf/BlockchainBlockLocalData.pb.h"

// #define OT_METHOD "opentxs::blockchain::block::Header::"

namespace opentxs::factory
{
auto GenesisBlockHeader(
    const api::Core& api,
    const blockchain::Type type) noexcept
    -> std::unique_ptr<blockchain::block::Header>
{
    switch (type) {
        case blockchain::Type::Bitcoin:
        case blockchain::Type::BitcoinCash:
        case blockchain::Type::Bitcoin_testnet3:
        case blockchain::Type::BitcoinCash_testnet3: {
            const auto& hex =
                blockchain::params::Data::chains_.at(type).genesis_header_hex_;
            const auto data = api.Factory().Data(hex, StringStyle::Hex);

            return factory::BitcoinBlockHeader(api, type, data->Bytes());
        }
        case opentxs::blockchain::Type::Unknown:
        case opentxs::blockchain::Type::Ethereum_frontier:
        case opentxs::blockchain::Type::Ethereum_ropsten:
        case opentxs::blockchain::Type::Litecoin:
        case opentxs::blockchain::Type::Litecoin_testnet4:
        default: {
            LogOutput("opentxs::factory::")(__FUNCTION__)(
                ": Unsupported type (")(static_cast<std::uint32_t>(type))(")")
                .Flush();

            return nullptr;
        }
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block
{
auto BlankHash() noexcept -> pHash
{
    return Data::Factory(
        "0x0000000000000000000000000000000000000000000000000000000000000000",
        Data::Mode::Hex);
}
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block::implementation
{
Header::Header(
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
    const blockchain::Work& inheritWork) noexcept
    : api_(api)
    , hash_(hash)
    , pow_(pow)
    , parent_hash_(parentHash)
    , version_(version)
    , type_(type)
    , work_(work)
    , height_(height)
    , status_(status)
    , inherit_status_(inheritStatus)
    , inherit_work_(inheritWork)
{
}

Header::Header(
    const api::Core& api,
    const block::Hash& hash,
    const block::Hash& pow,
    const block::Hash& parentHash,
    const SerializedType& serialized) noexcept
    : Header(
          api,
          serialized.version(),
          static_cast<blockchain::Type>(serialized.type()),
          hash,
          pow,
          parentHash,
          serialized.local().height(),
          static_cast<Status>(serialized.local().status()),
          static_cast<Status>(serialized.local().inherit_status()),
          OTWork{factory::Work(serialized.local().work())},
          OTWork{factory::Work(serialized.local().inherit_work())})
{
}

Header::Header(
    const api::Core& api,
    const blockchain::Type type,
    const block::Hash& hash,
    const block::Hash& pow,
    const block::Hash& parentHash,
    const block::Height height,
    const blockchain::Work& work) noexcept
    : Header(
          api,
          default_version_,
          type,
          hash,
          pow,
          parentHash,
          height,
          (0 == height) ? Status::Checkpoint : Status::Normal,
          Status::Normal,
          work,
          minimum_work(type))
{
}

Header::Header(const Header& rhs) noexcept
    : Header(
          rhs.api_,
          rhs.default_version_,
          rhs.type_,
          rhs.hash_,
          rhs.pow_,
          rhs.parent_hash_,
          rhs.height_,
          rhs.status_,
          rhs.inherit_status_,
          rhs.work_,
          rhs.inherit_work_)
{
}

auto Header::EffectiveState() const noexcept -> Header::Status
{
    if (Status::CheckpointBanned == inherit_status_) { return inherit_status_; }

    if (Status::Disconnected == inherit_status_) { return inherit_status_; }

    if (Status::Checkpoint == status_) { return Status::Normal; }

    return status_;
}

void Header::CompareToCheckpoint(const block::Position& checkpoint) noexcept
{
    const auto& [height, hash] = checkpoint;

    if (height == height_) {
        if (hash == hash_) {
            status_ = Status::Checkpoint;
        } else {
            status_ = Status::CheckpointBanned;
        }
    } else {
        status_ = Header::Status::Normal;
    }
}

auto Header::Hash() const noexcept -> const block::Hash& { return hash_; }

auto Header::Height() const noexcept -> block::Height { return height_; }

auto Header::InheritedState() const noexcept -> Header::Status
{
    return inherit_status_;
}

void Header::InheritHeight(const block::Header& parent)
{
    if (parent.Hash() != parent_hash_) {
        throw std::runtime_error("Invalid parent");
    }

    height_ = parent.Height() + 1;
}

void Header::InheritState(const block::Header& parent)
{
    if (parent.Hash() != parent_hash_) {
        throw std::runtime_error("Invalid parent");
    }

    inherit_status_ = parent.EffectiveState();
}

void Header::InheritWork(const blockchain::Work& work) noexcept
{
    inherit_work_ = work;
}

auto Header::IsDisconnected() const noexcept -> bool
{
    return Status::Disconnected == EffectiveState();
}

auto Header::IsBlacklisted() const noexcept -> bool
{
    return Status::CheckpointBanned == EffectiveState();
}

auto Header::LocalState() const noexcept -> Header::Status { return status_; }

auto Header::minimum_work(const blockchain::Type chain) -> OTWork
{
    const auto maxTarget =
        OTNumericHash{factory::NumericHashNBits(NumericHash::MaxTarget(chain))};

    return OTWork{factory::Work(chain, maxTarget)};
}

auto Header::NumericHash() const noexcept -> OTNumericHash
{
    return OTNumericHash{factory::NumericHash(pow_)};
}

auto Header::ParentHash() const noexcept -> const block::Hash&
{
    return parent_hash_;
}

auto Header::Position() const noexcept -> block::Position
{
    return {height_, hash_};
}

void Header::RemoveBlacklistState() noexcept
{
    status_ = Status::Normal;
    inherit_status_ = Status::Normal;
}

void Header::RemoveCheckpointState() noexcept { status_ = Status::Normal; }

auto Header::Serialize() const noexcept -> Header::SerializedType
{
    proto::BlockchainBlockHeader output{};
    output.set_version(version_);
    output.set_type(static_cast<std::uint32_t>(type_));
    auto& local = *output.mutable_local();
    local.set_version(local_data_version_);
    local.set_height(height_);
    local.set_status(static_cast<std::uint32_t>(status_));
    local.set_inherit_status(static_cast<std::uint32_t>(inherit_status_));
    local.set_work(work_->asHex());
    local.set_inherit_work(inherit_work_->asHex());

    return output;
}

void Header::SetDisconnectedState() noexcept
{
    status_ = Status::Disconnected;
    inherit_status_ = Status::Error;
}

auto Header::Valid() const noexcept -> bool { return NumericHash() < Target(); }

auto Header::Work() const noexcept -> OTWork
{
    if (parent_hash_->IsNull()) {
        return work_;
    } else {
        return work_ + inherit_work_;
    }
}
}  // namespace opentxs::blockchain::block::implementation
