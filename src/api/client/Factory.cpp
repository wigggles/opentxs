// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "api/client/Factory.hpp"  // IWYU pragma: associated

#include "2_Factory.hpp"
#include "internal/api/client/Client.hpp"
#if OT_BLOCKCHAIN
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#endif  // OT_BLOCKCHAIN
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/block/Header.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/protobuf/BlockchainBlockHeader.pb.h"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/verify/BlockchainBlockHeader.hpp"

#if OT_BLOCKCHAIN
#define OT_METHOD "opentxs::api::client::implementation::Factory::"
#endif  // OT_BLOCKCHAIN

namespace opentxs
{
auto Factory::FactoryAPIClient(const api::client::internal::Manager& api)
    -> api::internal::Factory*
{
    return new api::client::implementation::Factory(api);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
Factory::Factory(const api::client::internal::Manager& client)
    : api::implementation::Factory(client)
    , client_(client)
{
}

#if OT_BLOCKCHAIN
auto Factory::BitcoinBlock(
    const opentxs::blockchain::Type chain,
    const ReadView bytes) const noexcept
    -> std::shared_ptr<const opentxs::blockchain::block::bitcoin::Block>
{
    return factory::BitcoinBlock(client_, chain, bytes);
}

auto Factory::BlockHeader(const proto::BlockchainBlockHeader& serialized) const
    -> std::unique_ptr<opentxs::blockchain::block::Header>
{
    if (false == proto::Validate(serialized, VERBOSE)) { return {}; }

    const auto type(static_cast<opentxs::blockchain::Type>(serialized.type()));

    switch (type) {
        case opentxs::blockchain::Type::Bitcoin:
        case opentxs::blockchain::Type::Bitcoin_testnet3:
        case opentxs::blockchain::Type::BitcoinCash:
        case opentxs::blockchain::Type::BitcoinCash_testnet3: {
            return std::unique_ptr<opentxs::blockchain::block::Header>(
                factory::BitcoinBlockHeader(client_, serialized));
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported type (")(
                static_cast<std::uint32_t>(type))(")")
                .Flush();

            return {};
        }
    }
}

auto Factory::BlockHeader(
    const opentxs::blockchain::Type type,
    const opentxs::Data& raw) const
    -> std::unique_ptr<opentxs::blockchain::block::Header>
{
    switch (type) {
        case opentxs::blockchain::Type::Bitcoin:
        case opentxs::blockchain::Type::Bitcoin_testnet3:
        case opentxs::blockchain::Type::BitcoinCash:
        case opentxs::blockchain::Type::BitcoinCash_testnet3: {
            return factory::BitcoinBlockHeader(client_, type, raw.Bytes());
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported type (")(
                static_cast<std::uint32_t>(type))(")")
                .Flush();

            return {};
        }
    }
}

auto Factory::BlockHeader(
    const opentxs::blockchain::Type type,
    const opentxs::blockchain::block::Hash& hash,
    const opentxs::blockchain::block::Hash& parent,
    const opentxs::blockchain::block::Height height) const
    -> std::unique_ptr<opentxs::blockchain::block::Header>
{
    switch (type) {
        case opentxs::blockchain::Type::Bitcoin:
        case opentxs::blockchain::Type::Bitcoin_testnet3:
        case opentxs::blockchain::Type::BitcoinCash:
        case opentxs::blockchain::Type::BitcoinCash_testnet3: {
            return std::unique_ptr<opentxs::blockchain::block::Header>(
                factory::BitcoinBlockHeader(
                    client_, type, hash, parent, height));
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported type (")(
                static_cast<std::uint32_t>(type))(")")
                .Flush();

            return {};
        }
    }
}
#endif  // OT_BLOCKCHAIN

auto Factory::PeerObject(const Nym_p& senderNym, const std::string& message)
    const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::Factory::PeerObject(client_, senderNym, message)};
}

auto Factory::PeerObject(
    const Nym_p& senderNym,
    const std::string& payment,
    const bool isPayment) const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::Factory::PeerObject(client_, senderNym, payment, isPayment)};
}

#if OT_CASH
auto Factory::PeerObject(
    const Nym_p& senderNym,
    const std::shared_ptr<blind::Purse> purse) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::Factory::PeerObject(client_, senderNym, purse)};
}
#endif

auto Factory::PeerObject(
    const OTPeerRequest request,
    const OTPeerReply reply,
    const VersionNumber version) const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::Factory::PeerObject(client_, request, reply, version)};
}

auto Factory::PeerObject(
    const OTPeerRequest request,
    const VersionNumber version) const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::Factory::PeerObject(client_, request, version)};
}

auto Factory::PeerObject(
    const Nym_p& signerNym,
    const proto::PeerObject& serialized) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{opentxs::Factory::PeerObject(
        client_.Contacts(), client_, signerNym, serialized)};
}

auto Factory::PeerObject(
    const Nym_p& recipientNym,
    const opentxs::Armored& encrypted,
    const opentxs::PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{opentxs::Factory::PeerObject(
        client_.Contacts(), client_, recipientNym, encrypted, reason)};
}
}  // namespace opentxs::api::client::implementation
