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
#include "opentxs/core/crypto/SymmetricKey.hpp"
#if OT_CRYPTO_USING_TREZOR
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/TrezorCrypto.hpp"
#endif
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Log.hpp"

#include <functional>
#include <ostream>

extern "C" {
#ifdef _WIN32
#else
#include <sys/resource.h>
#endif
}

#define OT_METHOD "opentxs::CryptoEngine::"

namespace opentxs
{

CryptoEngine::CryptoEngine(OT& ot)
    : ot_(ot)
    , cached_key_lock_()
    , primary_key_(nullptr)
    , cached_keys_()
#if OT_CRYPTO_USING_TREZOR
    , bitcoincrypto_(new TrezorCrypto(ot_))
#endif
    , ed25519_(new Curve25519)
    , ssl_(new SSLImplementation)
{
    Init();
}

#if OT_CRYPTO_SUPPORTED_ALGO_AES
CryptoSymmetric& CryptoEngine::AES() const
{
    OT_ASSERT(nullptr != ssl_);

    return *ssl_;
}
#endif

#if OT_CRYPTO_WITH_BIP32
Bip32& CryptoEngine::BIP32() const
{
    OT_ASSERT(nullptr != bitcoincrypto_);

    return *bitcoincrypto_;
}
#endif

#if OT_CRYPTO_WITH_BIP39
Bip39& CryptoEngine::BIP39() const
{
    OT_ASSERT(nullptr != bitcoincrypto_);

    return *bitcoincrypto_;
}
#endif

const OTCachedKey& CryptoEngine::CachedKey(const Identifier& id) const
{
    Lock lock(cached_key_lock_);

    auto& output = cached_keys_[id];

    if (false == bool(output)) {
        output.reset(new OTCachedKey(OT_MASTER_KEY_TIMEOUT));
    }

    OT_ASSERT(output);

    return *output;
}

const OTCachedKey& CryptoEngine::CachedKey(const OTCachedKey& source) const
{
    Lock lock(cached_key_lock_);
    const Identifier id(source);
    auto& output = cached_keys_[id];

    if (false == bool(output)) {
        OTASCIIArmor serialized{};
        const bool haveSerialized = source.SerializeTo(serialized);

        OT_ASSERT(haveSerialized);

        output.reset(new OTCachedKey(serialized));
    }

    OT_ASSERT(output);

    return *output;
}

void CryptoEngine::Cleanup()
{
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    secp256k1_->Cleanup();
#endif
    ssl_->Cleanup();
}

const OTCachedKey& CryptoEngine::DefaultKey() const
{
    Lock lock(cached_key_lock_);

    init_default_key(lock);

    OT_ASSERT(primary_key_);

    return *primary_key_;
}

Editor<OTCachedKey> CryptoEngine::mutable_DefaultKey() const
{
    OT_ASSERT(primary_key_);

    std::function<void(OTCachedKey*, Lock&)> callback =
        [&](OTCachedKey*, Lock&) -> void {};

    return Editor<OTCachedKey>(cached_key_lock_, primary_key_.get(), callback);
}

CryptoAsymmetric& CryptoEngine::ED25519() const
{
    OT_ASSERT(nullptr != ed25519_);

    return *ed25519_;
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

void CryptoEngine::Init()
{
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    secp256k1_.reset(new secp256k1(*ssl_, *bitcoincrypto_));
#endif
    encode_.reset(new CryptoEncodingEngine(*this));
    hash_.reset(new CryptoHashEngine(*this));
    symmetric_.reset(new CryptoSymmetricEngine(*this));

    otWarn << OT_METHOD << __FUNCTION__
           << ": Setting up rlimits, and crypto libraries...\n";

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

void CryptoEngine::init_default_key(const Lock&) const
{
    if (false == bool(primary_key_)) {
        primary_key_.reset(new OTCachedKey(OT_MASTER_KEY_TIMEOUT));
    }
}

const OTCachedKey& CryptoEngine::LoadDefaultKey(
    const OTASCIIArmor& serialized) const
{
    Lock lock(cached_key_lock_);

    init_default_key(lock);

    OT_ASSERT(primary_key_);

    primary_key_->SetCachedKey(serialized);

    return *primary_key_;
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

void CryptoEngine::SetTimeout(const std::chrono::seconds& timeout) const
{
    Lock lock(cached_key_lock_);

    init_default_key(lock);

    OT_ASSERT(primary_key_);

    primary_key_->SetTimeoutSeconds(timeout.count());
}

void CryptoEngine::SetSystemKeyring(const bool useKeyring) const
{
    Lock lock(cached_key_lock_);

    init_default_key(lock);

    OT_ASSERT(primary_key_);

    primary_key_->UseSystemKeyring(useKeyring);
}

CryptoSymmetricEngine& CryptoEngine::Symmetric() const
{
    OT_ASSERT(symmetric_);

    return *symmetric_;
}

CryptoUtil& CryptoEngine::Util() const
{
    OT_ASSERT(nullptr != ssl_);

    return *ssl_;
}

std::unique_ptr<SymmetricKey> CryptoEngine::GetStorageKey(
    __attribute__((unused)) std::string& seed) const
{
#if OT_CRYPTO_WITH_BIP39
    auto serialized = BIP32().GetStorageKey(seed);

    if (false == bool(serialized)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load encryption key."
              << std::endl;

        return {};
    }

    OTPassword keySource{};
    auto sessionKey = Symmetric().Key(
        serialized->encryptedkey().key(), serialized->encryptedkey().mode());
    OTPasswordData blank(__FUNCTION__);
    const bool decrypted =
        sessionKey->Decrypt(serialized->encryptedkey(), blank, keySource);

    if (false == decrypted) {

        return {};
    }

    return Symmetric().Key(keySource);
#else
    return {};
#endif
}

CryptoEngine::~CryptoEngine() { Cleanup(); }
}  // namespace opentxs
