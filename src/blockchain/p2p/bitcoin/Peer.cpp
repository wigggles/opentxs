// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/core/Log.hpp"

#include "blockchain/bitcoin/Inventory.hpp"
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
#include "blockchain/p2p/Peer.hpp"
#include "internal/api/Api.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/blockchain/Blockchain.hpp"

#include <functional>
#include <optional>

#include "Peer.hpp"

#define OT_METHOD "opentxs::blockchain::p2p::bitcoin::implementation::Peer::"

namespace opentxs
{
blockchain::p2p::internal::Peer* Factory::BitcoinP2PPeerLegacy(
    const api::internal::Core& api,
    const blockchain::client::internal::Network& network,
    const blockchain::client::internal::PeerManager& manager,
    const int id,
    std::unique_ptr<blockchain::p2p::internal::Address> address,
    boost::asio::io_context& context)
{
    namespace p2p = blockchain::p2p;
    using ReturnType = p2p::bitcoin::implementation::Peer;

    if (false == bool(address)) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Invalid null address")
            .Flush();

        return nullptr;
    }

    switch (address->Type()) {
        case p2p::Network::ipv6:
        case p2p::Network::ipv4: {
            break;
        }
        default: {
            LogOutput("opentxs::Factory::")(__FUNCTION__)(
                ": Unsupported address type")
                .Flush();

            return nullptr;
        }
    }

    return new ReturnType(
        api, network, manager, id, std::move(address), context);
}
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::implementation
{
const std::map<Command, Peer::CommandFunction> Peer::command_map_{
    {Command::addr, &Peer::process_addr},
    {Command::block, &Peer::process_block},
    {Command::blocktxn, &Peer::process_blocktxn},
    {Command::cfcheckpt, &Peer::process_cfcheckpt},
    {Command::cfheaders, &Peer::process_cfheaders},
    {Command::cfilter, &Peer::process_cfilter},
    {Command::cmpctblock, &Peer::process_cmpctblock},
    {Command::feefilter, &Peer::process_feefilter},
    {Command::filteradd, &Peer::process_filteradd},
    {Command::filterclear, &Peer::process_filterclear},
    {Command::filterload, &Peer::process_filterload},
    {Command::getaddr, &Peer::process_getaddr},
    {Command::getblocks, &Peer::process_getblocks},
    {Command::getblocktxn, &Peer::process_getblocktxn},
    {Command::getcfcheckpt, &Peer::process_getcfcheckpt},
    {Command::getcfheaders, &Peer::process_getcfheaders},
    {Command::getcfilters, &Peer::process_getcfilters},
    {Command::getdata, &Peer::process_getdata},
    {Command::getheaders, &Peer::process_getheaders},
    {Command::headers, &Peer::process_headers},
    {Command::inv, &Peer::process_inv},
    {Command::mempool, &Peer::process_mempool},
    {Command::merkleblock, &Peer::process_merkleblock},
    {Command::notfound, &Peer::process_notfound},
    {Command::ping, &Peer::process_ping},
    {Command::pong, &Peer::process_pong},
    {Command::reject, &Peer::process_reject},
    {Command::sendcmpct, &Peer::process_sendcmpct},
    {Command::sendheaders, &Peer::process_sendheaders},
    {Command::tx, &Peer::process_tx},
    {Command::verack, &Peer::process_verack},
    {Command::version, &Peer::process_version},
};

const std::string Peer::user_agent_{"/opentxs:" OPENTXS_VERSION_STRING "/"};

Peer::Peer(
    const api::internal::Core& api,
    const client::internal::Network& network,
    const client::internal::PeerManager& manager,
    const int id,
    std::unique_ptr<internal::Address> address,
    boost::asio::io_context& context,
    const bool relay,
    const std::set<p2p::Service>& localServices,
    const ProtocolVersion protocol) noexcept
    : p2p::implementation::Peer(
          api,
          network,
          manager,
          id,
          HeaderType::Size(),
          MessageType::MaxPayload(),
          std::move(address),
          context)
    , chain_(address_.Chain())
    , protocol_((0 == protocol) ? default_protocol_version_ : protocol)
    , nonce_(nonce(api_))
    , magic_(GetMagic(chain_))
    , local_services_(get_local_services(protocol_, chain_, localServices))
    , relay_(relay)
{
    init();
}

void Peer::get_body_size() noexcept
{
    OT_ASSERT(HeaderType::Size() == header_->size());

    try {
        HeaderType::BitcoinFormat header{header_};
        body_bytes_ = header.PayloadSize();
    } catch (...) {
        body_bytes_ = 0;
    }
}

std::set<p2p::Service> Peer::get_local_services(
    const ProtocolVersion version,
    const blockchain::Type network,
    const std::set<p2p::Service>& input) noexcept
{
    auto output{input};

    if (blockchain::Type::BitcoinCash == network) {
        output.emplace(p2p::Service::BitcoinCash);
    }

    if (blockchain::Type::BitcoinCash_testnet3 == network) {
        output.emplace(p2p::Service::BitcoinCash);
    }

    return output;
}

Peer::Nonce Peer::nonce(const api::internal::Core& api) noexcept
{
    Nonce output{0};
    const auto random =
        api.Crypto().Util().RandomizeMemory(&output, sizeof(output));

    OT_ASSERT(random);

    return output;
}

void Peer::ping() noexcept
{
    std::unique_ptr<Message> pPing{
        Factory::BitcoinP2PPing(api_, chain_, nonce_)};

    if (false == bool(pPing)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct ping")
            .Flush();

        return;
    }

    const auto& ping = *pPing;
    send(ping.Encode());
}

void Peer::pong() noexcept
{
    std::unique_ptr<Message> pPong{
        Factory::BitcoinP2PPong(api_, chain_, nonce_)};

    if (false == bool(pPong)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct pong")
            .Flush();

        return;
    }

    const auto& pong = *pPong;
    send(pong.Encode());
}

void Peer::process_addr(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Addr> pMessage{
        Factory::BitcoinP2PAddr(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;
    download_peers_.Bump();

    for (const auto& address : message) {
        if (false == running_.get()) { return; }

        auto pAddress = address.clone_internal();

        OT_ASSERT(pAddress);

        pAddress->SetLastConnected({});
        manager_.Database().AddOrUpdate(std::move(pAddress));
    }
}

void Peer::process_block(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Block> pMessage{
        Factory::BitcoinP2PBlock(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_blocktxn(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Blocktxn> pMessage{
        Factory::BitcoinP2PBlocktxn(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_cfcheckpt(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Cfcheckpt> pMessage{
        Factory::BitcoinP2PCfcheckpt(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_cfheaders(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Cfheaders> pMessage{
        Factory::BitcoinP2PCfheaders(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_cfilter(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Cfilter> pMessage{
        Factory::BitcoinP2PCfilter(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;
    const auto type = message.Type();
    auto work = zmq::Message::Factory();
    work->AddFrame(Data::Factory(&type, sizeof(type)));
    work->AddFrame(message.Hash());
    work->AddFrame(message.Filter()->Serialize());
    network_.FilterPipeline().Push(work);
}

void Peer::process_cmpctblock(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::Cmpctblock> pMessage{
        Factory::BitcoinP2PCmpctblock(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_feefilter(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::Feefilter> pMessage{
        Factory::BitcoinP2PFeefilter(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_filteradd(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Filteradd> pMessage{
        Factory::BitcoinP2PFilteradd(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_filterclear(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Filterclear> pMessage{
        Factory::BitcoinP2PFilterclear(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_filterload(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Filterload> pMessage{
        Factory::BitcoinP2PFilterload(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_getaddr(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Getaddr> pMessage{
        Factory::BitcoinP2PGetaddr(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_getblocks(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::Getblocks> pMessage{
        Factory::BitcoinP2PGetblocks(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_getblocktxn(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::Getblocktxn> pMessage{
        Factory::BitcoinP2PGetblocktxn(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_getcfcheckpt(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Getcfcheckpt> pMessage{
        Factory::BitcoinP2PGetcfcheckpt(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_getcfheaders(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Getcfheaders> pMessage{
        Factory::BitcoinP2PGetcfheaders(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_getcfilters(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Getcfilters> pMessage{
        Factory::BitcoinP2PGetcfilters(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_getdata(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Getdata> pMessage{
        Factory::BitcoinP2PGetdata(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_getheaders(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Getheaders> pMessage{
        Factory::BitcoinP2PGetheaders(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_headers(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Headers> pMessage{
        Factory::BitcoinP2PHeaders(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    using Promise = std::promise<void>;
    auto* promise = new Promise{};

    OT_ASSERT(nullptr != promise);

    auto future = promise->get_future();
    auto pointer = reinterpret_cast<std::uintptr_t>(promise);
    const auto& message = *pMessage;
    auto zmq = api_.ZeroMQ().Message();
    zmq->AddFrame(pointer);
    zmq->AddFrame();

    for (const auto& header : message) { zmq->AddFrame(header.Serialize()); }

    network_.HeaderPipeline().Push(zmq);

    if (std::future_status::ready ==
        future.wait_for(std::chrono::seconds(30))) {
        if (false == network_.IsSynchronized()) { request_headers(); }
    }
}

void Peer::process_inv(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Inv> pMessage{
        Factory::BitcoinP2PInv(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;

    for (const auto& inv : message) {
        if (false == running_.get()) { return; }

        LogVerbose("Received ")(blockchain::internal::DisplayString(chain_))(
            " ")(inv.DisplayType())(" (")(inv.hash_->asHex())(")")
            .Flush();
        using Inventory = blockchain::bitcoin::Inventory;

        switch (inv.type_) {
            case Inventory::Type::MsgBlock:
            case Inventory::Type::MsgWitnessBlock: {
                request_headers(inv.hash_);
            } break;
            default: {
            }
        }
    }
}

void Peer::process_mempool(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Mempool> pMessage{
        Factory::BitcoinP2PMempool(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_merkleblock(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::Merkleblock> pMessage{
        Factory::BitcoinP2PMerkleblock(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_message(const zmq::Message& message) noexcept
{
    if (false == running_.get()) { return; }

    const auto& body = message.Body();

    if (2 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto& headerBytes = body.at(0);
    const auto& payloadBytes = body.at(1);
    std::unique_ptr<HeaderType> pHeader{
        Factory::BitcoinP2PHeader(api_, headerBytes)};

    if (false == bool(pHeader)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message header ")
            .Flush();
        disconnect();

        return;
    }

    auto& header = *pHeader;

    if (header.Network() != chain_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong network on message")
            .Flush();
        disconnect();

        return;
    }

    const auto command = header.Command();

    if (false == message::VerifyChecksum(api_, header, payloadBytes)) {
        LogNormal("Invalid checksum on ")(blockchain::internal::DisplayString(
            chain_))(" ")(CommandName(command))(" message")
            .Flush();
        disconnect();

        return;
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Received ")(
        blockchain::internal::DisplayString(chain_))(" ")(CommandName(command))(
        " command")
        .Flush();

    try {
        (this->*command_map_.at(command))(std::move(pHeader), payloadBytes);
    } catch (...) {
        auto raw = Data::Factory(headerBytes);
        auto unknown = Data::Factory();
        raw->Extract(12, unknown, 4);
        LogOutput(OT_METHOD)(__FUNCTION__)(": No handler for command ")(
            unknown->str())
            .Flush();

        return;
    }
}

void Peer::process_notfound(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Notfound> pMessage{
        Factory::BitcoinP2PNotfound(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_ping(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Ping> pMessage{
        Factory::BitcoinP2PPing(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;

    if (message.Nonce() == nonce_) { disconnect(); }

    pong();
}

void Peer::process_pong(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Pong> pMessage{
        Factory::BitcoinP2PPong(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;

    if (message.Nonce() == nonce_) { disconnect(); }
}

void Peer::process_reject(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::Reject> pMessage{Factory::BitcoinP2PReject(
        api_,
        std::move(header),
        protocol_.load(),
        payload.data(),
        payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_sendcmpct(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::Sendcmpct> pMessage{
        Factory::BitcoinP2PSendcmpct(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_sendheaders(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Sendheaders> pMessage{
        Factory::BitcoinP2PSendheaders(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_tx(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::Tx> pMessage{Factory::BitcoinP2PTx(
        api_,
        std::move(header),
        protocol_.load(),
        payload.data(),
        payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

void Peer::process_verack(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Verack> pMessage{
        Factory::BitcoinP2PVerack(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    incoming_handshake_ = true;
    check_handshake();
}

void Peer::process_version(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload)
{
    const std::unique_ptr<message::internal::Version> pVersion{
        Factory::BitcoinP2PVersion(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pVersion)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& version = *pVersion;
    network_.UpdateHeight(version.Height());
    protocol_.store(std::min(protocol_.load(), version.ProtocolVersion()));
    const auto services = version.RemoteServices();
    update_address_services(services);
    const bool unsuitable = (0 == services.count(p2p::Service::Network)) &&
                            (0 == services.count(p2p::Service::Limited));

    if (unsuitable) {
        disconnect();

        return;
    }

    if ((1 == services.count(p2p::Service::Network)) ||
        (1 == services.count(p2p::Service::Limited))) {
        header_worker_.Start(manager_.Endpoint(Task::Getheaders));
    }

    if (1 == services.count(p2p::Service::CompactFilters)) {
        cfilter_worker_.Start(manager_.Endpoint(Task::Getcfilters));
    }

    std::unique_ptr<Message> pVerack{Factory::BitcoinP2PVerack(api_, chain_)};

    if (false == bool(pVerack)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct verack")
            .Flush();

        return;
    }

    const auto& verack = *pVerack;
    send(verack.Encode());
    outgoing_handshake_ = true;
    check_handshake();
}

void Peer::request_addresses() noexcept
{
    std::unique_ptr<Message> pMessage{Factory::BitcoinP2PGetaddr(api_, chain_)};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct getaddr")
            .Flush();

        return;
    }

    const auto& message = *pMessage;
    send(message.Encode());
}

void Peer::request_cfilter(zmq::Message& in) noexcept
{
    if (false == running_.get()) { return; }

    const auto& body = in.Body();

    if (3 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid work").Flush();

        return;
    }

    try {
        std::unique_ptr<Message> pMessage{Factory::BitcoinP2PGetcfilters(
            api_,
            chain_,
            body.at(0).as<filter::Type>(),
            body.at(1).as<block::Height>(),
            Data::Factory(body.at(2)))};

        if (false == bool(pMessage)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to construct getcfilters")
                .Flush();

            return;
        }

        const auto& message = *pMessage;
        send(message.Encode());
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid parameters").Flush();
    }
}

void Peer::request_headers() noexcept
{
    request_headers(api_.Factory().Data());
}

void Peer::request_headers(const block::Hash& hash) noexcept
{
    std::unique_ptr<Message> pMessage{Factory::BitcoinP2PGetheaders(
        api_,
        chain_,
        protocol_.load(),
        network_.HeaderOracle().RecentHashes(),
        hash)};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct getheaders")
            .Flush();

        return;
    }

    const auto& message = *pMessage;
    send(message.Encode());
}

void Peer::start_handshake() noexcept
{
    try {
        const auto status = Connected().wait_for(std::chrono::seconds(5));

        if (std::future_status::ready != status) {
            disconnect();

            return;
        }
    } catch (...) {
        disconnect();

        return;
    }

    try {
        const auto local = local_endpoint();
        std::unique_ptr<Message> pVersion{Factory::BitcoinP2PVersion(
            api_,
            chain_,
            protocol_,
            local_services_,
            local.address().to_v6().to_string(),
            local.port(),
            address_.Services(),
            endpoint_.address().to_v6().to_string(),
            endpoint_.port(),
            nonce_,
            user_agent_,
            network_.GetHeight(),
            relay_)};

        OT_ASSERT(pVersion);

        const auto& version = *pVersion;
        send(version.Encode());
    } catch (...) {
        disconnect();
    }
}

Peer::~Peer() { Shutdown(); }
}  // namespace opentxs::blockchain::p2p::bitcoin::implementation
