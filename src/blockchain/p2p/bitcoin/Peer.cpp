// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/Peer.hpp"  // IWYU pragma: associated

#include <boost/asio.hpp>
#include <algorithm>
#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

#include "blockchain/bitcoin/Inventory.hpp"
#include "blockchain/p2p/Peer.hpp"
#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "blockchain/p2p/bitcoin/message/Cmpctblock.hpp"
#include "blockchain/p2p/bitcoin/message/Feefilter.hpp"
#include "blockchain/p2p/bitcoin/message/Getblocks.hpp"
#include "blockchain/p2p/bitcoin/message/Getblocktxn.hpp"
#include "blockchain/p2p/bitcoin/message/Merkleblock.hpp"
#include "blockchain/p2p/bitcoin/message/Reject.hpp"
#include "blockchain/p2p/bitcoin/message/Sendcmpct.hpp"
#include "blockchain/p2p/bitcoin/message/Tx.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Forward.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/p2p/Peer.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/protobuf/BlockchainBlockHeader.pb.h"  // IWYU pragma: keep
#include "util/ScopeGuard.hpp"

#define OT_METHOD "opentxs::blockchain::p2p::bitcoin::implementation::Peer::"

namespace opentxs::factory
{
auto BitcoinP2PPeerLegacy(
    const api::client::Manager& api,
    const blockchain::client::internal::Network& network,
    const blockchain::client::internal::PeerManager& manager,
    const blockchain::client::internal::IO& io,
    const int id,
    std::unique_ptr<blockchain::p2p::internal::Address> address,
    const std::string& shutdown) -> blockchain::p2p::internal::Peer*
{
    namespace p2p = blockchain::p2p;
    using ReturnType = p2p::bitcoin::implementation::Peer;

    if (false == bool(address)) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(": Invalid null address")
            .Flush();

        return nullptr;
    }

    switch (address->Type()) {
        case p2p::Network::ipv6:
        case p2p::Network::ipv4: {
            break;
        }
        default: {
            LogOutput("opentxs::factory::")(__FUNCTION__)(
                ": Unsupported address type")
                .Flush();

            return nullptr;
        }
    }

    return new ReturnType(
        api, network, manager, io, shutdown, id, std::move(address));
}
}  // namespace opentxs::factory

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
    const api::client::Manager& api,
    const client::internal::Network& network,
    const client::internal::PeerManager& manager,
    const blockchain::client::internal::IO& io,
    const std::string& shutdown,
    const int id,
    std::unique_ptr<internal::Address> address,
    const bool relay,
    const std::set<p2p::Service>& localServices,
    const ProtocolVersion protocol) noexcept
    : p2p::implementation::Peer(
          api,
          network,
          manager,
          io,
          id,
          shutdown,
          HeaderType::Size(),
          MessageType::MaxPayload(),
          std::move(address))
    , protocol_((0 == protocol) ? default_protocol_version_ : protocol)
    , nonce_(nonce(api_))
    , local_services_(get_local_services(protocol_, chain_, localServices))
    , relay_(relay)
    , get_headers_()
{
    init();
}

auto Peer::broadcast_transaction(zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    if (2 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid command").Flush();

        OT_FAIL;
    }

    auto& bytes = body.at(1);

    {
        auto raw = api_.Factory().Data(bytes);
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(raw->asHex()).Flush();
    }

    auto pTx = std::unique_ptr<Message>{
        factory::BitcoinP2PTx(api_, chain_, bytes.Bytes())};

    if (false == bool(pTx)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct tx").Flush();

        return;
    }

    const auto& tx = *pTx;
    send(tx.Encode());
}

auto Peer::get_body_size(const zmq::Frame& header) const noexcept -> std::size_t
{
    OT_ASSERT(HeaderType::Size() == header.size());

    try {
        auto raw = HeaderType::BitcoinFormat{header};

        return raw.PayloadSize();
    } catch (...) {

        return 0;
    }
}

auto Peer::get_local_services(
    const ProtocolVersion version,
    const blockchain::Type network,
    const std::set<p2p::Service>& input) noexcept -> std::set<p2p::Service>
{
    auto output{input};

    switch (network) {
        case blockchain::Type::Bitcoin:
        case blockchain::Type::Bitcoin_testnet3: {
            output.emplace(p2p::Service::Witness);
        } break;
        case blockchain::Type::BitcoinCash:
        case blockchain::Type::BitcoinCash_testnet3: {
            output.emplace(p2p::Service::BitcoinCash);
        } break;
        case blockchain::Type::Unknown:
        case blockchain::Type::Ethereum_frontier:
        case blockchain::Type::Ethereum_ropsten:
        case blockchain::Type::Litecoin:
        case blockchain::Type::Litecoin_testnet4:
        default: {
        }
    }

    return output;
}

auto Peer::nonce(const api::client::Manager& api) noexcept -> Nonce
{
    Nonce output{0};
    const auto random =
        api.Crypto().Util().RandomizeMemory(&output, sizeof(output));

    OT_ASSERT(random);

    return output;
}

auto Peer::ping() noexcept -> void
{
    std::unique_ptr<Message> pPing{
        factory::BitcoinP2PPing(api_, chain_, nonce_)};

    if (false == bool(pPing)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct ping")
            .Flush();

        return;
    }

    const auto& ping = *pPing;
    send(ping.Encode());
}

auto Peer::pong() noexcept -> void
{
    std::unique_ptr<Message> pPong{
        factory::BitcoinP2PPong(api_, chain_, nonce_)};

    if (false == bool(pPong)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct pong")
            .Flush();

        return;
    }

    const auto& pong = *pPong;
    send(pong.Encode());
}

auto Peer::process_addr(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Addr> pMessage{
        factory::BitcoinP2PAddr(
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
    using DB = blockchain::client::internal::PeerDatabase;
    auto peers = std::vector<DB::Address>{};

    for (const auto& address : message) {
        auto pAddress = address.clone_internal();

        OT_ASSERT(pAddress);

        pAddress->SetLastConnected({});
        peers.emplace_back(std::move(pAddress));
    }

    manager_.Database().Import(std::move(peers));
}

auto Peer::process_block(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    if (146 > payload.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid block").Flush();

        return;
    }

    using Task = client::internal::Network::Task;
    auto work = MakeWork(Task::SubmitBlock);
    work->AddFrame(payload);
    network_.Submit(work);
}

auto Peer::process_blocktxn(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Blocktxn> pMessage{
        factory::BitcoinP2PBlocktxn(
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

auto Peer::process_cfcheckpt(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Cfcheckpt> pMessage{
        factory::BitcoinP2PCfcheckpt(
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

auto Peer::process_cfheaders(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    auto& success = state_.verify_.second_action_;
    auto postcondition = ScopeGuard{[this, &success] {
        if (verifying() && (false == success)) {
            LogNormal("Disconnecting ")(
                blockchain::internal::DisplayString(chain_))(" peer ")(
                address_.Display())(" due to filter checkpoint failure.")
                .Flush();
            disconnect();
        }
    }};

    const auto pMessage = std::unique_ptr<message::internal::Cfheaders>{
        factory::BitcoinP2PCfheaders(
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

    if (verifying()) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Received checkpoint filter header message")
            .Flush();

        if (1 != pMessage->size()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unexpected filter header count: ")(pMessage->size())
                .Flush();

            return;
        }

        const auto checkpointData =
            network_.HeaderOracle().GetDefaultCheckpoint();
        const auto& [height, checkpointHash, parentHash, filterHash] =
            checkpointData;
        const auto receivedFilterHeader =
            blockchain::internal::FilterHashToHeader(
                api_, message.at(0).Bytes(), message.Previous().Bytes());

        if (filterHash != receivedFilterHeader) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unexpected filter header: ")(
                receivedFilterHeader->asHex())(". Expected: ")(
                filterHash->asHex())
                .Flush();

            return;
        }

        LogNormal("Filter checkpoint validated for ")(
            blockchain::internal::DisplayString(chain_))(" peer ")(
            address_.Display())
            .Flush();
        success = true;
        check_verify();
    } else {
        const auto type = message.Type();
        using Task = client::internal::Network::Task;
        auto work = MakeWork(Task::SubmitFilterHeader);
        work->AddFrame(type);
        work->AddFrame(message.Stop());
        work->AddFrame(message.Previous());

        for (const auto& header : message) { work->AddFrame(header); }

        network_.Submit(work);
    }
}

auto Peer::process_cfilter(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const auto pMessage =
        std::unique_ptr<message::internal::Cfilter>{factory::BitcoinP2PCfilter(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode cfilter payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;
    const auto type = message.Type();
    using Task = client::internal::Network::Task;
    auto work = MakeWork(Task::SubmitFilter);
    work->AddFrame(type);
    work->AddFrame(message.Hash());
    work->AddFrame(message.Bits());
    work->AddFrame(message.FPRate());
    work->AddFrame(message.ElementCount());
    work->AddFrame(message.Filter().data(), message.Filter().size());
    network_.Submit(work);
}

auto Peer::process_cmpctblock(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Cmpctblock> pMessage{
        factory::BitcoinP2PCmpctblock(
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

auto Peer::process_feefilter(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Feefilter> pMessage{
        factory::BitcoinP2PFeefilter(
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

auto Peer::process_filteradd(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Filteradd> pMessage{
        factory::BitcoinP2PFilteradd(
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

auto Peer::process_filterclear(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Filterclear> pMessage{
        factory::BitcoinP2PFilterclear(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_filterload(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Filterload> pMessage{
        factory::BitcoinP2PFilterload(
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

auto Peer::process_getaddr(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Getaddr> pMessage{
        factory::BitcoinP2PGetaddr(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_getblocks(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Getblocks> pMessage{
        factory::BitcoinP2PGetblocks(
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

auto Peer::process_getblocktxn(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Getblocktxn> pMessage{
        factory::BitcoinP2PGetblocktxn(
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

auto Peer::process_getcfcheckpt(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Getcfcheckpt> pMessage{
        factory::BitcoinP2PGetcfcheckpt(
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

auto Peer::process_getcfheaders(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Getcfheaders> pMessage{
        factory::BitcoinP2PGetcfheaders(
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

auto Peer::process_getcfilters(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Getcfilters> pMessage{
        factory::BitcoinP2PGetcfilters(
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

auto Peer::process_getdata(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Getdata> pMessage{
        factory::BitcoinP2PGetdata(
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

auto Peer::process_getheaders(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Getheaders> pMessage{
        factory::BitcoinP2PGetheaders(
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
}

auto Peer::process_headers(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    auto& success = state_.verify_.first_action_;
    auto postcondition = ScopeGuard{[this, &success] {
        if (verifying() && (false == success)) {
            LogNormal("Disconnecting ")(
                blockchain::internal::DisplayString(chain_))(" peer ")(
                address_.Display())(" due to block checkpoint failure.")
                .Flush();
            disconnect();
        }
    }};

    const auto pMessage =
        std::unique_ptr<message::internal::Headers>{factory::BitcoinP2PHeaders(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;

    if (verifying()) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Received checkpoint block header message")
            .Flush();

        if (1 != pMessage->size()) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Unexpected header count: ")(
                pMessage->size())
                .Flush();

            return;
        }

        const auto checkpointData =
            network_.HeaderOracle().GetDefaultCheckpoint();
        const auto& [height, checkpointHash, parentHash, filterHash] =
            checkpointData;
        const auto& receivedBlockHash = message.at(0).Hash();

        if (checkpointHash != receivedBlockHash) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(
                ": Unexpected block header hash: ")(receivedBlockHash.asHex())(
                ". Expected: ")(checkpointHash->asHex())
                .Flush();

            return;
        }

        LogNormal("Block checkpoint validated for ")(
            blockchain::internal::DisplayString(chain_))(" peer ")(
            address_.Display())
            .Flush();
        success = true;
        check_verify();
    } else {
        get_headers_.Finish();

        using Promise = std::promise<void>;
        auto* promise = new Promise{};

        OT_ASSERT(nullptr != promise);

        auto future = promise->get_future();
        auto pointer = reinterpret_cast<std::uintptr_t>(promise);
        using Task = client::internal::Network::Task;
        auto work = MakeWork(Task::SubmitBlockHeader);
        work->AddFrame(pointer);

        for (const auto& header : message) {
            work->AddFrame(header.Serialize());
        }

        network_.Submit(work);

        if (std::future_status::ready ==
            future.wait_for(std::chrono::seconds(10))) {
            if (false == network_.IsSynchronized()) { request_headers(); }
        }
    }
}

auto Peer::process_inv(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Inv> pMessage{
        factory::BitcoinP2PInv(
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
                if (State::Run == state_.value_.load()) {
                    request_headers(inv.hash_);
                }
            } break;
            default: {
            }
        }
    }
}

auto Peer::process_mempool(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Mempool> pMessage{
        factory::BitcoinP2PMempool(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_merkleblock(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Merkleblock> pMessage{
        factory::BitcoinP2PMerkleblock(
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

auto Peer::process_message(const zmq::Message& message) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto body = message.Body();

    if (3 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto& headerBytes = body.at(1);
    const auto& payloadBytes = body.at(2);
    std::unique_ptr<HeaderType> pHeader{
        factory::BitcoinP2PHeader(api_, headerBytes)};

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

auto Peer::process_notfound(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Notfound> pMessage{
        factory::BitcoinP2PNotfound(
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

auto Peer::process_ping(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Ping> pMessage{
        factory::BitcoinP2PPing(
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

    if (message.Nonce() == nonce_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Connection to self detected")
            .Flush();
        disconnect();
    }

    pong();
}

auto Peer::process_pong(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Pong> pMessage{
        factory::BitcoinP2PPong(
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
}

auto Peer::process_reject(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Reject> pMessage{factory::BitcoinP2PReject(
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

auto Peer::process_sendcmpct(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Sendcmpct> pMessage{
        factory::BitcoinP2PSendcmpct(
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

auto Peer::process_sendheaders(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Sendheaders> pMessage{
        factory::BitcoinP2PSendheaders(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_tx(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Tx> pMessage{factory::BitcoinP2PTx(
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

auto Peer::process_verack(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Verack> pMessage{
        factory::BitcoinP2PVerack(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode message payload")
            .Flush();

        return;
    }

    state_.handshake_.first_action_ = true;
    check_handshake();
}

auto Peer::process_version(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Version> pVersion{
        factory::BitcoinP2PVersion(
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
        LogNormal("Peer ")(address_.Display())(
            " does not advertise necessary services")
            .Flush();
        disconnect();

        return;
    }

    std::unique_ptr<Message> pVerack{factory::BitcoinP2PVerack(api_, chain_)};

    if (false == bool(pVerack)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct verack")
            .Flush();

        return;
    }

    const auto& verack = *pVerack;
    send(verack.Encode());
    state_.handshake_.second_action_ = true;
    check_handshake();
}

auto Peer::request_addresses() noexcept -> void
{
    auto pMessage =
        std::unique_ptr<Message>{factory::BitcoinP2PGetaddr(api_, chain_)};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct getaddr")
            .Flush();

        return;
    }

    const auto& message = *pMessage;
    send(message.Encode());
}

auto Peer::request_block(zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    if (2 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid command").Flush();

        OT_FAIL;
    }

    auto id = api_.Factory().Data(body.at(1));
    using Inventory = blockchain::bitcoin::Inventory;
    using Type = Inventory::Type;
    auto blocks = std::vector<Inventory>{};
    blocks.emplace_back(Type::MsgBlock, std::move(id));

    auto pMessage = std::unique_ptr<Message>{
        factory::BitcoinP2PGetdata(api_, chain_, std::move(blocks))};

    if (false == bool(pMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct getdata")
            .Flush();

        return;
    }

    const auto& message = *pMessage;
    send(message.Encode());
}

auto Peer::request_cfheaders(zmq::Message& in) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto body = in.Body();

    if (4 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid work").Flush();

        return;
    }

    try {
        auto pMessage =
            std::unique_ptr<Message>{factory::BitcoinP2PGetcfheaders(
                api_,
                chain_,
                body.at(1).as<filter::Type>(),
                body.at(2).as<block::Height>(),
                Data::Factory(body.at(3)))};

        if (false == bool(pMessage)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to construct getcfheaders")
                .Flush();

            return;
        }

        const auto& message = *pMessage;
        send(message.Encode());
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid parameters").Flush();
    }
}

auto Peer::request_cfilter(zmq::Message& in) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto body = in.Body();

    if (4 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid work").Flush();

        return;
    }

    try {
        auto pMessage = std::unique_ptr<Message>{factory::BitcoinP2PGetcfilters(
            api_,
            chain_,
            body.at(1).as<filter::Type>(),
            body.at(2).as<block::Height>(),
            Data::Factory(body.at(3)))};

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

auto Peer::request_checkpoint_block_header() noexcept -> void
{
    auto success = false;
    auto postcondition = ScopeGuard{[this, &success] {
        if (success) {
            LogVerbose("Requested checkpoint block header from ")(
                blockchain::internal::DisplayString(chain_))(" peer ")(
                address_.Display())(".")
                .Flush();
        } else {
            LogNormal("Failed to request checkpoint block header from ")(
                blockchain::internal::DisplayString(chain_))(" peer ")(
                address_.Display())(".")
                .Flush();
            disconnect();
        }
    }};

    try {
        auto checkpointData = network_.HeaderOracle().GetDefaultCheckpoint();
        auto [height, checkpointBlockHash, parentBlockHash, filterHash] =
            checkpointData;
        auto pMessage = std::unique_ptr<Message>{factory::BitcoinP2PGetheaders(
            api_,
            chain_,
            protocol_.load(),
            {std::move(parentBlockHash)},
            std::move(checkpointBlockHash))};

        if (false == bool(pMessage)) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(
                ": Failed to construct getheaders")
                .Flush();

            return;
        }

        const auto& message = *pMessage;
        send(message.Encode());
        success = true;
    } catch (...) {
    }
}

auto Peer::request_checkpoint_filter_header() noexcept -> void
{
    auto success = false;
    auto postcondition = ScopeGuard{[this, &success] {
        if (success) {
            LogVerbose("Requested checkpoint filter header from ")(
                blockchain::internal::DisplayString(chain_))(" peer ")(
                address_.Display())(".")
                .Flush();
        } else {
            LogNormal("Failed to request checkpoint filter header from ")(
                blockchain::internal::DisplayString(chain_))(" peer ")(
                address_.Display())(".")
                .Flush();
            disconnect();
        }
    }};

    try {
        auto checkpointData = network_.HeaderOracle().GetDefaultCheckpoint();
        auto [height, checkpointBlockHash, parentBlockHash, filterHash] =
            checkpointData;
        auto pMessage =
            std::unique_ptr<Message>{factory::BitcoinP2PGetcfheaders(
                api_,
                chain_,
                network_.FilterOracle().DefaultType(),
                height,
                checkpointBlockHash)};

        if (false == bool(pMessage)) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(
                ": Failed to construct getcfheaders")
                .Flush();

            return;
        }

        const auto& message = *pMessage;
        send(message.Encode());
        success = true;
    } catch (...) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Invalid parameters").Flush();
    }
}

auto Peer::request_headers() noexcept -> void
{
    request_headers(api_.Factory().Data());
}

auto Peer::request_headers(const block::Hash& hash) noexcept -> void
{
    if (get_headers_.Running()) { return; }

    auto pMessage = std::unique_ptr<Message>{factory::BitcoinP2PGetheaders(
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
    get_headers_.Start();
}

auto Peer::start_handshake() noexcept -> void
{
    try {
        const auto status = Connected().wait_for(std::chrono::seconds(5));

        if (std::future_status::ready != status) {
            LogNormal("Connection to peer ")(address_.Display())(
                " timed out during handshake")
                .Flush();
            disconnect();

            return;
        }
    } catch (...) {
        disconnect();

        return;
    }

    try {
        const auto local = local_endpoint();
        std::unique_ptr<Message> pVersion{factory::BitcoinP2PVersion(
            api_,
            chain_,
            protocol_.load(),
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
