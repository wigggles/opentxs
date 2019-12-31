// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"

namespace opentxs
{
class Factory
{
public:
    static opentxs::Armored* Armored();
    static opentxs::Armored* Armored(const opentxs::Data& input);
    static opentxs::Armored* Armored(const opentxs::String& input);
    static opentxs::Armored* Armored(const opentxs::crypto::Envelope& input);
    static identity::internal::Authority* Authority(
        const api::internal::Core& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const proto::KeyMode mode,
        const proto::Authority& serialized);
    static identity::internal::Authority* Authority(
        const api::internal::Core& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const NymParameters& nymParameters,
        const VersionNumber nymVersion,
        const opentxs::PasswordPrompt& reason);
    static ui::implementation::AccountActivity* AccountActivityModel(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const opentxs::Identifier& accountID
#if OT_QT
        ,
        const bool qt
#endif
    );
#if OT_QT
    static ui::AccountActivityQt* AccountActivityQtModel(
        ui::implementation::AccountActivity& parent);
#endif
    static ui::implementation::AccountList* AccountListModel(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID
#if OT_QT
        ,
        const bool qt
#endif
    );
    static ui::implementation::AccountListRowInternal* AccountListItem(
        const ui::implementation::AccountListInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::AccountListRowID& rowID,
        const ui::implementation::AccountListSortKey& sortKey,
        const ui::implementation::CustomData& custom);
#if OT_QT
    static ui::AccountListQt* AccountListQtModel(
        ui::implementation::AccountList& parent);
#endif
    static ui::implementation::AccountSummary* AccountSummaryModel(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const proto::ContactItemType currency
#if OT_QT
        ,
        const bool qt
#endif
    );
    static ui::implementation::IssuerItemRowInternal* AccountSummaryItem(
        const ui::implementation::IssuerItemInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::IssuerItemRowID& rowID,
        const ui::implementation::IssuerItemSortKey& sortKey,
        const ui::implementation::CustomData& custom);
#if OT_QT
    static ui::AccountSummaryQt* AccountSummaryQtModel(
        ui::implementation::AccountSummary& parent);
#endif
    static api::client::internal::Activity* Activity(
        const api::internal::Core& api,
        const api::client::Contacts& contact);
    static ui::implementation::ActivitySummary* ActivitySummaryModel(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const Flag& running,
        const identifier::Nym& nymID
#if OT_QT
        ,
        const bool qt
#endif
    );
    static ui::implementation::ActivitySummaryRowInternal* ActivitySummaryItem(
        const ui::implementation::ActivitySummaryInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const ui::implementation::ActivitySummaryRowID& rowID,
        const ui::implementation::ActivitySummarySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const Flag& running);
#if OT_QT
    static ui::ActivitySummaryQt* ActivitySummaryQtModel(
        ui::implementation::ActivitySummary& parent);
#endif
    static ui::implementation::ActivityThread* ActivityThreadModel(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const opentxs::Identifier& threadID
#if OT_QT
        ,
        const bool qt
#endif
    );
#if OT_QT
    static ui::ActivityThreadQt* ActivityThreadQtModel(
        ui::implementation::ActivityThread& parent);
#endif
    static api::crypto::internal::Asymmetric* AsymmetricAPI(
        const api::internal::Core& api);
    static ui::implementation::AccountActivityRowInternal* BalanceItem(
        const ui::implementation::AccountActivityInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::AccountActivityRowID& rowID,
        const ui::implementation::AccountActivitySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const identifier::Nym& nymID,
        const opentxs::Identifier& accountID);
    static auto BailmentNotice(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const opentxs::Identifier& requestID,
        const std::string& txid,
        const Amount& amount,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::BailmentNotice>;
    static auto BailmentNotice(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::BailmentNotice>;
    static auto BailmentReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::reply::Bailment>;
    static auto BailmentReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized) noexcept
        -> std::shared_ptr<contract::peer::reply::Bailment>;
    static auto BailmentRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const identifier::UnitDefinition& unit,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::Bailment>;
    static auto BailmentRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::Bailment>;
    static auto BasketContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::uint64_t weight,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version) noexcept
        -> std::shared_ptr<contract::unit::Basket>;
    static auto BasketContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::unit::Basket>;
    static crypto::Bitcoin* Bitcoin(const api::Crypto& crypto);
    static auto Bip39(const api::Crypto& api) noexcept
        -> std::unique_ptr<crypto::Bip39>;
#if OT_BLOCKCHAIN
    static blockchain::block::bitcoin::internal::Header* BitcoinBlockHeader(
        const api::internal::Core& api,
        const proto::BlockchainBlockHeader& serialized);
    static blockchain::block::bitcoin::internal::Header* BitcoinBlockHeader(
        const api::internal::Core& api,
        const Data& raw);
    static blockchain::block::bitcoin::internal::Header* BitcoinBlockHeader(
        const api::internal::Core& api,
        const blockchain::block::Hash& hash,
        const blockchain::block::Hash& parent,
        const blockchain::block::Height height);
    static blockchain::p2p::bitcoin::message::internal::Addr* BitcoinP2PAddr(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Addr* BitcoinP2PAddr(
        const api::internal::Core& api,
        const blockchain::Type network,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        std::vector<std::unique_ptr<blockchain::p2p::internal::Address>>&&
            addresses);
    static blockchain::p2p::bitcoin::message::internal::Block* BitcoinP2PBlock(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Block* BitcoinP2PBlock(
        const api::internal::Core& api,
        const blockchain::Type network,
        const Data& raw_block);
    static blockchain::p2p::bitcoin::message::internal::Blocktxn*
    BitcoinP2PBlocktxn(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Blocktxn*
    BitcoinP2PBlocktxn(
        const api::internal::Core& api,
        const blockchain::Type network,
        const Data& raw_Blocktxn);
    static blockchain::p2p::bitcoin::message::internal::Cfcheckpt*
    BitcoinP2PCfcheckpt(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Cfcheckpt*
    BitcoinP2PCfcheckpt(
        const api::internal::Core& api,
        const blockchain::Type network,
        const blockchain::filter::Type type,
        const blockchain::filter::Hash& stop,
        const std::vector<blockchain::filter::pHash>& headers);
    static blockchain::p2p::bitcoin::message::internal::Cfheaders*
    BitcoinP2PCfheaders(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Cfheaders*
    BitcoinP2PCfheaders(
        const api::internal::Core& api,
        const blockchain::Type network,
        const blockchain::filter::Type type,
        const blockchain::filter::Hash& stop,
        const blockchain::filter::Hash& previous,
        const std::vector<blockchain::filter::pHash>& headers);
    static blockchain::p2p::bitcoin::message::internal::Cfilter*
    BitcoinP2PCfilter(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Cfilter*
    BitcoinP2PCfilter(
        const api::internal::Core& api,
        const blockchain::Type network,
        const blockchain::filter::Type type,
        const blockchain::filter::Hash& hash,
        std::unique_ptr<blockchain::internal::GCS> filter);
    static blockchain::p2p::bitcoin::message::Cmpctblock* BitcoinP2PCmpctblock(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::Cmpctblock* BitcoinP2PCmpctblock(
        const api::internal::Core& api,
        const blockchain::Type network,
        const Data& raw_cmpctblock);
    static blockchain::p2p::bitcoin::message::Feefilter* BitcoinP2PFeefilter(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::Feefilter* BitcoinP2PFeefilter(
        const api::internal::Core& api,
        const blockchain::Type network,
        const std::uint64_t fee_rate);
    static blockchain::p2p::bitcoin::message::internal::Filteradd*
    BitcoinP2PFilteradd(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Filteradd*
    BitcoinP2PFilteradd(
        const api::internal::Core& api,
        const blockchain::Type network,
        const Data& element);
    static blockchain::p2p::bitcoin::message::internal::Filterclear*
    BitcoinP2PFilterclear(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header);
    static blockchain::p2p::bitcoin::message::internal::Filterclear*
    BitcoinP2PFilterclear(
        const api::internal::Core& api,
        const blockchain::Type network);
    static blockchain::p2p::bitcoin::message::internal::Filterload*
    BitcoinP2PFilterload(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Filterload*
    BitcoinP2PFilterload(
        const api::internal::Core& api,
        const blockchain::Type network,
        const blockchain::BloomFilter& filter);
    static blockchain::p2p::bitcoin::message::internal::Getaddr*
    BitcoinP2PGetaddr(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header);
    static blockchain::p2p::bitcoin::message::internal::Getaddr*
    BitcoinP2PGetaddr(
        const api::internal::Core& api,
        const blockchain::Type network);
    OPENTXS_EXPORT static auto BitcoinP2PGetblocks(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size)
        -> blockchain::p2p::bitcoin::message::Getblocks*;
    OPENTXS_EXPORT static auto BitcoinP2PGetblocks(
        const api::internal::Core& api,
        const blockchain::Type network,
        const std::uint32_t version,
        const std::vector<OTData>& header_hashes,
        const Data& stop_hash) -> blockchain::p2p::bitcoin::message::Getblocks*;
    static blockchain::p2p::bitcoin::message::Getblocktxn*
    BitcoinP2PGetblocktxn(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::Getblocktxn*
    BitcoinP2PGetblocktxn(
        const api::internal::Core& api,
        const blockchain::Type network,
        const Data& block_hash,
        const std::vector<std::size_t>& txn_indices);
    static blockchain::p2p::bitcoin::message::internal::Getcfcheckpt*
    BitcoinP2PGetcfcheckpt(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Getcfcheckpt*
    BitcoinP2PGetcfcheckpt(
        const api::internal::Core& api,
        const blockchain::Type network,
        const blockchain::filter::Type type,
        const blockchain::filter::Hash& stop);
    static blockchain::p2p::bitcoin::message::internal::Getcfheaders*
    BitcoinP2PGetcfheaders(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Getcfheaders*
    BitcoinP2PGetcfheaders(
        const api::internal::Core& api,
        const blockchain::Type network,
        const blockchain::filter::Type type,
        const blockchain::block::Height start,
        const blockchain::filter::Hash& stop);
    static blockchain::p2p::bitcoin::message::internal::Getcfilters*
    BitcoinP2PGetcfilters(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Getcfilters*
    BitcoinP2PGetcfilters(
        const api::internal::Core& api,
        const blockchain::Type network,
        const blockchain::filter::Type type,
        const blockchain::block::Height start,
        const blockchain::filter::Hash& stop);
    static blockchain::p2p::bitcoin::message::internal::Getdata*
    BitcoinP2PGetdata(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Getdata*
    BitcoinP2PGetdata(
        const api::internal::Core& api,
        const blockchain::Type network,
        std::vector<blockchain::bitcoin::Inventory>&& payload);
    static blockchain::p2p::bitcoin::message::internal::Getheaders*
    BitcoinP2PGetheaders(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Getheaders*
    BitcoinP2PGetheaders(
        const api::internal::Core& api,
        const blockchain::Type network,
        const blockchain::p2p::bitcoin::ProtocolVersionUnsigned version,
        std::vector<blockchain::block::pHash>&& history,
        blockchain::block::pHash&& stop);
    OPENTXS_EXPORT static blockchain::p2p::bitcoin::Header* BitcoinP2PHeader(
        const api::internal::Core& api,
        const network::zeromq::Frame& bytes);
    OPENTXS_EXPORT static blockchain::p2p::bitcoin::message::internal::Headers*
    BitcoinP2PHeaders(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Headers*
    BitcoinP2PHeaders(
        const api::internal::Core& api,
        const blockchain::Type network,
        std::vector<std::unique_ptr<blockchain::block::bitcoin::Header>>&&
            headers);
    static blockchain::p2p::bitcoin::message::internal::Inv* BitcoinP2PInv(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Inv* BitcoinP2PInv(
        const api::internal::Core& api,
        const blockchain::Type network,
        std::vector<blockchain::bitcoin::Inventory>&& payload);
    static blockchain::p2p::bitcoin::message::internal::Mempool*
    BitcoinP2PMempool(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header);
    static blockchain::p2p::bitcoin::message::internal::Mempool*
    BitcoinP2PMempool(
        const api::internal::Core& api,
        const blockchain::Type network);
    static blockchain::p2p::bitcoin::message::Merkleblock*
    BitcoinP2PMerkleblock(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::Merkleblock*
    BitcoinP2PMerkleblock(
        const api::internal::Core& api,
        const blockchain::Type network,
        const Data& block_header,
        const std::uint32_t txn_count,
        const std::vector<OTData>& hashes,
        const std::vector<std::byte>& flags);
    OPENTXS_EXPORT static auto BitcoinP2PMessage(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload = nullptr,
        const std::size_t size = 0) -> blockchain::p2p::bitcoin::Message*;
    static blockchain::p2p::bitcoin::message::internal::Notfound*
    BitcoinP2PNotfound(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Notfound*
    BitcoinP2PNotfound(
        const api::internal::Core& api,
        const blockchain::Type network,
        std::vector<blockchain::bitcoin::Inventory>&& payload);
    static blockchain::p2p::internal::Peer* BitcoinP2PPeerLegacy(
        const api::internal::Core& api,
        const blockchain::client::internal::Network& network,
        const blockchain::client::internal::PeerManager& manager,
        const int id,
        std::unique_ptr<blockchain::p2p::internal::Address> address,
        boost::asio::io_context& context);
    static blockchain::p2p::bitcoin::message::internal::Ping* BitcoinP2PPing(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Ping* BitcoinP2PPing(
        const api::internal::Core& api,
        const blockchain::Type network,
        const std::uint64_t nonce);
    static blockchain::p2p::bitcoin::message::internal::Pong* BitcoinP2PPong(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Pong* BitcoinP2PPong(
        const api::internal::Core& api,
        const blockchain::Type network,
        const std::uint64_t nonce);
    static blockchain::p2p::bitcoin::message::Reject* BitcoinP2PReject(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::Reject* BitcoinP2PReject(
        const api::internal::Core& api,
        const blockchain::Type network,
        const std::string& message,
        const std::uint8_t code,
        const std::string& reason,
        const Data& extra);
    static blockchain::p2p::bitcoin::message::Sendcmpct* BitcoinP2PSendcmpct(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::Sendcmpct* BitcoinP2PSendcmpct(
        const api::internal::Core& api,
        const blockchain::Type network,
        const bool announce,
        const std::uint64_t version);
    static blockchain::p2p::bitcoin::message::internal::Sendheaders*
    BitcoinP2PSendheaders(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header);
    static blockchain::p2p::bitcoin::message::internal::Sendheaders*
    BitcoinP2PSendheaders(
        const api::internal::Core& api,
        const blockchain::Type network);
    static blockchain::p2p::bitcoin::message::Tx* BitcoinP2PTx(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::Tx* BitcoinP2PTx(
        const api::internal::Core& api,
        const blockchain::Type network,
        const Data& raw_tx);
    static blockchain::p2p::bitcoin::message::internal::Verack*
    BitcoinP2PVerack(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header);
    static blockchain::p2p::bitcoin::message::internal::Verack*
    BitcoinP2PVerack(
        const api::internal::Core& api,
        const blockchain::Type network);
    static blockchain::p2p::bitcoin::message::internal::Version*
    BitcoinP2PVersion(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size);
    static blockchain::p2p::bitcoin::message::internal::Version*
    BitcoinP2PVersion(
        const api::internal::Core& api,
        const blockchain::Type network,
        const std::int32_t version,
        const std::set<blockchain::p2p::Service>& localServices,
        const std::string& localAddress,
        const std::uint16_t localPort,
        const std::set<blockchain::p2p::Service>& remoteServices,
        const std::string& remoteAddress,
        const std::uint16_t remotePort,
        const std::uint64_t nonce,
        const std::string& userAgent,
        const blockchain::block::Height height,
        const bool relay);
#endif  // OT_BLOCKCHAIN
    static api::client::Blockchain* BlockchainAPI(
        const api::client::internal::Manager& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contacts,
        const api::Legacy& legacy,
        const std::string& dataFolder);
    static api::client::blockchain::internal::BalanceList*
    BlockchainBalanceList(
        const api::client::internal::Blockchain& parent,
        const blockchain::Type chain);
    static api::client::blockchain::internal::BalanceTree*
    BlockchainBalanceTree(
        const api::client::blockchain::internal::BalanceList& parent,
        const identifier::Nym& id,
        const std::set<OTIdentifier>& hdAccounts,
        const std::set<OTIdentifier>& importedAccounts,
        const std::set<OTIdentifier>& paymentCodeAccounts);
    static api::client::blockchain::internal::HD* BlockchainHDBalanceNode(
        const api::client::blockchain::internal::BalanceTree& parent,
        const proto::HDPath& path,
        Identifier& id);
    static api::client::blockchain::internal::HD* BlockchainHDBalanceNode(
        const api::client::blockchain::internal::BalanceTree& parent,
        const proto::HDAccount& serialized,
        Identifier& id);
#if OT_BLOCKCHAIN
    static blockchain::p2p::internal::Address* BlockchainAddress(
        const api::internal::Core& api,
        const blockchain::p2p::Protocol protocol,
        const blockchain::p2p::Network network,
        const Data& bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const std::set<blockchain::p2p::Service>& services);
    static blockchain::internal::Database* BlockchainDatabase(
        const api::internal::Core& api,
        const blockchain::client::internal::Network& network,
        const api::client::blockchain::database::implementation::Database& db,
        const blockchain::Type type);
    static blockchain::client::internal::FilterOracle* BlockchainFilterOracle(
        const api::internal::Core& api,
        const blockchain::client::internal::Network& network,
        const blockchain::client::internal::FilterDatabase& database);
    OPENTXS_EXPORT static auto BlockchainNetworkBitcoin(
        const api::internal::Core& api,
        const api::client::internal::Blockchain& blockchain,
        const blockchain::Type type,
        const std::string& seednode) -> blockchain::client::internal::Network*;
    static blockchain::client::internal::PeerManager* BlockchainPeerManager(
        const api::internal::Core& api,
        const blockchain::client::internal::Network& network,
        const blockchain::client::internal::PeerDatabase& database,
        const blockchain::Type type,
        const std::string& seednode);
    OPENTXS_EXPORT static auto BloomFilter(
        const api::internal::Core& api,
        const std::uint32_t tweak,
        const blockchain::BloomUpdateFlag update,
        const std::size_t targets,
        const double falsePositiveRate) -> blockchain::BloomFilter*;
    OPENTXS_EXPORT static auto BloomFilter(
        const api::internal::Core& api,
        const Data& serialized) -> blockchain::BloomFilter*;
#endif  // OT_BLOCKCHAIN
    static internal::ClientContext* ClientContext(
        const api::internal::Core& api,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Server& server);
    static internal::ClientContext* ClientContext(
        const api::internal::Core& api,
        const proto::Context& serialized,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Server& server);
    static api::client::internal::Manager* ClientManager(
        const api::internal::Context& parent,
        Flag& running,
        const ArgList& args,
        const api::Settings& config,
        const api::Crypto& crypto,
        const network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance);
    static auto ConnectionReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::reply::Connection>;
    static auto ConnectionReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized) noexcept
        -> std::shared_ptr<contract::peer::reply::Connection>;
    static auto ConnectionRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const proto::ConnectionInfoType type,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::Connection>;
    static auto ConnectionRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::Connection>;
    static identity::credential::internal::Contact* ContactCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
    static identity::credential::internal::Contact* ContactCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& credential);
    static api::client::internal::Contacts* ContactAPI(
        const api::client::internal::Manager& api);
    static ui::implementation::ContactListRowInternal* ContactListItem(
        const ui::implementation::ContactListInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::ContactListRowID& rowID,
        const ui::implementation::ContactListSortKey& key);
    static ui::implementation::ContactSubsectionRowInternal* ContactItemWidget(
        const ui::implementation::ContactSubsectionInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::ContactSubsectionRowID& rowID,
        const ui::implementation::ContactSubsectionSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static ui::implementation::ContactList* ContactListModel(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID
#if OT_QT
        ,
        const bool qt
#endif
    );
#if OT_QT
    static ui::ContactListQt* ContactListQtModel(
        ui::implementation::ContactList& parent);
#endif
    static ui::implementation::Contact* ContactModel(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const opentxs::Identifier& contactID
#if OT_QT
        ,
        const bool qt
#endif
    );
#if OT_QT
    static ui::ContactQt* ContactQtModel(ui::implementation::Contact& parent);
#endif
    static ui::implementation::ContactRowInternal* ContactSectionWidget(
        const ui::implementation::ContactInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::ContactRowID& rowID,
        const ui::implementation::ContactSortKey& key,
        const ui::implementation::CustomData& custom
#if OT_QT
        ,
        const bool qt
#endif
    );
    static ui::implementation::ContactSectionRowInternal*
    ContactSubsectionWidget(
        const ui::implementation::ContactSectionInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::ContactSectionRowID& rowID,
        const ui::implementation::ContactSectionSortKey& key,
        const ui::implementation::CustomData& custom
#if OT_QT
        ,
        const bool qt
#endif
    );
    static api::internal::Context* Context(
        Flag& running,
        const ArgList& args,
        const std::chrono::seconds gcInterval,
        OTCaller* externalPasswordCallback = nullptr);
    template <class C>
    static C* Credential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const VersionNumber version,
        const NymParameters& parameters,
        const proto::CredentialRole role,
        const opentxs::PasswordPrompt& reason);
    template <class C>
    static C* Credential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& serialized,
        const proto::KeyMode mode,
        const proto::CredentialRole role);
    static api::Crypto* Crypto(const api::Settings& settings);
    static api::crypto::Config* CryptoConfig(const api::Settings& settings);
    static auto CurrencyContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::string& tla,
        const std::uint32_t power,
        const std::string& fraction,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::unit::Currency>;
    static auto CurrencyContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::unit::Currency>;
    static network::zeromq::socket::Dealer* DealerSocket(
        const network::zeromq::Context& context,
        const bool direction,
        const network::zeromq::ListenCallback& callback);
    static api::network::Dht* Dht(
        const bool defaultEnable,
        const api::internal::Core& api,
        std::int64_t& nymPublishInterval,
        std::int64_t& nymRefreshInterval,
        std::int64_t& serverPublishInterval,
        std::int64_t& serverRefreshInterval,
        std::int64_t& unitPublishInterval,
        std::int64_t& unitRefreshInterval);
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    static crypto::key::Ed25519* Ed25519Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey);
    static crypto::key::Ed25519* Ed25519Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::KeyRole role,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
#if OT_CRYPTO_WITH_BIP32
    static crypto::key::Ed25519* Ed25519Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const OTPassword& privateKey,
        const OTPassword& chainCode,
        const Data& publicKey,
        const proto::HDPath& path,
        const Bip32Fingerprint parent,
        const proto::KeyRole role,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
#endif  // OT_CRYPTO_WITH_BIP32
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
    static api::crypto::Encode* Encode(const api::Crypto& crypto);
    static api::Endpoints* Endpoints(
        const network::zeromq::Context& zmq,
        const int instance);
    static auto Envelope(const api::internal::Core& api) noexcept
        -> std::unique_ptr<crypto::Envelope>;
    static auto Envelope(
        const api::internal::Core& api,
        const proto::Envelope& serialized) noexcept(false)
        -> std::unique_ptr<crypto::Envelope>;
    static api::internal::Factory* FactoryAPIClient(
        const api::client::internal::Manager& api);
    static api::internal::Factory* FactoryAPIServer(
        const api::server::internal::Manager& api);
#if OT_BLOCKCHAIN
    OPENTXS_EXPORT static auto GCS(
        const api::internal::Core& api,
        const std::uint32_t bits,
        const std::uint32_t fpRate,
        const std::array<std::byte, 16>& key,
        const std::vector<OTData>& elements) -> blockchain::internal::GCS*;
    OPENTXS_EXPORT static auto GCS(
        const api::internal::Core& api,
        const proto::GCS& serialized) -> blockchain::internal::GCS*;
    OPENTXS_EXPORT static auto GCS(
        const api::internal::Core& api,
        const std::uint32_t bits,
        const std::uint32_t fpRate,
        const std::array<std::byte, 16>& key,
        const std::size_t filterElementCount,
        const Data& filter) -> blockchain::internal::GCS*;
    OPENTXS_EXPORT static auto GenesisBlockHeader(
        const api::internal::Core& api,
        const blockchain::Type type) -> blockchain::block::Header*;
#endif  // OT_BLOCKCHAIN
    static auto Hash(
        const api::crypto::Encode& encode,
        const crypto::HashingProvider& ssl,
        const crypto::HashingProvider& sodium,
        const crypto::Pbkdf2& pbkdf2,
        const crypto::Ripemd160& ripe) noexcept
        -> std::unique_ptr<api::crypto::Hash>;
    static api::HDSeed* HDSeed(
        const api::Factory& factory,
        const api::crypto::Asymmetric& asymmetric,
        const api::crypto::Symmetric& symmetric,
        const api::storage::Storage& storage,
        const crypto::Bip32& bip32,
        const crypto::Bip39& bip39);
#if OT_BLOCKCHAIN
    static blockchain::client::internal::HeaderOracle* HeaderOracle(
        const api::internal::Core& api,
        const blockchain::client::internal::Network& network,
        const blockchain::client::internal::HeaderDatabase& database,
        const blockchain::Type type);
#endif  // OT_BLOCKCHAIN
    static api::client::Issuer* Issuer(
        const api::Wallet& wallet,
        const identifier::Nym& nymID,
        const proto::Issuer& serialized);
    static api::client::Issuer* Issuer(
        const api::Wallet& wallet,
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID);
    static ui::implementation::AccountSummaryRowInternal* IssuerItem(
        const ui::implementation::AccountSummaryInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::AccountSummaryRowID& rowID,
        const ui::implementation::AccountSummarySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const proto::ContactItemType currency
#if OT_QT
        ,
        const bool qt
#endif
    );
    static auto Keypair() noexcept -> std::unique_ptr<crypto::key::Keypair>;
    static auto Keypair(
        const api::internal::Core& api,
        const proto::KeyRole role,
        std::unique_ptr<crypto::key::Asymmetric> publicKey,
        std::unique_ptr<crypto::key::Asymmetric> privateKey) noexcept(false)
        -> std::unique_ptr<crypto::key::Keypair>;
    static api::Legacy* Legacy(const std::string& home);
    static api::internal::Log* Log(
        const network::zeromq::Context& zmq,
        const std::string& endpoint);
    static ui::implementation::ActivityThreadRowInternal* MailItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const bool loading,
        const bool pending);
    static ui::implementation::ActivityThreadRowInternal* MailItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static opentxs::ManagedNumber* ManagedNumber(
        const TransactionNumber number,
        opentxs::ServerContext& context);
    static ui::implementation::MessagableList* MessagableListModel(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID
#if OT_QT
        ,
        const bool qt
#endif
    );
#if OT_QT
    static ui::MessagableListQt* MessagableListQtModel(
        ui::implementation::MessagableList& parent);
#endif
#if OT_CASH
    static blind::Mint* MintLucre(const api::internal::Core& core);
    static blind::Mint* MintLucre(
        const api::internal::Core& core,
        const String& strNotaryID,
        const String& strInstrumentDefinitionID);
    static blind::Mint* MintLucre(
        const api::internal::Core& core,
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID);
#endif
    static auto NoticeAcknowledgement(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const proto::PeerRequestType type,
        const bool& ack,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::reply::Acknowledgement>;
    static auto NoticeAcknowledgement(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized) noexcept
        -> std::shared_ptr<contract::peer::reply::Acknowledgement>;
#if OT_BLOCKCHAIN
    OPENTXS_EXPORT static auto NumericHash(const blockchain::block::Hash& hash)
        -> blockchain::NumericHash*;
    OPENTXS_EXPORT static auto NumericHashNBits(const std::int32_t nBits)
        -> blockchain::NumericHash*;
#endif  // OT_BLOCKCHAIN
    static OTCallback* NullCallback();
    OPENTXS_EXPORT static auto Nym(
        const api::internal::Core& api,
        const NymParameters& nymParameters,
        const proto::ContactItemType type,
        const std::string name,
        const opentxs::PasswordPrompt& reason) -> identity::internal::Nym*;
    OPENTXS_EXPORT static auto Nym(
        const api::internal::Core& api,
        const proto::Nym& serialized,
        const std::string& alias) -> identity::internal::Nym*;
    static internal::NymFile* NymFile(
        const api::internal::Core& core,
        Nym_p targetNym,
        Nym_p signerNym);
    static identity::Source* NymIDSource(
        const api::internal::Core& api,
        NymParameters& parameters,
        const opentxs::PasswordPrompt& reason);
    static identity::Source* NymIDSource(
        const api::internal::Core& api,
        const proto::NymIDSource& serialized);
    static crypto::OpenSSL* OpenSSL(const api::Crypto& crypto);
    OPENTXS_EXPORT static auto Operation(
        const api::client::internal::Manager& api,
        const identifier::Nym& nym,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason)
        -> otx::client::internal::Operation*;
    static auto OutBailmentReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::reply::Outbailment>;
    static auto OutBailmentReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized) noexcept
        -> std::shared_ptr<contract::peer::reply::Outbailment>;
    static auto OutbailmentRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const std::uint64_t& amount,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::Outbailment>;
    static auto OutbailmentRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::Outbailment>;
    static api::client::OTX* OTX(
        const Flag& running,
        const api::client::internal::Manager& api,
        OTClient& otclient,
        const ContextLockCallback& lockCallback);
    static opentxs::PasswordPrompt* PasswordPrompt(
        const api::internal::Core& api,
        const std::string& text);
    static api::client::internal::Pair* PairAPI(
        const Flag& running,
        const api::client::internal::Manager& client);
    static network::zeromq::socket::Pair* PairSocket(
        const network::zeromq::Context& context,
        const network::zeromq::ListenCallback& callback,
        const bool startThread);
    static network::zeromq::socket::Pair* PairSocket(
        const network::zeromq::ListenCallback& callback,
        const network::zeromq::socket::Pair& peer,
        const bool startThread);
    static network::zeromq::socket::Pair* PairSocket(
        const network::zeromq::Context& context,
        const network::zeromq::ListenCallback& callback,
        const std::string& endpoint);
    static ui::implementation::PayableListRowInternal* PayableListItem(
        const ui::implementation::PayableInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::PayableListRowID& rowID,
        const ui::implementation::PayableListSortKey& key,
        const std::string& paymentcode,
        const proto::ContactItemType& currency);
    static ui::implementation::PayableList* PayableListModel(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const proto::ContactItemType& currency
#if OT_QT
        ,
        const bool qt
#endif
    );
#if OT_QT
    static ui::PayableListQt* PayableListQtModel(
        ui::implementation::PayableList& parent);
#endif
    static auto PaymentCode(
        const api::internal::Core& api,
        const std::uint8_t version,
        const bool hasBitmessage,
        const ReadView pubkey,
        const ReadView chaincode,
        const std::uint8_t bitmessageVersion,
        const std::uint8_t bitmessageStream
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        ,
        std::unique_ptr<crypto::key::Secp256k1> key
#endif
        ) noexcept -> std::unique_ptr<opentxs::PaymentCode>;
    static ui::implementation::ActivityThreadRowInternal* PaymentItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static opentxs::PeerObject* PeerObject(
        const api::internal::Core& api,
        const Nym_p& senderNym,
        const std::string& message);
    static opentxs::PeerObject* PeerObject(
        const api::internal::Core& api,
        const Nym_p& senderNym,
        const std::string& payment,
        const bool isPayment);
#if OT_CASH
    static opentxs::PeerObject* PeerObject(
        const api::internal::Core& api,
        const Nym_p& senderNym,
        const std::shared_ptr<blind::Purse> purse);
#endif
    static opentxs::PeerObject* PeerObject(
        const api::internal::Core& api,
        const OTPeerRequest request,
        const OTPeerReply reply,
        const VersionNumber version);
    static opentxs::PeerObject* PeerObject(
        const api::internal::Core& api,
        const OTPeerRequest request,
        const VersionNumber version);
    static opentxs::PeerObject* PeerObject(
        const api::client::Contacts& contacts,
        const api::internal::Core& api,
        const Nym_p& signerNym,
        const proto::PeerObject& serialized);
    static opentxs::PeerObject* PeerObject(
        const api::client::Contacts& contacts,
        const api::internal::Core& api,
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason);
    static auto PeerReply(const api::Core& api) noexcept
        -> std::shared_ptr<contract::peer::Reply>;
    static auto PeerReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized) noexcept
        -> std::shared_ptr<contract::peer::Reply>;
    static auto PeerRequest(const api::Core& api) noexcept
        -> std::shared_ptr<contract::peer::Request>;
    static auto PeerRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::Request>;
    static ui::implementation::ActivityThreadRowInternal* PendingSend(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static opentxs::PIDFile* PIDFile(const std::string& path);
    static opentxs::network::zeromq::Pipeline* Pipeline(
        const api::internal::Core& api,
        const network::zeromq::Context& context,
        std::function<void(network::zeromq::Message&)> callback);
    static identity::credential::internal::Primary* PrimaryCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
    static identity::credential::internal::Primary* PrimaryCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const proto::Credential& credential);
    static ui::implementation::ProfileSubsectionRowInternal* ProfileItemWidget(
        const ui::implementation::ProfileSubsectionInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::ProfileSubsectionRowID& rowID,
        const ui::implementation::ProfileSubsectionSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static ui::implementation::Profile* ProfileModel(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID
#if OT_QT
        ,
        const bool qt
#endif
    );
#if OT_QT
    static ui::ProfileQt* ProfileQtModel(ui::implementation::Profile& parent);
#endif
    static ui::implementation::ProfileRowInternal* ProfileSectionWidget(
        const ui::implementation::ProfileInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::ProfileRowID& rowID,
        const ui::implementation::ProfileSortKey& key,
        const ui::implementation::CustomData& custom
#if OT_QT
        ,
        const bool qt
#endif
    );
    static ui::implementation::ProfileSectionRowInternal*
    ProfileSubsectionWidget(
        const ui::implementation::ProfileSectionInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::ProfileSectionRowID& rowID,
        const ui::implementation::ProfileSectionSortKey& key,
        const ui::implementation::CustomData& custom
#if OT_QT
        ,
        const bool qt
#endif
    );
#if OT_CASH
    OPENTXS_EXPORT static auto Purse(
        const api::internal::Core& api,
        const proto::Purse& serialized) -> blind::Purse*;
    OPENTXS_EXPORT static auto Purse(
        const api::internal::Core& api,
        const opentxs::ServerContext& context,
        const proto::CashType type,
        const blind::Mint& mint,
        const Amount totalValue,
        const opentxs::PasswordPrompt& reason) -> blind::Purse*;
    OPENTXS_EXPORT static auto Purse(
        const api::internal::Core& api,
        const identity::Nym& owner,
        const identifier::Server& server,
        const identity::Nym& serverNym,
        const proto::CashType type,
        const blind::Mint& mint,
        const Amount totalValue,
        const opentxs::PasswordPrompt& reason) -> blind::Purse*;
    OPENTXS_EXPORT static auto Purse(
        const api::internal::Core& api,
        const blind::Purse& request,
        const identity::Nym& requester,
        const opentxs::PasswordPrompt& reason) -> blind::Purse*;
    OPENTXS_EXPORT static auto Purse(
        const api::internal::Core& api,
        const identity::Nym& owner,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const proto::CashType type,
        const opentxs::PasswordPrompt& reason) -> blind::Purse*;
#endif
    static network::zeromq::socket::Publish* PublishSocket(
        const network::zeromq::Context& context);
    static network::zeromq::socket::Pull* PullSocket(
        const network::zeromq::Context& context,
        const bool direction);
    static network::zeromq::socket::Pull* PullSocket(
        const network::zeromq::Context& context,
        const bool direction,
        const network::zeromq::ListenCallback& callback);
    static network::zeromq::socket::Push* PushSocket(
        const network::zeromq::Context& context,
        const bool direction);
    static network::zeromq::socket::Reply* ReplySocket(
        const network::zeromq::Context& context,
        const bool direction,
        const network::zeromq::ReplyCallback& callback);
    static network::zeromq::socket::Request* RequestSocket(
        const network::zeromq::Context& context);
    static rpc::internal::RPC* RPC(const api::Context& native);
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    static auto RSAKey(
        const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKey& input) noexcept
        -> std::unique_ptr<crypto::key::RSA>;
    static auto RSAKey(
        const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::KeyRole input,
        const VersionNumber version,
        const NymParameters& options,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::unique_ptr<crypto::key::RSA>;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
    static network::zeromq::socket::Router* RouterSocket(
        const network::zeromq::Context& context,
        const bool direction,
        const network::zeromq::ListenCallback& callback);
    static identity::credential::internal::Secondary* SecondaryCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
    static identity::credential::internal::Secondary* SecondaryCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& credential);
#if OT_CRYPTO_USING_LIBSECP256K1
    static crypto::Secp256k1* Secp256k1(
        const api::Crypto& crypto,
        const api::crypto::Util& util);
#endif  // OT_CRYPTO_USING_LIBSECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    static crypto::key::Secp256k1* Secp256k1Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey);
    static crypto::key::Secp256k1* Secp256k1Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::KeyRole role,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
#if OT_CRYPTO_WITH_BIP32
    static crypto::key::Secp256k1* Secp256k1Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const OTPassword& privateKey,
        const OTPassword& chainCode,
        const Data& publicKey,
        const proto::HDPath& path,
        const Bip32Fingerprint parent,
        const proto::KeyRole role,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
#endif  // OT_CRYPTO_WITH_BIP32
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    static auto SecurityContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::unit::Security>;
    static auto SecurityContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::unit::Security>;
    static api::client::ServerAction* ServerAction(
        const api::client::internal::Manager& api,
        const ContextLockCallback& lockCallback);
    static internal::ServerContext* ServerContext(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& requestSent,
        const network::zeromq::socket::Publish& replyReceived,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Server& server,
        network::ServerConnection& connection);
    static internal::ServerContext* ServerContext(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& requestSent,
        const network::zeromq::socket::Publish& replyReceived,
        const proto::Context& serialized,
        const Nym_p& local,
        const Nym_p& remote,
        network::ServerConnection& connection);
    static auto ServerContract(const api::Core& api) noexcept
        -> std::unique_ptr<contract::Server>;
    static auto ServerContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const std::list<Endpoint>& endpoints,
        const std::string& terms,
        const std::string& name,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::unique_ptr<contract::Server>;
    static auto ServerContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::ServerContract& serialized) noexcept
        -> std::unique_ptr<contract::Server>;
    static api::server::Manager* ServerManager(
        const api::internal::Context& parent,
        Flag& running,
        const ArgList& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance);
    static api::Settings* Settings(
        const api::Legacy& legacy,
        const String& path);
    static crypto::Sodium* Sodium(const api::Crypto& crypto);
    static api::storage::StorageInternal* Storage(
        const Flag& running,
        const api::Crypto& crypto,
        const api::Settings& config,
        const api::Legacy& legacy,
        const std::string& dataFolder,
        const String& defaultPluginCLI,
        const String& archiveDirectoryCLI,
        const std::chrono::seconds gcIntervalCLI,
        String& encryptedDirectoryCLI,
        StorageConfig& storageConfig);
#if OT_STORAGE_FS
    static opentxs::api::storage::Plugin* StorageFSArchive(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket,
        const std::string& folder,
        crypto::key::Symmetric& key);
    static opentxs::api::storage::Plugin* StorageFSGC(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
#endif
    static opentxs::api::storage::Plugin* StorageMemDB(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
#if OT_STORAGE_LMDB
    static opentxs::api::storage::Plugin* StorageLMDB(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
#endif
    static opentxs::api::storage::Multiplex* StorageMultiplex(
        const api::storage::Storage& storage,
        const Flag& primaryBucket,
        const StorageConfig& config,
        const String& primary,
        const bool migrate,
        const String& previous,
        const Digest& hash,
        const Random& random);
#if OT_STORAGE_SQLITE
    static opentxs::api::storage::Plugin* StorageSqlite3(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
#endif
    static auto StoreSecret(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const proto::SecretType type,
        const std::string& primary,
        const std::string& secondary,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::StoreSecret>;
    static auto StoreSecret(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::StoreSecret>;
    static network::zeromq::socket::Subscribe* SubscribeSocket(
        const network::zeromq::Context& context,
        const network::zeromq::ListenCallback& callback);
    static api::crypto::Symmetric* Symmetric(const api::internal::Core& api);
    static crypto::key::Symmetric* SymmetricKey();
    static crypto::key::Symmetric* SymmetricKey(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const opentxs::PasswordPrompt& reason,
        const proto::SymmetricMode mode);
    static crypto::key::Symmetric* SymmetricKey(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized);
    static crypto::key::Symmetric* SymmetricKey(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const OTPassword& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const proto::SymmetricKeyType type);
    static crypto::key::Symmetric* SymmetricKey(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const OTPassword& raw,
        const opentxs::PasswordPrompt& reason);
#if OT_CASH
    static auto Token(const blind::Token& token, blind::Purse& purse) noexcept
        -> std::unique_ptr<blind::Token>;
    static auto Token(
        const api::internal::Core& api,
        blind::Purse& purse,
        const proto::Token& serialized) noexcept(false)
        -> std::unique_ptr<blind::Token>;
    static auto Token(
        const api::internal::Core& api,
        const identity::Nym& owner,
        const blind::Mint& mint,
        const std::uint64_t value,
        blind::Purse& purse,
        const opentxs::PasswordPrompt& reason) noexcept(false)
        -> std::unique_ptr<blind::Token>;
#endif
    static api::client::UI* UI(
        const api::client::internal::Manager& api,
        const Flag& running
#if OT_QT
        ,
        const bool qt
#endif
    );
    static auto UnitDefinition(const api::Core& api) noexcept
        -> std::shared_ptr<contract::Unit>;
    static auto UnitDefinition(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::Unit>;
    static identity::credential::internal::Verification* VerificationCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
    static identity::credential::internal::Verification* VerificationCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& credential);
    static identity::wot::verification::internal::Group* VerificationGroup(
        identity::wot::verification::internal::Set& parent,
        const VersionNumber version,
        bool external);
    static identity::wot::verification::internal::Group* VerificationGroup(
        identity::wot::verification::internal::Set& parent,
        const proto::VerificationGroup& serialized,
        bool external);
    static identity::wot::verification::internal::Item* VerificationItem(
        const identity::wot::verification::internal::Nym& parent,
        const opentxs::Identifier& claim,
        const identity::Nym& signer,
        const opentxs::PasswordPrompt& reason,
        const bool value,
        const Time start,
        const Time end,
        const VersionNumber version);
    static identity::wot::verification::internal::Item* VerificationItem(
        const identity::wot::verification::internal::Nym& parent,
        const proto::Verification& serialized);
    static identity::wot::verification::internal::Nym* VerificationNym(
        identity::wot::verification::internal::Group& parent,
        const identifier::Nym& nym,
        const VersionNumber version);
    static identity::wot::verification::internal::Nym* VerificationNym(
        identity::wot::verification::internal::Group& parent,
        const proto::VerificationIdentity& serialized);
    static identity::wot::verification::internal::Set* VerificationSet(
        const api::internal::Core& api,
        const identifier::Nym& nym,
        const VersionNumber version);
    static identity::wot::verification::internal::Set* VerificationSet(
        const api::internal::Core& api,
        const identifier::Nym& nym,
        const proto::VerificationSet& serialized);
    static api::Wallet* Wallet(const api::client::internal::Manager& client);
    static api::Wallet* Wallet(const api::server::internal::Manager& server);
#if OT_BLOCKCHAIN
    OPENTXS_EXPORT static auto Work(const std::string& hex)
        -> blockchain::Work*;
    OPENTXS_EXPORT static auto Work(const blockchain::NumericHash& target)
        -> blockchain::Work*;
#endif  // OT_BLOCKCHAIN
    static api::client::Workflow* Workflow(
        const api::internal::Core& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contact);
    static api::network::ZAP* ZAP(const network::zeromq::Context& context);
    static api::network::ZMQ* ZMQ(
        const api::internal::Core& api,
        const Flag& running);
    static network::zeromq::Context* ZMQContext();
    OPENTXS_EXPORT static auto ZMQFrame() -> network::zeromq::Frame*;
    OPENTXS_EXPORT static auto ZMQFrame(
        const void* data,
        const std::size_t size) -> network::zeromq::Frame*;
    OPENTXS_EXPORT static auto ZMQFrame(const ProtobufType& data)
        -> network::zeromq::Frame*;
    OPENTXS_EXPORT static auto ZMQMessage() -> network::zeromq::Message*;
    OPENTXS_EXPORT static auto ZMQMessage(
        const void* data,
        const std::size_t size) -> network::zeromq::Message*;
    OPENTXS_EXPORT static auto ZMQMessage(const ProtobufType& data)
        -> network::zeromq::Message*;
};
}  // namespace opentxs
