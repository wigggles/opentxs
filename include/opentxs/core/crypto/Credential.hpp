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

#ifndef OPENTXS_CORE_CRYPTO_CREDENTIAL_HPP
#define OPENTXS_CORE_CRYPTO_CREDENTIAL_HPP

#include <opentxs/core/Contract.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs-proto/verify/opentxs-verify.hpp>

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

class Contract;
class CredentialSet;
class Identifier;
class OTPassword;
class String;
class Tag;

// This is stored as an Contract, and it must be signed by the
// MasterCredential.
//

typedef std::shared_ptr<proto::Credential> serializedCredential;
typedef bool CredentialModeFlag;
typedef bool SerializationModeFlag;
typedef bool SerializationSignatureFlag;

class Credential : public Contract
{
public:

    static const CredentialModeFlag PRIVATE_VERSION = true;
    static const CredentialModeFlag PUBLIC_VERSION = false;

    static const SerializationModeFlag AS_PRIVATE = true;
    static const SerializationModeFlag AS_PUBLIC = false;

    static const SerializationSignatureFlag WITH_SIGNATURES = true;
    static const SerializationSignatureFlag WITHOUT_SIGNATURES = false;

    enum CredentialType: int32_t {
        ERROR_TYPE,
        LEGACY,
        URL,
        SECP256K1
    };

    static String CredentialTypeToString(CredentialType credentialType);

    static CredentialType StringToCredentialType(const String & credentialType);

    static OTAsymmetricKey::KeyType CredentialTypeToKeyType(CredentialType credentialType);

    virtual bool SaveContract();
    virtual bool SaveContract(const char* szFoldername,
                             const char* szFilename);

private: // Private prevents erroneous use by other classes.
    typedef Contract ot_super;
    friend class CredentialSet;
    Credential() = delete;

protected:
    CredentialType m_Type = Credential::ERROR_TYPE;
    proto::CredentialRole m_Role = proto::CREDROLE_ERROR;

public:
    CredentialType GetType() const;

protected:
    proto::KeyMode m_mode = proto::KEYMODE_ERROR;
    CredentialSet* m_pOwner = nullptr;   // a pointer for convenience only. Do not cleanup.
    String m_strMasterCredID; // All credentials within the same
                              // CredentialSet share the same
                              // m_strMasterCredID. It's a hash of the signed
                              // master credential.
    String m_strNymID;        // All credentials within the same CredentialSet
                              // (including m_MasterCredential) must have
    void SetMasterCredID(const String& strMasterCredID); // Used in all
                                                         // subclasses except
                                                         // MasterCredential. (It
                                                         // can't contain its
                                                         // own ID, since it
                                                         // is signed, and the
                                                         // ID is its hash
                                                         // AFTER it's signed.
                                                         // So it could never
                                                         // contain its own
                                                         // ID.)

    Credential(CredentialSet& theOwner, const proto::Credential& serializedCred);
    Credential(CredentialSet& theOwner, const NymParameters& nymParameters);
    virtual serializedCredential Serialize(
        SerializationModeFlag asPrivate,
        SerializationSignatureFlag asSigned) const;
    virtual serializedCredential SerializeForPublicSignature() const;
    virtual serializedCredential SerializeForPrivateSignature() const;
    virtual serializedCredential SerializeForIdentifier() const;
    OTData SerializeCredToData(const proto::Credential& serializedCred) const;

public:
    serializedSignature GetSelfSignature(CredentialModeFlag version = PUBLIC_VERSION) const;
    EXPORT virtual bool LoadContractFromString(const String& theStr);

    static serializedCredential ExtractArmoredCredential(const String stringCredential);
    static serializedCredential ExtractArmoredCredential(const OTASCIIArmor armoredCredential);

    bool isPrivate() const;
    bool isPublic() const;
    const String& GetMasterCredID() const
    {
        return m_strMasterCredID;
    } // MasterCredentialID (usually applicable.) MasterCredential doesn't use this.
    const String& GetNymID() const
    {
        return m_strNymID;
    } // NymID for this credential.

    std::string AsString(const bool asPrivate = false) const;

    EXPORT const serializedCredential GetSerializedPubCredential() const;
    virtual bool VerifyInternally(); // Call VerifyNymID. Also verify
                                     // m_strMasterCredID against the hash of
                                     // m_pOwner->m_MasterCredential (the master
                                     // credential.) Verify that
                                     // m_pOwner->m_MasterCredential and *this have the
                                     // same NymID. Then verify the signature of
                                     // m_pOwner->m_MasterCredential on
                                     // m_strMasterSigned.
    // We also inherit Contract::VerifyContractID() which hashes the contents
    // and compares to the ID as already set.
    // Unlike Contract, a credential's ID is formed by hashing GetContents(),
    // not by hashing m_xmlRawContents,
    // (that is, the public info only, not the version containing the private
    // keys.) So we override CalculateContractID
    // to account for that.
    //
    EXPORT virtual void CalculateContractID(Identifier& newID) const;

    // We also inherit Contract::VerifyContract() which tries to find the
    // "contract" key. Of course, there is no
    // "contract" key in this case, so we should override it and provide our own
    // version. What should it do? Well, it
    // should call VerifyContractID, VerifyInternally, VerifyMaster, and
    // VerifyAgainstSource. (If that last step later
    // on adds too much slowdown, then we'll modify that function to check a
    // signed file left for us by the IDENTITY
    // VERIFICATION SREVER which we can stick in a separate process.)
    // HOWEVER!! This may add vast unnecessary delay. For example, if we
    // "VerifyContract" on EACH credential, which
    // we SHOULD do, then that means EACH credential is going to verify its
    // Master (when they share the same master...)
    // and EACH credential is going to also re-verify its source (when they
    // all share the same source!)
    // Solution?
    // Clearly the master itself only needs to be verified once, including its
    // source, when the Nym is first loaded.
    // (Verifying it twice would be redundant.) After that, each credential
    // should be verified internally and against
    // its master -- again, when first loaded. No need to verify it again after
    // that, since it wouldn't have even loaded.
    // After that, any signature for that Nym should be verifiable using one of
    // that Nym's credentials.
    //
    virtual bool VerifyContract();
    bool VerifyNymID() const; // Verifies that m_strNymID is the same as the
                              // hash of
                              // m_strSourceForNymID.
    virtual bool VerifySignedByMaster();
    virtual ~Credential();
    virtual void Release();
    void Release_Credential();
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_CREDENTIAL_HPP
