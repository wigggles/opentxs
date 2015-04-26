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

#ifndef OPENTXS_CORE_CRYPTO_OTSUBCREDENTIAL_HPP
#define OPENTXS_CORE_CRYPTO_OTSUBCREDENTIAL_HPP

#include <opentxs/core/Contract.hpp>

#include <memory>

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

class Contract;
class OTCredential;
class Identifier;
class OTPassword;
class String;
class Tag;

// This is stored as an OTContract, and it must be signed by the
// master key. (which is also an OTSubcredential.)
//
class OTSubcredential : public Contract
{
private: // Private prevents erroneous use by other classes.
    typedef Contract ot_super;
    friend class OTCredential;

protected:
    enum CredStoreAs {
        credPrivateInfo =
            0, // For saving the private keys, too. Default behavior.
        credPublicInfo = 1,  // For saving a version with public keys only.
        credMasterSigned = 2 // For saving a version with the master signature
                             // included, so the subkey can then countersign on
                             // top of that. (To prove that the subkey
                             // authorizes the master key's signature.) Only
                             // used by subkeys.
    };
    CredStoreAs m_StoreAs; // Not serialized.
protected:
    OTCredential* m_pOwner;   // a pointer for convenience only. Do not cleanup.
    String m_strMasterCredID; // All subcredentials within the same
                              // OTCredential share the same
                              // m_strMasterCredID. It's a hash of the signed
                              // master credential.
    String m_strNymID;        // All subcredentials within the same OTCredential
                              // (including m_MasterKey) must have
    String m_strSourceForNymID;   // the same NymID and source.
    String::Map m_mapPublicInfo;  // A map of strings containing the
                                  // credential's public info. This was
                                  // originally 1 string but subclasses ended
                                  // up needing a map of them. Who'da thought.
    String::Map m_mapPrivateInfo; // A map of strings containing the
                                  // credential's private info. This was
                                  // originally 1 string but subclasses ended
                                  // up needing a map of them. Who'da thought.
    String m_strMasterSigned;     // A public version of the credential with the
                                  // master credential's signature on it. (The
                                  // final public version will contain the
                                  // subkey's own signature on top of that.)
    String m_strContents; // The actual final public credential as sent to the
                          // server. Does not include private keys, even on
                          // client side.
    void UpdatePublicContentsToTag(Tag& parent);   // Used in
                                                   // UpdateContents.
    void UpdatePublicCredentialToTag(Tag& parent); // Used in
                                                   // UpdateContents.
    void UpdatePrivateContentsToTag(Tag& parent);  // Used in
                                                   // UpdateContents.
    inline void SetMasterSigned(const String& strMasterSigned)
    {
        m_strMasterSigned = strMasterSigned;
    }
    inline void SetContents(const String& strContents)
    {
        m_strContents = strContents;
    }
    void SetNymIDandSource(const String& strNymID,
                           const String& strSourceForNymID);
    void SetMasterCredID(const String& strMasterCredID); // Used in all
                                                         // subclasses except
                                                         // OTMasterkey. (It
                                                         // can't contain its
                                                         // own ID, since it
                                                         // is signed, and the
                                                         // ID is its hash
                                                         // AFTER it's signed.
                                                         // So it could never
                                                         // contain its own
                                                         // ID.)
    inline void StoreAsMasterSigned()
    {
        m_StoreAs = credMasterSigned;
    } // Upon signing, the credential reverts to credPrivateInfo again.
    inline void StoreAsPublic()
    {
        m_StoreAs = credPublicInfo;
    } // Upon signing, the credential reverts to credPrivateInfo again.
    virtual bool SetPublicContents(const String::Map& mapPublic);
    virtual bool SetPrivateContents(
        const String::Map& mapPrivate,
        const OTPassword* pImportPassword = nullptr); // if not nullptr, it
                                                      // means to
                                                      // use
    // this password by default.
public:
    const String::Map& GetPublicMap() const
    {
        return m_mapPublicInfo;
    }
    const String::Map& GetPrivateMap() const
    {
        return m_mapPrivateInfo;
    }
    const String& GetMasterCredID() const
    {
        return m_strMasterCredID;
    } // MasterCredentialID (usually applicable.) OTMasterkey doesn't use this.
    const String& GetNymID() const
    {
        return m_strNymID;
    } // NymID for this credential.
    const String& GetNymIDSource() const
    {
        return m_strSourceForNymID;
    } // Source for NymID for this credential. (Hash it to get ID.)
    const String& GetContents() const
    {
        return m_strContents;
    } // The actual, final, signed public credential. Public keys only.

    EXPORT const String& GetPubCredential() const; // More intelligent version
                                                   // of GetContents. Higher
                                                   // level.
    const String& GetPriCredential() const; // I needed this for exporting a
                                            // Nym (with credentials) from the
                                            // wallet.

    const String& GetMasterSigned() const
    {
        return m_strMasterSigned;
    } // For subkeys, the master credential signs first, then the subkey signs a
      // version which contains the "master signed" version. (This proves the
      // subkey really authorizes all this.) That "master signed" version is
      // stored here in m_strMasterSigned. But the final actual public
      // credential (which must be hashed to get the credential ID) is the
      // contents, not the master signed. The contents is the public version,
      // signed by the subkey, which contains the master-signed version inside
      // of it as a data member (this variable in fact, m_strMasterSigned.) You
      // might ask: then what's in m_strRawContents? Answer: the version that
      // includes the private keys. Well at least, on the client side. On the
      // server side, the raw contents will contain only the public version
      // because that's all the client will send it. Que sera sera.
    virtual bool VerifyInternally(); // Call VerifyNymID. Also verify
                                     // m_strMasterCredID against the hash of
                                     // m_pOwner->m_MasterKey (the master
                                     // credential.) Verify that
                                     // m_pOwner->m_MasterKey and *this have the
                                     // same NymID. Then verify the signature of
                                     // m_pOwner->m_MasterKey on
                                     // m_strMasterSigned.
    // We also inherit OTContract::VerifyContractID() which hashes the contents
    // and compares to the ID as already set.
    // Unlike OTContract, a credential's ID is formed by hashing GetContents(),
    // not by hashing m_xmlRawContents,
    // (that is, the public info only, not the version containing the private
    // keys.) So we override CalculateContractID
    // to account for that.
    //
    EXPORT virtual void CalculateContractID(Identifier& newID) const;

    // We also inherit OTContract::VerifyContract() which tries to find the
    // "contract" key. Of course, there is no
    // "contract" key in this case, so we should override it and provide our own
    // version. What should it do? Well, it
    // should call VerifyContractID, VerifyInternally, VerifyMaster, and
    // VerifyAgainstSource. (If that last step later
    // on adds too much slowdown, then we'll modify that function to check a
    // signed file left for us by the IDENTITY
    // VERIFICATION SREVER which we can stick in a separate process.)
    // HOWEVER!! This may add vast unnecessary delay. For example, if we
    // "VerifyContract" on EACH subcredential, which
    // we SHOULD do, then that means EACH subcredential is going to verify its
    // Master (when they share the same master...)
    // and EACH subcredential is going to also re-verify its source (when they
    // all share the same source!)
    // Solution?
    // Clearly the master itself only needs to be verified once, including its
    // source, when the Nym is first loaded.
    // (Verifying it twice would be redundant.) After that, each subcredential
    // should be verified internally and against
    // its master -- again, when first loaded. No need to verify it again after
    // that, since it wouldn't have even loaded.
    // After that, any signature for that Nym should be verifiable using one of
    // that Nym's subcredentials.
    //
    virtual bool VerifyContract();
    bool VerifyNymID() const; // Verifies that m_strNymID is the same as the
                              // hash of
                              // m_strSourceForNymID.
    virtual bool VerifySignedByMaster();
    void SetOwner(OTCredential& theOwner);
    virtual void SetMetadata()
    {
    } // Only key-based subclasses will use this.
    OTSubcredential();
    OTSubcredential(OTCredential& theOwner);
    virtual ~OTSubcredential();
    virtual void Release();
    void Release_Subcredential();
    virtual void UpdateContents();
    virtual int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml);
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTSUBCREDENTIAL_HPP
