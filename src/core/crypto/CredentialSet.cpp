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

// A nym contains a list of credential sets.
// The whole purpose of a Nym is to be an identity, which can have
// master credentials.
//
// Each CredentialSet contains list of Credentials. One of the
// Credentials is a MasterCredential, and the rest are ChildCredentials
// signed by the MasterCredential.
//
// A Credential may contain keys, in which case it is a KeyCredential.
//
// Credentials without keys might be an interface to a hardware device
// or other kind of external encryption and authentication system.
//
// Non-key Credentials are not yet implemented.
//
// Each KeyCredential has 3 OTKeypairs: encryption, signing, and authentication.
// Each OTKeypair has 2 OTAsymmetricKeys (public and private.)
//
// A MasterCredential must be a KeyCredential, and is only used to sign
// ChildCredentials
//
// ChildCredentials are used for all other actions, and never sign other
// Credentials

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/crypto/CredentialSet.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/crypto/ChildKeyCredential.hpp>
#include <opentxs/core/util/Tag.hpp>

#include <memory>
#include <algorithm>

// '0' for cKeyType means, theSignature MUST have metadata in order for ANY keys
// to be returned, and it MUST match.
// Whereas if you pass 'A', 'E', or 'S' for cKeyType, that means it can ONLY
// return authentication, encryption, or signing
// keys. It also means that metadata must match IF it's present, but that
// otherwise, if theSignature has no metadata at
// all, then it will still be a "presumed match" and returned as a possibility.
// (With the 'A', 'E', or 'S' enforced.)
//

namespace opentxs
{

bool CredentialSet::HasPublic() const
{
    for (const auto& it : m_mapCredentials)
    {
        const Credential* pSub = it.second;
        OT_ASSERT(nullptr != pSub);

        const String::Map & theMap = pSub->GetPublicMap();

        if (theMap.size() > 0)
            return true;
    }

    return false;
}

bool CredentialSet::HasPrivate() const
{
    for (const auto& it : m_mapCredentials)
    {
        const Credential* pSub = it.second;
        OT_ASSERT(nullptr != pSub);

        const String::Map & theMap = pSub->GetPrivateMap();

        if (theMap.size() > 0)
            return true;
    }

    return false;
}

int32_t CredentialSet::GetPublicKeysBySignature(
    listOfAsymmetricKeys& listOutput, const OTSignature& theSignature,
    char cKeyType) const // 'S' (signing key) or 'E' (encryption key)
                         // or 'A' (authentication key)
{
    int32_t nCount = 0;
    for (const auto& it : m_mapCredentials) {
        const Credential* pSub = it.second;
        OT_ASSERT(nullptr != pSub);

        const ChildKeyCredential* pKey = dynamic_cast<const ChildKeyCredential*>(pSub);
        if (nullptr == pKey)
            continue; // Skip all non-key credentials. We're looking for keys.

        const int32_t nTempCount =
            pKey->GetPublicKeysBySignature(listOutput, theSignature, cKeyType);
        nCount += nTempCount;
    }
    return nCount;
}

bool CredentialSet::VerifyInternally() const
{

    Identifier theActualMasterCredID;
    theActualMasterCredID.CalculateDigest(m_MasterCredential.GetPubCredential());
    const String strActualMasterCredID(theActualMasterCredID);

    if (!m_strNymID.Compare(m_MasterCredential.GetNymID())) {
        otOut << __FUNCTION__
              << ": NymID did not match its "
                 "counterpart in m_MasterCredential (failed to verify): " << GetNymID()
              << "\n";
        return false;
    }

    if (!m_strMasterCredID.Compare(strActualMasterCredID)) {
        otOut << __FUNCTION__
              << ": Master Credential ID did not match its "
                 "counterpart in m_MasterCredential:\nExpected Master Credential ID: "
              << GetMasterCredID()
              << "\n "
                 "Hash of m_MasterCredential contents: " << strActualMasterCredID
              << "\nContents:\n" << m_MasterCredential.GetPubCredential() << "\n";
        return false;
    }

    if (!const_cast<MasterCredential&>(m_MasterCredential).VerifyContract()) {
        otOut << __FUNCTION__
              << ": Master Credential failed to verify: " << GetMasterCredID()
              << "\nNymID: " << GetNymID() << "\n";
        return false;
    }

    for (const auto& it : m_mapCredentials) {
        std::string str_sub_id = it.first;
        Credential* pSub = it.second;
        OT_ASSERT(nullptr != pSub);

        if (!pSub->VerifyContract()) {
            otOut << __FUNCTION__
                  << ": Child credential failed to verify: " << str_sub_id
                  << "\nNymID: " << GetNymID() << "\n";
            return false;
        }
    }

    return true;
}

bool CredentialSet::VerifyAgainstSource() const
{
    // * Any MasterCredential must (at some point, and/or regularly) verify against
    // its own source.
    //
    if (!m_MasterCredential.VerifyAgainstSource()) {
        otWarn
            << __FUNCTION__
            << ": Failed verifying master credential against its own source.\n";
        return false;
    }
    // NOTE: This spot will have a significant delay, TODO OPTIMIZE. Performing
    // a Freenet lookup, or DNS, etc,
    // will introduce delay inside the call VerifyAgainstSource. Therefore in
    // the int64_t term, we must have a
    // separate server process which will verify identities for some specified
    // period of time (specified in
    // their credentials I suppose...) That way, when we call
    // VerifyAgainstSource, we are verifying against
    // some server-signed authorization, based on a lookup that some separate
    // process did within the past
    // X allowed time, such that the lookup is still considered valid (without
    // having to lookup every single
    // time, which is untenable.)

    return true;
}

const String& CredentialSet::GetNymID() const
{
    return m_strNymID;
}

const String& CredentialSet::GetSourceForNymID() const
{
    return m_strSourceForNymID;
}

/// This sets m_strSourceForNymID.
/// This also sets m_strNymID, which is always a hash of strSourceForNymID.
///
void CredentialSet::SetSourceForNymID(const String& strSourceForNymID)
{
    m_strSourceForNymID = strSourceForNymID;

    //  Now re-calculate the NymID...
    //
    m_strNymID.Release();
    Identifier theTempID;
    const bool bCalculate = theTempID.CalculateDigest(m_strSourceForNymID);
    OT_ASSERT(bCalculate);
    theTempID.GetString(m_strNymID);

    m_MasterCredential.SetNymIDandSource(
        m_strNymID, m_strSourceForNymID); // The key in here must somehow verify
                                          // against its own source.

    // Success! By this point, m_strSourceForNymID and m_strNymID
    // are both set.
}

const String& CredentialSet::GetPubCredential() const
{
    return m_MasterCredential.GetPubCredential();
}

const String& CredentialSet::GetPriCredential() const
{
    return m_MasterCredential.GetPriCredential();
}

bool CredentialSet::SetPublicContents(const String::Map& mapPublic)
{
    return m_MasterCredential.SetPublicContents(mapPublic);
}

bool CredentialSet::SetPrivateContents(const String::Map& mapPrivate)
{
    return m_MasterCredential.SetPrivateContents(mapPrivate);
}

// private
CredentialSet::CredentialSet()
    : m_MasterCredential(*this)
{
}

CredentialSet::CredentialSet(
    const NymParameters& nymParameters,
    const OTPasswordData* pPWData,
    const String* psourceForNymID)
    : m_MasterCredential(*this, nymParameters)
{
    SetSourceForNymID(m_MasterCredential.GetNymIDSource()); // This also recalculates and sets  ** m_strNymID **

    OTPasswordData thePWData(
        "Signing new master credential... CredentialSet::CredentialSet()");

    // Using m_MasterCredential's actual signing key to sign "m_MasterCredential the
    // contract."
    //
    SignNewMaster(nullptr == pPWData ? &thePWData : pPWData);

    AddNewChildKeyCredential(nymParameters, pPWData);
}

CredentialSet::CredentialSet(Credential::CredentialType masterType)
    : m_MasterCredential(*this, masterType)
{
}

void CredentialSet::SetMasterCredID(const String& strID)
{
    m_strMasterCredID = strID;
}

const String& CredentialSet::GetMasterCredID() const
{
    return m_strMasterCredID;
}

// static  (Caller is responsible to delete.)
CredentialSet* CredentialSet::LoadMaster(
    const String& strNymID, // Caller is responsible to delete, in both
                            // CreateMaster and LoadMaster.
    const String& strMasterCredID,
    const Credential::CredentialType theType,
    const OTPasswordData* pPWData)
{
    CredentialSet* pCredential = new CredentialSet;
    std::unique_ptr<CredentialSet> theCredentialAngel(pCredential);
    OT_ASSERT(nullptr != pCredential);

    OTPasswordData thePWData("Loading master credential. (static 1.)");
    const bool bLoaded = pCredential->Load_Master(
        strNymID, strMasterCredID, theType, (nullptr == pPWData) ? &thePWData : pPWData);
    if (!bLoaded) {
        otErr << __FUNCTION__ << ": Failed trying to load master credential "
                                 "from local storage. 1\n";
        return nullptr;
    }

    return theCredentialAngel.release();
}

// static  (Caller is responsible to delete.)
CredentialSet* CredentialSet::LoadMasterFromString(
    const String& strInput,
    const String& strNymID, // Caller is responsible to delete, in both
                            // CreateMaster and LoadMaster.
    const String& strMasterCredID,
    const Credential::CredentialType theType,
    OTPasswordData* pPWData,
    const OTPassword* pImportPassword)
{
    CredentialSet* pCredential = new CredentialSet;
    std::unique_ptr<CredentialSet> theCredentialAngel(pCredential);
    OT_ASSERT(nullptr != pCredential);

    OTPasswordData thePWData(nullptr == pImportPassword
                                 ? "Enter wallet master passphrase."
                                 : "Enter passphrase for exported Nym.");
    const bool bLoaded = pCredential->Load_MasterFromString(
        strInput, strNymID, strMasterCredID, theType,
        (nullptr == pPWData) ? &thePWData : pPWData, pImportPassword);
    if (!bLoaded) {
        otErr << __FUNCTION__
              << ": Failed trying to load master credential from string. 2\n";
        return nullptr;
    }

    return theCredentialAngel.release();
}

// called by CredentialSet::CreateMaster
bool CredentialSet::SignNewMaster(const OTPasswordData* pPWData)
{
    OTPasswordData thePWData(
        "Signing new master credential... CredentialSet::SignNewMaster");

    m_MasterCredential.StoreAsPublic(); // So the version we create here only contains
                                 // public keys, not private.
    const bool bSignedPublic = m_MasterCredential.Sign(
        m_MasterCredential, nullptr == pPWData ? &thePWData : pPWData);
    if (bSignedPublic) {
        m_MasterCredential.SaveContract();
        // THE OFFICIAL "PUBLIC CREDENTIAL" FOR THE MASTER KEY
        // (Copied from the raw contents here into a member variable for
        // safe-keeping.)
        // Future verifiers can hash it and the output should match the master
        // credential ID.
        //
        String strPublicCredential;

        if (m_MasterCredential.SaveContractRaw(strPublicCredential)) {
            m_MasterCredential.SetContents(strPublicCredential); // <=== The "master
                                                          // public credential"
                                                          // string.
            // NEW MASTER CREDENTIAL ID.
            //
            // Only now can we calculate the master key's ID, since the ID is a
            // hash of
            // the contents.
            //
            Identifier theNewID;
            m_MasterCredential.CalculateContractID(theNewID);
            m_MasterCredential.SetIdentifier(theNewID); // Usually this will be set
                                                 // based on an expected value
                                                 // from above, then loaded from
                                                 // storage, and then verified
                                                 // against what was actually
                                                 // loaded (by hashing it.)

            const String strMasterCredID(theNewID);
            SetMasterCredID(
                strMasterCredID); // <=== Master Credential ID is now set.
        }
        else {
            otErr << "In " << __FILE__ << ", line " << __LINE__
                  << ": Failed calling m_MasterCredential.SaveContractRaw 1.\n";
            return false;
        }
        // THE PRIVATE KEYS
        //
        // Next, we sign / save it again, this time in its private form, and
        // also
        // m_MasterCredential.m_strContents and m_strIDsOnly will be contained within
        // that
        // private form along with the private keys.
        //
        m_MasterCredential.ReleaseSignatures(); // This time we'll sign it in private
                                         // mode.
        const bool bSignedPrivate = m_MasterCredential.Sign(
            m_MasterCredential, nullptr == pPWData ? &thePWData : pPWData);
        if (bSignedPrivate) {
            m_MasterCredential.SaveContract();

            m_MasterCredential.SetMetadata();
        }
        else {
            otErr << "In " << __FILE__ << ", line " << __LINE__
                  << ": Failed trying to sign the master private credential.\n";
            return false;
        }
    }
    else {
        otErr << "In " << __FILE__ << ", line " << __LINE__
              << ": Failed trying to sign the master public credential.\n";
        return false;
    }

    return true;
}

// When exporting a Nym, you don't want his private keys encrypted to the cached
// key
// for the wallet, so you have to load them up, and then pause OTCachedKey, and
// then
// save them to string again, re-encrypting them to the export passphrase (and
// not to
// any "master key" from the wallet.) And you have to release all the signatures
// on
// the private credentials, since the private info is being re-encrypted, and
// re-sign
// them all. Joy.
//
bool CredentialSet::ReEncryptPrivateCredentials(
    const OTPassword& theExportPassword, bool bImporting)
{
    if (m_MasterCredential.GetPrivateMap().size() > 0) {
        OTPasswordData thePWData(
            bImporting ? "2 Enter passphrase for the Nym being imported."
                       : "2 Enter new passphrase for exported Nym.");

        // Re-encrypt the private keys in the master credential. (THEN sign.)
        //
        const bool bReEncryptMaster =
            m_MasterCredential.ReEncryptKeys(theExportPassword, bImporting);
        bool bSignedMaster = false;

        if (bReEncryptMaster) {
            m_MasterCredential.ReleaseSignatures(); // This time we'll sign it in
                                             // private mode.
            bSignedMaster = m_MasterCredential.Sign(m_MasterCredential, &thePWData);
        }
        else {
            otErr << "In " << __FILE__ << ", line " << __LINE__
                  << ": Failed trying to re-encrypt the private master credential.\n";
            return false;
        }

        if (bSignedMaster) {
            m_MasterCredential.SaveContract();
            m_MasterCredential.SetMetadata(); // todo: can probably remove this, since
                                       // it was set based on public info when
                                       // the key was first created.

            for (auto& it : m_mapCredentials) {
                Credential* pSub = it.second;
                OT_ASSERT(nullptr != pSub);

                ChildKeyCredential* pKey = dynamic_cast<ChildKeyCredential*>(pSub);
                if (nullptr == pKey) continue;

                const bool bReEncryptChildKeyCredential =
                    pKey->ReEncryptKeys(theExportPassword, bImporting);
                bool bSignedChildCredential = false;

                if (bReEncryptChildKeyCredential) {
                    pKey->ReleaseSignatures();
                    bSignedChildCredential = pKey->Sign(*pKey, &thePWData);
                }
                else {
                    otErr << "In " << __FILE__ << ", line " << __LINE__
                          << ": Failed trying to re-encrypt the private "
                             "child key credential.\n";
                    return false;
                }

                if (bSignedChildCredential) {
                    pKey->SaveContract();
                    pKey->SetMetadata(); // todo: can probably remove this,
                                         // since it was set based on public
                                         // info when the key was first created.
                }
                else {
                    otErr << "In " << __FILE__ << ", line " << __LINE__
                          << ": Failed trying to re-sign the private child key credential.\n";
                    return false;
                }
            }

            return true; // <=== Success.
        }
        else
            otErr << "In " << __FILE__ << ", line " << __LINE__
                  << ": Failed trying to re-sign the master private "
                     "credential.\n";
    }
    else
        otErr << "In " << __FILE__ << ", line " << __LINE__
              << ": Failed: There is no private info on this master "
                 "credential.\n";

    return false;
}

bool CredentialSet::SignNewChildCredential(Credential& theChildCred,
                                        Identifier& theChildCredID_out,
                                        const OTPasswordData* pPWData)
{
    OTPasswordData thePWData(
        "Signing new child credential... CredentialSet::SignNewChildCredential");

    // First, we store the child credential itself with its basic info.
    // This version is signed with the master credential. Then we save a copy of it
    // in a member variable for safe-keeping, m_strMasterSigned. Next, we
    // save a "public" version of the child credential (the official version)
    // which will include m_strMasterSigned inside it, and be signed by the
    // child key credential. This version may not need to duplicate all that data, especially
    // if we end up just having to verify it twice as a result. So I might have
    // the public version be sparse (other than including the master signed
    // version,
    // and being signed by the child key credential.)
    // Though with many child credentials, there will ONLY be the master-signed
    // version,
    // and that WILL be the public version. Only with child key credentials will that be
    // different!
    //
    ChildKeyCredential* pChildKeyCredential = dynamic_cast<ChildKeyCredential*>(&theChildCred);
    const bool bIsChildKeyCredential = (nullptr != pChildKeyCredential); // It's not just any
                                                 // child credential -- it's a
                                                 // child key credential!

    // If it's not a child key credential, but rather, a normal child credential with no keys,
    // then it doesn't need to contain a "master signed" version,
    // since the entire child credential will already be master signed, since
    // there's no child key credential to sign in that case.
    if (!bIsChildKeyCredential) theChildCred.SetMasterSigned(String(""));

    // ELSE It's a child key credential...
    else // Child key credentials must be self-signed, and must contain a master-signed
         // version of themselves where the data is actually stored.
    {
        pChildKeyCredential->StoreAsMasterSigned(); // So the version we create here only
                                        // contains public keys, not private.
                                        // (And so it won't include
        // the "master signed" version in what it stores, since that's what
        // we're creating now.)

        const bool bMasterSigned = m_MasterCredential.Sign(
            *pChildKeyCredential, nullptr == pPWData ? &thePWData : pPWData);
        if (!bMasterSigned) {
            otErr << "In " << __FILE__ << ", line " << __LINE__
                  << ": Failed calling m_MasterCredential.Sign(*pChildKeyCredential) "
                     "after StoreAsMasterSigned.\n";
            return false;
        }
        else {
            pChildKeyCredential->SaveContract();

            // Make a copy of the "master signed" version of the public
            // child credential.
            //
            String strMasterSigned;

            if (pChildKeyCredential->SaveContractRaw(strMasterSigned)) // <=== The "master
                                                           // signed" version of
                                                           // the "public
                                                           // credential"
                                                           // string. Captured
                                                           // here
                pChildKeyCredential->SetMasterSigned(strMasterSigned); // so that the
                                                           // (child key credential-signed)
                                                           // public version of
                                                           // the child credential
                                                           // will contain it.
            else {
                otErr << "In " << __FILE__ << ", line " << __LINE__
                      << ": Failed calling pChildKeyCredential->SaveContractRaw 1.\n";
                return false;
            }
        }

        pChildKeyCredential->ReleaseSignatures();
    }
    theChildCred.StoreAsPublic(); // So the version we create here only contains
                                // public keys, not private.

    // Here, dynamic cast theChildCred to a child key credential and if successful, use it to
    // sign itself.
    // Otherwise, sign it with the master. (If it's a real child key credential, then it will
    // contain the
    // master-signed version, and it will be signed with itself, its own child key credential.
    // Whereas if
    // it's a child credential that is NOT a child key credential, such as a Google Authenticator
    // or some other
    // 3rd-party authentication, then it will HAVE no key to sign itself with,
    // since its primary
    // purpose in that case is to provide some OTHER authentication info INSTEAD
    // of a key.
    // So in that case, it must be signed by the master.)
    //
    bool bSignedPublic = false;

    if (bIsChildKeyCredential) // If it's a child key credential, its keys are already generated by the
                   // time we got here. So use it to sign its own public
                   // version.
        bSignedPublic = pChildKeyCredential->Sign(
            theChildCred, nullptr == pPWData ? &thePWData : pPWData);
    else // It's not a child key credential, but some other conventional child credential. So we
         // sign with master key, since that's all we've got.
        bSignedPublic = m_MasterCredential.Sign(
            theChildCred, nullptr == pPWData ? &thePWData : pPWData);

    if (!bSignedPublic) {
        otErr
            << "In " << __FILE__ << ", line " << __LINE__
            << ": Failed trying to sign the public child credential or child key credential.\n";
        return false;
    }
    else {
        theChildCred.SaveContract();
        // THE OFFICIAL "PUBLIC CREDENTIAL STRING" FOR THIS NEW SUB-CREDENTIAL
        // Set it aside for safe-keeping as the official contents, hashable to
        // form
        // the ID for this sub-credential.
        //
        String strPublicCredential;
        if (theChildCred.SaveContractRaw(strPublicCredential)) {
            theChildCred.SetContents(strPublicCredential); // <=== The "public
                                                         // credential" string
                                                         // aka the contents.
            // NEW SUB-CREDENTIAL ID.
            //
            // Only now that the contents have been set, can we calculate the
            // ID, which
            // is a hash of those contents. (Credential ID is a hash of
            // GetPubCredential instead
            // of m_strRawXML as most contracts would use, since we only want to
            // use the PUBLIC
            // info for calculating the ID, not the private info.)
            //
            theChildCred.CalculateContractID(theChildCredID_out);
            theChildCred.SetIdentifier(theChildCredID_out);
        }
        else {
            otErr << "In " << __FILE__ << ", line " << __LINE__
                  << ": Failed calling theChildCred.SaveContractRaw.\n";
            return false;
        }
        // CREATE THE PRIVATE FORM.
        //
        // Next, we sign / save it again, this time in its private form, and
        // also
        // theChildCred.m_strContents will be contained within that private form,
        // along with the private keys.
        //
        theChildCred.ReleaseSignatures(); // This time we'll sign it in private
                                        // mode.
        bool bSignedPrivate = false;

        if (bIsChildKeyCredential) // If it's a child key credential, its keys are already generated by
                       // the time we got here. So use it to sign its own
                       // private version.
            bSignedPrivate = pChildKeyCredential->Sign(
                theChildCred, nullptr == pPWData ? &thePWData : pPWData);
        else // It's not a child key credential, but some other conventional child credential. So
             // we sign the private info with the master key, since that's all
             // we've got.
            bSignedPrivate = m_MasterCredential.Sign(
                theChildCred, nullptr == pPWData ? &thePWData : pPWData);

        if (bSignedPrivate)
            theChildCred.SaveContract();
        else {
            otErr << "In " << __FILE__ << ", line " << __LINE__
                  << ": Failed trying to sign the private child credential.\n";
            return false;
        }
    }

    return true;
}

bool CredentialSet::Load_MasterFromString(const String& strInput,
                                         const String& strNymID,
                                         const String& strMasterCredID,
                                         const Credential::CredentialType theType,
                                         const OTPasswordData*,
                                         const OTPassword* pImportPassword)
{
    m_strNymID = strNymID;
    m_strMasterCredID = strMasterCredID;

    m_MasterCredential.SetIdentifier(strMasterCredID);

    // m_MasterCredential and the child key credentials all have a pointer to "owner" (who is *this)
    // and so I can set pImportPassword onto a member variable, perform the
    // load,
    // and then set that member nullptr again. During the call to
    // LoadContractFromString,
    // m_MasterCredential can reference m_pOwner->GetImportPassword() and if it's not
    // nullptr,
    // use it instead of using the wallet's cached master key. After all, if
    // it's not
    // nullptr here, that's why it was passed in.
    //

    SetImportPassword(pImportPassword); // might be nullptr.

    const bool bLoaded = m_MasterCredential.LoadContractFromString(strInput);
    if (!bLoaded) {
        otErr << __FUNCTION__
              << ": Failed trying to load master credential from string.\n";
        return false;
    }

    SetImportPassword(nullptr); // It was only set during the
                                // m_MasterCredential.LoadContractFromString (which
                                // references it.)

    m_strNymID = m_MasterCredential.GetNymID();
    m_strSourceForNymID = m_MasterCredential.GetNymIDSource();

    ClearChildCredentials(); // The master is loaded first, and then any
                           // child credentials. So this is probably already
                           // empty. Just looking ahead.

    m_MasterCredential.SetMetadata();

    return true;
}

bool CredentialSet::Load_Master(const String& strNymID,
                               const String& strMasterCredID,
                               const Credential::CredentialType theType,
                               const OTPasswordData* pPWData)
{

    std::string str_Folder =
        OTFolders::Credential().Get(); // Try private credential first. If that
                                       // fails, then public.

    if (false ==
        OTDB::Exists(str_Folder, strNymID.Get(), strMasterCredID.Get())) {
        str_Folder = OTFolders::Pubcred().Get();

        if (false ==
            OTDB::Exists(str_Folder, strNymID.Get(), strMasterCredID.Get())) {
            otErr << __FUNCTION__ << ": Failure: Master Credential "
                  << strMasterCredID << " doesn't exist for Nym " << strNymID
                  << "\n";
            return false;
        }
    }

    String strFileContents(OTDB::QueryPlainString(str_Folder, strNymID.Get(),
                                                  strMasterCredID.Get()));
    if (!strFileContents.Exists()) {
        otErr << __FUNCTION__ << ": Failed trying to load master credential "
                                 "from local storage.\n";
        return false;
    }

    if (false ==
        strFileContents.DecodeIfArmored()) // bEscapedIsAllowed=true by default.
    {
        otErr << __FUNCTION__ << ": File contents apparently were encoded and "
                                 "then failed decoding. Contents: \n"
              << strFileContents << "\n";
        return false;
    }

    return Load_MasterFromString(strFileContents, strNymID, strMasterCredID, theType,
                                 pPWData);
}

bool CredentialSet::LoadChildKeyCredentialFromString(const String& strInput,
                                        const String& strSubID,
                                        const Credential::CredentialType theType,
                                        const OTPassword* pImportPassword)
{
    // Make sure it's not already there.
    //
    auto it = m_mapCredentials.find(strSubID.Get());
    if (m_mapCredentials.end() != it) // It was already there. (Reload it.)
    {
        otErr << __FUNCTION__ << ": Warning: Deleting and re-loading "
                                 "keyCredential that was already loaded.\n";
        Credential* pSub = it->second;
        OT_ASSERT(nullptr != pSub);
        delete pSub;
        m_mapCredentials.erase(it);
    }

    ChildKeyCredential* pSub = new ChildKeyCredential(*this);
    std::unique_ptr<ChildKeyCredential> theSubAngel(pSub);
    OT_ASSERT(nullptr != pSub);

    pSub->SetIdentifier(strSubID);
    pSub->SetNymIDandSource(GetNymID(),
                            GetSourceForNymID()); // Set NymID and source
                                                  // string that hashes to
                                                  // it.
    pSub->SetMasterCredID(GetMasterCredID());     // Set master credential ID
                                                  // (onto this new
                                                  // child credential...)

    SetImportPassword(pImportPassword); // might be nullptr.

    if (!pSub->LoadContractFromString(strInput)) {
        otErr << __FUNCTION__
              << ": Failed trying to load keyCredential from string.\n";
        return false;
    }

    SetImportPassword(nullptr); // Only set int64_t enough for
                                // LoadContractFromString above to use it.

    pSub->SetMetadata();

    m_mapCredentials.insert(std::pair<std::string, Credential*>(
        strSubID.Get(), theSubAngel.release()));

    return true;
}

bool CredentialSet::LoadChildKeyCredential(const String& strSubID, const Credential::CredentialType theType)
{

    std::string str_Folder =
        OTFolders::Credential().Get(); // Try private credential first. If that
                                       // fails, then public.

    if (!OTDB::Exists(str_Folder, GetNymID().Get(), strSubID.Get())) {
        str_Folder = OTFolders::Pubcred().Get();

        if (false ==
            OTDB::Exists(str_Folder, GetNymID().Get(), strSubID.Get())) {
            otErr << __FUNCTION__ << ": Failure: Key Credential " << strSubID
                  << " doesn't exist for Nym " << GetNymID() << "\n";
            return false;
        }
    }

    String strFileContents(
        OTDB::QueryPlainString(str_Folder, GetNymID().Get(), strSubID.Get()));

    if (!strFileContents.Exists()) {
        otErr << __FUNCTION__
              << ": Failed trying to load keyCredential from local storage.\n";
        return false;
    }

    if (false ==
        strFileContents.DecodeIfArmored()) // bEscapedIsAllowed=true by default.
    {
        otErr << __FUNCTION__ << ": File contents apparently were encoded and "
                                 "then failed decoding. Contents: \n"
              << strFileContents << "\n";
        return false;
    }

    return LoadChildKeyCredentialFromString(strFileContents, strSubID, theType);
}

bool CredentialSet::LoadCredentialFromString(
    const String& strInput, const String& strSubID,
    const OTPassword* pImportPassword)
{
    // Make sure it's not already there.
    //
    auto it = m_mapCredentials.find(strSubID.Get());
    if (m_mapCredentials.end() != it) // It was already there. (Reload it.)
    {
        otErr << __FUNCTION__ << ": Warning: Deleting and re-loading "
                                 "child credential that was already loaded.\n";
        Credential* pSub = it->second;
        OT_ASSERT(nullptr != pSub);
        delete pSub;
        m_mapCredentials.erase(it);
    }

    Credential* pSub = new Credential(*this);
    std::unique_ptr<Credential> theSubAngel(pSub);
    OT_ASSERT(nullptr != pSub);

    pSub->SetIdentifier(strSubID);
    pSub->SetNymIDandSource(GetNymID(),
                            GetSourceForNymID()); // Set NymID and source
                                                  // string that hashes to
                                                  // it.
    pSub->SetMasterCredID(GetMasterCredID());     // Set master credential ID
                                                  // (onto this new
                                                  // child credential...)

    SetImportPassword(pImportPassword); // might be nullptr.

    if (!pSub->LoadContractFromString(strInput)) {
        otErr << __FUNCTION__
              << ": Failed trying to load child credential from string.\n";
        return false;
    }

    SetImportPassword(nullptr); // This is only set int64_t enough for
                                // LoadContractFromString to use it. (Then
                                // back to nullptr.)

    m_mapCredentials.insert(std::pair<std::string, Credential*>(
        strSubID.Get(), theSubAngel.release()));

    return true;
}

bool CredentialSet::LoadCredential(const String& strSubID)
{

    std::string str_Folder =
        OTFolders::Credential().Get(); // Try private credential first. If that
                                       // fails, then public.

    if (!OTDB::Exists(str_Folder, GetNymID().Get(), strSubID.Get())) {
        str_Folder = OTFolders::Pubcred().Get();

        if (false ==
            OTDB::Exists(str_Folder, GetNymID().Get(), strSubID.Get())) {
            otErr << __FUNCTION__ << ": Failure: Credential " << strSubID
                  << " doesn't exist for Nym " << GetNymID() << "\n";
            return false;
        }
    }

    String strFileContents(
        OTDB::QueryPlainString(str_Folder, GetNymID().Get(), strSubID.Get()));
    if (!strFileContents.Exists()) {
        otErr << __FUNCTION__
              << ": Failed trying to load child credential from local storage.\n";
        return false;
    }

    if (false ==
        strFileContents.DecodeIfArmored()) // bEscapedIsAllowed=true by default.
    {
        otErr << __FUNCTION__ << ": File contents apparently were encoded and "
                                 "then failed decoding. Contents: \n"
              << strFileContents << "\n";
        return false;
    }

    return LoadCredentialFromString(strFileContents, strSubID);
}

// For adding child credentials that are specifically *child key credentials*. Meaning it will
// contain 3 keypairs: signing, authentication, and encryption.
//
bool CredentialSet::AddNewChildKeyCredential(
    const NymParameters& nymParameters,
    const OTPasswordData* pPWData,  // The master key will sign the child key credential.
    ChildKeyCredential** ppChildKeyCredential)            // output
{
    ChildKeyCredential* newChildCredential = new ChildKeyCredential(*this, nymParameters);
    OT_ASSERT(nullptr != newChildCredential);

    newChildCredential->SetNymIDandSource(GetNymID(),
                            GetSourceForNymID()); // Set NymID and source
                                                // string that hashes to
                                                // it.
    newChildCredential->SetMasterCredID(GetMasterCredID());     // Set master credential ID
                                                // (onto this new
                                                // child credential...)

    // By this point we've set up the child key credential with its NymID, the source
    // string for that NymID,
    // my master credential ID, and the public and private certs for the
    // child key credential. Now let's sign it...
    OTPasswordData thePWData(
        "Signing new child key credential... CredentialSet::AddNewChildKeyCredential");
    Identifier theChildCredID;

    // SignNewChildCredential uses m_MasterCredential's actual signing key to sign
    // "pSub the contract."
    //
    if (false ==
        SignNewChildCredential(*newChildCredential, theChildCredID,
                            nullptr == pPWData ? &thePWData : pPWData)) {
        otErr << "In " << __FILE__ << ", line " << __LINE__
            << ": Failed trying to call SignNewChildCredential\n";
        delete newChildCredential;
        newChildCredential = nullptr;
        return false;
    }

    const String strChildCredID(
        theChildCredID); // SignNewChildCredential also generates the ID.

    newChildCredential->SetMetadata();

    // ADD IT TO THE MAP
    // Only after pSub is signed and saved can we then calculate its ID and
    // use that ID
    // as the key in m_mapCredentials (with pSub being the value.)
    //
    m_mapCredentials.insert(
        std::pair<std::string, Credential*>(strChildCredID.Get(), newChildCredential));

    if (nullptr != ppChildKeyCredential) // output
    {
        *ppChildKeyCredential = newChildCredential;
    }

    return true;
}

// For adding non-key credentials, such as for 3rd-party authentication.
//
bool CredentialSet::AddNewChildCredential(
    const String::Map& mapPrivate, const String::Map& mapPublic,
    const OTPasswordData* pPWData, // The master key will sign the
                                   // child credential.
    Credential** ppChildCred)   // output
{
    Credential* pSub = new Credential(*this);
    OT_ASSERT(nullptr != pSub);

    pSub->SetNymIDandSource(m_strNymID, m_strSourceForNymID); // Set NymID and
                                                              // source string
                                                              // that hashes to
                                                              // it.
    pSub->SetMasterCredID(GetMasterCredID()); // Set master credential ID
                                              // (onto this new
                                              // child credential...)

    if (!pSub->SetPublicContents(mapPublic)) {
        otErr << "In " << __FILE__ << ", line " << __LINE__
              << ": Failed while calling pSub->SetPublicContents.\n";
        delete pSub;
        pSub = nullptr;
        return false;
    }
    else if (!pSub->SetPrivateContents(mapPrivate)) {
        otErr << "In " << __FILE__ << ", line " << __LINE__
              << ": Failed while trying to pSub->SetPrivateContents.\n";
        delete pSub;
        pSub = nullptr;
        return false;
    }
    else // By this point we've set up the child credential with its NymID, the
           // source string for that NymID,
    { // my master credential ID, and the public and private contents for the
        // child credential. Let's sign
        // it...
        OTPasswordData thePWData(
            "Signing new child credential... CredentialSet::AddNewChildCredential");
        Identifier theChildCredID;

        // SignNewChildCredential uses m_MasterCredential's actual signing key to sign
        // "pSub the contract."
        //
        if (false ==
            SignNewChildCredential(*pSub, theChildCredID,
                                 nullptr == pPWData ? &thePWData : pPWData)) {
            otErr << "In " << __FILE__ << ", line " << __LINE__
                  << ": Failed trying to call SignNewChildCredential\n";
            delete pSub;
            pSub = nullptr;
            return false;
        }

        const String strChildCredID(theChildCredID);

        // ADD IT TO THE MAP
        // Only after pSub is signed and saved can we then calculate its ID and
        // use that ID
        // as the key in m_mapCredentials (with pSub being the value.)
        //
        m_mapCredentials.insert(
            std::pair<std::string, Credential*>(strChildCredID.Get(), pSub));
        if (nullptr != ppChildCred) // output
            *ppChildCred = pSub;
        return true;
    }
}

// After calling this, you still need to save it to disk (or not.) This function
// alone doesn't save anything to disk.
//
// static
CredentialSet* CredentialSet::CreateMaster(
    const String& strSourceForNymID,
    const NymParameters& nymParameters,
    const OTPasswordData* pPWData)
{
    CredentialSet* pCredential = new CredentialSet(nymParameters);
    OT_ASSERT(nullptr != pCredential);

    pCredential->SetSourceForNymID(
        strSourceForNymID); // This also recalculates and sets  ** m_strNymID **

    OTPasswordData thePWData(
        "Signing new master credential... CredentialSet::CreateMaster");

    // Using m_MasterCredential's actual signing key to sign "m_MasterCredential the
    // contract."
    //
    if (false ==
        pCredential->SignNewMaster(nullptr == pPWData ? &thePWData
                                                    : pPWData)) {
        otErr << "In " << __FILE__ << ", line " << __LINE__
            << ": Failed trying to call pCredential->SignNewMaster\n";
        delete pCredential;
        pCredential = nullptr;
        return nullptr;
    }
    // By this point, we have instantiated a new CredentialSet, set the source
    // string, hashed that
    // source string to get the NymID for this credential, and set the public
    // and private info for
    // this credential (each a map of strings.) Since pCredential->m_MasterCredential
    // is derived from
    // KeyCredential, it also loaded up the 3 keypairs (authentication,
    // encryption, and signing.)
    // Then we signed that master key with itself, with its signing key. (It's
    // also an Contract,
    // so it can be signed.) This also calculated the new master credential ID,
    // and called
    // pCredential->SetMasterCredID. That is, the CredentialSet's "master
    // credential ID" is formed
    // as a hash of the signed contract that is its MasterCredential.
    // BUT!!! We don't want to use a hash of the private key information, since
    // others cannot verify
    // the hash without seeing our private key. We want MasterCredential to create an
    // 'official' signed
    // public version of itself, minus private keys, which is what can be sent
    // to servers and to
    // other users, and which can be hashed to form the master credential ID
    // (and verified later.)
    // ...Which is exactly what it does. Inside pCredential->SignNewMaster, a
    // public version is created
    // and signed, and set onto that master credential as m_strContents. It's then
    // re-signed as the private
    // version, which contains m_strContents in encoded form, along with the
    // private keys.
    //
    return pCredential;
}

size_t CredentialSet::GetChildCredentialCount() const
{
    return m_mapCredentials.size();
}

const Credential* CredentialSet::GetChildCredential(
    const String& strSubID, const String::List* plistRevokedIDs) const
{
    for (const auto& it : m_mapCredentials) {
        const std::string str_cred_id = it.first;
        const Credential* pSub = it.second;
        OT_ASSERT(nullptr != pSub);

        // See if pSub, with ID str_cred_id, is on plistRevokedIDs...
        //
        if (nullptr != plistRevokedIDs) {
            auto iter = std::find(plistRevokedIDs->begin(),
                                  plistRevokedIDs->end(), str_cred_id);
            if (iter != plistRevokedIDs->end()) // It was on the revoked list.
                continue;                       // Skip this revoked credential.
        }
        // At this point we know it's not on the revoked list, if one was passed
        // in.

        if (strSubID.Compare(str_cred_id.c_str())) return pSub;
    }
    return nullptr;
}

const Credential* CredentialSet::GetChildCredentialByIndex(
    int32_t nIndex) const
{
    if ((nIndex < 0) ||
        (nIndex >= static_cast<int64_t>(m_mapCredentials.size()))) {
        otErr << __FUNCTION__ << ": Index out of bounds: " << nIndex << "\n";
    }
    else {
        int32_t nLoopIndex = -1;

        for (const auto& it : m_mapCredentials) {
            const Credential* pSub = it.second;
            OT_ASSERT(nullptr != pSub);

            ++nLoopIndex; // 0 on first iteration.

            if (nIndex == nLoopIndex) return pSub;
        }
    }

    return nullptr;
}

const std::string CredentialSet::GetChildCredentialIDByIndex(size_t nIndex) const
{
    if (nIndex >= m_mapCredentials.size()) {
        otErr << __FUNCTION__ << ": Index out of bounds: " << nIndex << "\n";
    }
    else {
        int32_t nLoopIndex = -1;

        for (const auto& it : m_mapCredentials) {
            const std::string str_cred_id = it.first;
            const Credential* pSub = it.second;
            OT_ASSERT(nullptr != pSub);

            ++nLoopIndex; // 0 on first iteration.

            if (static_cast<int64_t>(nIndex) == nLoopIndex) return str_cred_id;
        }
    }

    return nullptr;
}

const OTKeypair& CredentialSet::GetAuthKeypair(
    const String::List* plistRevokedIDs) const
{
    for (const auto& it : m_mapCredentials) {
        const std::string str_cred_id = it.first;
        const Credential* pSub = it.second;
        OT_ASSERT(nullptr != pSub);

        const ChildKeyCredential* pKey = dynamic_cast<const ChildKeyCredential*>(pSub);
        if (nullptr == pKey) continue;

        OT_ASSERT(pKey->m_AuthentKey);

        // See if pKey, with ID str_cred_id, is on plistRevokedIDs...
        //
        if (nullptr != plistRevokedIDs) {
            auto iter = std::find(plistRevokedIDs->begin(),
                                  plistRevokedIDs->end(), str_cred_id);
            if (iter != plistRevokedIDs->end()) // It was on the revoked list.
                continue;                       // Skip this revoked key.
        }
        // At this point we know it's a key credential, and we know it's not on
        // the revoked list. Therefore, let's return the key! (Any other future
        // smart criteria might go here before taking this final step...)
        //
        return *(pKey->m_AuthentKey);
    }

    // Didn't find any child credentials we can use? For now, we'll return the
    // master key instead.
    // This is purely for backwards compatibility reasons and eventually should
    // be removed. That is,
    // master credentials should only be able to verify child credentials, and only child credentials should be
    // able to do actions.
    // Capiche?
    //
    OT_ASSERT(m_MasterCredential.m_AuthentKey);
    return *(m_MasterCredential.m_AuthentKey);
}

const OTKeypair& CredentialSet::GetEncrKeypair(
    const String::List* plistRevokedIDs) const
{
    for (const auto& it : m_mapCredentials) {
        const std::string str_cred_id = it.first;
        const Credential* pSub = it.second;
        OT_ASSERT(nullptr != pSub);

        const ChildKeyCredential* pKey = dynamic_cast<const ChildKeyCredential*>(pSub);
        if (nullptr == pKey) continue;

        OT_ASSERT(pKey->m_EncryptKey);

        // See if pKey, with ID str_cred_id, is on plistRevokedIDs...
        //
        if (nullptr != plistRevokedIDs) {
            auto iter = std::find(plistRevokedIDs->begin(),
                                  plistRevokedIDs->end(), str_cred_id);
            if (iter != plistRevokedIDs->end()) // It was on the revoked list.
                continue;                       // Skip this revoked key.
        }
        // At this point we know it's a key credential, and we know it's not on
        // the revoked list. Therefore, let's return the key! (Any other future
        // smart criteria might go here before taking this final step...)
        //
        return *(pKey->m_EncryptKey);
    }

    // Didn't find any child credentials we can use? For now, we'll return the
    // master key instead.
    // This is purely for backwards compatibility reasons and eventually should
    // be removed. That is,
    // master credentials should only be able to verify child credentials, and only child credentials should be
    // able to do actions.
    // Capiche?
    //
    OT_ASSERT(m_MasterCredential.m_EncryptKey);
    return *(m_MasterCredential.m_EncryptKey);
}

const OTKeypair& CredentialSet::GetSignKeypair(
    const String::List* plistRevokedIDs) const
{
    for (const auto& it : m_mapCredentials) {
        const std::string str_cred_id = it.first;
        const Credential* pSub = it.second;
        OT_ASSERT(nullptr != pSub);

        const ChildKeyCredential* pKey = dynamic_cast<const ChildKeyCredential*>(pSub);
        if (nullptr == pKey) continue;

        OT_ASSERT(pKey->m_SigningKey);

        // See if pKey, with ID str_cred_id, is on plistRevokedIDs...
        //
        if (nullptr != plistRevokedIDs) {
            auto iter = std::find(plistRevokedIDs->begin(),
                                  plistRevokedIDs->end(), str_cred_id);
            if (iter != plistRevokedIDs->end()) // It was on the revoked list.
                continue;                       // Skip this revoked key.
        }
        // At this point we know it's a key credential, and we know it's not on
        // the revoked list. Therefore, let's return the key! (Any other future
        // smart criteria might go here before taking this final step...)
        //
        return *(pKey->m_SigningKey);
    }

    // Didn't find any child credentials we can use? For now, we'll return the
    // master key instead.
    // This is purely for backwards compatibility reasons and eventually should
    // be removed. That is,
    // master credentials should only be able to verify child credentials, and only child credentials should be
    // able to do actions.
    // Capiche?
    //
    OT_ASSERT(m_MasterCredential.m_SigningKey);
    return *(m_MasterCredential.m_SigningKey);
}

// NOTE: Until we figure out the rule by which we decide WHICH authentication
// key is the right auth key,
// or WHICH signing key is the right signing key, we'll just go with the first
// one we find.
// We'll also weed out any that appear on plistRevokedIDs, if it's passed in.
// (Optional.)

const OTAsymmetricKey& CredentialSet::GetPublicAuthKey(
    const String::List* plistRevokedIDs) const
{
    return GetAuthKeypair(plistRevokedIDs).GetPublicKey();
}

const OTAsymmetricKey& CredentialSet::GetPublicEncrKey(
    const String::List* plistRevokedIDs) const
{
    return GetEncrKeypair(plistRevokedIDs).GetPublicKey();
}

const OTAsymmetricKey& CredentialSet::GetPublicSignKey(
    const String::List* plistRevokedIDs) const
{
    return GetSignKeypair(plistRevokedIDs).GetPublicKey();
}

const OTAsymmetricKey& CredentialSet::GetPrivateAuthKey(
    const String::List* plistRevokedIDs) const
{
    return GetAuthKeypair(plistRevokedIDs).GetPrivateKey();
}

const OTAsymmetricKey& CredentialSet::GetPrivateEncrKey(
    const String::List* plistRevokedIDs) const
{
    return GetEncrKeypair(plistRevokedIDs).GetPrivateKey();
}

const OTAsymmetricKey& CredentialSet::GetPrivateSignKey(
    const String::List* plistRevokedIDs) const
{
    return GetSignKeypair(plistRevokedIDs).GetPrivateKey();
}

CredentialSet::~CredentialSet()
{
    ClearChildCredentials();
}

void CredentialSet::ClearChildCredentials()
{

    while (!m_mapCredentials.empty()) {
        Credential* pSub = m_mapCredentials.begin()->second;
        OT_ASSERT(nullptr != pSub);

        delete pSub;
        pSub = nullptr;

        m_mapCredentials.erase(m_mapCredentials.begin());
    } // while
}

// listRevokedIDs should contain a list of std::strings for IDs of
// already-revoked child credentials.
// That way, SerializeIDs will know whether to mark them as valid while
// serializing them.
// bShowRevoked allows us to include/exclude the revoked credentials from the
// output (filter for valid-only.)
// bValid=true means we are saving OTPseudonym::m_mapCredentials. Whereas
// bValid=false means we're saving m_mapRevoked.
// pmapPubInfo is optional output, the public info for all the credentials will
// be placed inside, if a pointer is provided.
//
void CredentialSet::SerializeIDs(Tag& parent, const String::List& listRevokedIDs,
                                String::Map* pmapPubInfo,
                                String::Map* pmapPriInfo, bool bShowRevoked,
                                bool bValid) const
{
    if (bValid || bShowRevoked) {
        TagPtr pTag(new Tag("masterCredential"));

        pTag->add_attribute("ID", GetMasterCredID().Get());
        pTag->add_attribute("valid", formatBool(bValid));
        pTag->add_attribute("type",
            Credential::CredentialTypeToString(m_MasterCredential.GetType()).Get());

        parent.add_tag(pTag);

        if (nullptr != pmapPubInfo) // optional out-param.
            pmapPubInfo->insert(std::pair<std::string, std::string>(
                GetMasterCredID().Get(), GetPubCredential().Get()));

        if (nullptr != pmapPriInfo) // optional out-param.
            pmapPriInfo->insert(std::pair<std::string, std::string>(
                GetMasterCredID().Get(), GetPriCredential().Get()));
    }

    for (const auto& it : m_mapCredentials) {
        const std::string str_cred_id = it.first;
        const Credential* pSub = it.second;
        OT_ASSERT(nullptr != pSub);

        // See if the current child credential is on the Nym's list of "revoked"
        // child credential IDs.
        // If so, we'll skip serializing it here.
        //
        auto iter = std::find(listRevokedIDs.begin(), listRevokedIDs.end(),
                              str_cred_id);

        // Was it on the 'revoked' list?
        // If not, then the credential isn't revoked, so it's still valid.
        //
        const bool bChildCredValid =
            bValid ? (iter == listRevokedIDs.end()) : false;

        if (bChildCredValid || bShowRevoked) {
            const ChildKeyCredential* pChildKeyCredential = dynamic_cast<const ChildKeyCredential*>(pSub);

            TagPtr pTag;

            if (nullptr != pChildKeyCredential) {
                pTag.reset(new Tag("keyCredential"));
                pTag->add_attribute("type",
                    Credential::CredentialTypeToString(pChildKeyCredential->GetType()).Get());
                pTag->add_attribute("masterID",
                                    pChildKeyCredential->GetMasterCredID().Get());
            }
            else {
                pTag.reset(new Tag("credential"));
                pTag->add_attribute("masterID", pSub->GetMasterCredID().Get());
            }

            pTag->add_attribute("ID", str_cred_id);
            pTag->add_attribute("valid", formatBool(bChildCredValid));

            parent.add_tag(pTag);

            if (nullptr != pmapPubInfo) // optional out-param.
                pmapPubInfo->insert(std::pair<std::string, std::string>(
                    str_cred_id.c_str(), pSub->GetPubCredential().Get()));

            if (nullptr != pmapPriInfo) // optional out-param.
                pmapPriInfo->insert(std::pair<std::string, std::string>(
                    str_cred_id.c_str(), pSub->GetPriCredential().Get()));

        } // if (bChildCredValid)
    }
}

} // namespace opentxs
