// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/Factory.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/api/HDSeed.hpp"
#endif
#if OT_CASH
#include "opentxs/blind/Mint.hpp"
#include "opentxs/blind/Purse.hpp"
#endif
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/contract/basket/Basket.hpp"
#include "opentxs/core/contract/basket/BasketContract.hpp"
#include "opentxs/core/contract/peer/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/BailmentRequest.hpp"
#include "opentxs/core/contract/peer/BailmentReply.hpp"
#include "opentxs/core/contract/peer/ConnectionRequest.hpp"
#include "opentxs/core/contract/peer/ConnectionReply.hpp"
#include "opentxs/core/contract/peer/NoticeAcknowledgement.hpp"
#include "opentxs/core/contract/peer/OutBailmentRequest.hpp"
#include "opentxs/core/contract/peer/OutBailmentReply.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/peer/StoreSecret.hpp"
#include "opentxs/core/contract/CurrencyContract.hpp"
#include "opentxs/core/contract/SecurityContract.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTSignedFile.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/recurring/OTAgreement.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/trade/OTOffer.hpp"
#include "opentxs/core/trade/OTTrade.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/crypto/key/HD.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"

#include "internal/api/Api.hpp"
#if OT_BLOCKCHAIN
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#endif  // OT_BLOCKCHAIN

#include <array>

#include "Factory.hpp"

#define OT_METHOD "opentxs::api::implementation::Factory::"

namespace opentxs::api::implementation
{
Factory::Factory(const api::internal::Core& api)
    : api::internal::Factory()
    , api_(api)
    , pAsymmetric_(opentxs::Factory::AsymmetricAPI(api_))
    , asymmetric_(*pAsymmetric_)
    , pSymmetric_(opentxs::Factory::Symmetric(api_))
    , symmetric_(*pSymmetric_)
{
    OT_ASSERT(pAsymmetric_);
    OT_ASSERT(pSymmetric_);
}

OTArmored Factory::Armored() const
{
    return OTArmored{opentxs::Factory::Armored()};
}

OTArmored Factory::Armored(const std::string& input) const
{
    return OTArmored{opentxs::Factory::Armored(String::Factory(input.c_str()))};
}

OTArmored Factory::Armored(const opentxs::Data& input) const
{
    return OTArmored{opentxs::Factory::Armored(input)};
}

OTArmored Factory::Armored(const opentxs::String& input) const
{
    return OTArmored{opentxs::Factory::Armored(input)};
}

OTArmored Factory::Armored(const opentxs::OTEnvelope& input) const
{
    return OTArmored{opentxs::Factory::Armored(input)};
}

OTArmored Factory::Armored(const ProtobufType& input) const
{
    return OTArmored{opentxs::Factory::Armored(Data(input))};
}

OTString Factory::Armored(const ProtobufType& input, const std::string& header)
    const
{
    auto armored = Armored(Data(input));
    auto output = String::Factory();
    armored->WriteArmoredString(output, header);

    return output;
}

OTAsymmetricKey Factory::AsymmetricKey(
    const NymParameters& params,
    const opentxs::PasswordPrompt& reason,
    const proto::KeyRole role,
    const VersionNumber version) const
{
    return OTAsymmetricKey{
        asymmetric_.NewKey(params, reason, role, version).release()};
}

OTAsymmetricKey Factory::AsymmetricKey(
    const proto::AsymmetricKey& serialized,
    const opentxs::PasswordPrompt& reason) const
{
    return OTAsymmetricKey{
        asymmetric_.InstantiateKey(serialized, reason).release()};
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
    const proto::PeerRequest& serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTBailmentNotice
{
    auto output =
        opentxs::Factory::BailmentNotice(api_, nym, serialized, reason);

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
    const proto::PeerReply& serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTBailmentReply
{
    auto output =
        opentxs::Factory::BailmentReply(api_, nym, serialized, reason);

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
    const proto::PeerRequest& serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTBailmentRequest
{
    auto output =
        opentxs::Factory::BailmentRequest(api_, nym, serialized, reason);

    if (output) {
        return OTBailmentRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment request");
    }
}

std::unique_ptr<opentxs::Basket> Factory::Basket() const
{
    std::unique_ptr<opentxs::Basket> basket;
    basket.reset(new opentxs::Basket(api_));

    return basket;
}

std::unique_ptr<opentxs::Basket> Factory::Basket(
    std::int32_t nCount,
    std::int64_t lMinimumTransferAmount) const
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
    const proto::UnitDefinition serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTBasketContract
{
    auto output =
        opentxs::Factory::BasketContract(api_, nym, serialized, reason);

    if (output) {
        return OTBasketContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment request");
    }
}

#if OT_BLOCKCHAIN
OTBlockchainAddress Factory::BlockchainAddress(
    const blockchain::p2p::Protocol protocol,
    const blockchain::p2p::Network network,
    const opentxs::Data& bytes,
    const std::uint16_t port,
    const blockchain::Type chain,
    const Time lastConnected,
    const std::set<blockchain::p2p::Service>& services) const
{
    return OTBlockchainAddress{opentxs::Factory::BlockchainAddress(
        api_, protocol, network, bytes, port, chain, lastConnected, services)};
}

std::unique_ptr<blockchain::block::Header> Factory::BlockHeader(
    const proto::BlockchainBlockHeader& serialized) const
{
    if (false == proto::Validate(serialized, VERBOSE)) { return {}; }

    const auto type(static_cast<blockchain::Type>(serialized.type()));

    switch (type) {
        case blockchain::Type::Bitcoin: {
            return std::unique_ptr<blockchain::block::Header>(
                opentxs::Factory::BitcoinBlockHeader(api_, serialized));
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported type (")(
                static_cast<std::uint32_t>(type))(")")
                .Flush();

            return {};
        }
    }
}

std::unique_ptr<blockchain::block::Header> Factory::BlockHeader(
    const blockchain::Type type,
    const opentxs::Data& raw) const
{
    switch (type) {
        case blockchain::Type::Bitcoin: {
            return std::unique_ptr<blockchain::block::Header>(
                opentxs::Factory::BitcoinBlockHeader(api_, raw));
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported type (")(
                static_cast<std::uint32_t>(type))(")")
                .Flush();

            return {};
        }
    }
}

std::unique_ptr<blockchain::block::Header> Factory::BlockHeader(
    const blockchain::Type type,
    const blockchain::block::Hash& hash,
    const blockchain::block::Hash& parent,
    const blockchain::block::Height height) const
{
    switch (type) {
        case blockchain::Type::Bitcoin: {
            return std::unique_ptr<blockchain::block::Header>(
                opentxs::Factory::BitcoinBlockHeader(
                    api_, hash, parent, height));
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

std::unique_ptr<OTPassword> Factory::BinarySecret() const
{
    auto output = std::make_unique<OTPassword>();

    OT_ASSERT(output);

    auto& secret = *output;
    std::array<std::uint8_t, 32> empty{0};
    secret.setMemory(empty.data(), empty.size());

    return output;
}

std::unique_ptr<opentxs::Cheque> Factory::Cheque(
    const OTTransaction& receipt,
    const opentxs::PasswordPrompt& reason) const
{
    std::unique_ptr<opentxs::Cheque> output{new opentxs::Cheque{api_}};

    OT_ASSERT(output)

    auto serializedItem = String::Factory();
    receipt.GetReferenceString(serializedItem);
    std::unique_ptr<opentxs::Item> item{Item(
        serializedItem,
        receipt.GetRealNotaryID(),
        receipt.GetReferenceToNum(),
        reason)};

    OT_ASSERT(false != bool(item));

    auto serializedCheque = String::Factory();
    item->GetAttachment(serializedCheque);
    const auto loaded =
        output->LoadContractFromString(serializedCheque, reason);

    if (false == loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load cheque.").Flush();
    }

    return output;
}

std::unique_ptr<opentxs::Cheque> Factory::Cheque() const
{
    std::unique_ptr<opentxs::Cheque> cheque;
    cheque.reset(new opentxs::Cheque(api_));

    return cheque;
}

std::unique_ptr<opentxs::Cheque> Factory::Cheque(
    const identifier::Server& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
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
    const proto::PeerReply& serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTConnectionReply
{
    auto output =
        opentxs::Factory::ConnectionReply(api_, nym, serialized, reason);

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
    const proto::PeerRequest& serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTConnectionRequest
{
    auto output =
        opentxs::Factory::ConnectionRequest(api_, nym, serialized, reason);

    if (output) {
        return OTConnectionRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment reply");
    }
}

std::unique_ptr<opentxs::Contract> Factory::Contract(
    const opentxs::String& strInput,
    const opentxs::PasswordPrompt& reason) const
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
        } else if (!pContract->LoadContractFromString(strContract, reason)) {
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

std::unique_ptr<OTCron> Factory::Cron() const { return {}; }

std::unique_ptr<OTCronItem> Factory::CronItem(
    const String& strCronItem,
    const opentxs::PasswordPrompt& reason) const
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
    if (pItem->LoadContractFromString(strContract, reason)) { return pItem; }

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
    const proto::UnitDefinition serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTCurrencyContract
{
    auto output =
        opentxs::Factory::CurrencyContract(api_, nym, serialized, reason);

    if (output) {
        return OTCurrencyContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate currency contract");
    }
}

OTData Factory::Data() const { return Data::Factory(); }

OTData Factory::Data(const opentxs::Armored& input) const
{
    return Data::Factory(input);
}

OTData Factory::Data(const ProtobufType& input) const
{
    auto output = Data::Factory();
    output->SetSize(input.ByteSize());
    input.SerializeToArray(output->data(), output->size());

    return output;
}

OTData Factory::Data(const opentxs::network::zeromq::Frame& input) const
{
    return Data::Factory(input);
}

OTData Factory::Data(const std::uint8_t input) const
{
    return Data::Factory(input);
}

OTData Factory::Data(const std::uint32_t input) const
{
    return Data::Factory(input);
}

OTData Factory::Data(const std::string input, const StringStyle mode) const
{
    return Data::Factory(input, static_cast<Data::Mode>(mode));
}

OTData Factory::Data(const std::vector<unsigned char>& input) const
{
    return Data::Factory(input);
}

OTData Factory::Data(const std::vector<std::byte>& input) const
{
    return Data::Factory(input);
}

OTIdentifier Factory::Identifier() const { return Identifier::Factory(); }

OTIdentifier Factory::Identifier(const std::string& serialized) const
{
    return Identifier::Factory(serialized);
}

OTIdentifier Factory::Identifier(const opentxs::String& serialized) const
{
    return Identifier::Factory(serialized);
}

OTIdentifier Factory::Identifier(const opentxs::Contract& contract) const
{
    return Identifier::Factory(contract);
}

OTIdentifier Factory::Identifier(const opentxs::Item& item) const
{
    return Identifier::Factory(item);
}

std::unique_ptr<opentxs::Item> Factory::Item(
    const std::string& serialized,
    const opentxs::PasswordPrompt& reason) const
{
    return Item(String::Factory(serialized), reason);
}

std::unique_ptr<opentxs::Item> Factory::Item(
    const String& serialized,
    const opentxs::PasswordPrompt& reason) const
{
    std::unique_ptr<opentxs::Item> output{new opentxs::Item(api_)};

    if (output) {
        const auto loaded = output->LoadContractFromString(serialized, reason);

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

std::unique_ptr<opentxs::Item> Factory::Item(
    const identifier::Nym& theNymID,
    const opentxs::Item& theOwner) const
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(api_, theNymID, theOwner));

    return item;
}

std::unique_ptr<opentxs::Item> Factory::Item(
    const identifier::Nym& theNymID,
    const OTTransaction& theOwner) const
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(api_, theNymID, theOwner));

    return item;
}

std::unique_ptr<opentxs::Item> Factory::Item(
    const identifier::Nym& theNymID,
    const OTTransaction& theOwner,
    itemType theType,
    const opentxs::Identifier& pDestinationAcctID) const
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
std::unique_ptr<opentxs::Item> Factory::Item(
    const String& strItem,
    const identifier::Server& theNotaryID,
    std::int64_t lTransactionNumber,
    const opentxs::PasswordPrompt& reason) const
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
    if (pItem->LoadContractFromString(strItem, reason)) {
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
std::unique_ptr<opentxs::Item> Factory::Item(
    const OTTransaction& theOwner,
    itemType theType,
    const opentxs::Identifier& pDestinationAcctID) const
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

OTKeypair Factory::Keypair(
    const NymParameters& nymParameters,
    const VersionNumber version,
    const proto::KeyRole role,
    const opentxs::PasswordPrompt& reason) const
{
    return OTKeypair{
        opentxs::Factory::Keypair(api_, nymParameters, version, role, reason)};
}

OTKeypair Factory::Keypair(
    const proto::AsymmetricKey& serializedPubkey,
    const proto::AsymmetricKey& serializedPrivkey,
    const opentxs::PasswordPrompt& reason) const
{
    return OTKeypair{opentxs::Factory::Keypair(
        api_, serializedPubkey, serializedPrivkey, reason)};
}

OTKeypair Factory::Keypair(
    const proto::AsymmetricKey& serializedPubkey,
    const opentxs::PasswordPrompt& reason) const
{
    return OTKeypair{opentxs::Factory::Keypair(api_, serializedPubkey, reason)};
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
OTKeypair Factory::Keypair(
    const std::string& fingerprint,
    const Bip32Index nym,
    const Bip32Index credset,
    const Bip32Index credindex,
    const EcdsaCurve& curve,
    const proto::KeyRole role,
    const opentxs::PasswordPrompt& reason) const
{
    std::string input(fingerprint);
    Bip32Index roleIndex{0};

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

            return OTKeypair{opentxs::Factory::Keypair()};
        }
    }

    const api::HDSeed::Path path{
        HDIndex{Bip43Purpose::NYM, Bip32Child::HARDENED},
        HDIndex{nym, Bip32Child::HARDENED},
        HDIndex{credset, Bip32Child::HARDENED},
        HDIndex{credindex, Bip32Child::HARDENED},
        roleIndex};
    auto pPrivateKey = api_.Seeds().GetHDKey(input, curve, path, reason, role);

    if (false == bool(pPrivateKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive private key")
            .Flush();

        return OTKeypair{opentxs::Factory::Keypair()};
    }

    auto& privateKey = *pPrivateKey;
    const auto pSerialized = privateKey.Serialize();

    if (false == bool(pSerialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to serialize private key")
            .Flush();

        return OTKeypair{opentxs::Factory::Keypair()};
    }

    const auto& serialized = *pSerialized;
    proto::AsymmetricKey publicKey;
    const bool haveKey =
        privateKey.ECDSA().PrivateToPublic(api_, serialized, publicKey, reason);

    if (false == haveKey) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive public key")
            .Flush();

        return OTKeypair{opentxs::Factory::Keypair()};
    }

    return Keypair(publicKey, serialized, reason);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

std::unique_ptr<opentxs::Ledger> Factory::Ledger(
    const opentxs::Identifier& theAccountID,
    const identifier::Server& theNotaryID) const
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(new opentxs::Ledger(api_, theAccountID, theNotaryID));

    return ledger;
}

std::unique_ptr<opentxs::Ledger> Factory::Ledger(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAccountID,
    const identifier::Server& theNotaryID) const
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(
        new opentxs::Ledger(api_, theNymID, theAccountID, theNotaryID));

    return ledger;
}

std::unique_ptr<opentxs::Ledger> Factory::Ledger(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAcctID,
    const identifier::Server& theNotaryID,
    ledgerType theType,
    bool bCreateFile) const
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(new opentxs::Ledger(api_, theNymID, theAcctID, theNotaryID));

    ledger->generate_ledger(
        theNymID, theAcctID, theNotaryID, theType, bCreateFile);

    return ledger;
}

std::unique_ptr<OTMarket> Factory::Market() const
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(api_));

    return market;
}

std::unique_ptr<OTMarket> Factory::Market(const char* szFilename) const
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(api_, szFilename));

    return market;
}

std::unique_ptr<OTMarket> Factory::Market(
    const identifier::Server& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const identifier::UnitDefinition& CURRENCY_TYPE_ID,
    const std::int64_t& lScale) const
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(
        api_, NOTARY_ID, INSTRUMENT_DEFINITION_ID, CURRENCY_TYPE_ID, lScale));

    return market;
}

std::unique_ptr<opentxs::Message> Factory::Message() const
{
    std::unique_ptr<opentxs::Message> message;
    message.reset(new opentxs::Message(api_));

    return message;
}

#if OT_CASH
std::unique_ptr<blind::Mint> Factory::Mint() const
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

std::unique_ptr<blind::Mint> Factory::Mint(
    const String& strNotaryID,
    const String& strInstrumentDefinitionID) const
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

std::unique_ptr<blind::Mint> Factory::Mint(
    const String& strNotaryID,
    const String& strServerNymID,
    const String& strInstrumentDefinitionID) const
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

OTNymID Factory::NymID() const { return identifier::Nym::Factory(); }

OTNymID Factory::NymID(const std::string& serialized) const
{
    return identifier::Nym::Factory(serialized);
}

OTNymID Factory::NymID(const opentxs::String& serialized) const
{
    return identifier::Nym::Factory(serialized);
}

OTNymID Factory::NymIDFromPaymentCode(const std::string& input) const
{
    auto output = NymID();
    auto key = Data::Factory();
    const auto bytes = Data::Factory(
        api_.Crypto().Encode().IdentifierDecode(input), Data::Mode::Raw);

    if (81 != bytes->size()) { return output; }

    if (bytes->Extract(65, key, 3)) { output->CalculateDigest(key); }

    return output;
}

std::unique_ptr<OTOffer> Factory::Offer() const
{
    std::unique_ptr<OTOffer> offer;
    offer.reset(new OTOffer(api_));

    return offer;
}

std::unique_ptr<OTOffer> Factory::Offer(
    const identifier::Server& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const identifier::UnitDefinition& CURRENCY_ID,
    const std::int64_t& MARKET_SCALE) const
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
    const proto::PeerReply& serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTOutbailmentReply
{
    auto output =
        opentxs::Factory::OutBailmentReply(api_, nym, serialized, reason);

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
    const proto::PeerRequest& serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTOutbailmentRequest
{
    auto output =
        opentxs::Factory::OutbailmentRequest(api_, nym, serialized, reason);

    if (output) {
        return OTOutbailmentRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate outbailment request");
    }
}

OTPasswordPrompt Factory::PasswordPrompt(const std::string& text) const
{
    return OTPasswordPrompt{opentxs::Factory::PasswordPrompt(api_, text)};
}

std::unique_ptr<OTPayment> Factory::Payment() const
{
    std::unique_ptr<OTPayment> payment;
    payment.reset(new OTPayment(api_));

    return payment;
}

std::unique_ptr<OTPayment> Factory::Payment(const String& strPayment) const
{
    std::unique_ptr<OTPayment> payment;
    payment.reset(new OTPayment(api_, strPayment));

    return payment;
}

std::unique_ptr<OTPayment> Factory::Payment(
    const opentxs::Contract& contract,
    const opentxs::PasswordPrompt& reason) const
{
    auto payment = Factory::Payment(String::Factory(contract));

    if (payment) { payment->SetTempValues(reason); }

    return payment;
}

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
OTPaymentCode Factory::PaymentCode(
    const std::string& base58,
    const opentxs::PasswordPrompt& reason) const
{
    return OTPaymentCode{opentxs::Factory::PaymentCode(api_, base58, reason)};
}

OTPaymentCode Factory::PaymentCode(
    const proto::PaymentCode& serialized,
    const opentxs::PasswordPrompt& reason) const
{
    return OTPaymentCode{
        opentxs::Factory::PaymentCode(api_, serialized, reason)};
}

OTPaymentCode Factory::PaymentCode(
    const std::string& seed,
    const Bip32Index nym,
    const std::uint8_t version,
    const opentxs::PasswordPrompt& reason,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream) const
{
    return OTPaymentCode{opentxs::Factory::PaymentCode(
        api_,
        seed,
        nym,
        version,
        reason,
        bitmessage,
        bitmessageVersion,
        bitmessageStream)};
}
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

std::unique_ptr<OTPaymentPlan> Factory::PaymentPlan() const
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(new OTPaymentPlan(api_));

    return paymentplan;
}

std::unique_ptr<OTPaymentPlan> Factory::PaymentPlan(
    const identifier::Server& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(
        new OTPaymentPlan(api_, NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    return paymentplan;
}

std::unique_ptr<OTPaymentPlan> Factory::PaymentPlan(
    const identifier::Server& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const opentxs::Identifier& SENDER_ACCT_ID,
    const identifier::Nym& SENDER_NYM_ID,
    const opentxs::Identifier& RECIPIENT_ACCT_ID,
    const identifier::Nym& RECIPIENT_NYM_ID) const
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

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    [[maybe_unused]] const Nym_p& senderNym,
    [[maybe_unused]] const std::string& message,
    [[maybe_unused]] const opentxs::PasswordPrompt& reason) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    [[maybe_unused]] const Nym_p& senderNym,
    [[maybe_unused]] const std::string& payment,
    [[maybe_unused]] const bool isPayment,
    [[maybe_unused]] const opentxs::PasswordPrompt& reason) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

#if OT_CASH
std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    [[maybe_unused]] const Nym_p& senderNym,
    [[maybe_unused]] const std::shared_ptr<blind::Purse> purse,
    [[maybe_unused]] const opentxs::PasswordPrompt& reason) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}
#endif

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    [[maybe_unused]] const OTPeerRequest request,
    [[maybe_unused]] const OTPeerReply reply,
    [[maybe_unused]] const VersionNumber version,
    [[maybe_unused]] const opentxs::PasswordPrompt& reason) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    [[maybe_unused]] const OTPeerRequest request,
    [[maybe_unused]] const VersionNumber version,
    [[maybe_unused]] const opentxs::PasswordPrompt& reason) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    [[maybe_unused]] const Nym_p& signerNym,
    [[maybe_unused]] const proto::PeerObject& serialized,
    [[maybe_unused]] const opentxs::PasswordPrompt& reason) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    [[maybe_unused]] const Nym_p& recipientNym,
    [[maybe_unused]] const opentxs::Armored& encrypted,
    [[maybe_unused]] const opentxs::PasswordPrompt& reason) const
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

auto Factory::PeerReply(
    const Nym_p& nym,
    const proto::PeerReply& serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false) -> OTPeerReply
{
    switch (serialized.type()) {
        case proto::PEERREQUEST_BAILMENT: {
            return BailmentReply(nym, serialized, reason)
                .as<contract::peer::Reply>();
        }
        case proto::PEERREQUEST_CONNECTIONINFO: {
            return ConnectionReply(nym, serialized, reason)
                .as<contract::peer::Reply>();
        }
        case proto::PEERREQUEST_OUTBAILMENT: {
            return OutbailmentReply(nym, serialized, reason)
                .as<contract::peer::Reply>();
        }
        case proto::PEERREQUEST_PENDINGBAILMENT:
        case proto::PEERREQUEST_STORESECRET: {
            return ReplyAcknowledgement(nym, serialized, reason)
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
    const proto::PeerRequest& serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTPeerRequest
{
    switch (serialized.type()) {
        case proto::PEERREQUEST_BAILMENT: {
            return BailmentRequest(nym, serialized, reason)
                .as<contract::peer::Request>();
        }
        case proto::PEERREQUEST_OUTBAILMENT: {
            return OutbailmentRequest(nym, serialized, reason)
                .as<contract::peer::Request>();
        }
        case proto::PEERREQUEST_PENDINGBAILMENT: {
            return BailmentNotice(nym, serialized, reason)
                .as<contract::peer::Request>();
        }
        case proto::PEERREQUEST_CONNECTIONINFO: {
            return ConnectionRequest(nym, serialized, reason)
                .as<contract::peer::Request>();
        }
        case proto::PEERREQUEST_STORESECRET: {
            return StoreSecret(nym, serialized, reason)
                .as<contract::peer::Request>();
        }
        case proto::PEERREQUEST_VERIFICATIONOFFER:
        case proto::PEERREQUEST_FAUCET:
        case proto::PEERREQUEST_ERROR:
        default: {
            throw std::runtime_error("Unsupported reply type");
        }
    }
}

OTZMQPipeline Factory::Pipeline(
    std::function<void(opentxs::network::zeromq::Message&)> callback) const
{
    return OTZMQPipeline{
        opentxs::Factory::Pipeline(api_, api_.ZeroMQ(), callback)};
}

#if OT_CASH
std::unique_ptr<blind::Purse> Factory::Purse(
    const ServerContext& context,
    const identifier::UnitDefinition& unit,
    const blind::Mint& mint,
    const Amount totalValue,
    const opentxs::PasswordPrompt& reason,
    const proto::CashType type) const
{
    return std::unique_ptr<blind::Purse>(
        opentxs::Factory::Purse(api_, context, type, mint, totalValue, reason));
}

std::unique_ptr<blind::Purse> Factory::Purse(
    const proto::Purse& serialized) const
{
    return std::unique_ptr<blind::Purse>(
        opentxs::Factory::Purse(api_, serialized));
}

std::unique_ptr<blind::Purse> Factory::Purse(
    const identity::Nym& owner,
    const identifier::Server& server,
    const identifier::UnitDefinition& unit,
    const opentxs::PasswordPrompt& reason,
    const proto::CashType type) const
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
    const proto::PeerReply& serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTReplyAcknowledgement
{
    auto output =
        opentxs::Factory::NoticeAcknowledgement(api_, nym, serialized, reason);

    if (output) {
        return OTReplyAcknowledgement{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate peer acknowledgement");
    }
}

std::unique_ptr<OTScriptable> Factory::Scriptable(
    const String& strInput,
    const opentxs::PasswordPrompt& reason) const
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
    if (pItem->LoadContractFromString(strContract, reason)) return pItem;

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
    const proto::UnitDefinition serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTSecurityContract
{
    auto output =
        opentxs::Factory::SecurityContract(api_, nym, serialized, reason);

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

OTServerID Factory::ServerID() const { return identifier::Server::Factory(); }

OTServerID Factory::ServerID(const std::string& serialized) const
{
    return identifier::Server::Factory(serialized);
}

OTServerID Factory::ServerID(const opentxs::String& serialized) const
{
    return identifier::Server::Factory(serialized);
}

std::unique_ptr<OTSignedFile> Factory::SignedFile() const
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_));

    return signedfile;
}

std::unique_ptr<OTSignedFile> Factory::SignedFile(
    const String& LOCAL_SUBDIR,
    const String& FILE_NAME) const
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}
std::unique_ptr<OTSignedFile> Factory::SignedFile(
    const char* LOCAL_SUBDIR,
    const String& FILE_NAME) const
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}

std::unique_ptr<OTSignedFile> Factory::SignedFile(
    const char* LOCAL_SUBDIR,
    const char* FILE_NAME) const
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}

std::unique_ptr<OTSmartContract> Factory::SmartContract() const
{
    std::unique_ptr<OTSmartContract> smartcontract;
    smartcontract.reset(new OTSmartContract(api_));

    return smartcontract;
}

std::unique_ptr<OTSmartContract> Factory::SmartContract(
    const identifier::Server& NOTARY_ID) const
{
    std::unique_ptr<OTSmartContract> smartcontract;
    smartcontract.reset(new OTSmartContract(api_, NOTARY_ID));

    return smartcontract;
}

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
    const proto::PeerRequest& serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTStoreSecret
{
    auto output = opentxs::Factory::StoreSecret(api_, nym, serialized, reason);

    if (output) {
        return OTStoreSecret{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment request");
    }
}

OTSymmetricKey Factory::SymmetricKey() const
{
    return OTSymmetricKey{opentxs::Factory::SymmetricKey()};
}

OTSymmetricKey Factory::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const opentxs::PasswordPrompt& password,
    const proto::SymmetricMode mode) const
{
    return OTSymmetricKey{
        opentxs::Factory::SymmetricKey(api_, engine, password, mode)};
}

OTSymmetricKey Factory::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const proto::SymmetricKey serialized) const
{
    return OTSymmetricKey{
        opentxs::Factory::SymmetricKey(api_, engine, serialized)};
}

OTSymmetricKey Factory::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const OTPassword& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::size_t size,
    const proto::SymmetricKeyType type) const
{
    return OTSymmetricKey{opentxs::Factory::SymmetricKey(
        api_, engine, seed, operations, difficulty, size, type)};
}

OTSymmetricKey Factory::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const OTPassword& raw,
    const opentxs::PasswordPrompt& reason) const
{
    return OTSymmetricKey{
        opentxs::Factory::SymmetricKey(api_, engine, raw, reason)};
}

std::unique_ptr<OTTrade> Factory::Trade() const
{
    std::unique_ptr<OTTrade> trade;
    trade.reset(new OTTrade(api_));

    return trade;
}

std::unique_ptr<OTTrade> Factory::Trade(
    const identifier::Server& notaryID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const opentxs::Identifier& assetAcctId,
    const identifier::Nym& nymID,
    const identifier::UnitDefinition& currencyId,
    const opentxs::Identifier& currencyAcctId) const
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

std::unique_ptr<OTTransactionType> Factory::Transaction(
    const String& strInput,
    const opentxs::PasswordPrompt& reason) const
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
        if (pContract->LoadContractFromString(strContract, reason)) {
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

std::unique_ptr<OTTransaction> Factory::Transaction(
    const opentxs::Ledger& theOwner) const
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(api_, theOwner));

    return transaction;
}

std::unique_ptr<OTTransaction> Factory::Transaction(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAccountID,
    const identifier::Server& theNotaryID,
    originType theOriginType) const
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_, theNymID, theAccountID, theNotaryID, theOriginType));

    return transaction;
}

std::unique_ptr<OTTransaction> Factory::Transaction(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAccountID,
    const identifier::Server& theNotaryID,
    std::int64_t lTransactionNum,
    originType theOriginType) const
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
std::unique_ptr<OTTransaction> Factory::Transaction(
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
    NumList* pNumList) const
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

std::unique_ptr<OTTransaction> Factory::Transaction(
    const opentxs::Ledger& theOwner,
    transactionType theType,
    originType theOriginType /*=originType::not_applicable*/,
    std::int64_t lTransactionNum /*=0*/) const
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

std::unique_ptr<OTTransaction> Factory::Transaction(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAccountID,
    const identifier::Server& theNotaryID,
    transactionType theType,
    originType theOriginType /*=originType::not_applicable*/,
    std::int64_t lTransactionNum /*=0*/) const
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
    const proto::UnitDefinition serialized,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTUnitDefinition
{
    auto output =
        opentxs::Factory::UnitDefinition(api_, nym, serialized, reason);

    if (output) {
        return OTUnitDefinition{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate unit definition");
    }
}

OTUnitID Factory::UnitID() const
{
    return identifier::UnitDefinition::Factory();
}

OTUnitID Factory::UnitID(const std::string& serialized) const
{
    return identifier::UnitDefinition::Factory(serialized);
}

OTUnitID Factory::UnitID(const opentxs::String& serialized) const
{
    return identifier::UnitDefinition::Factory(serialized);
}
}  // namespace opentxs::api::implementation
