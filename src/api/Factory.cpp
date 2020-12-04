// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"     // IWYU pragma: associated
#include "1_Internal.hpp"   // IWYU pragma: associated
#include "api/Factory.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <array>
#include <cstring>
#include <stdexcept>
#include <utility>

#include "2_Factory.hpp"
#include "core/crypto/PaymentCode.hpp"
#include "crypto/key/Null.hpp"
#include "internal/api/Api.hpp"
#include "internal/api/crypto/Factory.hpp"
#if OT_BLOCKCHAIN
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#endif  // OT_BLOCKCHAIN
#include "internal/network/zeromq/socket/Socket.hpp"
#include "opentxs/Forward.hpp"
#include "opentxs/OT.hpp"  // TODO remove
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.tpp"
#include "opentxs/api/Context.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/api/HDSeed.hpp"
#endif  // OT_CRYPTO_WITH_BIP32
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#if OT_CASH
#include "opentxs/blind/Mint.hpp"
#include "opentxs/blind/Purse.hpp"
#endif  // OT_CASH
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/contract/CurrencyContract.hpp"
#include "opentxs/core/contract/SecurityContract.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/contract/basket/Basket.hpp"
#include "opentxs/core/contract/basket/BasketContract.hpp"
#include "opentxs/core/contract/peer/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/BailmentReply.hpp"
#include "opentxs/core/contract/peer/BailmentRequest.hpp"
#include "opentxs/core/contract/peer/ConnectionReply.hpp"
#include "opentxs/core/contract/peer/ConnectionRequest.hpp"
#include "opentxs/core/contract/peer/NoticeAcknowledgement.hpp"
#include "opentxs/core/contract/peer/OutBailmentReply.hpp"
#include "opentxs/core/contract/peer/OutBailmentRequest.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/peer/StoreSecret.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/crypto/OTSignedFile.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTScriptable.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/trade/OTMarket.hpp"
#include "opentxs/core/trade/OTOffer.hpp"
#include "opentxs/core/trade/OTTrade.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/protobuf/AsymmetricKey.pb.h"
#include "opentxs/protobuf/CashEnums.pb.h"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/Ciphertext.pb.h"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/protobuf/Enums.pb.h"
#include "opentxs/protobuf/Envelope.pb.h"  // IWYU pragma: keep
#include "opentxs/protobuf/PaymentCode.pb.h"
#include "opentxs/protobuf/PeerEnums.pb.h"
#include "opentxs/protobuf/PeerReply.pb.h"
#include "opentxs/protobuf/PeerRequest.pb.h"
#include "opentxs/protobuf/UnitDefinition.pb.h"
#include "opentxs/protobuf/verify/Envelope.hpp"
#include "util/HDIndex.hpp"

#define OT_METHOD "opentxs::api::implementation::Factory::"

namespace opentxs::api::implementation
{
Factory::Factory(const api::internal::Core& api)
    : api::internal::Factory()
    , api_(api)
    , primitives_(opentxs::Context().Factory())  // TODO pass in as argument
    , pAsymmetric_(factory::AsymmetricAPI(api_))
    , asymmetric_(*pAsymmetric_)
    , pSymmetric_(factory::Symmetric(api_))
    , symmetric_(*pSymmetric_)
{
    OT_ASSERT(pAsymmetric_);
    OT_ASSERT(pSymmetric_);
}

auto Factory::Armored() const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored()};
}

auto Factory::Armored(const std::string& input) const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(String::Factory(input.c_str()))};
}

auto Factory::Armored(const opentxs::Data& input) const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(input)};
}

auto Factory::Armored(const opentxs::String& input) const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(input)};
}

auto Factory::Armored(const opentxs::crypto::Envelope& input) const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(input)};
}

auto Factory::Armored(const ProtobufType& input) const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(Data(input))};
}

auto Factory::Armored(const ProtobufType& input, const std::string& header)
    const -> OTString
{
    auto armored = Armored(Data(input));
    auto output = String::Factory();
    armored->WriteArmoredString(output, header);

    return output;
}

auto Factory::AsymmetricKey(
    const NymParameters& params,
    const opentxs::PasswordPrompt& reason,
    const proto::KeyRole role,
    const VersionNumber version) const -> OTAsymmetricKey
{
    auto output = asymmetric_.NewKey(params, reason, role, version).release();

    if (output) {
        return OTAsymmetricKey{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create asymmetric key");
    }
}

auto Factory::AsymmetricKey(const proto::AsymmetricKey& serialized) const
    -> OTAsymmetricKey
{
    auto output = asymmetric_.InstantiateKey(serialized).release();

    if (output) {
        return OTAsymmetricKey{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate asymmetric key");
    }
}

auto Factory::BailmentNotice(
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const identifier::UnitDefinition& unitID,
    const identifier::Server& serverID,
    const opentxs::Identifier& requestID,
    const std::string& txid,
    const Amount& amount,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTBailmentNotice
{
    auto output = opentxs::Factory::BailmentNotice(
        api_,
        nym,
        recipientID,
        unitID,
        serverID,
        requestID,
        txid,
        amount,
        reason);

    if (output) {
        return OTBailmentNotice{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create bailment notice");
    }
}

auto Factory::BailmentNotice(
    const Nym_p& nym,
    const proto::PeerRequest& serialized) const noexcept(false)
    -> OTBailmentNotice
{
    auto output = opentxs::Factory::BailmentNotice(api_, nym, serialized);

    if (output) {
        return OTBailmentNotice{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment notice");
    }
}

auto Factory::BailmentReply(
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const opentxs::Identifier& request,
    const identifier::Server& server,
    const std::string& terms,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTBailmentReply
{
    auto output = opentxs::Factory::BailmentReply(
        api_, nym, initiator, request, server, terms, reason);

    if (output) {
        return OTBailmentReply{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create bailment reply");
    }
}

auto Factory::BailmentReply(
    const Nym_p& nym,
    const proto::PeerReply& serialized) const noexcept(false) -> OTBailmentReply
{
    auto output = opentxs::Factory::BailmentReply(api_, nym, serialized);

    if (output) {
        return OTBailmentReply{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment reply");
    }
}

auto Factory::BailmentRequest(
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const identifier::UnitDefinition& unit,
    const identifier::Server& server,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTBailmentRequest
{
    auto output = opentxs::Factory::BailmentRequest(
        api_, nym, recipient, unit, server, reason);

    if (output) {
        return OTBailmentRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create bailment reply");
    }
}

auto Factory::BailmentRequest(
    const Nym_p& nym,
    const proto::PeerRequest& serialized) const noexcept(false)
    -> OTBailmentRequest
{
    auto output = opentxs::Factory::BailmentRequest(api_, nym, serialized);

    if (output) {
        return OTBailmentRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment request");
    }
}

auto Factory::Basket() const -> std::unique_ptr<opentxs::Basket>
{
    std::unique_ptr<opentxs::Basket> basket;
    basket.reset(new opentxs::Basket(api_));

    return basket;
}

auto Factory::Basket(std::int32_t nCount, std::int64_t lMinimumTransferAmount)
    const -> std::unique_ptr<opentxs::Basket>
{
    std::unique_ptr<opentxs::Basket> basket;
    basket.reset(new opentxs::Basket(api_, nCount, lMinimumTransferAmount));

    return basket;
}

auto Factory::BasketContract(
    const Nym_p& nym,
    const std::string& shortname,
    const std::string& name,
    const std::string& symbol,
    const std::string& terms,
    const std::uint64_t weight,
    const proto::ContactItemType unitOfAccount,
    const VersionNumber version) const noexcept(false) -> OTBasketContract
{
    auto output = opentxs::Factory::BasketContract(
        api_,
        nym,
        shortname,
        name,
        symbol,
        terms,
        weight,
        unitOfAccount,
        version);

    if (output) {
        return OTBasketContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create bailment reply");
    }
}

auto Factory::BasketContract(
    const Nym_p& nym,
    const proto::UnitDefinition serialized) const noexcept(false)
    -> OTBasketContract
{
    auto output = opentxs::Factory::BasketContract(api_, nym, serialized);

    if (output) {
        return OTBasketContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment request");
    }
}

#if OT_BLOCKCHAIN
auto Factory::BitcoinScriptNullData(
    const opentxs::blockchain::Type chain,
    const std::vector<ReadView>& data) const noexcept
    -> std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = opentxs::blockchain::block::bitcoin;

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(bb::OP::RETURN));

    for (const auto& element : data) {
        elements.emplace_back(bb::internal::PushData(element));
    }

    return factory::BitcoinScript(chain, std::move(elements));
}

auto Factory::BitcoinScriptP2MS(
    const opentxs::blockchain::Type chain,
    const std::uint8_t M,
    const std::uint8_t N,
    const std::vector<const opentxs::crypto::key::EllipticCurve*>& publicKeys)
    const noexcept
    -> std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = opentxs::blockchain::block::bitcoin;

    if ((0u == M) || (16u < M)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid M").Flush();

        return {};
    }

    if ((0u == N) || (16u < N)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid N").Flush();

        return {};
    }

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(static_cast<bb::OP>(M + 80)));

    for (const auto& pKey : publicKeys) {
        if (nullptr == pKey) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid key").Flush();

            return {};
        }

        const auto& key = *pKey;
        elements.emplace_back(bb::internal::PushData(key.PublicKey()));
    }

    elements.emplace_back(bb::internal::Opcode(static_cast<bb::OP>(N + 80)));
    elements.emplace_back(bb::internal::Opcode(bb::OP::CHECKMULTISIG));

    return factory::BitcoinScript(chain, std::move(elements));
}

auto Factory::BitcoinScriptP2PK(
    const opentxs::blockchain::Type chain,
    const opentxs::crypto::key::EllipticCurve& key) const noexcept
    -> std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>
{
    namespace bb = opentxs::blockchain::block::bitcoin;

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::PushData(key.PublicKey()));
    elements.emplace_back(bb::internal::Opcode(bb::OP::CHECKSIG));

    return factory::BitcoinScript(chain, std::move(elements));
}

auto Factory::BitcoinScriptP2PKH(
    const opentxs::blockchain::Type chain,
    const opentxs::crypto::key::EllipticCurve& key) const noexcept
    -> std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = opentxs::blockchain::block::bitcoin;

    auto hash = Space{};

    if (false == b::PubkeyHash(api_, chain, key.PublicKey(), writer(hash))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate pubkey hash")
            .Flush();

        return {};
    }

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(bb::OP::DUP));
    elements.emplace_back(bb::internal::Opcode(bb::OP::HASH160));
    elements.emplace_back(bb::internal::PushData(reader(hash)));
    elements.emplace_back(bb::internal::Opcode(bb::OP::EQUALVERIFY));
    elements.emplace_back(bb::internal::Opcode(bb::OP::CHECKSIG));

    return factory::BitcoinScript(chain, std::move(elements));
}

auto Factory::BitcoinScriptP2SH(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::block::bitcoin::Script& script) const noexcept
    -> std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = opentxs::blockchain::block::bitcoin;

    auto bytes = Space{};
    auto hash = Space{};

    if (false == script.Serialize(writer(bytes))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to serialize script")
            .Flush();

        return {};
    }

    if (false == b::ScriptHash(api_, chain, reader(bytes), writer(hash))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate script hash")
            .Flush();

        return {};
    }

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(bb::OP::HASH160));
    elements.emplace_back(bb::internal::PushData(reader(hash)));
    elements.emplace_back(bb::internal::Opcode(bb::OP::EQUAL));

    return factory::BitcoinScript(chain, std::move(elements));
}

auto Factory::BlockchainAddress(
    const opentxs::blockchain::p2p::Protocol protocol,
    const opentxs::blockchain::p2p::Network network,
    const opentxs::Data& bytes,
    const std::uint16_t port,
    const opentxs::blockchain::Type chain,
    const Time lastConnected,
    const std::set<opentxs::blockchain::p2p::Service>& services,
    const bool incoming) const -> OTBlockchainAddress
{
    return OTBlockchainAddress{factory::BlockchainAddress(
                                   api_,
                                   protocol,
                                   network,
                                   bytes,
                                   port,
                                   chain,
                                   lastConnected,
                                   services,
                                   incoming)
                                   .release()};
}

auto Factory::BlockchainAddress(
    const opentxs::blockchain::p2p::Address::SerializedType& serialized) const
    -> OTBlockchainAddress
{
    return OTBlockchainAddress{
        factory::BlockchainAddress(api_, serialized).release()};
}
#endif  // OT_BLOCKCHAIN

auto Factory::Cheque(const OTTransaction& receipt) const
    -> std::unique_ptr<opentxs::Cheque>
{
    std::unique_ptr<opentxs::Cheque> output{new opentxs::Cheque{api_}};

    OT_ASSERT(output)

    auto serializedItem = String::Factory();
    receipt.GetReferenceString(serializedItem);
    std::unique_ptr<opentxs::Item> item{Item(
        serializedItem,
        receipt.GetRealNotaryID(),
        receipt.GetReferenceToNum())};

    OT_ASSERT(false != bool(item));

    auto serializedCheque = String::Factory();
    item->GetAttachment(serializedCheque);
    const auto loaded = output->LoadContractFromString(serializedCheque);

    if (false == loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load cheque.").Flush();
    }

    return output;
}

auto Factory::Cheque() const -> std::unique_ptr<opentxs::Cheque>
{
    std::unique_ptr<opentxs::Cheque> cheque;
    cheque.reset(new opentxs::Cheque(api_));

    return cheque;
}

auto Factory::Cheque(
    const identifier::Server& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
    -> std::unique_ptr<opentxs::Cheque>
{
    std::unique_ptr<opentxs::Cheque> cheque;
    cheque.reset(
        new opentxs::Cheque(api_, NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    return cheque;
}

auto Factory::ConnectionReply(
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const opentxs::Identifier& request,
    const identifier::Server& server,
    const bool ack,
    const std::string& url,
    const std::string& login,
    const std::string& password,
    const std::string& key,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTConnectionReply
{
    auto output = opentxs::Factory::ConnectionReply(
        api_,
        nym,
        initiator,
        request,
        server,
        ack,
        url,
        login,
        password,
        key,
        reason);

    if (output) {
        return OTConnectionReply{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create connection reply");
    }
}

auto Factory::ConnectionReply(
    const Nym_p& nym,
    const proto::PeerReply& serialized) const noexcept(false)
    -> OTConnectionReply
{
    auto output = opentxs::Factory::ConnectionReply(api_, nym, serialized);

    if (output) {
        return OTConnectionReply{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate connection reply");
    }
}

auto Factory::ConnectionRequest(
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const proto::ConnectionInfoType type,
    const identifier::Server& server,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTConnectionRequest
{
    auto output = opentxs::Factory::ConnectionRequest(
        api_, nym, recipient, type, server, reason);

    if (output) {
        return OTConnectionRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create bailment reply");
    }
}

auto Factory::ConnectionRequest(
    const Nym_p& nym,
    const proto::PeerRequest& serialized) const noexcept(false)
    -> OTConnectionRequest
{
    auto output = opentxs::Factory::ConnectionRequest(api_, nym, serialized);

    if (output) {
        return OTConnectionRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment reply");
    }
}

auto Factory::Contract(const opentxs::String& strInput) const
    -> std::unique_ptr<opentxs::Contract>
{

    using namespace opentxs;
    auto strContract = String::Factory(),
         strFirstLine = String::Factory();  // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {

        std::unique_ptr<opentxs::Contract> pContract;

        if (strFirstLine->Contains(
                "-----BEGIN SIGNED SMARTCONTRACT-----"))  // this string is 36
                                                          // chars long.
        {
            pContract.reset(new OTSmartContract(api_));
            OT_ASSERT(false != bool(pContract));
        }

        if (strFirstLine->Contains(
                "-----BEGIN SIGNED PAYMENT PLAN-----"))  // this string is 35
                                                         // chars long.
        {
            pContract.reset(new OTPaymentPlan(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED TRADE-----"))  // this string is 28
                                                         // chars long.
        {
            pContract.reset(new OTTrade(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED OFFER-----")) {
            pContract.reset(new OTOffer(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED INVOICE-----")) {
            pContract.reset(new opentxs::Cheque(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED VOUCHER-----")) {
            pContract.reset(new opentxs::Cheque(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED CHEQUE-----")) {
            pContract.reset(new opentxs::Cheque(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED MESSAGE-----")) {
            pContract.reset(new opentxs::Message(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED MINT-----")) {
#if OT_CASH
            auto mint = Mint();
            pContract.reset(mint.release());
            OT_ASSERT(false != bool(pContract));
#endif  // OT_CASH
        } else if (strFirstLine->Contains("-----BEGIN SIGNED FILE-----")) {
            OT_ASSERT(false != bool(pContract));
        }

        // The string didn't match any of the options in the factory.
        //
        if (!pContract) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Object type not yet supported by class factory: ")(
                strFirstLine)
                .Flush();
            // Does the contract successfully load from the string passed in?
        } else if (!pContract->LoadContractFromString(strContract)) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failed loading contract from string (first line): ")(
                strFirstLine)
                .Flush();
        } else {
            return pContract;
        }
    }
    return nullptr;
}

auto Factory::Cron() const -> std::unique_ptr<OTCron> { return {}; }

auto Factory::CronItem(const String& strCronItem) const
    -> std::unique_ptr<OTCronItem>
{
    std::array<char, 45> buf{};

    if (!strCronItem.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Empty string was passed in (returning nullptr).")
            .Flush();
        return nullptr;
    }

    auto strContract = String::Factory(strCronItem.Get());

    if (!strContract->DecodeIfArmored(false)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Input string apparently was encoded and "
            "then failed decoding. Contents: ")(strCronItem)(".")
            .Flush();
        return nullptr;
    }

    strContract->reset();  // for sgets
    bool bGotLine = strContract->sgets(buf.data(), 40);

    if (!bGotLine) return nullptr;

    auto strFirstLine = String::Factory(buf.data());
    // set the "file" pointer within this string back to index 0.
    strContract->reset();

    // Now I feel pretty safe -- the string I'm examining is within
    // the first 45 characters of the beginning of the contract, and
    // it will NOT contain the escape "- " sequence. From there, if
    // it contains the proper sequence, I will instantiate that type.
    if (!strFirstLine->Exists() || strFirstLine->Contains("- -"))
        return nullptr;

    // By this point we know already that it's not escaped.
    // BUT it might still be ARMORED!

    std::unique_ptr<OTCronItem> pItem;
    // this string is 35 chars long.
    if (strFirstLine->Contains("-----BEGIN SIGNED PAYMENT PLAN-----")) {
        pItem.reset(new OTPaymentPlan(api_));
    }
    // this string is 28 chars long.
    else if (strFirstLine->Contains("-----BEGIN SIGNED TRADE-----")) {
        pItem.reset(new OTTrade(api_));
    }
    // this string is 36 chars long.
    else if (strFirstLine->Contains("-----BEGIN SIGNED SMARTCONTRACT-----")) {
        pItem.reset(new OTSmartContract(api_));
    } else {
        return nullptr;
    }

    // Does the contract successfully load from the string passed in?
    if (pItem->LoadContractFromString(strContract)) { return pItem; }

    return nullptr;
}

auto Factory::CurrencyContract(
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
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTCurrencyContract
{
    auto output = opentxs::Factory::CurrencyContract(
        api_,
        nym,
        shortname,
        name,
        symbol,
        terms,
        tla,
        power,
        fraction,
        unitOfAccount,
        version,
        reason);

    if (output) {
        return OTCurrencyContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create currency contract");
    }
}

auto Factory::CurrencyContract(
    const Nym_p& nym,
    const proto::UnitDefinition serialized) const noexcept(false)
    -> OTCurrencyContract
{
    auto output = opentxs::Factory::CurrencyContract(api_, nym, serialized);

    if (output) {
        return OTCurrencyContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate currency contract");
    }
}

auto Factory::Data() const -> OTData { return Data::Factory(); }

auto Factory::Data(const opentxs::Armored& input) const -> OTData
{
    return Data::Factory(input);
}

auto Factory::Data(const ProtobufType& input) const -> OTData
{
    auto output = Data::Factory();
    output->SetSize(input.ByteSize());
    input.SerializeToArray(output->data(), output->size());

    return output;
}

auto Factory::Data(const opentxs::network::zeromq::Frame& input) const -> OTData
{
    return Data::Factory(input);
}

auto Factory::Data(const std::uint8_t input) const -> OTData
{
    return Data::Factory(input);
}

auto Factory::Data(const std::uint32_t input) const -> OTData
{
    return Data::Factory(input);
}

auto Factory::Data(const std::string& input, const StringStyle mode) const
    -> OTData
{
    return Data::Factory(input, static_cast<Data::Mode>(mode));
}

auto Factory::Data(const std::vector<unsigned char>& input) const -> OTData
{
    return Data::Factory(input);
}

auto Factory::Data(const std::vector<std::byte>& input) const -> OTData
{
    return Data::Factory(input);
}

auto Factory::Data(ReadView input) const -> OTData
{
    return Data::Factory(input.data(), input.size());
}

auto Factory::Envelope() const noexcept -> OTEnvelope
{
    return OTEnvelope{opentxs::Factory::Envelope(api_).release()};
}

auto Factory::Envelope(const opentxs::Armored& in) const noexcept(false)
    -> OTEnvelope
{
    auto data = Data();

    if (false == in.GetData(data)) {
        throw std::runtime_error("Invalid armored envelope");
    }

    return Envelope(
        proto::Factory<opentxs::crypto::Envelope::SerializedType>(data));
}

auto Factory::Envelope(
    const opentxs::crypto::Envelope::SerializedType& serialized) const
    noexcept(false) -> OTEnvelope
{
    if (false == proto::Validate(serialized, VERBOSE)) {
        throw std::runtime_error("Invalid serialized envelope");
    }

    return OTEnvelope{opentxs::Factory::Envelope(api_, serialized).release()};
}

auto Factory::Identifier() const -> OTIdentifier
{
    return Identifier::Factory();
}

auto Factory::Identifier(const std::string& serialized) const -> OTIdentifier
{
    return Identifier::Factory(serialized);
}

auto Factory::Identifier(const opentxs::String& serialized) const
    -> OTIdentifier
{
    return Identifier::Factory(serialized);
}

auto Factory::Identifier(const opentxs::Contract& contract) const
    -> OTIdentifier
{
    return Identifier::Factory(contract);
}

auto Factory::Identifier(const opentxs::Item& item) const -> OTIdentifier
{
    return Identifier::Factory(item);
}

auto Factory::Identifier(const ReadView bytes) const -> OTIdentifier
{
    auto output = this->Identifier();
    output->CalculateDigest(bytes);

    return output;
}

auto Factory::Identifier(const ProtobufType& proto) const -> OTIdentifier
{
    const auto bytes = Data(proto);

    return Identifier(bytes->Bytes());
}

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
auto Factory::instantiate_secp256k1(const ReadView key) const noexcept
    -> std::unique_ptr<opentxs::crypto::key::Secp256k1>
{
    using ReturnType = opentxs::crypto::key::Secp256k1;

    auto serialized = ReturnType::Serialized{};
    serialized.set_version(ReturnType::DefaultVersion);
    serialized.set_type(proto::AKEYTYPE_SECP256K1);
    serialized.set_mode(proto::KEYMODE_PUBLIC);
    serialized.set_role(proto::KEYROLE_SIGN);
    serialized.set_key(key.data(), key.size());
    auto output = std::unique_ptr<opentxs::crypto::key::Secp256k1>{
        opentxs::Factory::Secp256k1Key(
            api_, api_.Crypto().SECP256K1(), serialized)};

    if (false == bool(output)) {
        output = std::make_unique<
            opentxs::crypto::key::implementation::NullSecp256k1>();
    }

    OT_ASSERT(output);

    return output;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

auto Factory::Item(const std::string& serialized) const
    -> std::unique_ptr<opentxs::Item>
{
    return Item(String::Factory(serialized));
}

auto Factory::Item(const String& serialized) const
    -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> output{new opentxs::Item(api_)};

    if (output) {
        const auto loaded = output->LoadContractFromString(serialized);

        if (false == loaded) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to deserialize.")
                .Flush();
            output.reset();
        }
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to instantiate.").Flush();
    }

    return output;
}

auto Factory::Item(
    const identifier::Nym& theNymID,
    const opentxs::Item& theOwner) const -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(api_, theNymID, theOwner));

    return item;
}

auto Factory::Item(
    const identifier::Nym& theNymID,
    const OTTransaction& theOwner) const -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(api_, theNymID, theOwner));

    return item;
}

auto Factory::Item(
    const identifier::Nym& theNymID,
    const OTTransaction& theOwner,
    itemType theType,
    const opentxs::Identifier& pDestinationAcctID) const
    -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(
        api_, theNymID, theOwner, theType, pDestinationAcctID));

    return item;
}

// Sometimes I don't know user ID of the originator, or the account ID of the
// originator,
// until after I have loaded the item. It's simply impossible to set those
// values ahead
// of time, sometimes. In those cases, we set the values appropriately but then
// we need
// to verify that the user ID is actually the owner of the AccountID. TOdo that.
auto Factory::Item(
    const String& strItem,
    const identifier::Server& theNotaryID,
    std::int64_t lTransactionNumber) const -> std::unique_ptr<opentxs::Item>
{
    if (!strItem.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": strItem is empty. (Expected an "
                                           "item).")
            .Flush();
        return nullptr;
    }

    std::unique_ptr<opentxs::Item> pItem{new opentxs::Item(api_)};

    // So when it loads its own server ID, we can compare to this one.
    pItem->SetRealNotaryID(theNotaryID);

    // This loads up the purported account ID and the user ID.
    if (pItem->LoadContractFromString(strItem)) {
        const opentxs::Identifier& ACCOUNT_ID = pItem->GetPurportedAccountID();
        pItem->SetRealAccountID(ACCOUNT_ID);  // I do this because it's all
                                              // we've got in this case. It's
                                              // what's in the
        // xml, so it must be right. If it's a lie, the signature will fail or
        // the
        // user will not show as the owner of that account. But remember, the
        // server
        // sent the message in the first place.

        pItem->SetTransactionNum(lTransactionNumber);

        if (pItem->VerifyContractID())  // this compares purported and real
                                        // account IDs, as well as server IDs.
        {
            return pItem;
        }
    }

    return nullptr;
}

// Let's say you have created a transaction, and you are creating an item to put
// into it.
// Well in that case, you don't care to verify that the real IDs match the
// purported IDs, since
// you are creating this item yourself, not verifying it from someone else.
// Use this function to create the new Item before you add it to your new
// Transaction.
auto Factory::Item(
    const OTTransaction& theOwner,
    itemType theType,
    const opentxs::Identifier& pDestinationAcctID) const
    -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> pItem{new opentxs::Item(
        api_, theOwner.GetNymID(), theOwner, theType, pDestinationAcctID)};

    if (false != bool(pItem)) {
        pItem->SetPurportedAccountID(theOwner.GetPurportedAccountID());
        pItem->SetPurportedNotaryID(theOwner.GetPurportedNotaryID());
        return pItem;
    }
    return nullptr;
}

auto Factory::Keypair(
    const NymParameters& params,
    const VersionNumber version,
    const proto::KeyRole role,
    const opentxs::PasswordPrompt& reason) const -> OTKeypair
{
    auto pPrivateKey = asymmetric_.NewKey(params, reason, role, version);

    if (false == bool(pPrivateKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive private key")
            .Flush();

        return OTKeypair{opentxs::Factory::Keypair().release()};
    }

    auto& privateKey = *pPrivateKey;
    auto pPublicKey = privateKey.asPublic();

    if (false == bool(pPublicKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive public key")
            .Flush();

        return OTKeypair{opentxs::Factory::Keypair().release()};
    }

    try {
        return OTKeypair{
            opentxs::Factory::Keypair(
                api_, role, std::move(pPublicKey), std::move(pPrivateKey))
                .release()};
    } catch (...) {
        return OTKeypair{opentxs::Factory::Keypair().release()};
    }
}

auto Factory::Keypair(
    const proto::AsymmetricKey& serializedPubkey,
    const proto::AsymmetricKey& serializedPrivkey) const -> OTKeypair
{
    auto pPrivateKey = asymmetric_.InstantiateKey(serializedPrivkey);

    if (false == bool(pPrivateKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to instantiate private key")
            .Flush();

        return OTKeypair{opentxs::Factory::Keypair().release()};
    }

    auto pPublicKey = asymmetric_.InstantiateKey(serializedPubkey);

    if (false == bool(pPublicKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate public key")
            .Flush();

        return OTKeypair{opentxs::Factory::Keypair().release()};
    }

    try {
        return OTKeypair{opentxs::Factory::Keypair(
                             api_,
                             serializedPrivkey.role(),
                             std::move(pPublicKey),
                             std::move(pPrivateKey))
                             .release()};
    } catch (...) {
        return OTKeypair{opentxs::Factory::Keypair().release()};
    }
}

auto Factory::Keypair(const proto::AsymmetricKey& serializedPubkey) const
    -> OTKeypair
{
    auto pPublicKey = asymmetric_.InstantiateKey(serializedPubkey);

    if (false == bool(pPublicKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate public key")
            .Flush();

        return OTKeypair{opentxs::Factory::Keypair().release()};
    }

    try {
        return OTKeypair{
            opentxs::Factory::Keypair(
                api_,
                serializedPubkey.role(),
                std::move(pPublicKey),
                std::make_unique<opentxs::crypto::key::implementation::Null>())
                .release()};
    } catch (...) {
        return OTKeypair{opentxs::Factory::Keypair().release()};
    }
}

#if OT_CRYPTO_WITH_BIP32
auto Factory::Keypair(
    const std::string& fingerprint,
    const Bip32Index nym,
    const Bip32Index credset,
    const Bip32Index credindex,
    const EcdsaCurve& curve,
    const proto::KeyRole role,
    const opentxs::PasswordPrompt& reason) const -> OTKeypair
{
    auto input(fingerprint);
    auto roleIndex = Bip32Index{0};

    switch (role) {
        case proto::KEYROLE_AUTH: {
            roleIndex = HDIndex{Bip32Child::AUTH_KEY, Bip32Child::HARDENED};
        } break;
        case proto::KEYROLE_ENCRYPT: {
            roleIndex = HDIndex{Bip32Child::ENCRYPT_KEY, Bip32Child::HARDENED};
        } break;
        case proto::KEYROLE_SIGN: {
            roleIndex = HDIndex{Bip32Child::SIGN_KEY, Bip32Child::HARDENED};
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid key role").Flush();

            return OTKeypair{opentxs::Factory::Keypair().release()};
        }
    }

    const auto path = api::HDSeed::Path{
        HDIndex{Bip43Purpose::NYM, Bip32Child::HARDENED},
        HDIndex{nym, Bip32Child::HARDENED},
        HDIndex{credset, Bip32Child::HARDENED},
        HDIndex{credindex, Bip32Child::HARDENED},
        roleIndex};
    auto pPrivateKey = api_.Seeds().GetHDKey(input, curve, path, reason, role);

    if (false == bool(pPrivateKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive private key")
            .Flush();

        return OTKeypair{opentxs::Factory::Keypair().release()};
    }

    auto& privateKey = *pPrivateKey;
    auto pPublicKey = privateKey.asPublic();

    if (false == bool(pPublicKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive public key")
            .Flush();

        return OTKeypair{opentxs::Factory::Keypair().release()};
    }

    try {
        return OTKeypair{
            opentxs::Factory::Keypair(
                api_, role, std::move(pPublicKey), std::move(pPrivateKey))
                .release()};
    } catch (...) {
        return OTKeypair{opentxs::Factory::Keypair().release()};
    }
}
#endif  // OT_CRYPTO_WITH_BIP32

auto Factory::Ledger(
    const opentxs::Identifier& theAccountID,
    const identifier::Server& theNotaryID) const
    -> std::unique_ptr<opentxs::Ledger>
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(new opentxs::Ledger(api_, theAccountID, theNotaryID));

    return ledger;
}

auto Factory::Ledger(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAccountID,
    const identifier::Server& theNotaryID) const
    -> std::unique_ptr<opentxs::Ledger>
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(
        new opentxs::Ledger(api_, theNymID, theAccountID, theNotaryID));

    return ledger;
}

auto Factory::Ledger(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAcctID,
    const identifier::Server& theNotaryID,
    ledgerType theType,
    bool bCreateFile) const -> std::unique_ptr<opentxs::Ledger>
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(new opentxs::Ledger(api_, theNymID, theAcctID, theNotaryID));

    ledger->generate_ledger(
        theNymID, theAcctID, theNotaryID, theType, bCreateFile);

    return ledger;
}

auto Factory::Market() const -> std::unique_ptr<OTMarket>
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(api_));

    return market;
}

auto Factory::Market(const char* szFilename) const -> std::unique_ptr<OTMarket>
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(api_, szFilename));

    return market;
}

auto Factory::Market(
    const identifier::Server& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const identifier::UnitDefinition& CURRENCY_TYPE_ID,
    const std::int64_t& lScale) const -> std::unique_ptr<OTMarket>
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(
        api_, NOTARY_ID, INSTRUMENT_DEFINITION_ID, CURRENCY_TYPE_ID, lScale));

    return market;
}

auto Factory::Message() const -> std::unique_ptr<opentxs::Message>
{
    std::unique_ptr<opentxs::Message> message;
    message.reset(new opentxs::Message(api_));

    return message;
}

#if OT_CASH
auto Factory::Mint() const -> std::unique_ptr<blind::Mint>
{
    std::unique_ptr<blind::Mint> pMint;

#if OT_CASH_USING_LUCRE
    pMint.reset(opentxs::Factory::MintLucre(api_));

    OT_ASSERT(false != bool(pMint));

#else
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Open-Transactions isn't built with any digital cash algorithms, "
        "so it's impossible to instantiate a mint.")
        .Flush();
#endif

    return pMint;
}

auto Factory::Mint(
    const String& strNotaryID,
    const String& strInstrumentDefinitionID) const
    -> std::unique_ptr<blind::Mint>
{
    std::unique_ptr<blind::Mint> pMint;

#if OT_CASH_USING_LUCRE
    pMint.reset(opentxs::Factory::MintLucre(
        api_, strNotaryID, strInstrumentDefinitionID));

    OT_ASSERT(false != bool(pMint));
#else
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Open-Transactions isn't built with any digital cash algorithms, "
        "so it's impossible to instantiate a mint.")
        .Flush();
#endif

    return pMint;
}

auto Factory::Mint(
    const String& strNotaryID,
    const String& strServerNymID,
    const String& strInstrumentDefinitionID) const
    -> std::unique_ptr<blind::Mint>
{
    std::unique_ptr<blind::Mint> pMint;

#if OT_CASH_USING_LUCRE
    pMint.reset(opentxs::Factory::MintLucre(
        api_, strNotaryID, strServerNymID, strInstrumentDefinitionID));

    OT_ASSERT(false != bool(pMint));
#else
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Open-Transactions isn't built with any digital cash algorithms, "
        "so it's impossible to instantiate a mint.")
        .Flush();
#endif

    return pMint;
}
#endif

auto Factory::NymID() const -> OTNymID { return identifier::Nym::Factory(); }

auto Factory::NymID(const std::string& serialized) const -> OTNymID
{
    return identifier::Nym::Factory(serialized);
}

auto Factory::NymID(const opentxs::String& serialized) const -> OTNymID
{
    return identifier::Nym::Factory(serialized);
}

auto Factory::NymIDFromPaymentCode(const std::string& input) const -> OTNymID
{
    auto output = NymID();
    auto key = Data::Factory();
    const auto bytes = Data::Factory(
        api_.Crypto().Encode().IdentifierDecode(input), Data::Mode::Raw);

    if (81 != bytes->size()) { return output; }

    if (bytes->Extract(65, key, 3)) { output->CalculateDigest(key->Bytes()); }

    return output;
}

auto Factory::Offer() const -> std::unique_ptr<OTOffer>
{
    std::unique_ptr<OTOffer> offer;
    offer.reset(new OTOffer(api_));

    return offer;
}

auto Factory::Offer(
    const identifier::Server& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const identifier::UnitDefinition& CURRENCY_ID,
    const std::int64_t& MARKET_SCALE) const -> std::unique_ptr<OTOffer>
{
    std::unique_ptr<OTOffer> offer;
    offer.reset(new OTOffer(
        api_, NOTARY_ID, INSTRUMENT_DEFINITION_ID, CURRENCY_ID, MARKET_SCALE));

    return offer;
}

auto Factory::OutbailmentReply(
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const opentxs::Identifier& request,
    const identifier::Server& server,
    const std::string& terms,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTOutbailmentReply
{
    auto output = opentxs::Factory::OutBailmentReply(
        api_, nym, initiator, request, server, terms, reason);

    if (output) {
        return OTOutbailmentReply{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create outbailment reply");
    }
}

auto Factory::OutbailmentReply(
    const Nym_p& nym,
    const proto::PeerReply& serialized) const noexcept(false)
    -> OTOutbailmentReply
{
    auto output = opentxs::Factory::OutBailmentReply(api_, nym, serialized);

    if (output) {
        return OTOutbailmentReply{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate outbailment reply");
    }
}

auto Factory::OutbailmentRequest(
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const identifier::UnitDefinition& unit,
    const identifier::Server& server,
    const std::uint64_t& amount,
    const std::string& terms,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTOutbailmentRequest
{
    auto output = opentxs::Factory::OutbailmentRequest(
        api_, nym, recipient, unit, server, amount, terms, reason);

    if (output) {
        return OTOutbailmentRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create outbailment reply");
    }
}

auto Factory::OutbailmentRequest(
    const Nym_p& nym,
    const proto::PeerRequest& serialized) const noexcept(false)
    -> OTOutbailmentRequest
{
    auto output = opentxs::Factory::OutbailmentRequest(api_, nym, serialized);

    if (output) {
        return OTOutbailmentRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate outbailment request");
    }
}

auto Factory::PasswordPrompt(const std::string& text) const -> OTPasswordPrompt
{
    return OTPasswordPrompt{opentxs::Factory::PasswordPrompt(api_, text)};
}

auto Factory::Payment() const -> std::unique_ptr<OTPayment>
{
    std::unique_ptr<OTPayment> payment;
    payment.reset(new OTPayment(api_));

    return payment;
}

auto Factory::Payment(const String& strPayment) const
    -> std::unique_ptr<OTPayment>
{
    std::unique_ptr<OTPayment> payment;
    payment.reset(new OTPayment(api_, strPayment));

    return payment;
}

auto Factory::Payment(
    const opentxs::Contract& contract,
    const opentxs::PasswordPrompt& reason) const -> std::unique_ptr<OTPayment>
{
    auto payment = Factory::Payment(String::Factory(contract));

    if (payment) { payment->SetTempValues(reason); }

    return payment;
}

auto Factory::PaymentCode(const std::string& base58) const noexcept
    -> OTPaymentCode
{
    using ReturnType = opentxs::implementation::PaymentCode;
    auto raw = ReturnType::SerializedForBase58{};

    {
        const auto bytes = api_.Crypto().Encode().IdentifierDecode(base58);

        if (0 < bytes.size()) {
            std::memcpy(
                &raw, bytes.data(), std::min(sizeof(raw), bytes.size()));
        }
    }

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    auto key = instantiate_secp256k1({raw.key_.data(), raw.key_.size()});
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

    return OTPaymentCode
    {
        opentxs::Factory::PaymentCode(
            api_,
            raw.version_,
            raw.haveBitmessage(),
            {raw.key_.data(), raw.key_.size()},
            {raw.code_.data(), raw.code_.size()},
            raw.bm_version_,
            raw.bm_stream_
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
            ,
            std::move(key)
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
                )
            .release()
    };
}

auto Factory::PaymentCode(const proto::PaymentCode& serialized) const noexcept
    -> OTPaymentCode
{
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    auto key = instantiate_secp256k1(serialized.key());
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

    return OTPaymentCode
    {
        opentxs::Factory::PaymentCode(
            api_,
            static_cast<std::uint8_t>(serialized.version()),
            serialized.bitmessage(),
            serialized.key(),
            serialized.chaincode(),
            static_cast<std::uint8_t>(serialized.bitmessageversion()),
            static_cast<std::uint8_t>(serialized.bitmessagestream())
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
                ,
            std::move(key)
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
                )
            .release()
    };
}

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
auto Factory::PaymentCode(
    const std::string& seed,
    const Bip32Index nym,
    const std::uint8_t version,
    const opentxs::PasswordPrompt& reason,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream) const noexcept -> OTPaymentCode
{
    auto fingerprint{seed};
    auto pKey = api_.Seeds().GetPaymentCode(fingerprint, nym, reason);

    if (false == bool(pKey)) {
        pKey = std::make_unique<
            opentxs::crypto::key::implementation::NullSecp256k1>();
    }

    OT_ASSERT(pKey);

    const auto& key = *pKey;
    auto pubkey = key.PublicKey();
    auto chaincode = key.Chaincode(reason);

    return OTPaymentCode{opentxs::Factory::PaymentCode(
                             api_,
                             version,
                             bitmessage,
                             std::move(pubkey),
                             std::move(chaincode),
                             bitmessageVersion,
                             bitmessageStream,
                             std::move(pKey))
                             .release()};
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32

auto Factory::PaymentPlan() const -> std::unique_ptr<OTPaymentPlan>
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(new OTPaymentPlan(api_));

    return paymentplan;
}

auto Factory::PaymentPlan(
    const identifier::Server& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
    -> std::unique_ptr<OTPaymentPlan>
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(
        new OTPaymentPlan(api_, NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    return paymentplan;
}

auto Factory::PaymentPlan(
    const identifier::Server& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const opentxs::Identifier& SENDER_ACCT_ID,
    const identifier::Nym& SENDER_NYM_ID,
    const opentxs::Identifier& RECIPIENT_ACCT_ID,
    const identifier::Nym& RECIPIENT_NYM_ID) const
    -> std::unique_ptr<OTPaymentPlan>
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(new OTPaymentPlan(
        api_,
        NOTARY_ID,
        INSTRUMENT_DEFINITION_ID,
        SENDER_ACCT_ID,
        SENDER_NYM_ID,
        RECIPIENT_ACCT_ID,
        RECIPIENT_NYM_ID));

    return paymentplan;
}

auto Factory::PeerObject(
    [[maybe_unused]] const Nym_p& senderNym,
    [[maybe_unused]] const std::string& message) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

auto Factory::PeerObject(
    [[maybe_unused]] const Nym_p& senderNym,
    [[maybe_unused]] const std::string& payment,
    [[maybe_unused]] const bool isPayment) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

#if OT_CASH
auto Factory::PeerObject(
    [[maybe_unused]] const Nym_p& senderNym,
    [[maybe_unused]] const std::shared_ptr<blind::Purse> purse) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}
#endif

auto Factory::PeerObject(
    [[maybe_unused]] const OTPeerRequest request,
    [[maybe_unused]] const OTPeerReply reply,
    [[maybe_unused]] const VersionNumber version) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

auto Factory::PeerObject(
    [[maybe_unused]] const OTPeerRequest request,
    [[maybe_unused]] const VersionNumber version) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

auto Factory::PeerObject(
    [[maybe_unused]] const Nym_p& signerNym,
    [[maybe_unused]] const proto::PeerObject& serialized) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

auto Factory::PeerObject(
    [[maybe_unused]] const Nym_p& recipientNym,
    [[maybe_unused]] const opentxs::Armored& encrypted,
    [[maybe_unused]] const opentxs::PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

auto Factory::PeerReply() const noexcept -> OTPeerReply
{
    return OTPeerReply{opentxs::Factory::PeerReply(api_)};
}

auto Factory::PeerReply(const Nym_p& nym, const proto::PeerReply& serialized)
    const noexcept(false) -> OTPeerReply
{
    switch (serialized.type()) {
        case proto::PEERREQUEST_BAILMENT: {
            return BailmentReply(nym, serialized).as<contract::peer::Reply>();
        }
        case proto::PEERREQUEST_CONNECTIONINFO: {
            return ConnectionReply(nym, serialized).as<contract::peer::Reply>();
        }
        case proto::PEERREQUEST_OUTBAILMENT: {
            return OutbailmentReply(nym, serialized)
                .as<contract::peer::Reply>();
        }
        case proto::PEERREQUEST_PENDINGBAILMENT:
        case proto::PEERREQUEST_STORESECRET: {
            return ReplyAcknowledgement(nym, serialized)
                .as<contract::peer::Reply>();
        }
        case proto::PEERREQUEST_VERIFICATIONOFFER:
        case proto::PEERREQUEST_FAUCET:
        case proto::PEERREQUEST_ERROR:
        default: {
            throw std::runtime_error("Unsupported reply type");
        }
    }
}

auto Factory::PeerRequest() const noexcept -> OTPeerRequest
{
    return OTPeerRequest{opentxs::Factory::PeerRequest(api_)};
}

auto Factory::PeerRequest(
    const Nym_p& nym,
    const proto::PeerRequest& serialized) const noexcept(false) -> OTPeerRequest
{
    switch (serialized.type()) {
        case proto::PEERREQUEST_BAILMENT: {
            return BailmentRequest(nym, serialized)
                .as<contract::peer::Request>();
        }
        case proto::PEERREQUEST_OUTBAILMENT: {
            return OutbailmentRequest(nym, serialized)
                .as<contract::peer::Request>();
        }
        case proto::PEERREQUEST_PENDINGBAILMENT: {
            return BailmentNotice(nym, serialized)
                .as<contract::peer::Request>();
        }
        case proto::PEERREQUEST_CONNECTIONINFO: {
            return ConnectionRequest(nym, serialized)
                .as<contract::peer::Request>();
        }
        case proto::PEERREQUEST_STORESECRET: {
            return StoreSecret(nym, serialized).as<contract::peer::Request>();
        }
        case proto::PEERREQUEST_VERIFICATIONOFFER:
        case proto::PEERREQUEST_FAUCET:
        case proto::PEERREQUEST_ERROR:
        default: {
            throw std::runtime_error("Unsupported reply type");
        }
    }
}

auto Factory::Pipeline(
    std::function<void(opentxs::network::zeromq::Message&)> callback) const
    -> OTZMQPipeline
{
    return OTZMQPipeline{factory::Pipeline(api_, api_.ZeroMQ(), callback)};
}

#if OT_CASH
auto Factory::Purse(
    const otx::context::Server& context,
    const identifier::UnitDefinition& unit,
    const blind::Mint& mint,
    const Amount totalValue,
    const opentxs::PasswordPrompt& reason,
    const proto::CashType type) const -> std::unique_ptr<blind::Purse>
{
    return std::unique_ptr<blind::Purse>(
        opentxs::Factory::Purse(api_, context, type, mint, totalValue, reason));
}

auto Factory::Purse(const proto::Purse& serialized) const
    -> std::unique_ptr<blind::Purse>
{
    return std::unique_ptr<blind::Purse>(
        opentxs::Factory::Purse(api_, serialized));
}

auto Factory::Purse(
    const identity::Nym& owner,
    const identifier::Server& server,
    const identifier::UnitDefinition& unit,
    const opentxs::PasswordPrompt& reason,
    const proto::CashType type) const -> std::unique_ptr<blind::Purse>
{
    return std::unique_ptr<blind::Purse>(
        opentxs::Factory::Purse(api_, owner, server, unit, type, reason));
}
#endif  // OT_CASH

auto Factory::ReplyAcknowledgement(
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const opentxs::Identifier& request,
    const identifier::Server& server,
    const proto::PeerRequestType type,
    const bool& ack,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTReplyAcknowledgement
{
    auto output = opentxs::Factory::NoticeAcknowledgement(
        api_, nym, initiator, request, server, type, ack, reason);

    if (output) {
        return OTReplyAcknowledgement{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create peer acknowledgement");
    }
}

auto Factory::ReplyAcknowledgement(
    const Nym_p& nym,
    const proto::PeerReply& serialized) const noexcept(false)
    -> OTReplyAcknowledgement
{
    auto output =
        opentxs::Factory::NoticeAcknowledgement(api_, nym, serialized);

    if (output) {
        return OTReplyAcknowledgement{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate peer acknowledgement");
    }
}

auto Factory::Scriptable(const String& strInput) const
    -> std::unique_ptr<OTScriptable>
{
    std::array<char, 45> buf{};

    if (!strInput.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: Input string is empty.")
            .Flush();
        return nullptr;
    }

    auto strContract = String::Factory(strInput.Get());

    if (!strContract->DecodeIfArmored(false))  // bEscapedIsAllowed=true
                                               // by default.
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Input string apparently was encoded and "
            "then failed decoding. Contents: ")(strInput)(".")
            .Flush();
        return nullptr;
    }

    // At this point, strContract contains the actual contents, whether they
    // were originally ascii-armored OR NOT. (And they are also now trimmed,
    // either way.)
    //
    strContract->reset();  // for sgets
    bool bGotLine = strContract->sgets(buf.data(), 40);

    if (!bGotLine) return nullptr;

    std::unique_ptr<OTScriptable> pItem;

    auto strFirstLine = String::Factory(buf.data());
    strContract->reset();  // set the "file" pointer within this string back to
                           // index 0.

    // Now I feel pretty safe -- the string I'm examining is within
    // the first 45 characters of the beginning of the contract, and
    // it will NOT contain the escape "- " sequence. From there, if
    // it contains the proper sequence, I will instantiate that type.
    if (!strFirstLine->Exists() || strFirstLine->Contains("- -"))
        return nullptr;

    // There are actually two factories that load smart contracts. See
    // OTCronItem.
    //
    else if (strFirstLine->Contains(
                 "-----BEGIN SIGNED SMARTCONTRACT-----"))  // this string is 36
                                                           // chars long.
    {
        pItem.reset(new OTSmartContract(api_));
        OT_ASSERT(false != bool(pItem));
    }

    // The string didn't match any of the options in the factory.
    if (false == bool(pItem)) return nullptr;

    // Does the contract successfully load from the string passed in?
    if (pItem->LoadContractFromString(strContract)) return pItem;

    return nullptr;
}

auto Factory::SecurityContract(
    const Nym_p& nym,
    const std::string& shortname,
    const std::string& name,
    const std::string& symbol,
    const std::string& terms,
    const proto::ContactItemType unitOfAccount,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTSecurityContract
{
    auto output = opentxs::Factory::SecurityContract(
        api_,
        nym,
        shortname,
        name,
        symbol,
        terms,
        unitOfAccount,
        version,
        reason);

    if (output) {
        return OTSecurityContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create currency contract");
    }
}

auto Factory::SecurityContract(
    const Nym_p& nym,
    const proto::UnitDefinition serialized) const noexcept(false)
    -> OTSecurityContract
{
    auto output = opentxs::Factory::SecurityContract(api_, nym, serialized);

    if (output) {
        return OTSecurityContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate currency contract");
    }
}

auto Factory::ServerContract() const noexcept(false) -> OTServerContract
{
    return OTServerContract{opentxs::Factory::ServerContract(api_)};
}

auto Factory::ServerID() const -> OTServerID
{
    return identifier::Server::Factory();
}

auto Factory::ServerID(const std::string& serialized) const -> OTServerID
{
    return identifier::Server::Factory(serialized);
}

auto Factory::ServerID(const opentxs::String& serialized) const -> OTServerID
{
    return identifier::Server::Factory(serialized);
}

auto Factory::SignedFile() const -> std::unique_ptr<OTSignedFile>
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_));

    return signedfile;
}

auto Factory::SignedFile(const String& LOCAL_SUBDIR, const String& FILE_NAME)
    const -> std::unique_ptr<OTSignedFile>
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}
auto Factory::SignedFile(const char* LOCAL_SUBDIR, const String& FILE_NAME)
    const -> std::unique_ptr<OTSignedFile>
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}

auto Factory::SignedFile(const char* LOCAL_SUBDIR, const char* FILE_NAME) const
    -> std::unique_ptr<OTSignedFile>
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}

#ifdef OPENTXS_CHAINCONFIG_SMARTCONTRACT
auto Factory::SmartContract() const -> std::unique_ptr<OTSmartContract>
{
    std::unique_ptr<OTSmartContract> smartcontract;
    smartcontract.reset(new OTSmartContract(api_));

    return smartcontract;
}

auto Factory::SmartContract(const identifier::Server& NOTARY_ID) const
    -> std::unique_ptr<OTSmartContract>
{
    std::unique_ptr<OTSmartContract> smartcontract;
    smartcontract.reset(new OTSmartContract(api_, NOTARY_ID));

    return smartcontract;
}
#else // Disabled smart contracts
auto Factory::SmartContract() 
{
    bool disabled = true;
    return disabled;
}

auto Factory::SmartContract(const identifier::Server& NOTARY_ID) const
    -> std::unique_ptr<OTSmartContract>
{
    bool disabled = true;
    return disabled;
}
#endif

auto Factory::StoreSecret(
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const proto::SecretType type,
    const std::string& primary,
    const std::string& secondary,
    const identifier::Server& server,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTStoreSecret
{
    auto output = opentxs::Factory::StoreSecret(
        api_, nym, recipient, type, primary, secondary, server, reason);

    if (output) {
        return OTStoreSecret{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create bailment reply");
    }
}

auto Factory::StoreSecret(
    const Nym_p& nym,
    const proto::PeerRequest& serialized) const noexcept(false) -> OTStoreSecret
{
    auto output = opentxs::Factory::StoreSecret(api_, nym, serialized);

    if (output) {
        return OTStoreSecret{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment request");
    }
}

auto Factory::SymmetricKey() const -> OTSymmetricKey
{
    return OTSymmetricKey{opentxs::Factory::SymmetricKey()};
}

auto Factory::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const opentxs::PasswordPrompt& password,
    const proto::SymmetricMode mode) const -> OTSymmetricKey
{
    return OTSymmetricKey{
        opentxs::Factory::SymmetricKey(api_, engine, password, mode)};
}

auto Factory::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const proto::SymmetricKey serialized) const -> OTSymmetricKey
{
    return OTSymmetricKey{
        opentxs::Factory::SymmetricKey(api_, engine, serialized)};
}

auto Factory::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::size_t size,
    const proto::SymmetricKeyType type) const -> OTSymmetricKey
{
    return OTSymmetricKey{opentxs::Factory::SymmetricKey(
        api_, engine, seed, operations, difficulty, size, type)};
}

auto Factory::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const opentxs::Secret& raw,
    const opentxs::PasswordPrompt& reason) const -> OTSymmetricKey
{
    return OTSymmetricKey{
        opentxs::Factory::SymmetricKey(api_, engine, raw, reason)};
}

auto Factory::Trade() const -> std::unique_ptr<OTTrade>
{
    std::unique_ptr<OTTrade> trade;
    trade.reset(new OTTrade(api_));

    return trade;
}

auto Factory::Trade(
    const identifier::Server& notaryID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const opentxs::Identifier& assetAcctId,
    const identifier::Nym& nymID,
    const identifier::UnitDefinition& currencyId,
    const opentxs::Identifier& currencyAcctId) const -> std::unique_ptr<OTTrade>
{
    std::unique_ptr<OTTrade> trade;
    trade.reset(new OTTrade(
        api_,
        notaryID,
        instrumentDefinitionID,
        assetAcctId,
        nymID,
        currencyId,
        currencyAcctId));

    return trade;
}

auto Factory::Transaction(const String& strInput) const
    -> std::unique_ptr<OTTransactionType>
{
    auto strContract = String::Factory(),
         strFirstLine = String::Factory();  // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        std::unique_ptr<OTTransactionType> pContract;

        if (strFirstLine->Contains(
                "-----BEGIN SIGNED TRANSACTION-----"))  // this string is 34
                                                        // chars long.
        {
            pContract.reset(new OTTransaction(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED TRANSACTION ITEM-----"))  // this
                                                                    // string is
                                                                    // 39 chars
                                                                    // long.
        {
            pContract.reset(new opentxs::Item(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED LEDGER-----"))  // this string is 29
                                                          // chars long.
        {
            pContract.reset(new opentxs::Ledger(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED ACCOUNT-----"))  // this string is 30
                                                           // chars long.
        {
            OT_FAIL;
        }

        // The string didn't match any of the options in the factory.
        //

        //        const char* szFunc = "OTTransactionType::TransactionFactory";
        // The string didn't match any of the options in the factory.
        if (nullptr == pContract) {
            LogNormal(OT_METHOD)(__FUNCTION__)(  //<< szFunc
                ": Object type not yet supported by class factory: ")(
                strFirstLine)
                .Flush();
            return nullptr;
        }

        // This causes pItem to load ASSUMING that the PurportedAcctID and
        // PurportedNotaryID are correct.
        // The object is still expected to be internally consistent with its
        // sub-items, regarding those IDs,
        // but the big difference is that it will SET the Real Acct and Real
        // Notary IDs based on the purported
        // values. This way you can load a transaction without knowing the
        // account in advance.
        //
        pContract->SetLoadInsecure();

        // Does the contract successfully load from the string passed in?
        if (pContract->LoadContractFromString(strContract)) {
            // NOTE: this already happens in OTTransaction::ProcessXMLNode and
            // OTLedger::ProcessXMLNode.
            // Specifically, it happens when m_bLoadSecurely is set to false.
            //
            //          pContract->SetRealNotaryID(pItem->GetPurportedNotaryID());
            //          pContract->SetRealAccountID(pItem->GetPurportedAccountID());

            return pContract;
        } else {
            LogNormal(OT_METHOD)(__FUNCTION__)(  //<< szFunc
                ": Failed loading contract from string (first line): ")(
                strFirstLine)
                .Flush();
        }
    }

    return nullptr;
}

auto Factory::Transaction(const opentxs::Ledger& theOwner) const
    -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(api_, theOwner));

    return transaction;
}

auto Factory::Transaction(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAccountID,
    const identifier::Server& theNotaryID,
    originType theOriginType) const -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_, theNymID, theAccountID, theNotaryID, theOriginType));

    return transaction;
}

auto Factory::Transaction(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAccountID,
    const identifier::Server& theNotaryID,
    std::int64_t lTransactionNum,
    originType theOriginType) const -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_,
        theNymID,
        theAccountID,
        theNotaryID,
        lTransactionNum,
        theOriginType));

    return transaction;
}
// THIS factory only used when loading an abbreviated box receipt
// (inbox, nymbox, or outbox receipt).
// The full receipt is loaded only after the abbreviated ones are loaded,
// and verified against them.
auto Factory::Transaction(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAccountID,
    const identifier::Server& theNotaryID,
    const std::int64_t& lNumberOfOrigin,
    originType theOriginType,
    const std::int64_t& lTransactionNum,
    const std::int64_t& lInRefTo,
    const std::int64_t& lInRefDisplay,
    const Time the_DATE_SIGNED,
    transactionType theType,
    const String& strHash,
    const std::int64_t& lAdjustment,
    const std::int64_t& lDisplayValue,
    const std::int64_t& lClosingNum,
    const std::int64_t& lRequestNum,
    bool bReplyTransSuccess,
    NumList* pNumList) const -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_,
        theNymID,
        theAccountID,
        theNotaryID,
        lNumberOfOrigin,
        theOriginType,
        lTransactionNum,
        lInRefTo,
        lInRefDisplay,
        the_DATE_SIGNED,
        theType,
        strHash,
        lAdjustment,
        lDisplayValue,
        lClosingNum,
        lRequestNum,
        bReplyTransSuccess,
        pNumList));

    return transaction;
}

auto Factory::Transaction(
    const opentxs::Ledger& theOwner,
    transactionType theType,
    originType theOriginType /*=originType::not_applicable*/,
    std::int64_t lTransactionNum /*=0*/) const -> std::unique_ptr<OTTransaction>
{
    auto pTransaction = Transaction(
        theOwner.GetNymID(),
        theOwner.GetPurportedAccountID(),
        theOwner.GetPurportedNotaryID(),
        theType,
        theOriginType,
        lTransactionNum);
    if (false != bool(pTransaction)) pTransaction->SetParent(theOwner);

    return pTransaction;
}

auto Factory::Transaction(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAccountID,
    const identifier::Server& theNotaryID,
    transactionType theType,
    originType theOriginType /*=originType::not_applicable*/,
    std::int64_t lTransactionNum /*=0*/) const -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_,
        theNymID,
        theAccountID,
        theNotaryID,
        lTransactionNum,
        theOriginType));
    OT_ASSERT(false != bool(transaction));

    transaction->m_Type = theType;

    // Since we're actually generating this transaction, then we can go ahead
    // and set the purported account and server IDs (we have already set the
    // real ones in the constructor). Now both sets are fill with matching data.
    // No need to security check the IDs since we are creating this transaction
    // versus loading and inspecting it.
    transaction->SetPurportedAccountID(theAccountID);
    transaction->SetPurportedNotaryID(theNotaryID);

    return transaction;
}

auto Factory::UnitDefinition() const noexcept -> OTUnitDefinition
{
    return OTUnitDefinition{opentxs::Factory::UnitDefinition(api_)};
}

auto Factory::UnitDefinition(
    const Nym_p& nym,
    const proto::UnitDefinition serialized) const noexcept(false)
    -> OTUnitDefinition
{
    auto output = opentxs::Factory::UnitDefinition(api_, nym, serialized);

    if (output) {
        return OTUnitDefinition{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate unit definition");
    }
}

auto Factory::UnitID() const -> OTUnitID
{
    return identifier::UnitDefinition::Factory();
}

auto Factory::UnitID(const std::string& serialized) const -> OTUnitID
{
    return identifier::UnitDefinition::Factory(serialized);
}

auto Factory::UnitID(const opentxs::String& serialized) const -> OTUnitID
{
    return identifier::UnitDefinition::Factory(serialized);
}
}  // namespace opentxs::api::implementation
