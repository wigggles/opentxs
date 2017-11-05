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

#ifndef OPENTXS_CORE_CRYPTO_CRYPTOENGINE_HPP
#define OPENTXS_CORE_CRYPTO_CRYPTOENGINE_HPP

#include "opentxs/Version.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/Proto.hpp"

#include <map>
#include <memory>
#include <mutex>

namespace opentxs
{

class OT;
class Bip32;
class Bip39;
class CryptoAsymmetric;
class CryptoEncodingEngine;
class CryptoHashEngine;
class CryptoSymmetric;
class CryptoSymmetricEngine;
class CryptoUtil;
class Libsecp256k1;
class Libsodium;
class OpenSSL;
class OTASCIIArmor;
class OTCachedKey;
class TrezorCrypto;
class SymmetricKey;

// Choose your OpenSSL-compatible library here.
#if OT_CRYPTO_USING_OPENSSL
typedef OpenSSL SSLImplementation;
#endif

#if OT_CRYPTO_USING_LIBSECP256K1
typedef Libsecp256k1 secp256k1;
#endif

#if OT_CRYPTO_USING_TREZOR
typedef TrezorCrypto bitcoincrypto;
#endif

typedef Libsodium Curve25519;

// Singlton class for providing an interface to external crypto libraries
// and hold the state required by those libraries.
class CryptoEngine
{
    friend class OT;
    friend class CryptoEncodingEngine;
    friend class CryptoHashEngine;
    friend class CryptoSymmetricEngine;

private:
    OT& ot_;
    mutable std::mutex cached_key_lock_;
    mutable std::unique_ptr<OTCachedKey> primary_key_;
    mutable std::map<Identifier, std::unique_ptr<OTCachedKey>> cached_keys_;
#if OT_CRYPTO_USING_TREZOR
    std::unique_ptr<bitcoincrypto> bitcoincrypto_;
#endif
    std::unique_ptr<Curve25519> ed25519_;
    std::unique_ptr<SSLImplementation> ssl_;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    std::unique_ptr<secp256k1> secp256k1_;
#endif
    std::unique_ptr<CryptoEncodingEngine> encode_;
    std::unique_ptr<CryptoHashEngine> hash_;
    std::unique_ptr<CryptoSymmetricEngine> symmetric_;

    void init_default_key(const Lock& lock) const;

    void Init();
    void Cleanup();

    CryptoEngine(OT& ot);
    CryptoEngine() = delete;
    CryptoEngine(const CryptoEngine&) = delete;
    CryptoEngine(CryptoEngine&&) = delete;
    CryptoEngine& operator=(const CryptoEngine&) = delete;
    CryptoEngine& operator=(CryptoEngine&&) = delete;

public:
    static const proto::HashType StandardHash{proto::HASHTYPE_BLAKE2B256};

    EXPORT const OTCachedKey& DefaultKey() const;
    EXPORT Editor<OTCachedKey> mutable_DefaultKey() const;
    EXPORT const OTCachedKey& CachedKey(const Identifier& id) const;
    EXPORT const OTCachedKey& CachedKey(const OTCachedKey& source) const;
    EXPORT const OTCachedKey& LoadDefaultKey(
        const OTASCIIArmor& serialized) const;
    EXPORT void SetTimeout(const std::chrono::seconds& timeout) const;
    EXPORT void SetSystemKeyring(const bool useKeyring) const;

    // Encoding function interface
    EXPORT CryptoEncodingEngine& Encode() const;

    // Hash function interface
    EXPORT CryptoHashEngine& Hash() const;

    // Utility class for misc OpenSSL-provided functions
    EXPORT CryptoUtil& Util() const;

    // Asymmetric encryption engines
    EXPORT CryptoAsymmetric& ED25519() const;
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    EXPORT CryptoAsymmetric& RSA() const;
#endif
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    EXPORT CryptoAsymmetric& SECP256K1() const;
#endif

    // Symmetric encryption engines
    EXPORT CryptoSymmetricEngine& Symmetric() const;

#if OT_CRYPTO_SUPPORTED_ALGO_AES
    EXPORT CryptoSymmetric& AES() const;
#endif
#if OT_CRYPTO_WITH_BIP32
    EXPORT Bip32& BIP32() const;
#endif
#if OT_CRYPTO_WITH_BIP39
    EXPORT Bip39& BIP39() const;
#endif

    std::unique_ptr<SymmetricKey> GetStorageKey(std::string& seed) const;

    ~CryptoEngine();
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_CRYPTO_CRYPTOENGINE_HPP
