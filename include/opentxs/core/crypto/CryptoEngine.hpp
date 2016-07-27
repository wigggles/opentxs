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

#if OT_CRYPTO_WITH_BIP39
#include "opentxs/core/crypto/Bip39.hpp"
#endif
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/core/crypto/Bip32.hpp"
#endif
#include "opentxs/core/crypto/CryptoAsymmetric.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/CryptoUtil.hpp"
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/core/crypto/Libsecp256k1.hpp"
#endif
#include "opentxs/core/crypto/Libsodium.hpp"
#if OT_CRYPTO_USING_OPENSSL
#include "opentxs/core/crypto/OpenSSL.hpp"
#endif
#if OT_CRYPTO_USING_TREZOR
#include "opentxs/core/crypto/TrezorCrypto.hpp"
#endif

#include <memory>

namespace opentxs
{

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

class CryptoHashEngine;

//Singlton class for providing an interface to external crypto libraries
//and hold the state required by those libraries.
class CryptoEngine
{
    friend class CryptoHashEngine;

private:
#if OT_CRYPTO_USING_TREZOR
    std::unique_ptr<bitcoincrypto> bitcoincrypto_;
#endif
    std::unique_ptr<Curve25519> ed25519_;
    static CryptoEngine* instance_;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    std::unique_ptr<secp256k1> secp256k1_;
#endif
    std::unique_ptr<CryptoHashEngine> hash_;
    std::unique_ptr<SSLImplementation> ssl_;

    void Init();

    CryptoEngine();
    CryptoEngine(const CryptoEngine&) = delete;
    CryptoEngine& operator=(const CryptoEngine&) = delete;

public:
    //Hash function interface
    EXPORT CryptoHashEngine& Hash() const;

    //Utility class for misc OpenSSL-provided functions
    EXPORT CryptoUtil& Util() const;

    //Asymmetric encryption engines
    EXPORT CryptoAsymmetric& ED25519() const;
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    EXPORT CryptoAsymmetric& RSA() const;
#endif
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    EXPORT CryptoAsymmetric& SECP256K1() const;
#endif

    //Symmetric encryption engines
#if OT_CRYPTO_SUPPORTED_ALGO_AES
    EXPORT CryptoSymmetric& AES() const;
#endif
#if OT_CRYPTO_WITH_BIP32
    EXPORT Bip32& BIP32() const;
#endif
#if OT_CRYPTO_WITH_BIP39
    EXPORT Bip39& BIP39() const;
#endif

    EXPORT void Cleanup();
    EXPORT static CryptoEngine& It();

    ~CryptoEngine();
};
}  // namespace opentxs
#endif // OPENTXS_CORE_CRYPTO_CRYPTOENGINE_HPP
