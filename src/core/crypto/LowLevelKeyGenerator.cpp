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

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/crypto/mkcert.hpp>
#include <opentxs/core/crypto/NymParameters.hpp>
#include <opentxs/core/crypto/LowLevelKeyGenerator.hpp>

#include <opentxs/core/crypto/OTKeypair.hpp>
#include <opentxs/core/Log.hpp>

namespace opentxs
{

class LowLevelKeyGenerator::LowLevelKeyGeneratordp
{
public:
    LowLevelKeyGeneratordp() = default;
    virtual ~LowLevelKeyGeneratordp() = default;
    virtual void Cleanup() = 0;
};

} // namespace opentxs

#if defined(OT_CRYPTO_USING_OPENSSL)

#include <opentxs/core/crypto/OTAsymmetricKey_OpenSSLPrivdp.hpp>

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace opentxs
{

class LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp : public LowLevelKeyGeneratordp
{
public:
    virtual void Cleanup();

    X509* m_pX509 = nullptr;
    EVP_PKEY* m_pKey = nullptr; // Instantiated form of key. (For private keys especially,
                      // we don't want it instantiated for any longer than
                      // absolutely necessary.)
};

} // namespace opentxs

#endif

#if defined(OT_CRYPTO_USING_LIBSECP256K1)

#include <opentxs/core/crypto/CryptoEngine.hpp>
#include <opentxs/core/crypto/Libsecp256k1.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>

namespace opentxs
{

class LowLevelKeyGenerator::LowLevelKeyGeneratorSecp256k1dp : public LowLevelKeyGeneratordp
{
public:
    virtual void Cleanup();
    OTPassword privateKey_;
    OTPassword publicKey_;
};

} // namespace opentxs

#endif

namespace opentxs
{

LowLevelKeyGenerator::~LowLevelKeyGenerator()
{
    if (m_bCleanup) {
        Cleanup();
    }
    if (nullptr != dp) {
        delete dp;
    }
}

LowLevelKeyGenerator::LowLevelKeyGenerator(const std::shared_ptr<NymParameters>& pkeyData)
    : pkeyData_(pkeyData), m_bCleanup(true)
{

    #if defined(OT_CRYPTO_USING_OPENSSL)
    if (pkeyData_->nymParameterType() == NymParameters::LEGACY) {
        dp = new LowLevelKeyGeneratorOpenSSLdp;
        #endif
        //-------------------------
        #if defined(OT_CRYPTO_USING_GPG)

        #endif
    } else if (pkeyData_->nymParameterType() == NymParameters::SECP256K1) {
        #if defined(OT_CRYPTO_USING_LIBSECP256K1)
        dp = new LowLevelKeyGeneratorSecp256k1dp;
        #endif
    }

}

// Don't force things by explicitly calling this function, unless you are SURE
// there's no one else cleaning up the same objects. Notice the if (m_bCleanup)
// just above in the destructor, for that very reason.
//
void LowLevelKeyGenerator::Cleanup()
{

    if (nullptr != dp) {
        dp->Cleanup();
    }
}

void LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp::Cleanup()
{

    #if defined(OT_CRYPTO_USING_OPENSSL)
    if (nullptr != m_pKey) {
        EVP_PKEY_free(m_pKey);
        m_pKey = nullptr;
    }
    if (nullptr != m_pX509) {
        X509_free(m_pX509);
        m_pX509 = nullptr;
    }
    #endif
    //-------------------------
    #if defined(OT_CRYPTO_USING_GPG)
    #endif
}

void LowLevelKeyGenerator::LowLevelKeyGeneratorSecp256k1dp::Cleanup()
{
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
    } else if (pkeyData_->nymParameterType() == NymParameters::SECP256K1) {
        #if defined(OT_CRYPTO_USING_LIBSECP256K1)

        bool validPrivkey = false;
        uint8_t candidateKey [32]{};
        uint8_t nullKey [32]{};
        Libsecp256k1& engine = static_cast<Libsecp256k1&>(CryptoEngine::Instance().SECP256K1());

        LowLevelKeyGenerator::LowLevelKeyGeneratorSecp256k1dp* ldp =
            static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorSecp256k1dp*>(dp);

        while (!validPrivkey) {
            ldp->privateKey_.randomizeMemory_uint8(candidateKey, 32);
            // We add the random key to a zero value key because secp256k1_privkey_tweak_add
            // checks the result to make sure it's in the correct range for secp256k1.
            //
            // This loop should almost always run exactly one time (about 1/(2^128) chance of
            // randomly generating an invalid key thus requiring a second attempt)
            if (engine.secp256k1_privkey_tweak_add(candidateKey, nullKey)) {
                ldp->privateKey_.setMemory(candidateKey, 32);
                validPrivkey = true;
            };
        }

        secp256k1_pubkey_t pubkey;
        bool validPubkey = engine.secp256k1_pubkey_create(pubkey, ldp->privateKey_);
        bool serializedKey = false;

        if (validPubkey) {
            serializedKey = engine.secp256k1_pubkey_serialize(ldp->publicKey_, pubkey);
        }

        return (validPrivkey & serializedKey);
        #endif
    }
//-------------------------
    return false; //unsupported keyType
}

bool LowLevelKeyGenerator::SetOntoKeypair(OTKeypair& theKeypair)
{
    // pkeyData can not be null if LowLevelkeyGenerator has been constructed
    if (pkeyData_->nymParameterType() == NymParameters::LEGACY) {
        #if defined(OT_CRYPTO_USING_OPENSSL)

        LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp* ldp =
            static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp*>(dp);

        OT_ASSERT(nullptr != ldp->m_pKey);
        OT_ASSERT(nullptr != ldp->m_pX509);

        OT_ASSERT(nullptr != theKeypair.m_pkeyPublic);
        OT_ASSERT(nullptr != theKeypair.m_pkeyPrivate);

        // Since we are in OpenSSL-specific code, we have to make sure these are
        // OpenSSL-specific keys.
        //
        OTAsymmetricKey_OpenSSL* pPublicKey =
            dynamic_cast<OTAsymmetricKey_OpenSSL*>(theKeypair.m_pkeyPublic);
        OTAsymmetricKey_OpenSSL* pPrivateKey =
            dynamic_cast<OTAsymmetricKey_OpenSSL*>(theKeypair.m_pkeyPrivate);

        if (nullptr == pPublicKey) {
            otErr << __FUNCTION__ << ": dynamic_cast to OTAsymmetricKey_OpenSSL "
                                        "failed. (theKeypair.m_pkeyPublic)\n";
            return false;
        }
        if (nullptr == pPrivateKey) {
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
        // Keep in mind though, they still won't be "quite right" until saved and
        // loaded
        // again, at least according to existing logic. That saving/reloading is
        // currently
        // performed in OTPseudonym::GenerateNym().
        //
        return true;
        #elif defined(OT_CRYPTO_USING_GPG)

        #endif
    } else if (pkeyData_->nymParameterType() == NymParameters::SECP256K1) {
        #if defined(OT_CRYPTO_USING_LIBSECP256K1)
        return false;
        #endif
    }
//-------------------------
    return false; //unsupported keyType
}

} // namespace opentxs
