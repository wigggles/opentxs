// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/crypto/key/Secp256k1.hpp"
#endif
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/identity/credential/Primary.hpp"
#include "opentxs/Proto.tpp"
#include "opentxs/Types.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>

#include "PaymentCode.hpp"

template class opentxs::Pimpl<opentxs::PaymentCode>;

#define PREFIX_OFFSET 0
#define PREFIX_BYTES 1
#define VERSION_OFFSET PREFIX_OFFSET + PREFIX_BYTES
#define VERSION_BYTES 1
#define FEATURE_OFFSET VERSION_OFFSET + VERSION_BYTES
#define FEATURE_BYTES 1
#define PUBLIC_KEY_OFFSET FEATURE_OFFSET + FEATURE_BYTES
#define PUBLIC_KEY_BYTES 33
#define CHAIN_CODE_OFFSET PUBLIC_KEY_OFFSET + PUBLIC_KEY_BYTES
#define CHAIN_CODE_BYTES 32
#define CUSTOM_OFFSET CHAIN_CODE_OFFSET + CHAIN_CODE_BYTES
#define CUSTOM_BYTES 13
#define SERIALIZED_BYTES CUSTOM_OFFSET + CUSTOM_BYTES

#define BITMESSAGE_VERSION_OFFSET CUSTOM_OFFSET
#define BITMESSAGE_VERSION_SIZE 1
#define BITMESSAGE_STREAM_OFFSET                                               \
    BITMESSAGE_VERSION_OFFSET + BITMESSAGE_VERSION_SIZE
#define BITMESSAGE_STREAM_SIZE 1

#define XPUB_KEY_OFFSET 0
#define XPUB_CHAIN_CODE_OFFSET XPUB_KEY_OFFSET + PUBLIC_KEY_BYTES
#define XPUB_BYTES XPUB_CHAIN_CODE_OFFSET + CHAIN_CODE_BYTES

#define OT_METHOD "opentxs::implementation::PaymentCode::"

namespace opentxs
{
opentxs::PaymentCode* Factory::PaymentCode(
    const api::Core& api,
    const std::string& base58,
    const opentxs::PasswordPrompt& reason)
{
    return new implementation::PaymentCode(api, reason, base58);
}

opentxs::PaymentCode* Factory::PaymentCode(
    const api::Core& api,
    const proto::PaymentCode& serialized,
    const opentxs::PasswordPrompt& reason)
{
    return new implementation::PaymentCode(api, reason, serialized);
}

opentxs::PaymentCode* Factory::PaymentCode(
    const api::Core& api,
    const std::string& seed,
    const Bip32Index nym,
    const std::uint8_t version,
    const opentxs::PasswordPrompt& reason,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream)
{
    return new implementation::PaymentCode(
        api,
        reason,
        seed,
        nym,
        version,
        bitmessage,
        bitmessageVersion,
        bitmessageStream);
}
}  // namespace opentxs

namespace opentxs::implementation
{
PaymentCode::PaymentCode(
    const api::Core& api,
    const PasswordPrompt& reason,
    const std::string& base58)
    : api_(api)
    , version_(0)
    , seed_("")
    , index_(-1)
    , asymmetric_key_{crypto::key::Asymmetric::Factory()}
    , pubkey_(nullptr)
    , chain_code_(new OTPassword)
    , hasBitmessage_(false)
    , bitmessage_version_(0)
    , bitmessage_stream_(0)
{
    std::string rawCode = api_.Crypto().Encode().IdentifierDecode(base58);

    if (SERIALIZED_BYTES == rawCode.size()) {
        version_ = rawCode[VERSION_OFFSET];
        const std::uint8_t features = rawCode[FEATURE_OFFSET];

        if (features & 0x80) { hasBitmessage_ = true; }

        auto key = Data::Factory(&rawCode[PUBLIC_KEY_OFFSET], PUBLIC_KEY_BYTES);

        OT_ASSERT(chain_code_);

        chain_code_->setMemory(&rawCode[CHAIN_CODE_OFFSET], CHAIN_CODE_BYTES);

        ConstructKey(key, reason);

        if (hasBitmessage_) {
            bitmessage_version_ = rawCode[BITMESSAGE_VERSION_OFFSET];
            bitmessage_stream_ = rawCode[BITMESSAGE_STREAM_SIZE];
        }
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Can not construct payment code.")(
            " Required size: ")(SERIALIZED_BYTES)(" Actual size: ")(
            rawCode.size())
            .Flush();
        chain_code_.reset();
    }
}

PaymentCode::PaymentCode(
    const api::Core& api,
    const PasswordPrompt& reason,
    const proto::PaymentCode& paycode)
    : api_(api)
    , version_(paycode.version())
    , seed_("")
    , index_(-1)
    , asymmetric_key_{crypto::key::Asymmetric::Factory()}
    , pubkey_(nullptr)
    , chain_code_(new OTPassword)
    , hasBitmessage_(paycode.has_bitmessage())
    , bitmessage_version_(0)
    , bitmessage_stream_(0)
{
    OT_ASSERT(chain_code_);

    chain_code_->setMemory(
        paycode.chaincode().c_str(), paycode.chaincode().size());

    auto key = Data::Factory(paycode.key().c_str(), paycode.key().size());
    ConstructKey(key, reason);

    if (paycode.has_bitmessageversion()) {
        bitmessage_version_ = paycode.bitmessageversion();
    }

    if (paycode.has_bitmessagestream()) {
        bitmessage_stream_ = paycode.bitmessagestream();
    }
}

PaymentCode::PaymentCode(
    const api::Core& api,
    const PasswordPrompt& reason,
    const std::string& seed,
    const Bip32Index nym,
    const std::uint8_t version,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream)
    : api_(api)
    , version_(version)
    , seed_(seed)
    , index_(nym)
    , asymmetric_key_{crypto::key::Asymmetric::Factory()}
    , pubkey_(nullptr)
    , chain_code_(nullptr)
    , hasBitmessage_(bitmessage)
    , bitmessage_version_(bitmessageVersion)
    , bitmessage_stream_(bitmessageStream)
{
    auto [success, chainCode, publicKey] =
        make_key(api_, seed_, index_, reason);

    if (success) {
        chain_code_.swap(chainCode);
        ConstructKey(publicKey, reason);
    }
}

PaymentCode::PaymentCode(const PaymentCode& rhs)
    : opentxs::PaymentCode()
    , api_(rhs.api_)
    , version_(rhs.version_)
    , seed_(rhs.seed_)
    , index_(rhs.index_)
    , asymmetric_key_{crypto::key::Asymmetric::Factory()}
    , pubkey_(rhs.pubkey_)
    , chain_code_(nullptr)
    , hasBitmessage_(rhs.hasBitmessage_)
    , bitmessage_version_(rhs.bitmessage_version_)
    , bitmessage_stream_(rhs.bitmessage_stream_)
{
    if (rhs.asymmetric_key_.get()) {
        asymmetric_key_ = rhs.asymmetric_key_;
        pubkey_ = dynamic_cast<crypto::key::Secp256k1*>(&asymmetric_key_.get());

        OT_ASSERT(nullptr != pubkey_)
    }

    if (rhs.chain_code_) {
        chain_code_.reset(new OTPassword(*rhs.chain_code_));
    }
}

bool PaymentCode::operator==(const proto::PaymentCode& rhs) const
{
    SerializedPaymentCode tempPaycode = Serialize();

    auto LHData = api_.Factory().Data(*tempPaycode);
    auto RHData = api_.Factory().Data(rhs);

    return (LHData == RHData);
}

PaymentCode::operator const crypto::key::Asymmetric&() const
{
    return asymmetric_key_;
}

bool PaymentCode::AddPrivateKeys(
    const std::string& seed,
    const Bip32Index index,
    const PasswordPrompt& reason)
{
    if (false == seed_.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Seed already set.").Flush();

        return false;
    }

    if (0 <= index_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Index already set.").Flush();

        return false;
    }

    const PaymentCode candidate(
        api_,
        reason,
        seed,
        index,
        version_,
        hasBitmessage_,
        bitmessage_version_,
        bitmessage_stream_);

    if (this->ID() != candidate.ID()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong parameters.").Flush();

        return false;
    }

    seed_ = candidate.seed_;
    index_ = candidate.index_;

    return true;
}

const std::string PaymentCode::asBase58() const
{
    if (chain_code_) {
        auto pubkey = Pubkey();
        std::array<std::uint8_t, SERIALIZED_BYTES> serialized{};
        serialized[PREFIX_OFFSET] = PaymentCode::BIP47_VERSION_BYTE;
        serialized[VERSION_OFFSET] = version_;
        serialized[FEATURE_OFFSET] = hasBitmessage_ ? 0x80 : 0;
        OTPassword::safe_memcpy(
            &serialized[PUBLIC_KEY_OFFSET],
            PUBLIC_KEY_BYTES,
            pubkey->data(),
            pubkey->size(),
            false);
        OTPassword::safe_memcpy(
            &serialized[CHAIN_CODE_OFFSET],
            CHAIN_CODE_BYTES,
            chain_code_->getMemory(),
            chain_code_->getMemorySize(),
            false);
        serialized[BITMESSAGE_VERSION_OFFSET] = bitmessage_version_;
        serialized[BITMESSAGE_STREAM_OFFSET] = bitmessage_stream_;
        auto binaryVersion =
            Data::Factory(serialized.data(), serialized.size());

        return api_.Crypto().Encode().IdentifierEncode(binaryVersion);
    } else {

        return {};
    }
}

PaymentCode* PaymentCode::clone() const { return new PaymentCode(*this); }

void PaymentCode::ConstructKey(
    const opentxs::Data& pubkey,
    const PasswordPrompt& reason)
{
    proto::AsymmetricKey newKey;
    newKey.set_version(1);
    newKey.set_type(proto::AKEYTYPE_SECP256K1);
    newKey.set_mode(proto::KEYMODE_PUBLIC);
    newKey.set_role(proto::KEYROLE_SIGN);
    newKey.set_key(pubkey.data(), pubkey.size());
    asymmetric_key_ = api_.Factory().AsymmetricKey(newKey, reason);

    if (asymmetric_key_.get()) {
        pubkey_ = dynamic_cast<crypto::key::Secp256k1*>(&asymmetric_key_.get());

        OT_ASSERT(nullptr != pubkey_)
    } else {
        pubkey_ = nullptr;
    }
}

const OTNymID PaymentCode::ID() const
{
    std::uint8_t core[XPUB_BYTES]{};

    auto pubkey = Pubkey();
    OTPassword::safe_memcpy(
        &core[XPUB_KEY_OFFSET],
        PUBLIC_KEY_BYTES,
        pubkey->data(),
        pubkey->size(),
        false);

    if (chain_code_) {
        if (chain_code_->getMemorySize() == CHAIN_CODE_BYTES) {
            OTPassword::safe_memcpy(
                &core[XPUB_CHAIN_CODE_OFFSET],
                CHAIN_CODE_BYTES,
                chain_code_->getMemory(),
                chain_code_->getMemorySize(),
                false);
        }
    }

    auto dataVersion = Data::Factory(core, sizeof(core));
    auto paymentCodeID = identifier::Nym::Factory();
    paymentCodeID->CalculateDigest(dataVersion);

    return paymentCodeID;
}

std::tuple<bool, std::unique_ptr<OTPassword>, OTData> PaymentCode::make_key(
    const api::Core& api,
    const std::string& seed,
    const Bip32Index index,
    const PasswordPrompt& reason)
{
    std::tuple<bool, std::unique_ptr<OTPassword>, OTData> output{
        false, new OTPassword, Data::Factory()};
    auto& [success, chainCode, publicKey] = output;
    auto fingerprint{seed};
    auto privatekey = api.Seeds().GetPaymentCode(fingerprint, index, reason);

    OT_ASSERT(seed == fingerprint)

    if (privatekey) {
        OT_ASSERT(chainCode)

        OTPassword privkey{};
        auto symmetricKey = api.Symmetric().Key(
            privatekey->encryptedkey().key(),
            privatekey->encryptedkey().mode());
        symmetricKey->Decrypt(privatekey->chaincode(), reason, *chainCode);
        proto::AsymmetricKey key{};
        bool haveKey{false};
        haveKey = api.Crypto().SECP256K1().PrivateToPublic(
            api, *privatekey, key, reason);

        if (haveKey) {
            publicKey = Data::Factory(key.key().c_str(), key.key().size());
        }
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to generate private key.")
            .Flush();
    }

    success = (CHAIN_CODE_BYTES == chainCode->getMemorySize()) &&
              (PUBLIC_KEY_BYTES == publicKey->size());

    return output;
}

const OTData PaymentCode::Pubkey() const
{
    auto pubkey = Data::Factory();
    pubkey->SetSize(PUBLIC_KEY_BYTES);

    if (nullptr != pubkey_) { pubkey_->GetKey(pubkey); }

    OT_ASSERT(PUBLIC_KEY_BYTES == pubkey->size());

    return pubkey;
}

SerializedPaymentCode PaymentCode::Serialize() const
{
    SerializedPaymentCode serialized = std::make_shared<proto::PaymentCode>();
    serialized->set_version(version_);

    if (nullptr != pubkey_) {
        auto pubkey = Pubkey();
        serialized->set_key(pubkey->data(), pubkey->size());
    }

    if (chain_code_) {
        serialized->set_chaincode(
            chain_code_->getMemory(), chain_code_->getMemorySize());
    }

    serialized->set_bitmessageversion(bitmessage_version_);
    serialized->set_bitmessagestream(bitmessage_stream_);

    return serialized;
}

bool PaymentCode::Sign(
    const identity::credential::Base& credential,
    proto::Signature& sig,
    const PasswordPrompt& reason) const
{
    const auto signingKey = signing_key(reason);

    if (false == bool(signingKey.get())) { return false; }

    auto serialized = credential.Serialized(AS_PUBLIC, WITHOUT_SIGNATURES);
    auto& signature = *serialized->add_signature();
    const bool goodSig = signingKey->Sign(
        [&]() -> std::string { return proto::ToString(*serialized); },
        proto::SIGROLE_NYMIDSOURCE,
        signature,
        ID(),
        reason,
        proto::KEYROLE_SIGN);
    sig.CopyFrom(signature);

    return goodSig;
}

bool PaymentCode::Sign(
    const Data& data,
    Data& output,
    const PasswordPrompt& reason) const
{
    const auto signingKey = signing_key(reason);

    if (false == bool(signingKey.get())) { return false; }

    auto success = signingKey->engine().Sign(
        api_, data, signingKey.get(), proto::HASHTYPE_SHA256, output, reason);

    return success;
}

OTAsymmetricKey PaymentCode::signing_key(const PasswordPrompt& reason) const
{
    if (nullptr == pubkey_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Payment code not instantiated.")
            .Flush();

        return crypto::key::Asymmetric::Factory();
    }

    if (0 > index_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Private key is unavailable (unknown index).")
            .Flush();

        return crypto::key::Asymmetric::Factory();
    }

    if (seed_.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Private key is unavailable (unknown seed).")
            .Flush();

        return crypto::key::Asymmetric::Factory();
    }

    std::string fingerprint = seed_;
    std::shared_ptr<proto::AsymmetricKey> privatekey =
        api_.Seeds().GetPaymentCode(fingerprint, index_, reason);

    if (fingerprint != seed_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Specified seed could not be loaded.")
            .Flush();

        return crypto::key::Asymmetric::Factory();
    }

    if (!privatekey) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to derive private key for payment code.")
            .Flush();

        return crypto::key::Asymmetric::Factory();
    }

    auto existingKeyData = Data::Factory();
    auto compareKeyData = Data::Factory();
    proto::AsymmetricKey compareKey;
    const bool haveKey = api_.Crypto().SECP256K1().PrivateToPublic(
        api_, *privatekey, compareKey, reason);

    if (!haveKey) { return crypto::key::Asymmetric::Factory(); }

    compareKey.clear_path();
    pubkey_->GetKey(existingKeyData);
    compareKeyData->Assign(compareKey.key().c_str(), compareKey.key().size());

    if (!(existingKeyData == compareKeyData)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Private key is not valid for this payment code.")
            .Flush();

        return crypto::key::Asymmetric::Factory();
    }

    const auto signingKey = api_.Factory().AsymmetricKey(*privatekey, reason);

    return signingKey;
}

bool PaymentCode::Verify(
    const proto::Credential& master,
    const proto::Signature& sourceSignature,
    const PasswordPrompt& reason) const
{
    if (!proto::Validate<proto::Credential>(
            master,
            VERBOSE,
            proto::KEYMODE_PUBLIC,
            proto::CREDROLE_MASTERKEY,
            false)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Invalid master credential syntax.")
            .Flush();

        return false;
    }

    bool sameSource = (*this == master.masterdata().source().paymentcode());

    if (!sameSource) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Master credential was not derived from this source.")
            .Flush();

        return false;
    }

    if (nullptr == pubkey_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Payment code is missing public key.")
            .Flush();

        return false;
    }

    proto::Credential copy;
    copy.CopyFrom(master);
    auto& signature = *copy.add_signature();
    signature.CopyFrom(sourceSignature);
    signature.clear_signature();

    return pubkey_->Verify(api_.Factory().Data(copy), sourceSignature, reason);
}

bool PaymentCode::VerifyInternally() const
{
    return (proto::Validate<proto::PaymentCode>(*Serialize(), SILENT));
}
}  // namespace opentxs::implementation
#endif
