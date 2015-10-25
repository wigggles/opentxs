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

#ifndef OPENTXS_CORE_CRYPTO_LIBSECP256K1_HPP
#define OPENTXS_CORE_CRYPTO_LIBSECP256K1_HPP

#include <opentxs/core/crypto/Crypto.hpp>
#include <opentxs/core/crypto/CryptoAsymmetric.hpp>

extern "C" {
#include "secp256k1.h"
#include "secp256k1_ecdh.h"
}

namespace opentxs
{

class OTAsymmetricKey;
class OTData;
class OTPassword;
class OTPasswordData;
class Nym;
class OTSignature;
class CryptoUtil;

class Libsecp256k1 : public Crypto, public CryptoAsymmetric
{
    friend class CryptoEngine;

private:
    Libsecp256k1() = delete;
    secp256k1_context_t* context_ = nullptr;

    CryptoUtil& ssl_;

protected:
    Libsecp256k1(CryptoUtil& ssl);
    virtual void Init_Override() const;
    virtual void Cleanup_Override() const;

public:
    virtual ~Libsecp256k1();

    EXPORT static const int PrivateKeySize = 32;
    bool Seal(mapOfAsymmetricKeys& RecipPubKeys, const String& theInput,
                      OTData& dataOutput) const;
    bool Open(OTData& dataInput, const Nym& theRecipient,
                      String& theOutput,
                      const OTPasswordData* pPWData = nullptr) const;
    bool SignContract(
        const String& strContractUnsigned,
        const OTAsymmetricKey& theKey,
        OTSignature& theSignature, // output
        const CryptoHash::HashType hashType,
        const OTPasswordData* pPWData = nullptr);
    bool VerifySignature(
        const String& strContractToVerify,
        const OTAsymmetricKey& theKey,
        const OTSignature& theSignature,
        const CryptoHash::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const;

    bool OTSignatureToECDSASignature(
        const OTSignature& inSignature,
        secp256k1_ecdsa_signature_t& outSignature) const;
    bool ECDSASignatureToOTSignature(
        const secp256k1_ecdsa_signature_t& inSignature,
        OTSignature& outSignature) const;
    bool AsymmetricKeyToECDSAPubkey(
        const OTAsymmetricKey& asymmetricKey,
        secp256k1_pubkey_t& pubkey) const;
    bool ECDSAPubkeyToAsymmetricKey(
        const secp256k1_pubkey_t& pubkey,
        OTAsymmetricKey& asymmetricKey) const;
    bool AsymmetricKeyToECDSAPrivkey(
        const OTAsymmetricKey& asymmetricKey,
        OTPassword& privkey) const;
    bool ECDSAPrivkeyToAsymmetricKey(
        const OTPassword& privkey,
        OTAsymmetricKey& asymmetricKey) const;

    bool ECDH(
        const OTAsymmetricKey& publicKey,
        const OTAsymmetricKey& privateKey,
             OTPassword& secret) const;

    bool secp256k1_privkey_tweak_add(
        uint8_t key [PrivateKeySize],
        const uint8_t tweak [PrivateKeySize]) const;
    bool secp256k1_pubkey_create(
        secp256k1_pubkey_t& pubkey,
        const OTPassword& privkey) const;
    bool secp256k1_pubkey_serialize(
        OTData& serializedPubkey,
        const secp256k1_pubkey_t& pubkey) const;
    bool secp256k1_pubkey_parse(
        secp256k1_pubkey_t& pubkey,
        const OTPassword& serializedPubkey) const;
    String Nonce(uint32_t size) const;
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTCRYPTO_HPP
