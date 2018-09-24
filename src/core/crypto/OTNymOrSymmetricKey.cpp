// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTNymOrSymmetricKey.hpp"

#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/LegacySymmetric.hpp"

extern "C" {
#if defined(OPENTXS_HAVE_NETINET_IN_H)
#include <netinet/in.h>
#endif
}

#include <algorithm>
#include <ostream>

namespace opentxs
{

// There are certain cases where we want the option to pass a Nym OR a
// symmetric key, and the function should be able to handle either.
// This class is used to make that possible.
//
/// (This constructor is private.)
OTNym_or_SymmetricKey::OTNym_or_SymmetricKey()
    : m_pNym(nullptr)
    , m_pKey(nullptr)
    , m_pPassword(nullptr)
    , m_bCleanupPassword(false)
    , m_pstrDisplay(nullptr)
{
}

OTNym_or_SymmetricKey::~OTNym_or_SymmetricKey()
{
    // We don't own these objects.
    // Rather, we own a pointer to ONE of them, since we are a wrapper
    // for this one or that.
    //
    m_pNym = nullptr;

    m_pKey = nullptr;

    if (m_bCleanupPassword && (nullptr != m_pPassword)) delete m_pPassword;
    m_pPassword = nullptr;  // optional

    m_pstrDisplay = String::Factory();

    // Since this is merely a wrapper class, we don't even Release() these
    // things.
    // However, we DO have a release function, since the programmatic USER of
    // this class
    // MAY wish to Release() whatever it is wrapping. That's his choice. But we
    // don't call
    // it here in the destructor, because we aren't the owner.
    //
    //  Release_Nym_or_SymmetricKey();
}

OTNym_or_SymmetricKey::OTNym_or_SymmetricKey(
    const OTNym_or_SymmetricKey& rhs)  // same type
    : m_pNym(nullptr)
    , m_pKey(nullptr)
    , m_pPassword(nullptr)
    , m_bCleanupPassword(false)
    , m_pstrDisplay(rhs.m_pstrDisplay)
{
    // This class doesn't do any cleanup, it's just a temporary wrapper.
    // So we won't have anything get deleted twice, because this class won't
    // even delete it once.
    //
    m_pNym = rhs.m_pNym;

    m_pKey = rhs.m_pKey;
    m_pPassword = rhs.m_pPassword;  // optional

    // m_bCleanupPassword  = rhs.m_bCleanupPassword; // optional
    //
    // This is commented out because this object keeps a POINTER to the
    // password,
    // which is usually owned by the caller. (So we normally wouldn't delete
    // it.)
    // But sometimes, we have to CREATE the password, in which case we store it
    // until
    // we destruct, and then destroy it in our destructor. (Having it available
    // in the
    // meantime to use, without having to load again.)
    // m_bCleanupPassword thus normally tells us whether the password was passed
    // in
    // by its owner for reference purposes only, or whether we created it
    // internally and
    // thus need to clean it up ourselves.
    // THEREFORE, here in the COPY CONSTRUCTOR, we need to keep a pointer to the
    // password
    // from the rhs object, in case we need that password, but we cannot DESTROY
    // that password,
    // if rhs is also destroying it! Therefore we copy the password, but we
    // leave m_bCleanupPassword
    // as false (its default) since THIS INSTANCE definitely does not own
    // m_pPassword.
    //
}

OTNym_or_SymmetricKey::OTNym_or_SymmetricKey(
    const Nym& theNym,
    const String& pstrDisplay)  // construct with nym
    : m_pNym(const_cast<Nym*>(&theNym))
    , m_pKey(nullptr)
    , m_pPassword(nullptr)
    , m_bCleanupPassword(false)
    , m_pstrDisplay(pstrDisplay)
{
}

OTNym_or_SymmetricKey::OTNym_or_SymmetricKey(
    const crypto::key::LegacySymmetric& theKey,
    const String& pstrDisplay)  // construct with key
    : m_pNym(nullptr)
    , m_pKey(const_cast<crypto::key::LegacySymmetric*>(&theKey))
    , m_pPassword(nullptr)
    , m_bCleanupPassword(false)
    , m_pstrDisplay(pstrDisplay)
{
}

OTNym_or_SymmetricKey::OTNym_or_SymmetricKey(
    const crypto::key::LegacySymmetric& theKey,
    const OTPassword& thePassword,  // construct with key and password.
    const String& pstrDisplay)
    : m_pNym(nullptr)
    , m_pKey(const_cast<crypto::key::LegacySymmetric*>(&theKey))
    , m_pPassword(const_cast<OTPassword*>(&thePassword))
    , m_bCleanupPassword(false)
    , m_pstrDisplay(pstrDisplay)
{
}

void OTNym_or_SymmetricKey::swap(OTNym_or_SymmetricKey& other)
{
    if (&other != this) {
        std::swap(m_pNym, other.m_pNym);
        std::swap(m_pKey, other.m_pKey);
        std::swap(m_pPassword, other.m_pPassword);
        std::swap(m_bCleanupPassword, other.m_bCleanupPassword);
        std::swap(m_pstrDisplay, other.m_pstrDisplay);
    }
}

OTNym_or_SymmetricKey& OTNym_or_SymmetricKey::operator=(
    OTNym_or_SymmetricKey other)  // passed by value.
{
    // swap this with other
    swap(other);

    // by convention, always return *this
    return *this;
}

// This is just a wrapper class.
void OTNym_or_SymmetricKey::Release()  // Someday make this virtual, if we ever
                                       // subclass it.
{
    OT_ASSERT((m_pNym != nullptr) || (m_pKey != nullptr));  // m_pPassword is
                                                            // optional

    Release_Nym_or_SymmetricKey();

    // no need to call ot_super::Release here, since this class has no
    // superclass.
}

// This is just a wrapper class. (Destructor doesn't call this because we aren't
// the owner.)
void OTNym_or_SymmetricKey::Release_Nym_or_SymmetricKey()
{
    if (nullptr != m_pNym) {
        //      m_pNym->Release(); // no such call on OTPseudonym. (Otherwise it
        // wouldn't be commented out.)
    }

    if (nullptr != m_pKey) m_pKey->Release();

    if (nullptr != m_pPassword) {
        m_pPassword->zeroMemory();

        if (m_bCleanupPassword)  // Only in cases where *this is the actual
                                 // owner
                                 // of m_pPassword.
        {
            delete m_pPassword;
            m_pPassword = nullptr;
        }
    }
}

bool OTNym_or_SymmetricKey::CompareID(const OTNym_or_SymmetricKey& rhs) const
{
    auto idTHIS = Identifier::Factory(), idRHS = Identifier::Factory();

    GetIdentifier(idTHIS);
    rhs.GetIdentifier(idRHS);

    return (idTHIS == idRHS);
}

void OTNym_or_SymmetricKey::GetIdentifier(Identifier& theIdentifier) const
{
    if (IsNym()) {
        m_pNym->GetIdentifier(theIdentifier);
    } else if (IsKey()) {
        m_pKey->GetIdentifier(theIdentifier);
    } else {
        OT_FAIL;  // should never happen
    }
}

void OTNym_or_SymmetricKey::GetIdentifier(String& strIdentifier) const
{
    if (IsNym()) {
        m_pNym->GetIdentifier(strIdentifier);
    } else if (IsKey()) {
        m_pKey->GetIdentifier(strIdentifier);
    } else {
        OT_FAIL;  // should never happen
    }
}

bool OTNym_or_SymmetricKey::Open_or_Decrypt(
    const OTEnvelope& inputEnvelope,
    String& strOutput,
    const String& pstrDisplay)
{
    const char* szFunc = "OTNym_or_SymmetricKey::Open_or_Decrypt";

    bool bSuccess = false;
    bool bHadToInstantiatePassword = false;

    // Decrypt/Open inputEnvelope into strOutput
    //
    if (IsNym())  // *this is a Nym.
    {
        bSuccess = (const_cast<OTEnvelope&>(inputEnvelope))
                       .Open(*(GetNym()), strOutput);
    } else if (IsKey())  // *this is a symmetric key, possibly with a
                         // password already as well.
    {
        OTPassword* pPassword = nullptr;

        if (HasPassword())  // Password is already available. Let's use it.
            pPassword = GetPassword();
        else  // NO PASSWORD already? let's collect it from the user...
        {
            const auto strDisplay = String::Factory(
                (!pstrDisplay.Exists()) ? szFunc : pstrDisplay.Get());
            // NOTE: m_pstrDisplay overrides this below.

            // returns a text OTPassword, or nullptr.
            //
            pPassword = crypto::key::LegacySymmetric::GetPassphraseFromUser(
                (!m_pstrDisplay->Exists())
                    ? strDisplay
                    : m_pstrDisplay);  // bool bAskTwice=false

            if (nullptr == pPassword)  // Unable to retrieve passphrase from
                                       // user.
            {
                otOut << szFunc
                      << ": Failed trying to retrieve passphrase for key. "
                         "Returning false.\n";
                return false;
            } else  // OTNym_or_SymmetricKey stores this, if it creates it.
                // (And cleans it up on destruction, IF it created it.)
                //
                bHadToInstantiatePassword = true;
        }

        bSuccess = (const_cast<OTEnvelope&>(inputEnvelope))
                       .Decrypt(strOutput, *(GetKey()), *pPassword);

        // We only set this, presuming we have to at all, if it was a success.
        if (bHadToInstantiatePassword) {
            if (bSuccess) {
                m_bCleanupPassword = true;
                m_pPassword = pPassword;  // Not bothering to cleanup whatever
                                          // was here before, since we only end
                                          // up here if m_pPassword was set to
                // nullptr (according to above logic...)
            } else  // We instantiated the password, but the decrypt failed.
                    // (Need to cleanup the password then.)
            {
                delete pPassword;
                pPassword = nullptr;
            }
        }
    }
    // else ? should never happen.

    return bSuccess;
}

bool OTNym_or_SymmetricKey::Seal_or_Encrypt(
    OTEnvelope& outputEnvelope,
    const String& strInput,
    const String& pstrDisplay)
{
    const char* szFunc = "OTNym_or_SymmetricKey::Seal_or_Encrypt";

    bool bSuccess = false;
    bool bHadToInstantiatePassword = false;

    // Encrypt/Seal strInput into outputEnvelope
    //
    if (IsNym()) {
        bSuccess = outputEnvelope.Seal(*(GetNym()), strInput);
    } else if (IsKey()) {
        OTPassword* pPassword = nullptr;

        if (HasPassword())  // Password is already available. Let's use it.
            pPassword = GetPassword();
        else  // no password? let's collect it from the user...
        {
            const auto strDisplay = String::Factory(
                (!pstrDisplay.Exists()) ? szFunc : pstrDisplay.Get());
            // NOTE: m_pstrDisplay overrides this below.

            // returns a text OTPassword, or nullptr.
            //
            pPassword = crypto::key::LegacySymmetric::GetPassphraseFromUser(
                (!m_pstrDisplay->Exists())
                    ? strDisplay
                    : m_pstrDisplay);  // bool bAskTwice=false

            if (nullptr == pPassword)  // Unable to retrieve passphrase from
                                       // user.
            {
                otOut << szFunc
                      << ": Failed trying to retrieve passphrase for key. "
                         "Returning false.\n";
                return false;
            } else  // OTNym_or_SymmetricKey stores this, if it creates it.
                // (And cleans it up on destruction, IF it created it.)
                //
                bHadToInstantiatePassword = true;
        }

        bSuccess = outputEnvelope.Encrypt(strInput, *(GetKey()), *pPassword);

        // We only set this, presuming we have to at all, if it was a success.
        if (bHadToInstantiatePassword) {
            if (bSuccess) {
                m_bCleanupPassword = true;
                m_pPassword = pPassword;  // Not bothering to cleanup whatever
                                          // was here before, since we only end
                                          // up here if m_pPassword was set to
                // nullptr (according to above logic...)
            } else  // We instantiated the password, but the encrypt failed.
                    // (Need to cleanup the password then.)
            {
                delete pPassword;
                pPassword = nullptr;
            }
        }
    }
    // else ? should never happen.

    return bSuccess;
}

}  // namespace opentxs
