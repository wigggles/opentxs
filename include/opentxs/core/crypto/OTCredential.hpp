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

#ifndef OPENTXS_CORE_CRYPTO_OTCREDENTIAL_HPP
#define OPENTXS_CORE_CRYPTO_OTCREDENTIAL_HPP

#include "OTMasterkey.hpp"
#include <opentxs/core/String.hpp>

// A nym contains a list of master credentials, via OTCredential.
// The whole purpose of a Nym is to be an identity, which can have
// master credentials.
//
// Each credential is like a master key for the Nym's identity,
// which can issue its own subkeys.
//
// Each subkey has 3 key pairs: encryption, signing, and authentication.
// Not all subcredentials are a subkey. For example, you might have a
// subcredential that uses Google Authenticator, and thus doesn't contain
// any keys, because it uses alternate methods for its own authentication.
//
// Each OTCredential contains a "master" subkey, and a list of subcredentials
// (some of them subkeys) signed by that master.
//
// The same class (subcredential/subkey) is used because there are master
// credentials and subcredentials, so we're using inheritance for
// "subcredential"
// and "subkey" to encapsulate the credentials, so we don't have to repeat code
// across both.
// We're using a "has-a" model here, since the OTCredential "has a" master
// subkey, and also "has a" list of subcredentials, some of which are subkeys.
//
// Each subcredential must be signed by the subkey that is the master key.
// Each subkey has 3 key pairs: encryption, signing, and authentication.
//
// Each key pair has 2 OTAsymmetricKeys (public and private.)
//
// I'm thinking that the Nym should also have a key pair (for whatever is
// its current key pair, copied from its credentials.)
//
// the master should never be able to do any actions except for sign subkeys.
// the subkeys, meanwhile should only be able to do actions, and not issue
// any new keys.

namespace opentxs
{

class OTCredential;
class Identifier;
class OTPassword;
class OTPasswordData;
class OTSubcredential;
class OTSubkey;
class Tag;

typedef std::map<std::string, OTSubcredential*> mapOfSubcredentials;

// THE MASTER CREDENTIAL (below -- OTCredential)
//
// Contains a "master" subkey,
// and a list of subcredentials signed by that master.
// (Some of which are subkeys, since subkey inherits from
// subcredential.)
// Each subcredential can generate its own "credential" contract,
// even the master subcredential, so an OTCredential object
// actually may include many "credentials." (That is, each may be
// issued at separate times. Each may be registered on a server at
// separate times. Etc.)
//
// Each nym has multiple OTCredentials because there may be
// several master keys, each with their own subcredentials.
//
// Two things to verify on a master credential:
//
// 1. If you hash m_pstrSourceForNymID, you should get m_pstrNymID.
// 2. m_pstrSourceForNymID should somehow verify m_Masterkey.GetContents().
//    For example, if m_pstrSourceForNymID contains CA DN info, then GetContents
//    should contain a verifiable Cert with that same DN info. Another example,
//    if m_pstrSourceForNymID contains a public key, then
// m_Masterkey.GetContents
//    should contain that same public key, or a cert that contains it. Another
// example,
//    if m_pstrSourceForNymID contains a URL, then m_Masterkey.GetContents
// should contain
//    a public key found at that URL, or a public key that, when hashed, matches
// one of
//    the hashes posted at that URL.
//
class OTCredential
{
private:
    OTMasterkey m_Masterkey;
    mapOfSubcredentials m_mapSubcredentials;
    String m_strNymID;
    String m_strSourceForNymID;
    // --------------------------------------
    String m_strMasterCredID; // This can't be stored in the master itself
                              // since it's a hash of that master. But this
                              // SHOULD be found in every subcredential signed
                              // by that master.

    const OTPassword* m_pImportPassword; // Not owned. Just here for
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
    OTCredential();
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
                                                  // credential m_MasterKey
    bool GenerateMasterkey(int32_t nBits = 1024); // CreateMaster is able to
                                                  // create keys from scratch
                                                  // (by calling this function.)
    bool SignNewMaster(const OTPasswordData* pPWData = nullptr); // SignMaster
                                                                 // is used
    // when creating master
    // credential.
    bool SignNewSubcredential(OTSubcredential& theSubCred,
                              Identifier& theSubCredID_out,
                              const OTPasswordData* pPWData = nullptr); // Used
                                                                        // when
    // creating a new
    // subcredential.
public:
    EXPORT const OTPassword* GetImportPassword() const
    {
        return m_pImportPassword;
    }
    EXPORT void SetImportPassword(const OTPassword* pImportPassword)
    {
        m_pImportPassword = pImportPassword;
    }
    static OTCredential* CreateMaster(const String& strSourceForNymID,
                                      int32_t nBits = 1024, // Ignored unless
                                                            // pmapPrivate is
                                                            // nullptr
                                      const String::Map* pmapPrivate = nullptr,
                                      const String::Map* pmapPublic = nullptr,
                                      const OTPasswordData* pPWData = nullptr);
    static OTCredential* LoadMaster(const String& strNymID, // Caller is
                                                            // responsible to
                                                            // delete, in both
                                    // CreateMaster and LoadMaster.
                                    const String& strMasterCredID,
                                    const OTPasswordData* pPWData = nullptr);
    static OTCredential* LoadMasterFromString(
        const String& strInput,
        const String& strNymID, // Caller is responsible to delete, in both
                                // CreateMaster and LoadMaster.
        const String& strMasterCredID, OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);
    EXPORT bool Load_Master(const String& strNymID,
                            const String& strMasterCredID,
                            const OTPasswordData* pPWData = nullptr);
    EXPORT bool Load_MasterFromString(
        const String& strInput, const String& strNymID,
        const String& strMasterCredID, const OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);
    // For subcredentials that are specifically *subkeys*. Meaning it will
    // contain 3 keypairs: signing, authentication, and encryption.
    //
    EXPORT bool AddNewSubkey(
        int32_t nBits = 1024, // Ignored unless pmapPrivate is nullptr
        const String::Map* pmapPrivate = nullptr, // Public keys are derived
                                                  // from the private.
        const OTPasswordData* pPWData = nullptr, // The master key will sign the
                                                 // subkey.
        OTSubkey* *ppSubkey = nullptr);          // output
    // For non-key credentials, such as for 3rd-party authentication.
    //
    EXPORT bool AddNewSubcredential(
        const String::Map& mapPrivate, const String::Map& mapPublic,
        const OTPasswordData* pPWData = nullptr, // The master key will sign the
                                                 // subcredential.
        OTSubcredential* *ppSubcred = nullptr);  // output
    EXPORT bool ReEncryptPrivateCredentials(const OTPassword& theExportPassword,
                                            bool bImporting); // Like for when
                                                              // you are
                                                              // exporting a Nym
                                                              // from the
                                                              // wallet.
    EXPORT bool LoadSubkey(const String& strSubID);
    EXPORT bool LoadSubcredential(const String& strSubID);
    EXPORT bool LoadSubkeyFromString(
        const String& strInput, const String& strSubID,
        const OTPassword* pImportPassword = nullptr);
    EXPORT bool LoadSubcredentialFromString(
        const String& strInput, const String& strSubID,
        const OTPassword* pImportPassword = nullptr);
    EXPORT size_t GetSubcredentialCount() const;
    EXPORT const OTSubcredential* GetSubcredential(
        const String& strSubID,
        const String::List* plistRevokedIDs = nullptr) const;
    EXPORT const OTSubcredential* GetSubcredentialByIndex(int32_t nIndex) const;
    EXPORT const std::string GetSubcredentialIDByIndex(size_t nIndex) const;
    EXPORT const String& GetPubCredential() const; // Returns: m_Masterkey's
                                                   // public credential
                                                   // string.
    EXPORT const String& GetPriCredential() const; // Returns: m_Masterkey's
                                                   // private credential
                                                   // string.
    EXPORT const String& GetMasterCredID() const;  // Returns: Master
                                                   // Credential ID!
    EXPORT const String& GetNymID() const;
    EXPORT const String& GetSourceForNymID() const;
    // listRevokedIDs should contain a list of std::strings for IDs of
    // already-revoked subcredentials.
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
    EXPORT const OTMasterkey& GetMasterkey() const
    {
        return m_Masterkey;
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
    EXPORT void ClearSubcredentials();
    EXPORT ~OTCredential();
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTCREDENTIAL_HPP
