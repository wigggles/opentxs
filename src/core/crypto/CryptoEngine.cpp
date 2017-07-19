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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/crypto/CryptoEngine.hpp"

#if OT_CRYPTO_WITH_BIP32
#include "opentxs/core/crypto/Bip32.hpp"
#endif
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/core/crypto/Bip39.hpp"
#endif
#include "opentxs/core/crypto/CryptoAsymmetric.hpp"
#include "opentxs/core/crypto/CryptoEncodingEngine.hpp"
#include "opentxs/core/crypto/CryptoHashEngine.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/CryptoSymmetricEngine.hpp"
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
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Log.hpp"

#include <ostream>

extern "C" {
#ifdef _WIN32
#else
#include <sys/resource.h>
#endif
}

namespace opentxs
{

CryptoEngine* CryptoEngine::instance_ = nullptr;

CryptoEngine::CryptoEngine()
{
    ed25519_.reset(new Curve25519);
    ssl_.reset(new SSLImplementation);
#if OT_CRYPTO_USING_TREZOR
    bitcoincrypto_.reset(new TrezorCrypto);
#endif

    Init();
}

void CryptoEngine::Init()
{
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    secp256k1_.reset(new secp256k1(*ssl_, *bitcoincrypto_));
#endif
    hash_.reset(new CryptoHashEngine(*this));
    symmetric_.reset(new CryptoSymmetricEngine(*this));
    encode_.reset(new CryptoEncodingEngine(*this));

    otWarn
        << "CryptoEngine::Init: Setting up rlimits, and crypto libraries...\n";

// Here is a security measure intended to make it more difficult to
// capture a core dump. (Not used in debug mode, obviously.)
//
#if !defined(PREDEF_MODE_DEBUG) && defined(PREDEF_PLATFORM_UNIX)
    struct rlimit rlim;
    getrlimit(RLIMIT_CORE, &rlim);
    rlim.rlim_max = rlim.rlim_cur = 0;
    if (setrlimit(RLIMIT_CORE, &rlim)) {
        OT_FAIL_MSG("Crypto::Init: ASSERT: setrlimit failed. (Used for "
                    "preventing core dumps.)\n");
    }
#endif

    ssl_->Init();
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    // WARNING: The below call to secp256k1_->Init() DEPENDS on the fact
    // that the above call to ssl_->Init() happened FIRST.
    secp256k1_->Init();
#endif
    ed25519_->Init();
}

CryptoUtil& CryptoEngine::Util() const
{
    OT_ASSERT(nullptr != ssl_);

    return *ssl_;
}

CryptoEncodingEngine& CryptoEngine::Encode() const
{
    OT_ASSERT(encode_);

    return *encode_;
}

CryptoHashEngine& CryptoEngine::Hash() const
{
    OT_ASSERT(hash_);

    return *hash_;
}

#if OT_CRYPTO_SUPPORTED_KEY_RSA
CryptoAsymmetric& CryptoEngine::RSA() const
{
    OT_ASSERT(nullptr != ssl_);

    return *ssl_;
}
#endif

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
CryptoAsymmetric& CryptoEngine::SECP256K1() const
{
    OT_ASSERT(nullptr != secp256k1_);

    return *secp256k1_;
}
#endif

CryptoAsymmetric& CryptoEngine::ED25519() const
{
    OT_ASSERT(nullptr != ed25519_);

    return *ed25519_;
}

#if OT_CRYPTO_SUPPORTED_ALGO_AES
CryptoSymmetric& CryptoEngine::AES() const
{
    OT_ASSERT(nullptr != ssl_);

    return *ssl_;
}
#endif

#if OT_CRYPTO_WITH_BIP39
Bip39& CryptoEngine::BIP39() const
{
    OT_ASSERT(nullptr != bitcoincrypto_);

    return *bitcoincrypto_;
}
#endif

#if OT_CRYPTO_WITH_BIP32
Bip32& CryptoEngine::BIP32() const
{
    OT_ASSERT(nullptr != bitcoincrypto_);

    return *bitcoincrypto_;
}
#endif

CryptoEngine& CryptoEngine::It()
{
    if (nullptr == instance_) {
        instance_ = new CryptoEngine;
    }

    return *instance_;
}

void CryptoEngine::Cleanup()
{
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    secp256k1_->Cleanup();
#endif

    ssl_->Cleanup();
}

CryptoSymmetricEngine& CryptoEngine::Symmetric() const
{
    OT_ASSERT(symmetric_);

    return *symmetric_;
}

CryptoEngine::~CryptoEngine() { Cleanup(); }
}  // namespace opentxs
