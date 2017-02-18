/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/core/crypto/PaymentCode.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/Bip32.hpp"
#include "opentxs/core/crypto/AsymmetricKeyEC.hpp"
#include "opentxs/core/crypto/AsymmetricKeySecp256k1.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/CryptoEncodingEngine.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoSymmetricEngine.hpp"
#include "opentxs/core/crypto/Libsecp256k1.hpp"
#include "opentxs/core/crypto/MasterCredential.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/SymmetricKey.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTData.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"

#include <array>
#include <cstdint>
#include <stdint.h>
#include <memory>
#include <ostream>
#include <string>

namespace opentxs
{

PaymentCode::PaymentCode(const std::string& base58)
    : chain_code_(new OTPassword)
{
    std::string rawCode = OT::App().Crypto().Encode().IdentifierDecode(base58);

    if (81 == rawCode.size()) {
        version_ = rawCode[1];
        const uint8_t features = rawCode[2];

        if (features & 0x80) {
            hasBitmessage_ = true;
        }

        OTData key(&rawCode[3], 33);

        OT_ASSERT(chain_code_);

        chain_code_->setMemory(&rawCode[36], 32);

        ConstructKey(key);

        if (hasBitmessage_) {
            bitmessage_version_ = rawCode[68];
            bitmessage_stream_ = rawCode[69];
        }
    } else {
        otErr << "Can not construct payment code." << std::endl
              << "Required size: 81" << std::endl
              << "Actual size: " <<  rawCode.size() << std::endl;
    }
}

PaymentCode::PaymentCode(const proto::PaymentCode& paycode)
    : version_(paycode.version())
    , chain_code_(new OTPassword)
    , hasBitmessage_(paycode.has_bitmessage())
{
    OT_ASSERT(chain_code_);

    chain_code_->setMemory(
        paycode.chaincode().c_str(), paycode.chaincode().size());

    OTData key(paycode.key().c_str(), paycode.key().size());
    ConstructKey(key);

    if (paycode.has_bitmessageversion()) {
        bitmessage_version_ = paycode.bitmessageversion();
    }
    if (paycode.has_bitmessagestream()) {
        bitmessage_stream_ = paycode.bitmessagestream();
    }
}

PaymentCode::PaymentCode(
    const std::string& seed,
    const uint32_t nym,
    const bool bitmessage,
    const uint8_t bitmessageVersion,
    const uint8_t bitmessageStream)
        : seed_(seed)
        , index_(nym)
        , hasBitmessage_(bitmessage)
        , bitmessage_version_(bitmessageVersion)
        , bitmessage_stream_(bitmessageStream)
{
    serializedAsymmetricKey privatekey =
        OT::App().Crypto().BIP32().GetPaymentCode(seed_, index_);

    if (privatekey) {
        OTPassword privkey;
        chain_code_.reset(new OTPassword);

        OT_ASSERT(chain_code_);

        auto symmetricKey = OT::App().Crypto().Symmetric().Key(
            privatekey->encryptedkey().key(),
            privatekey->encryptedkey().mode());
        OTPasswordData password(__FUNCTION__);
        symmetricKey->Decrypt(
            privatekey->chaincode(),
            password,
            *chain_code_);

        proto::AsymmetricKey key;
#if OT_CRYPTO_USING_LIBSECP256K1
        const bool haveKey =
            static_cast<Libsecp256k1&>(
                OT::App().Crypto().SECP256K1()).PrivateToPublic(
                    *privatekey,
                    key);
#endif

        if (haveKey) {
            OTData pubkey(key.key().c_str(), key.key().size());
            ConstructKey(pubkey);
        }
    }
}

bool PaymentCode::operator==(const proto::PaymentCode& rhs) const
{
    SerializedPaymentCode tempPaycode = Serialize();

    OTData LHData = proto::ProtoAsData<proto::PaymentCode>(*tempPaycode);
    OTData RHData = proto::ProtoAsData<proto::PaymentCode>(rhs);

    return (LHData == RHData);
}

const OTData PaymentCode::Pubkey() const
{
    OTData pubkey;
    pubkey.SetSize(33);

    if (pubkey_) {
#if OT_CRYPTO_USING_LIBSECP256K1
        std::dynamic_pointer_cast<AsymmetricKeySecp256k1>(pubkey_)->GetKey(
            pubkey);
#endif
    }

    OT_ASSERT(33 == pubkey.GetSize());

    return pubkey;
}

const std::string PaymentCode::asBase58() const
{
    OT_ASSERT(chain_code_);

    OTData pubkey = Pubkey();
    std::array<std::uint8_t, 81> serialized{};
    serialized[0] = PaymentCode::BIP47_VERSION_BYTE;
    serialized[1] = version_;
    serialized[2] = hasBitmessage_ ? 0x80 : 0;
    OTPassword::safe_memcpy(
        &serialized[3], 33, pubkey.GetPointer(), pubkey.GetSize(), false);
    OTPassword::safe_memcpy(
        &serialized[36],
        32,
        chain_code_->getMemory(),
        chain_code_->getMemorySize(),
        false);
    serialized[68] = bitmessage_version_;
    serialized[69] = bitmessage_stream_;
    OTData binaryVersion(serialized.data(), serialized.size());

    return OT::App().Crypto().Encode().IdentifierEncode(binaryVersion);
}

SerializedPaymentCode PaymentCode::Serialize() const
{
    SerializedPaymentCode serialized = std::make_shared<proto::PaymentCode>();
    serialized->set_version(version_);

    if (pubkey_) {
        OTData pubkey = Pubkey();
        serialized->set_key(pubkey.GetPointer(), pubkey.GetSize());
    }

    if (chain_code_) {
        serialized->set_chaincode(
            chain_code_->getMemory(), chain_code_->getMemorySize());
    }
    serialized->set_bitmessageversion(bitmessage_version_);
    serialized->set_bitmessagestream(bitmessage_stream_);

    return serialized;
}

const Identifier PaymentCode::ID() const
{

    uint8_t core[65]{};

    OTData pubkey = Pubkey();
    OTPassword::safe_memcpy(
        &core[0], 33, pubkey.GetPointer(), pubkey.GetSize(), false);
    if (chain_code_) {
        if (chain_code_->getMemorySize() == 32) {
            OTPassword::safe_memcpy(
                &core[33],
                32,
                chain_code_->getMemory(),
                chain_code_->getMemorySize(),
                false);
        }
    }

    OTData dataVersion(core, sizeof(core));

    Identifier paymentCodeID;

    paymentCodeID.CalculateDigest(dataVersion);

    return paymentCodeID;
}

bool PaymentCode::Verify(const MasterCredential& credential) const
{
    serializedCredential serializedMaster = credential.asSerialized(
        AS_PUBLIC, WITHOUT_SIGNATURES);

    if (!proto::Check<proto::Credential>(
            *serializedMaster,
            0,
            0xFFFFFFFF,
            proto::KEYMODE_PUBLIC,
            proto::CREDROLE_MASTERKEY,
            false)) {
        otErr << __FUNCTION__ << ": Invalid master credential syntax.\n";
        return false;
    }

    bool sameSource =
        (*this == serializedMaster->masterdata().source().paymentcode());

    if (!sameSource) {
        otErr << __FUNCTION__ << ": Master credential was not derived from"
              << " this source->\n";
        return false;
    }

    SerializedSignature sourceSig = credential.SourceSignature();

    if (!sourceSig) {
        otErr << __FUNCTION__ << ": Master credential not signed by its"
              << " source.\n";
        return false;
    }

    if (!pubkey_) {
        otErr << __FUNCTION__ << ": Payment code is missing public key.\n";
        return false;
    }

    auto& signature = *serializedMaster->add_signature();
    signature.CopyFrom(*sourceSig);
    signature.clear_signature();

    return pubkey_->Verify(
        proto::ProtoAsData<proto::Credential>(*serializedMaster), *sourceSig);
}

bool PaymentCode::Sign(
    const Credential& credential,
    proto::Signature& sig,
    const OTPasswordData* pPWData) const
{
    if (!pubkey_) {
        otErr << __FUNCTION__ << ": Payment code not instantiated.\n";
        return false;
    }

    std::string fingerprint = seed_;
    serializedAsymmetricKey privatekey =
        OT::App().Crypto().BIP32().GetPaymentCode(fingerprint, index_);

    if (fingerprint != seed_) {
        otErr << __FUNCTION__ << ": Specified seed could not be loaded."
              << std::endl;

        return false;
    }

    if (!privatekey) {
        otErr << __FUNCTION__ << ": Failed to derive private key for"
              << " payment code.\n";

        return false;
    }

    OTData existingKeyData, compareKeyData;
    proto::AsymmetricKey compareKey;
#if OT_CRYPTO_USING_LIBSECP256K1
    const bool haveKey =
        static_cast<Libsecp256k1&>(
            OT::App().Crypto().SECP256K1()).PrivateToPublic(
                *privatekey,
                compareKey);
#endif

    if (!haveKey) { return false; }

    compareKey.clear_path();
    pubkey_->GetKey(existingKeyData);
    compareKeyData.Assign(compareKey.key().c_str(), compareKey.key().size());

    if (!(existingKeyData == compareKeyData)) {
        otErr << __FUNCTION__ << ": Private key is not valid for this"
              << " payment code.\n";
        return false;
    }
    std::unique_ptr<OTAsymmetricKey> signingKey(
        OTAsymmetricKey::KeyFactory(*privatekey));

    serializedCredential serialized = credential.asSerialized(
        AS_PUBLIC, WITHOUT_SIGNATURES);
    auto& signature = *serialized->add_signature();
    signature.set_role(proto::SIGROLE_NYMIDSOURCE);

    bool goodSig = signingKey->SignProto(
        *serialized,
        signature,
        String(ID()),
        pPWData);

    sig.CopyFrom(signature);

    return goodSig;
}

void PaymentCode::ConstructKey(
    const OTData& pubkey)
{
    proto::AsymmetricKey newKey;
    newKey.set_version(1);
    newKey.set_type(proto::AKEYTYPE_SECP256K1);
    newKey.set_mode(proto::KEYMODE_PUBLIC);
    newKey.set_role(proto::KEYROLE_SIGN);
    newKey.set_key(pubkey.GetPointer(), pubkey.GetSize());
    AsymmetricKeyEC* key = dynamic_cast<AsymmetricKeySecp256k1*>
        (OTAsymmetricKey::KeyFactory(newKey));

    if (nullptr !=  key) {
        pubkey_.reset(key);
    }
}

bool PaymentCode::VerifyInternally() const
{
    return (proto::Check<proto::PaymentCode>(*Serialize(), version_, version_));
}
}  // namespace opentxs
#endif
