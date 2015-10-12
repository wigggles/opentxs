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

#ifndef OPENTXS_CORE_CRYPTO_CREDENTIALSET_HPP
#define OPENTXS_CORE_CRYPTO_CREDENTIALSET_HPP

#include "MasterCredential.hpp"
#include <opentxs/core/String.hpp>
#include <opentxs/core/crypto/NymParameters.hpp>

#include <opentxs/core/crypto/Credential.hpp>

#include <memory>

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

namespace opentxs
{

class CredentialSet;
class Identifier;
class OTPassword;
class OTPasswordData;
class Credential;
class ChildKeyCredential;
class Tag;

typedef std::map<std::string, Credential*> mapOfCredentials;

// CredentialSet
//
// Contains a MasterCredential, and a list of child credentials signed by that master.
// Each child credential can generate its own Credential contract,
// even the MasterCredential, so a CredentialSet object
// includes many Credentials. (That is, each may be issued at separate times,
// each may be registered on a server at separate times, etc.)
//
// Each nym has multiple CredentialSets because there may be
// several master keys, each with their own child credentials.
//
// Two things to verify on a credential set:
//
// 1. If you hash m_pstrSourceForNymID, you should get m_pstrNymID.
// 2. m_pstrSourceForNymID should somehow verify m_MasterCredential.GetContents().
//    For example, if m_pstrSourceForNymID contains CA DN info, then GetContents
//    should contain a verifiable Cert with that same DN info. Another example,
//    if m_pstrSourceForNymID contains a public key, then m_MasterCredential.GetContents
//    should contain that same public key, or a cert that contains it. Another
//    example, if m_pstrSourceForNymID contains a URL, then m_MasterCredential.GetContents
//    should contain a public key found at that URL, or a public key that, when
//    hashed, matches one of the hashes posted at that URL.
//
class CredentialSet
{
private:
    MasterCredential m_MasterCredential;
    mapOfCredentials m_mapCredentials;
    String m_strNymID;
    String m_strSourceForNymID;
    // --------------------------------------
    String m_strMasterCredID; // This can't be stored in the master itself
                              // since it's a hash of that master. But this
                              // SHOULD be found in every credential signed
                              // by that master.

    const OTPassword* m_pImportPassword = nullptr; // Not owned. Just here for
                                         // convenience.
    // Sometimes it will be set, so that when
    // loading something up (and decrypting it)
    // the password is already available, so the
    // user doesn't have to type it a million
    // times (such as during import.) So we use
    // it when it's available. And usually
    // whoever set it, will immediately set it
    // back to nullptr when he's done.
private:
    CredentialSet();
    CredentialSet(const Credential::CredentialType masterType);
    bool SetPublicContents(const String::Map& mapPublic);    // For master
                                                             // credential.
    bool SetPrivateContents(const String::Map& mapPrivate);  // For master
                                                             // credential.
    void SetSourceForNymID(const String& strSourceForNymID); // The source is
                                                             // the
                                                             // URL/DN/pubkey
                                                             // that hashes to
                                                             // form the
                                                             // NymID. Any
                                                             // credential
                                                             // must verify
                                                             // against its
                                                             // own source.
    void SetMasterCredID(const String& strID);    // The master credential ID is
                                                  // a hash of the master
                                                  // credential m_MasterCredential
    bool SignNewMaster(const OTPasswordData* pPWData = nullptr); // SignMaster
                                                                 // is used
                                                                 // when creating master
                                                                 // credential.
    bool SignNewChildCredential(Credential& theChildCred,
                              Identifier& theChildCredID_out,
                              const OTPasswordData* pPWData = nullptr); // Used
                                                                        // when
                                                                        // creating a new
                                                                        // child credential.
    static CredentialSet* CreateMaster(const String& strSourceForNymID,
                                      const std::shared_ptr<NymParameters>& pKeyData,
                                      const OTPasswordData* pPWData = nullptr);
public:
    EXPORT CredentialSet(
        const std::shared_ptr<NymParameters>& nymParameters,
        const OTPasswordData* pPWData = nullptr, const String* psourceForNymID = nullptr
    );
    EXPORT const OTPassword* GetImportPassword() const
    {
        return m_pImportPassword;
    }
    EXPORT void SetImportPassword(const OTPassword* pImportPassword)
    {
        m_pImportPassword = pImportPassword;
    }
    static CredentialSet* LoadMaster(const String& strNymID, // Caller is
                                                            // responsible to
                                                            // delete, in both
                                    // CreateMaster and LoadMaster.
                                    const String& strMasterCredID,
                                    const Credential::CredentialType theType,
                                    const OTPasswordData* pPWData = nullptr);
    static CredentialSet* LoadMasterFromString(
        const String& strInput,
        const String& strNymID, // Caller is responsible to delete, in both
                                // CreateMaster and LoadMaster.
        const String& strMasterCredID,
        const Credential::CredentialType theType,
        OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);
    EXPORT bool Load_Master(const String& strNymID,
                            const String& strMasterCredID,
                            const Credential::CredentialType theType,
                            const OTPasswordData* pPWData = nullptr);
    EXPORT bool Load_MasterFromString(
        const String& strInput, const String& strNymID,
        const String& strMasterCredID,
        Credential::CredentialType theType,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);
    // For credentials that are specifically KeyCredentials. Meaning it will
    // contain 3 keypairs: signing, authentication, and encryption.
    //
    EXPORT bool AddNewChildKeyCredential(
        const std::shared_ptr<NymParameters>& pKeyData,
        const OTPasswordData* pPWData = nullptr, // The master credential will sign the
                                                 // child key credential.
        ChildKeyCredential* *ppChildKeyCredential = nullptr);          // output
    // For non-key credentials, such as for 3rd-party authentication.
    //
    EXPORT bool AddNewChildCredential(
        const String::Map& mapPrivate,
        const String::Map& mapPublic,
        const OTPasswordData* pPWData = nullptr, // The master key will sign the
                                                 // child credential.
        Credential* *ppChildCred = nullptr);  // output
    EXPORT bool ReEncryptPrivateCredentials(const OTPassword& theExportPassword,
                                            bool bImporting); // Like for when
                                                              // you are
                                                              // exporting a Nym
                                                              // from the
                                                              // wallet.
    EXPORT bool LoadChildKeyCredential(const String& strSubID, const Credential::CredentialType theType);
    EXPORT bool LoadCredential(const String& strSubID);
    EXPORT bool LoadChildKeyCredentialFromString(
        const String& strInput,
        const String& strSubID,
        const Credential::CredentialType theType,
        const OTPassword* pImportPassword = nullptr);
    EXPORT bool LoadCredentialFromString(
        const String& strInput, const String& strSubID,
        const OTPassword* pImportPassword = nullptr);
    EXPORT size_t GetChildCredentialCount() const;
    EXPORT const Credential* GetChildCredential(
        const String& strSubID,
        const String::List* plistRevokedIDs = nullptr) const;
    EXPORT const Credential* GetChildCredentialByIndex(int32_t nIndex) const;
    EXPORT const std::string GetChildCredentialIDByIndex(size_t nIndex) const;
    EXPORT const String& GetPubCredential() const; // Returns: m_MasterCredential's
                                                   // public credential
                                                   // string.
    EXPORT const String& GetPriCredential() const; // Returns: m_MasterCredential's
                                                   // private credential
                                                   // string.
    EXPORT const String& GetMasterCredID() const;  // Returns: Master
                                                   // Credential ID!
    EXPORT const String& GetNymID() const;
    EXPORT const String& GetSourceForNymID() const;

    EXPORT bool HasPublic() const;
    EXPORT bool HasPrivate() const;

    // listRevokedIDs should contain a list of std::strings for IDs of
    // already-revoked credentials.
    // That way, SerializeIDs will know whether to mark them as valid while
    // serializing them.
    // bShowRevoked allows us to include/exclude the revoked credentials from
    // the output (filter for valid-only.)
    // bValid=true means we are saving OTPseudonym::m_mapCredentials. Whereas
    // bValid=false means we're saving m_mapRevoked.
    //
    EXPORT void SerializeIDs(Tag& parent, const String::List& listRevokedIDs,
                             String::Map* pmapPubInfo = nullptr,
                             String::Map* pmapPriInfo = nullptr,
                             bool bShowRevoked = false,
                             bool bValid = true) const;
    EXPORT bool VerifyInternally() const;
    EXPORT bool VerifyAgainstSource() const;
    EXPORT const MasterCredential& GetMasterCredential() const
    {
        return m_MasterCredential;
    }
    EXPORT int32_t GetPublicKeysBySignature(
        listOfAsymmetricKeys& listOutput, const OTSignature& theSignature,
        char cKeyType = '0') const; // 'S' (signing key) or
                                    // 'E' (encryption key)
                                    // or 'A'
                                    // (authentication key)
    EXPORT const OTAsymmetricKey& GetPublicAuthKey(
        const String::List* plistRevokedIDs = nullptr) const;
    EXPORT const OTAsymmetricKey& GetPublicEncrKey(
        const String::List* plistRevokedIDs = nullptr) const;
    EXPORT const OTAsymmetricKey& GetPublicSignKey(
        const String::List* plistRevokedIDs = nullptr) const;
    EXPORT const OTAsymmetricKey& GetPrivateSignKey(
        const String::List* plistRevokedIDs = nullptr) const;
    EXPORT const OTAsymmetricKey& GetPrivateEncrKey(
        const String::List* plistRevokedIDs = nullptr) const;
    EXPORT const OTAsymmetricKey& GetPrivateAuthKey(
        const String::List* plistRevokedIDs = nullptr) const;
    EXPORT const OTKeypair& GetAuthKeypair(
        const String::List* plistRevokedIDs = nullptr) const;
    EXPORT const OTKeypair& GetEncrKeypair(
        const String::List* plistRevokedIDs = nullptr) const;
    EXPORT const OTKeypair& GetSignKeypair(
        const String::List* plistRevokedIDs = nullptr) const;
    EXPORT void ClearChildCredentials();
    EXPORT ~CredentialSet();
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_CREDENTIALSET_HPP
