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

#include <opentxs/core/crypto/PaymentCode.hpp>

#include <opentxs/core/Log.hpp>
#include <opentxs/core/Proto.hpp>
#include <opentxs/core/crypto/AsymmetricKeySecp256k1.hpp>
#include <opentxs/core/crypto/CryptoEngine.hpp>
#include <opentxs/core/crypto/MasterCredential.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>

namespace opentxs
{

PaymentCode::PaymentCode(proto::PaymentCode& paycode)
    : version_(paycode.version())
    , chain_code_(paycode.chaincode().c_str(), paycode.chaincode().size())
    , hasBitmessage_(paycode.has_bitmessage())
{
    OT_ASSERT(paycode.has_key());

    OTData key(paycode.key().c_str(), paycode.key().size());
    OTData chaincode(paycode.chaincode().c_str(), paycode.chaincode().size());
    ConstructKey(key, chaincode);

    if (paycode.has_bitmessageversion()) {
        bitmessage_version_ = paycode.bitmessageversion();
    }
    if (paycode.has_bitmessagestream()) {
        bitmessage_stream_ = paycode.bitmessagestream();
    }
}

PaymentCode::PaymentCode(
    const uint32_t nym,
    const bool bitmessage,
    const uint8_t bitmessageVersion,
    const uint8_t bitmessageStream)
        : hasBitmessage_(bitmessage)
        , bitmessage_version_(bitmessageVersion)
        , bitmessage_stream_(bitmessageStream)
{
    proto::HDPath path;
    path.add_child(PC_PURPOSE | HARDENED);
    path.add_child(BITCOIN_TYPE | HARDENED);
    path.add_child(nym | HARDENED);

    serializedAsymmetricKey privatekey =
        CryptoEngine::Instance().BIP32().GetHDKey(path);

    chain_code_.Assign(
        privatekey->chaincode().c_str(),
        privatekey->chaincode().size());

    serializedAsymmetricKey key =
        CryptoEngine::Instance().BIP32().PrivateToPublic(*privatekey);

    OTData pubkey(key->key().c_str(), key->key().size());

    ConstructKey(pubkey, chain_code_);
}

const OTData PaymentCode::Pubkey() const
{
    OTData pubkey;
    pubkey.SetSize(33);

    if (pubkey_) {
        std::dynamic_pointer_cast<AsymmetricKeySecp256k1>(pubkey_)->GetKey(pubkey);
    }

    OT_ASSERT(33 == pubkey.GetSize());

    return pubkey;
}

const std::string PaymentCode::asBase58() const
{
    OTData pubkey = Pubkey();

    uint8_t serialized[81]{};

    serialized[0] = PaymentCode::BIP47_VERSION_BYTE;
    serialized[1] = version_;
    serialized[2] = hasBitmessage_ ? 0x80 : 0;
    OTPassword::safe_memcpy(
        &serialized[3],
        33,
        pubkey.GetPointer(),
        pubkey.GetSize(),
        false);
    OTPassword::safe_memcpy(
        &serialized[36],
        32,
        chain_code_.GetPointer(),
        chain_code_.GetSize(),
        false);
    serialized[68] = bitmessage_version_;
    serialized[69] = bitmessage_stream_;

    OTData binaryVersion(serialized, sizeof(serialized));

    return CryptoEngine::Instance().Util().Base58CheckEncode(binaryVersion).Get();
}

const SerializedPaymentCode PaymentCode::Serialize() const
{
    SerializedPaymentCode serialized = std::make_shared<proto::PaymentCode>();

    serialized->set_version(version_);

    if (pubkey_) {
        OTData pubkey = Pubkey();
        serialized->set_key(pubkey.GetPointer(), pubkey.GetSize());
    }

    serialized->set_chaincode(chain_code_.GetPointer(), chain_code_.GetSize());
    serialized->set_bitmessageversion(bitmessage_version_);
    serialized->set_bitmessagestream(bitmessage_stream_);

    return serialized;
}

Identifier PaymentCode::ID() const
{

    uint8_t core[65]{};

    OTData pubkey = Pubkey();
    OTPassword::safe_memcpy(
        &core[0],
        33,
        pubkey.GetPointer(),
        pubkey.GetSize(),
        false);
    OTPassword::safe_memcpy(
        &core[33],
        32,
        chain_code_.GetPointer(),
        chain_code_.GetSize(),
        false);

    OTData dataVersion(core, sizeof(core));

    Identifier paymentCodeID;

    paymentCodeID.CalculateDigest(dataVersion);

    return paymentCodeID;
}

bool PaymentCode::Verify(const MasterCredential& credential) const
{
    serializedCredential serializedMaster =
        credential.Serialize(
            Credential::AS_PUBLIC,
            Credential::WITH_SIGNATURES);

    if (!proto::Verify(*serializedMaster, proto::CREDROLE_MASTERKEY, true)) {
        otErr << __FUNCTION__ << ": Invalid master credential syntax.\n";
        return false;
    }

    serializedSignature sourceSig = credential.GetSourceSignature();

    if (!sourceSig) {
        otErr << __FUNCTION__ << ": Credential not signed by a source.\n";
        return false;
    }

    if (!pubkey_) {
        otErr << __FUNCTION__ << ": Payment code is missing public key.\n";
        return false;
    }

    return pubkey_->Verify(
        proto::ProtoAsData<proto::Credential>(*serializedMaster),
        *sourceSig);
}

void PaymentCode::ConstructKey(const OTData& pubkey, const OTData& chaincode)
{
    proto::AsymmetricKey newKey;

    newKey.set_version(1);
    newKey.set_type(proto::AKEYTYPE_SECP256K1);
    newKey.set_mode(proto::KEYMODE_PUBLIC);
    newKey.set_role(proto::KEYROLE_SIGN);
    newKey.set_key(pubkey.GetPointer(), pubkey.GetSize());
    newKey.set_chaincode(chaincode.GetPointer(), chaincode.GetSize());

    OTAsymmetricKey* key = OTAsymmetricKey::KeyFactory(newKey);

    pubkey_.reset(key);
}

} // namespace opentxs
