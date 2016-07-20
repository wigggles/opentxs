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

#include "opentxs/core/crypto/LowLevelKeyGenerator.hpp"

#include "opentxs/core/app/App.hpp"
#include "opentxs/core/crypto/AsymmetricKeyEd25519.hpp"
#if defined(OT_CRYPTO_USING_LIBSECP256K1)
#include "opentxs/core/crypto/AsymmetricKeySecp256k1.hpp"
#endif
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/Libsodium.hpp"
#if defined(OT_CRYPTO_USING_OPENSSL)
#include "opentxs/core/crypto/mkcert.hpp"
#endif
#include "opentxs/core/crypto/NymParameters.hpp"
#if defined(OT_CRYPTO_USING_OPENSSL)
#include "opentxs/core/crypto/OTAsymmetricKey_OpenSSLPrivdp.hpp"
#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif
#include "opentxs/core/crypto/OTKeypair.hpp"
#if defined(OT_CRYPTO_USING_LIBSECP256K1)
#include "opentxs/core/crypto/OTPassword.hpp"
#endif
#include "opentxs/core/util/Assert.hpp"
#if defined(OT_CRYPTO_USING_LIBSECP256K1)
#include "opentxs/core/OTData.hpp"
#endif
#include "opentxs/core/Log.hpp"

#include <stdint.h>
#include <ostream>

namespace opentxs
{

class LowLevelKeyGenerator::LowLevelKeyGeneratordp
{
public:
    LowLevelKeyGeneratordp() = default;
    virtual void Cleanup() = 0;
    virtual ~LowLevelKeyGeneratordp() = default;
};

class LowLevelKeyGenerator::LowLevelKeyGeneratorECdp
    : public LowLevelKeyGeneratordp
{
public:
    OTPassword privateKey_;
    std::unique_ptr<OTData> publicKey_;

    LowLevelKeyGeneratorECdp();
    virtual void Cleanup();
};

#if defined(OT_CRYPTO_USING_OPENSSL)
class LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp
    : public LowLevelKeyGeneratordp
{
public:
    virtual void Cleanup();

    X509* m_pX509 = nullptr;
    /** Instantiated form of key. (For private keys especially, we don't want it
     *  instantiated for any longer than absolutely necessary.)
    */
    EVP_PKEY* m_pKey = nullptr;
};
#endif

LowLevelKeyGenerator::~LowLevelKeyGenerator()
{
    if (m_bCleanup) {
        Cleanup();
    }
    if (nullptr != dp) {
        delete dp;
    }
    if (pkeyData_) {
        pkeyData_.release();
    }
}

LowLevelKeyGenerator::LowLevelKeyGenerator(const NymParameters& pkeyData)
    : m_bCleanup(true)
{
    pkeyData_.reset(const_cast<NymParameters*>(&pkeyData));

#if defined(OT_CRYPTO_USING_OPENSSL)
    if (pkeyData_->nymParameterType() == NymParameters::LEGACY) {
        dp = new LowLevelKeyGeneratorOpenSSLdp;
#endif
#if defined(OT_CRYPTO_USING_GPG)

#endif
    } else if (pkeyData_->nymParameterType() == NymParameters::ED25519) {
        dp = new LowLevelKeyGeneratorECdp;
    } else if (pkeyData_->nymParameterType() == NymParameters::SECP256K1) {
#if defined(OT_CRYPTO_USING_LIBSECP256K1)
        dp = new LowLevelKeyGeneratorECdp;
#endif
    }
}

// Don't force things by explicitly calling this function, unless you are SURE
// there's no one else cleaning up the same objects. Notice the if (m_bCleanup)
// just above in the destructor, for that very reason.
void LowLevelKeyGenerator::Cleanup()
{

    if (nullptr != dp) {
        dp->Cleanup();
    }
}

#if defined(OT_CRYPTO_USING_OPENSSL)
void LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp::Cleanup()
{
    if (nullptr != m_pKey) {
        EVP_PKEY_free(m_pKey);
        m_pKey = nullptr;
    }
    if (nullptr != m_pX509) {
        X509_free(m_pX509);
        m_pX509 = nullptr;
    }
}
#endif

LowLevelKeyGenerator::LowLevelKeyGeneratorECdp::
    LowLevelKeyGeneratorECdp()
{
    publicKey_.reset(new OTData);
}

void LowLevelKeyGenerator::LowLevelKeyGeneratorECdp::Cleanup()
{
    privateKey_.zeroMemory();

}

bool LowLevelKeyGenerator::MakeNewKeypair()
{
    // pkeyData can not be null if LowLevelkeyGenerator has been constructed
    if (pkeyData_->nymParameterType() == NymParameters::LEGACY) {
#if defined(OT_CRYPTO_USING_OPENSSL)
        //  OpenSSL_BIO bio_err = nullptr;
        X509* x509          = nullptr;
        EVP_PKEY* pNewKey   = nullptr;

        //  CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON); // memory leak detection.
            // Leaving this for now.
        //  bio_err    =    BIO_new_fp(stderr, BIO_NOCLOSE);

        // actually generate the things. // TODO THESE PARAMETERS...(mkcert)
        mkcert(&x509, &pNewKey, pkeyData_->keySize(), 0, 3650); // 3650=10 years. Todo hardcoded.
        // Note: 512 bit key CRASHES
        // 1024 is apparently a minimum requirement, if not an only requirement.
        // Will need to go over just what sorts of keys are involved here... todo.

        if (nullptr == x509) {
            otErr << __FUNCTION__
                << ": Failed attempting to generate new x509 cert.\n";

            if (nullptr != pNewKey) EVP_PKEY_free(pNewKey);
            pNewKey = nullptr;

            return false;
        }

        if (nullptr == pNewKey) {
            otErr << __FUNCTION__
                << ": Failed attempting to generate new private key.\n";

            if (nullptr != x509) X509_free(x509);
            x509 = nullptr;

            return false;
        }

        // Below this point, x509 and pNewKey will need to be cleaned up properly.

        if (m_bCleanup) Cleanup();

        m_bCleanup = true;

        LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp* ldp =
            static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp*>(dp);

        ldp->m_pKey = pNewKey;
        ldp->m_pX509 = x509;

        return true;
#elif defined(OT_CRYPTO_USING_GPG)
#endif
    } else if (pkeyData_->nymParameterType() == NymParameters::ED25519) {
        Libsodium& engine =
            static_cast<Libsodium&>(App::Me().Crypto().ED25519());
        LowLevelKeyGenerator::LowLevelKeyGeneratorECdp* ldp =
            static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorECdp*>
                (dp);

        return engine.RandomKeypair(ldp->privateKey_, *ldp->publicKey_);
    } else if (pkeyData_->nymParameterType() == NymParameters::SECP256K1) {
#if defined(OT_CRYPTO_USING_LIBSECP256K1)
        Libsecp256k1& engine =
            static_cast<Libsecp256k1&>(App::Me().Crypto().SECP256K1());
        LowLevelKeyGenerator::LowLevelKeyGeneratorECdp* ldp =
            static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorECdp*>
                (dp);

        return engine.RandomKeypair(ldp->privateKey_, *ldp->publicKey_);
#endif
    }

    return false; //unsupported keyType
}

bool LowLevelKeyGenerator::SetOntoKeypair(
    OTKeypair& theKeypair,
    OTPasswordData& passwordData)
{
    // pkeyData can not be null if LowLevelkeyGenerator has been constructed
    if (pkeyData_->nymParameterType() == NymParameters::LEGACY) {
#if defined(OT_CRYPTO_USING_OPENSSL)
        LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp* ldp =
            static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp*>(dp);

        OT_ASSERT(nullptr != ldp->m_pKey);
        OT_ASSERT(nullptr != ldp->m_pX509);

        OT_ASSERT(theKeypair.m_pkeyPublic);
        OT_ASSERT(theKeypair.m_pkeyPrivate);

        // Since we are in OpenSSL-specific code, we have to make sure these are
        // OpenSSL-specific keys.
        //
        std::shared_ptr<OTAsymmetricKey_OpenSSL> pPublicKey = std::dynamic_pointer_cast<OTAsymmetricKey_OpenSSL>(theKeypair.m_pkeyPublic);

        std::shared_ptr<OTAsymmetricKey_OpenSSL> pPrivateKey = std::dynamic_pointer_cast<OTAsymmetricKey_OpenSSL>(theKeypair.m_pkeyPrivate);

        if (!pPublicKey) {
            otErr << __FUNCTION__ << ": dynamic_cast to OTAsymmetricKey_OpenSSL "
                                        "failed. (theKeypair.m_pkeyPublic)\n";
            return false;
        }
        if (!pPrivateKey) {
            otErr << __FUNCTION__ << ": dynamic_cast to OTAsymmetricKey_OpenSSL "
                                        "failed. (theKeypair.m_pkeyPrivate)\n";
            return false;
        }

        // Now we can call OpenSSL-specific methods on these keys...
        //
        pPublicKey->SetAsPublic();
        //  EVP_PKEY * pEVP_PubKey = X509_get_pubkey(m_pX509);
        //  OT_ASSERT(nullptr != pEVP_PubKey);
        //  pPublicKey-> SetKeyAsCopyOf(*pEVP_PubKey); // bool bIsPrivateKey=false
        // by default.
        pPublicKey->dp->SetKeyAsCopyOf(
            *ldp->m_pKey); // bool bIsPrivateKey=false by default.
                            //  EVP_PKEY_free(pEVP_PubKey);
                            //  pEVP_PubKey = nullptr;

        pPublicKey->dp->SetX509(ldp->m_pX509); // m_pX509 is now owned by pPublicKey.
                                                // (No need to free it in our own
                                                // destructor anymore.)
        ldp->m_pX509 =
            nullptr; // pPublicKey took ownership, so we don't want to ALSO
                        // clean it up, since pPublicKey already will do so.

        pPrivateKey->SetAsPrivate();
        pPrivateKey->dp->SetKeyAsCopyOf(
            *ldp->m_pKey, true); // bool bIsPrivateKey=true; (Default is false)
        // Since pPrivateKey only takes a COPY of m_pKey, we are still responsible
        // to clean up m_pKey in our own destructor.
        // (Assuming m_bCleanup is set to true, which is the default.) That's why
        // I'm NOT setting it to nullptr, as I did above
        // with m_pX509.

        EVP_PKEY_free(ldp->m_pKey);
        ldp->m_pKey = nullptr;

        // Success! At this point, theKeypair's public and private keys have been
        // set.

        return true;
#elif defined(OT_CRYPTO_USING_GPG)
#endif
    } else if (pkeyData_->nymParameterType() == NymParameters::ED25519) {
        Libsodium& engine =
            static_cast<Libsodium&>(App::Me().Crypto().SECP256K1());
        LowLevelKeyGenerator::LowLevelKeyGeneratorECdp* ldp =
            static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorECdp*>
                (dp);

        OT_ASSERT(theKeypair.m_pkeyPublic);
        OT_ASSERT(theKeypair.m_pkeyPrivate);

        // Since we are in ed25519-specific code, we have to make sure these
        // are ed25519-specific keys.
        std::shared_ptr<AsymmetricKeyEd25519> pPublicKey =
            std::dynamic_pointer_cast<AsymmetricKeyEd25519>
            (theKeypair.m_pkeyPublic);

        std::shared_ptr<AsymmetricKeyEd25519> pPrivateKey =
            std::dynamic_pointer_cast<AsymmetricKeyEd25519>
                (theKeypair.m_pkeyPrivate);

        if (!pPublicKey) {
            otErr << __FUNCTION__ << ": dynamic_cast of public key to "
                  << "AsymmetricKeyEd25519 failed." << std::endl;

            return false;
        }

        if (!pPrivateKey) {
            otErr << __FUNCTION__ << ": dynamic_cast of private key to "
                  << "AsymmetricKeyEd25519 failed." << std::endl;

            return false;
        }

        pPublicKey->SetAsPublic();
        pPrivateKey->SetAsPrivate();

        bool pubkeySet =
            engine.ECPubkeyToAsymmetricKey(ldp->publicKey_, *pPublicKey);
        bool privkeySet = engine.ECPrivatekeyToAsymmetricKey(
                ldp->privateKey_, passwordData, *pPrivateKey);

        return (pubkeySet && privkeySet);
    } else if (pkeyData_->nymParameterType() == NymParameters::SECP256K1) {
#if defined(OT_CRYPTO_USING_LIBSECP256K1)
        Libsecp256k1& engine =
            static_cast<Libsecp256k1&>(App::Me().Crypto().SECP256K1());
        LowLevelKeyGenerator::LowLevelKeyGeneratorECdp* ldp =
            static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorECdp*>
                (dp);

        OT_ASSERT(theKeypair.m_pkeyPublic);
        OT_ASSERT(theKeypair.m_pkeyPrivate);

        // Since we are in secp256k1-specific code, we have to make sure these
        // are secp256k1-specific keys.
        std::shared_ptr<AsymmetricKeySecp256k1> pPublicKey =
            std::dynamic_pointer_cast<AsymmetricKeySecp256k1>
            (theKeypair.m_pkeyPublic);

        std::shared_ptr<AsymmetricKeySecp256k1> pPrivateKey =
            std::dynamic_pointer_cast<AsymmetricKeySecp256k1>
                (theKeypair.m_pkeyPrivate);

        if (!pPublicKey) {
            otErr << __FUNCTION__ << ": dynamic_cast of public key to "
                  << "OTAsymmetricKeySecp256k1 failed." << std::endl;

            return false;
        }

        if (!pPrivateKey) {
            otErr << __FUNCTION__ << ": dynamic_cast of private key to "
                  << "OTAsymmetricKeySecp256k1 failed." << std::endl;

            return false;
        }

        pPublicKey->SetAsPublic();
        pPrivateKey->SetAsPrivate();

        bool pubkeySet =
            engine.ECPubkeyToAsymmetricKey(ldp->publicKey_, *pPublicKey);
        bool privkeySet = engine.ECPrivatekeyToAsymmetricKey(
                ldp->privateKey_, passwordData, *pPrivateKey);

        return (pubkeySet && privkeySet);
#endif
    }

    return false; //unsupported keyType
}
} // namespace opentxs
