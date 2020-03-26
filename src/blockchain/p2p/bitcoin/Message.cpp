// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/blockchain/Blockchain.hpp"

#include "blockchain/p2p/bitcoin/message/Cmpctblock.hpp"
#include "blockchain/p2p/bitcoin/message/Feefilter.hpp"
#include "blockchain/p2p/bitcoin/message/Getblocks.hpp"
#include "blockchain/p2p/bitcoin/message/Getblocktxn.hpp"
#include "blockchain/p2p/bitcoin/message/Merkleblock.hpp"
#include "blockchain/p2p/bitcoin/message/Reject.hpp"
#include "blockchain/p2p/bitcoin/message/Sendcmpct.hpp"
#include "blockchain/p2p/bitcoin/message/Tx.hpp"
#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/api/Api.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"

#include <algorithm>
#include <limits>
#include <memory>

#define OT_METHOD "opentxs::blockchain::p2p::bitcoin::Message::"

namespace opentxs
{
blockchain::p2p::bitcoin::Message* Factory::BitcoinP2PMessage(
    const api::internal::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::Message;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Invalid header")
            .Flush();

        return nullptr;
    }

    ReturnType* pMessage{nullptr};

    switch (pHeader->Command()) {
        case bitcoin::Command::addr: {
            pMessage = Factory::BitcoinP2PAddr(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::block: {
            pMessage = Factory::BitcoinP2PBlock(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::blocktxn: {
            pMessage = Factory::BitcoinP2PBlocktxn(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::cmpctblock: {
            pMessage = Factory::BitcoinP2PCmpctblock(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::feefilter: {
            pMessage = Factory::BitcoinP2PFeefilter(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::filteradd: {
            pMessage = Factory::BitcoinP2PFilteradd(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::filterclear: {
            pMessage = Factory::BitcoinP2PFilterclear(api, std::move(pHeader));
        } break;
        case bitcoin::Command::filterload: {
            pMessage = Factory::BitcoinP2PFilterload(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::getaddr: {
            pMessage = Factory::BitcoinP2PGetaddr(api, std::move(pHeader));
        } break;
        case bitcoin::Command::getblocks: {
            pMessage = Factory::BitcoinP2PGetblocks(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::getblocktxn: {
            pMessage = Factory::BitcoinP2PGetblocktxn(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::getdata: {
            pMessage = Factory::BitcoinP2PGetdata(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::getheaders: {
            pMessage = Factory::BitcoinP2PGetheaders(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::headers: {
            pMessage = Factory::BitcoinP2PHeaders(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::inv: {
            pMessage = Factory::BitcoinP2PInv(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::mempool: {
            pMessage = Factory::BitcoinP2PMempool(api, std::move(pHeader));
        } break;
        case bitcoin::Command::merkleblock: {
            pMessage = Factory::BitcoinP2PMerkleblock(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::notfound: {
            pMessage = Factory::BitcoinP2PNotfound(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::ping: {
            pMessage = Factory::BitcoinP2PPing(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::pong: {
            pMessage = Factory::BitcoinP2PPong(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::reject: {
            pMessage = Factory::BitcoinP2PReject(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::sendcmpct: {
            pMessage = Factory::BitcoinP2PSendcmpct(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::sendheaders: {
            pMessage = Factory::BitcoinP2PSendheaders(api, std::move(pHeader));
        } break;
        case bitcoin::Command::tx: {
            pMessage = Factory::BitcoinP2PTx(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::verack: {
            pMessage = Factory::BitcoinP2PVerack(api, std::move(pHeader));
        } break;
        case bitcoin::Command::version: {
            pMessage = Factory::BitcoinP2PVersion(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::getcfilters: {
            pMessage = Factory::BitcoinP2PGetcfilters(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::cfilter: {
            pMessage = Factory::BitcoinP2PCfilter(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::getcfheaders: {
            pMessage = Factory::BitcoinP2PGetcfheaders(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::cfheaders: {
            pMessage = Factory::BitcoinP2PCfheaders(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::getcfcheckpt: {
            pMessage = Factory::BitcoinP2PGetcfcheckpt(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::cfcheckpt: {
            pMessage = Factory::BitcoinP2PCfcheckpt(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::alert:
        case bitcoin::Command::checkorder:
        case bitcoin::Command::reply:
        case bitcoin::Command::submitorder:
        case bitcoin::Command::unknown:
        default: {
            LogOutput("opentxs::Factory::")(__FUNCTION__)(
                ": Unsupported message type")
                .Flush();
            return nullptr;
        }
    }

    return pMessage;
}
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin
{
Message::Message(
    const api::internal::Core& api,
    const blockchain::Type network,
    const bitcoin::Command command) noexcept
    : api_(api)
    , header_(std::make_unique<Header>(api, network, command))
{
    OT_ASSERT(header_);
}

Message::Message(
    const api::internal::Core& api,
    std::unique_ptr<Header> header) noexcept
    : api_(api)
    , header_(std::move(header))
{
    OT_ASSERT(header_);
}

OTData Message::Encode() const
{
    OTData output = header_->Encode();
    output += payload();

    return output;
}

OTData Message::calculate_checksum(const Data& payload) const noexcept
{
    auto output = Data::Factory();
    P2PMessageHash(
        api_, header_->Network(), payload.Bytes(), output->WriteInto());

    return output;
}

void Message::init_hash() noexcept
{
    const auto data = payload();
    const auto size = data->size();
    header_->SetChecksum(size, calculate_checksum(data));
}

std::size_t Message::MaxPayload()
{
    static_assert(
        std::numeric_limits<std::size_t>::max() >=
        std::numeric_limits<std::uint32_t>::max());

    return std::numeric_limits<std::uint32_t>::max();
}

void Message::verify_checksum() const noexcept(false)
{
    const auto calculated = calculate_checksum(payload());
    const auto& header = header_->Checksum();

    if (header != calculated) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Checksum failure").Flush();
        LogOutput("*  Calculated Payload:  ")(payload()->asHex()).Flush();
        LogOutput("*  Calculated Checksum: ")(calculated->asHex()).Flush();
        LogOutput("*  Provided Checksum:   ")(header.asHex()).Flush();

        throw std::runtime_error("checksum failure");
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin
