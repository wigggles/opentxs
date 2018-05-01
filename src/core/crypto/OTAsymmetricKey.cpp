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

#include "opentxs/stdafx.hpp"

#include "opentxs/core/crypto/OTAsymmetricKey.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/AsymmetricKeyEd25519.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/core/crypto/AsymmetricKeySecp256k1.hpp"
#endif
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/core/crypto/Bip32.hpp"
#endif
#include "opentxs/core/crypto/CryptoAsymmetric.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/core/crypto/OTAsymmetricKeyOpenSSL.hpp"
#endif
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <cstring>
#include <memory>
#include <ostream>
#include <string>

#define OT_METHOD "opentxs::OTAsymmetricKey::"

namespace opentxs
{

// static
OTAsymmetricKey* OTAsymmetricKey::KeyFactory(
    const proto::AsymmetricKeyType keyType,
    const proto::KeyRole role)
{
    OTAsymmetricKey* pKey = nullptr;

    switch (keyType) {
        case (proto::AKEYTYPE_ED25519): {
            pKey = new AsymmetricKeyEd25519(role);

            break;
        }
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            pKey = new AsymmetricKeySecp256k1(role);

            break;
        }
#endif
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (proto::AKEYTYPE_LEGACY): {
            pKey = new OTAsymmetricKey_OpenSSL(role);

            break;
        }
#endif
        default: {
            otErr << __FUNCTION__ << ": Open-Transactions isn't built with "
                  << "support for this key type." << std::endl;
        }
    }

    return pKey;
}

// static
OTAsymmetricKey* OTAsymmetricKey::KeyFactory(
    const proto::AsymmetricKeyType keyType,
    const String& pubkey)  // Caller IS responsible to
                           // delete!
{
    OTAsymmetricKey* pKey = nullptr;

    switch (keyType) {
        case (proto::AKEYTYPE_ED25519): {
            pKey = new AsymmetricKeyEd25519(pubkey);

            break;
        }
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            pKey = new AsymmetricKeySecp256k1(pubkey);

            break;
        }
#endif
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (proto::AKEYTYPE_LEGACY): {
            pKey = new OTAsymmetricKey_OpenSSL(pubkey);

            break;
        }
#endif
        default: {
            otErr << __FUNCTION__ << ": Open-Transactions isn't built with "
                  << "support for this key type." << std::endl;
        }
    }

    return pKey;
}

// static
OTAsymmetricKey* OTAsymmetricKey::KeyFactory(
    const NymParameters& nymParameters,
    const proto::KeyRole role)  // Caller IS responsible to delete!
{
    auto keyType = nymParameters.AsymmetricKeyType();

    return KeyFactory(keyType, role);
}

OTAsymmetricKey* OTAsymmetricKey::KeyFactory(
    const proto::AsymmetricKey& serializedKey)  // Caller IS responsible to
                                                // delete!
{
    auto keyType = serializedKey.type();

    OTAsymmetricKey* pKey = nullptr;

    switch (keyType) {
        case (proto::AKEYTYPE_ED25519): {
            pKey = new AsymmetricKeyEd25519(serializedKey);

            break;
        }
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            pKey = new AsymmetricKeySecp256k1(serializedKey);

            break;
        }
#endif
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (proto::AKEYTYPE_LEGACY): {
            pKey = new OTAsymmetricKey_OpenSSL(serializedKey);

            break;
        }
#endif
        default: {
            otErr << __FUNCTION__ << ": Open-Transactions isn't built with "
                  << "support for this key type." << std::endl;
        }
    }

    return pKey;
}

bool OTAsymmetricKey::CalculateID(Identifier& theOutput) const  // Only works
                                                                // for public
                                                                // keys.
{
    const char* szFunc = "OTAsymmetricKey::CalculateID";

    theOutput.Release();

    if (!IsPublic()) {
        otErr << szFunc
              << ": Error: !IsPublic() (This function should only be "
                 "called on a public key.)\n";
        return false;
    }

    String strPublicKey;
    bool bGotPublicKey = GetPublicKey(strPublicKey);

    if (!bGotPublicKey) {
        otErr << szFunc << ": Error getting public key.\n";
        return false;
    }

    bool bSuccessCalculateDigest = theOutput.CalculateDigest(strPublicKey);

    if (!bSuccessCalculateDigest) {
        theOutput.Release();
        otErr << szFunc << ": Error calculating digest of public key.\n";
        return false;
    }

    return true;
}

OTAsymmetricKey::OTAsymmetricKey()
    : m_bIsPublicKey(false)
    , m_bIsPrivateKey(false)
    , m_pMetadata(new OTSignatureMetadata)
{
}

OTAsymmetricKey::OTAsymmetricKey(
    const proto::AsymmetricKeyType keyType,
    const proto::KeyRole role)
    : m_keyType(keyType)
    , role_(role)
    , m_bIsPublicKey(false)
    , m_bIsPrivateKey(false)
    , m_pMetadata(new OTSignatureMetadata)
{
}

OTAsymmetricKey::OTAsymmetricKey(const proto::AsymmetricKey& serializedKey)
    : OTAsymmetricKey(serializedKey.type(), serializedKey.role())
{
    if (proto::KEYMODE_PUBLIC == serializedKey.mode()) {
        SetAsPublic();
    } else if (proto::KEYMODE_PRIVATE == serializedKey.mode()) {
        SetAsPrivate();
    }
}

OTAsymmetricKey::~OTAsymmetricKey()
{
    //    Release_AsymmetricKey(); // ******

    m_timer.clear();  // Since ReleaseKeyLowLevel is no longer called here (via
                      // Release_AsymmetricKey) then
    // m_timer.clear() was no longer getting called, so I added it here to
    // rectify that. See below NOTE for
    // more details.
    //
    // NOTE: It's very unusual that the above is commented out, since my normal
    // convention is to
    // call this function both in the destructor, as well as in the Release()
    // method for any given
    // class.
    // Here's why it's commented out, in this case. Because all it does is call
    // ReleaseKeyLowLevel,
    // which calls ReleaseKeyLowLevel_Hook, which is pure virtual and is what
    // allows the
    // OTAsymmetricKey_OpenSSL class to clean up its OpenSSL-specific private
    // members.
    // We CANNOT call a pure virtual method from a destructor (which is where we
    // currently are)
    // and so we cannot call Release_AsymmetricKey without removing that pure
    // virtual call. The
    // problem is, ReleaseKeyLowLevel USES that pure virtual call all over this
    // file (i.e. in many
    // other places BESIDES the destructor) and so we cannot just remove the
    // pure virtual call it
    // uses which is, in fact, the entire purpose it's being called in the first
    // place. So what I
    // did was, I changed OTAsymmetricKey_OpenSSL to directly clean up its
    // OpenSSL-specific key data,
    // and it just ALSO has the hook override doing the same thing. This way
    // Release_AsymmetricKey
    // can continue to be used by ReleaseKeyLowLevel all over this file, as it
    // was designed, using
    // the hook and the override, yet also this destructor will continue to work
    // perfectly... because
    // Release_AsymmetricKey() is commented out above, we won't have any runtime
    // errors from trying to
    // run a pure virtual method from a destructor. And because
    // OTAsymmetricKey_OpenSSL now cleans itself
    // up in its own destructor automatically, we have no need whatsoever to
    // call a virtual function here
    // to clean it up. So it's commented out.
    // Makes sense? Of course we didn't have any virtuality before
    // OTAsymmetricKey_OpenSSL was added,
    // since OTAsymmetricKey previously had no subclasses at all. But that has
    // changed recently, so that
    // it is now an abstract interface, so that someday a GPG implementation, or
    // NaCl implementation can
    // someday be added.

    if (nullptr != m_pMetadata) delete m_pMetadata;
    m_pMetadata = nullptr;
}

void OTAsymmetricKey::Release_AsymmetricKey()
{

    // Release the ascii-armored version of the key (safe to store in this
    // form.)
    //

    // Release the instantiated key (unsafe to store in this form.)
    //
    ReleaseKeyLowLevel();
}

void OTAsymmetricKey::ReleaseKeyLowLevel()
{
    ReleaseKeyLowLevel_Hook();

    m_timer.clear();
}

// High-level, used only by programmatic user, not by this class internally.
void OTAsymmetricKey::ReleaseKey()
{
    // Todo: someday put a timer inside this class, so it doesn't REALLY release
    // the
    // key until the Timer has expired.
    // NOTE: marking this as "todo" because I won't necessarily want it done
    // this way. This
    // solution would keep the actual private key loaded in OpenSSL, meaning I'd
    // have to take
    // over the OpenSSL memory management to make it into a safe solution
    // (though I probably
    // have to do that anyway.)
    // A better solution might be to have a random session key used for
    // protecting a hashed
    // version of the passphrase, and then ALWAYS destroying the key as fast as
    // possible,
    // instead using the hashed passphrase within the bounds of a timer, with
    // the hashed passphrase
    // being decrypted JUST before use and then destroyed, just as the private
    // key is destroyed.
    // I might also wish such a thing was stored not just in OT protected
    // memory, but in a standard
    // *API* for protected memory, such as ssh-agent or the Mac Keychain.
    // However, that can easily
    // be just an option to be added later, meaning I can go ahead and code my
    // hashed password solution
    // in the meantime. But will that be coded here at the OTAsymmetricKey
    // level? Or at the Nym level,
    // or at the static Nym level, or the app level? Hmm...
    //
    // Security:
    // UPDATE: Since the above solution is coming at some point anyway, I'm
    // going ahead and adding a
    // Timer version that works at this level (OTAsymmetricKey.)  The reason is
    // because it will be quick
    // and easy, and will give me the functionality I need for now, until I code
    // all the stuff above.
    // Just keep in mind: This is good enough for now, but it WILL result in the
    // private key staying
    // loaded in memory until the timer runs out, meaning if an attacker has
    // access to the RAM on the
    // local machine, if I haven't replaced the OpenSSL memory management, then
    // that is a security issue.
    //
    // TODO (remove theTimer entirely. OTCachedKey replaces already.)
    // I set this timer because the above required a password. But now that
    // master key is working,
    // the above would flow through even WITHOUT the user typing his passphrase
    // (since master key still
    // not timed out.) Resulting in THIS timer being reset!  Todo: I already
    // shortened this timer to 30
    // seconds, but need to phase it down to 0 and then remove it entirely!
    // Master key takes over now!
    //

    //  if (m_timer.getElapsedTimeInSec() > OT_KEY_TIMER)
    ReleaseKeyLowLevel();

    // Programmatic user (developer) may call ReleaseKey, but then we don't
    // actually release it until it's
    // been at least X minutes.
}

void OTAsymmetricKey::Release()
{
    Release_AsymmetricKey();  // My own cleanup is done here.

    // Next give the base class a chance to do the same...
    //    ot_super::Release(); // THERE IS NO base class in this case. But
    // normally this would be here for most classes.
}

String OTAsymmetricKey::KeyTypeToString(const proto::AsymmetricKeyType keyType)

{
    String keytypeString;

    switch (keyType) {
        case proto::AKEYTYPE_LEGACY:
            keytypeString = "legacy";
            break;
        case proto::AKEYTYPE_SECP256K1:
            keytypeString = "secp256k1";
            break;
        case proto::AKEYTYPE_ED25519:
            keytypeString = "ed25519";
            break;
        default:
            keytypeString = "error";
    }
    return keytypeString;
}

proto::AsymmetricKeyType OTAsymmetricKey::StringToKeyType(const String& keyType)

{
    if (keyType.Compare("legacy")) return proto::AKEYTYPE_LEGACY;
    if (keyType.Compare("secp256k1")) return proto::AKEYTYPE_SECP256K1;
    if (keyType.Compare("ed25519")) return proto::AKEYTYPE_ED25519;

    return proto::AKEYTYPE_ERROR;
}

proto::AsymmetricKeyType OTAsymmetricKey::keyType() const { return m_keyType; }

serializedAsymmetricKey OTAsymmetricKey::Serialize() const

{
    serializedAsymmetricKey serializedKey =
        std::make_shared<proto::AsymmetricKey>();

    serializedKey->set_version(1);
    serializedKey->set_role(role_);
    serializedKey->set_type(static_cast<proto::AsymmetricKeyType>(m_keyType));

    return serializedKey;
}

OTData OTAsymmetricKey::SerializeKeyToData(
    const proto::AsymmetricKey& serializedKey) const
{
    return proto::ProtoAsData(serializedKey);
}

bool OTAsymmetricKey::operator==(const proto::AsymmetricKey& rhs) const
{
    serializedAsymmetricKey tempKey = Serialize();
    auto LHData = SerializeKeyToData(*tempKey);
    auto RHData = SerializeKeyToData(rhs);

    return (LHData == RHData);
}

bool OTAsymmetricKey::Verify(const Data& plaintext, const proto::Signature& sig)
    const
{
    if (IsPrivate()) {
        otErr << "You must use public keys to verify signatures.\n";
        return false;
    }

    auto signature = Data::Factory();
    signature->Assign(sig.signature().c_str(), sig.signature().size());

    return engine().Verify(
        plaintext, *this, signature, sig.hashtype(), nullptr);
}

bool OTAsymmetricKey::Sign(
    const Data& plaintext,
    proto::Signature& sig,
    const OTPasswordData* pPWData,
    const OTPassword* exportPassword,
    const String& credID,
    const proto::SignatureRole role) const
{
    if (IsPublic()) {
        otErr << "You must use private keys to create signatures.\n";
        return false;
    }

    auto signature = Data::Factory();
    const auto hash = SigHashType();

    bool goodSig = engine().Sign(
        plaintext, *this, hash, signature, pPWData, exportPassword);

    if (goodSig) {
        sig.set_version(1);
        if (credID.Exists()) {
            sig.set_credentialid(credID.Get());
        }
        if (proto::SIGROLE_ERROR != role) {
            sig.set_role(role);
        }
        sig.set_hashtype(hash);
        sig.set_signature(signature->GetPointer(), signature->GetSize());
    }

    return goodSig;
}

const std::string OTAsymmetricKey::Path() const { return ""; }

bool OTAsymmetricKey::Path(proto::HDPath&) const
{
    otErr << OT_METHOD << __FUNCTION__ << ": Incorrect key type." << std::endl;

    return false;
}

bool OTAsymmetricKey::hasCapability(const NymCapability& capability) const
{
    switch (capability) {
        case (NymCapability::SIGN_CHILDCRED): {
        }
        case (NymCapability::SIGN_MESSAGE): {
        }
        case (NymCapability::ENCRYPT_MESSAGE): {

            return true;
        }
        default: {
        }
    }

    return false;
}
}  // namespace opentxs
