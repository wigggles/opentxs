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
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/ChildKeyCredential.hpp>
#include <opentxs/core/util/Tag.hpp>
#include <opentxs/core/Proto.hpp>

#include <memory>
#include <algorithm>

namespace opentxs
{

bool CredentialSet::HasPublic() const
{
    for (const auto& it : m_mapCredentials)
    {
        const Credential* pSub = it.second;
        OT_ASSERT(nullptr != pSub);

        bool isPublic = pSub->isPublic();

        if (isPublic) {
            return true;
        }

        // A private credential is by definition a public one as well.
        bool isPrivate = pSub->isPrivate();

        if (isPrivate) {
            return true;
        }
    }

    return false;
}

bool CredentialSet::HasPrivate() const
{
    for (const auto& it : m_mapCredentials)
    {
        const Credential* pSub = it.second;
        OT_ASSERT(nullptr != pSub);

        bool isPrivate = pSub->isPrivate();

        if (isPrivate) {
            return true;
        }
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
    OT_ASSERT(m_MasterCredential);

    Identifier theActualMasterCredID;
    m_MasterCredential->CalculateContractID(theActualMasterCredID);
    const String strActualMasterCredID(theActualMasterCredID);

    if (!m_strNymID.Compare(m_MasterCredential->GetNymID())) {
        otOut << __FUNCTION__
              << ": NymID did not match its "
                 "counterpart in m_MasterCredential (failed to verify): " << GetNymID()
              << "\n";
        return false;
    }

    if (!GetMasterCredID().Compare(strActualMasterCredID)) {
        otOut << __FUNCTION__
              << ": Master Credential ID did not match its "
                 "counterpart in m_MasterCredential:\nExpected Master Credential ID: "
              << GetMasterCredID()
              << "\n "
                 "Hash of m_MasterCredential contents: " << strActualMasterCredID << "\n";
        return false;
    }

    if (!(m_MasterCredential->VerifyContract())) {
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
    OT_ASSERT(m_MasterCredential);

    if (!m_MasterCredential->VerifyAgainstSource()) {
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

const NymIDSource& CredentialSet::Source() const
{
    return *nym_id_source_;
}

/// This sets nym_id_source_.
/// This also sets m_strNymID.
void CredentialSet::SetSource(const std::shared_ptr<NymIDSource>& source)
{
    nym_id_source_ = source;

    m_strNymID.Release();

    m_strNymID = nym_id_source_->NymID();
}

const serializedCredential CredentialSet::GetSerializedPubCredential() const
{
    OT_ASSERT(m_MasterCredential)
    return m_MasterCredential->GetSerializedPubCredential();
}

// private
CredentialSet::CredentialSet()
{
}

CredentialSet::CredentialSet(const proto::CredentialSet& serializedCredentialSet)
{
    version_ = serializedCredentialSet.version();
    m_strNymID = serializedCredentialSet.nymid();
    Load_Master(
        serializedCredentialSet.nymid(),
        serializedCredentialSet.masterid());

    for (auto& it: serializedCredentialSet.activechildren()) {
        LoadChildKeyCredential(it);
    }
}

CredentialSet::CredentialSet(
    const NymParameters& nymParameters,
    const OTPasswordData* pPWData)
{
    m_MasterCredential.reset(new MasterCredential(*this, nymParameters));
    OT_ASSERT(m_MasterCredential)

    ChildKeyCredential* newChildCredential = new ChildKeyCredential(
                                                                *this,
                                                                nymParameters);
    OT_ASSERT(nullptr != newChildCredential);

    String strChildCredID;
    newChildCredential->GetIdentifier(strChildCredID);

    m_mapCredentials.insert(
        std::pair<std::string, Credential*>(strChildCredID.Get(), newChildCredential));
}

const String CredentialSet::GetMasterCredID() const
{
    String masterCredID;
    m_MasterCredential->GetIdentifier(masterCredID);
    return masterCredID;
}

String CredentialSet::MasterAsString() const
{
    if (m_MasterCredential) {
        return m_MasterCredential->AsString();
    } else {
        return "";
    }
}

// static  (Caller is responsible to delete.)
CredentialSet* CredentialSet::LoadMaster(
    const String& strNymID, // Caller is responsible to delete, in both
                            // CreateMaster and LoadMaster.
    const String& strMasterCredID,
    const OTPasswordData* pPWData)
{
    CredentialSet* pCredential = new CredentialSet;
    std::unique_ptr<CredentialSet> theCredentialAngel(pCredential);
    OT_ASSERT(nullptr != pCredential);

    OTPasswordData thePWData("Loading master credential. (static 1.)");
    const bool bLoaded = pCredential->Load_Master(
        strNymID, strMasterCredID, (nullptr == pPWData) ? &thePWData : pPWData);
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
    OT_ASSERT(m_MasterCredential)
    if (m_MasterCredential->isPrivate()) {
        OTPasswordData thePWData(
            bImporting ? "2 Enter passphrase for the Nym being imported."
                       : "2 Enter new passphrase for exported Nym.");

        // Re-encrypt the private keys in the master credential. (THEN sign.)
        //
        const bool bReEncryptMaster =
            m_MasterCredential->ReEncryptKeys(theExportPassword, bImporting);
        bool bSignedMaster = false;

        if (bReEncryptMaster) {
            m_MasterCredential->ReleaseSignatures(); // This time we'll sign it in
                                             // private mode.
            bSignedMaster = m_MasterCredential->Sign(*m_MasterCredential, &thePWData);
        }
        else {
            otErr << "In " << __FILE__ << ", line " << __LINE__
                  << ": Failed trying to re-encrypt the private master credential.\n";
            return false;
        }

        if (bSignedMaster) {
            m_MasterCredential->SaveContract();

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

bool CredentialSet::Load_MasterFromString(const String& strInput,
                                         const String& strNymID,
                                         const String& strMasterCredID,
                                         const Credential::CredentialType theType,
                                         const OTPasswordData*,
                                         const OTPassword* pImportPassword)
{
    m_strNymID = strNymID;

    MasterCredential* newMaster = new MasterCredential(*this, strInput);
    m_MasterCredential.reset(newMaster);

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

    SetImportPassword(nullptr); // It was only set during the
                                // m_MasterCredential->LoadContractFromString (which
                                // references it.)

    ClearChildCredentials(); // The master is loaded first, and then any
                           // child credentials. So this is probably already
                           // empty. Just looking ahead.

    return true;
}

bool CredentialSet::Load_Master(const String& strNymID,
                               const String& strMasterCredID,
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
    OTASCIIArmor ascFileContents;

    String strFileContents(OTDB::QueryPlainString(str_Folder, strNymID.Get(),
                                                  strMasterCredID.Get()));
    if (!strFileContents.Exists()) {
        otErr << __FUNCTION__ << ": Failed trying to load master credential "
                                 "from local storage.\n";
        return false;
    }

    ascFileContents.Set(strFileContents);

    serializedCredential serializedCred = Credential::ExtractArmoredCredential(ascFileContents);

    if (!serializedCred) {
        otErr << __FUNCTION__ << ": Could not parse retrieved credential as a protobuf.\n";
        return false;
    }

    bool validProto = proto::Verify(*serializedCred, proto::CREDROLE_MASTERKEY, true);
    if (!validProto) {
        otErr << __FUNCTION__ << ": Invalid serialized master credential.\n";
        return false;
    }

    m_MasterCredential.reset(new MasterCredential(*this, *serializedCred));

    if (!m_MasterCredential) {
        otErr << __FUNCTION__ << ": Failed to construct credential from protobuf.\n";
        return false;
    }

    return true;
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

    ChildKeyCredential* pSub = new ChildKeyCredential(*this, strInput);
    std::unique_ptr<ChildKeyCredential> theSubAngel(pSub);
    OT_ASSERT(nullptr != pSub);

    SetImportPassword(pImportPassword); // might be nullptr.

    if (!pSub->VerifyInternally()) {
        otErr << __FUNCTION__
              << ": Failed trying to verify freshly-loaded keyCredential.\n";
        return false;
    }

    SetImportPassword(nullptr); // Only set int64_t enough for
                                // LoadContractFromString above to use it.
    m_mapCredentials.insert(std::pair<std::string, Credential*>(
        strSubID.Get(), theSubAngel.release()));

    return true;
}

bool CredentialSet::LoadChildKeyCredential(const String& strSubID)
{

    OT_ASSERT(GetNymID().Exists());

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

    OTASCIIArmor ascFileContents;

    String strFileContents(
        OTDB::QueryPlainString(str_Folder, GetNymID().Get(), strSubID.Get()));

    if (!strFileContents.Exists()) {
        otErr << __FUNCTION__
              << ": Failed trying to load keyCredential from local storage.\n";
        return false;
    }

    ascFileContents.Set(strFileContents);

    serializedCredential serializedCred = Credential::ExtractArmoredCredential(ascFileContents);

    if (!serializedCred) {
        otErr << __FUNCTION__ << ": Could not parse retrieved credential as a protobuf.\n";
        return false;
    }

    bool validProto = proto::Verify(*serializedCred, proto::CREDROLE_CHILDKEY, true);
    if (!validProto) {
        otErr << __FUNCTION__ << ": Invalid serialized child key credential.\n";
        return false;
    }

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

    std::unique_ptr<ChildKeyCredential> pSub;
    pSub.reset(new ChildKeyCredential(*this, *serializedCred));

    m_mapCredentials.insert(std::pair<std::string, Credential*>(
        strSubID.Get(), pSub.release()));

    return true;
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
    OT_ASSERT(m_MasterCredential->m_AuthentKey);
    return *(m_MasterCredential->m_AuthentKey);
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
    OT_ASSERT(m_MasterCredential->m_EncryptKey);
    return *(m_MasterCredential->m_EncryptKey);
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
    OT_ASSERT(m_MasterCredential->m_SigningKey);
    return *(m_MasterCredential->m_SigningKey);
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
    OT_ASSERT(m_MasterCredential)
    if (bValid || bShowRevoked) {
        TagPtr pTag(new Tag("masterCredential"));

        pTag->add_attribute("ID", GetMasterCredID().Get());
        pTag->add_attribute("valid", formatBool(bValid));
        pTag->add_attribute("type",
            Credential::CredentialTypeToString(m_MasterCredential->GetType()).Get());

        parent.add_tag(pTag);

        if (nullptr != pmapPubInfo) // optional out-param.
            pmapPubInfo->insert(std::pair<std::string, std::string>(
                GetMasterCredID().Get(), m_MasterCredential->AsString(false)));

        if (nullptr != pmapPriInfo) // optional out-param.
            pmapPriInfo->insert(std::pair<std::string, std::string>(
                GetMasterCredID().Get(), m_MasterCredential->AsString(true)));
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
                    str_cred_id.c_str(), pSub->AsString(false)));

            if (nullptr != pmapPriInfo) // optional out-param.
                pmapPriInfo->insert(std::pair<std::string, std::string>(
                    str_cred_id.c_str(), pSub->AsString(true)));

        } // if (bChildCredValid)
    }
}

bool CredentialSet::WriteCredentials() const
{
    String publicFolder, privateFolder;
    String masterFilename, childFilename;
    String masterFolder, childFolder;
    String credID;

    publicFolder.Format("%s%s%s", OTFolders::Pubcred().Get(), Log::PathSeparator(), GetNymID().Get());
    privateFolder.Format("%s%s%s", OTFolders::Credential().Get(), Log::PathSeparator(), GetNymID().Get());

    masterFilename.Format("%s", GetMasterCredID().Get());

    if (m_MasterCredential->isPrivate()) {
        masterFolder = privateFolder;
    } else {
        masterFolder = publicFolder;
    }

    if (!m_MasterCredential->SaveContract(masterFolder.Get(), masterFilename.Get())) {
        otErr << __FUNCTION__ << ": Failed to save master credential.\n";
        return false;
    };

    for (auto& it: m_mapCredentials) {
        it.second->GetIdentifier(credID);
        childFilename.Format("%s", credID.Get());

        if (it.second->isPrivate()) {
            childFolder = privateFolder;
        } else {
            childFolder = publicFolder;
        }

        if (!it.second->SaveContract(childFolder.Get(), childFilename.Get())) {
            otErr << __FUNCTION__ << ": Failed to save child credential.\n";
            return false;
        }
    }

    return true;
}

SerializedCredentialSet CredentialSet::Serialize() const
{
    SerializedCredentialSet credSet = std::make_shared<proto::CredentialSet>();

    credSet->set_version(1);
    credSet->set_nymid(m_strNymID.Get());
    credSet->set_masterid(GetMasterCredID().Get());

    for (auto& it: m_mapCredentials) {
        credSet->add_activechildren(it.first);
    }
    //FIXME this list is always empty, because revocation isn't implemented
    for (auto& it: m_mapRevokedCredentials) {
        credSet->add_revokedchildren(it.first);
    }

    return credSet;
}

} // namespace opentxs
