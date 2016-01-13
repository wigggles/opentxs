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

#include <memory>
#include <opentxs-proto/verify/VerifyCredentials.hpp>

#include <opentxs/core/Contract.hpp>
#include <opentxs/core/String.hpp>

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
class NymParameters;

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

    static String CredentialTypeToString(proto::CredentialType credentialType);
    static serializedCredential ExtractArmoredCredential(
        const String stringCredential);
    static serializedCredential ExtractArmoredCredential(
        const OTASCIIArmor armoredCredential);
    static Credential* CredentialFactory(
        CredentialSet& parent,
        const proto::Credential& serialized,
        const proto::CredentialRole& purportedRole = proto::CREDROLE_ERROR);

private:
    typedef Contract ot_super;
    friend class CredentialSet;
    Credential() = delete;

    // Syntax (non cryptographic) validation
    bool isValid() const;
    // Returns the serialized form to prevent unnecessary serializations
    bool isValid(serializedCredential& credential) const;

    bool VerifyCredentialID() const;
    bool VerifyMasterID() const;
    bool VerifyNymID() const;
    bool VerifySignedByMaster() const;

protected:
    proto::CredentialType type_ = proto::CREDTYPE_ERROR;
    proto::CredentialRole role_ = proto::CREDROLE_ERROR;
    proto::KeyMode mode_ = proto::KEYMODE_ERROR;
    CredentialSet* owner_backlink_ = nullptr; // Do not cleanup.
    String master_id_;
    String nym_id_;
    uint32_t version_ = 0;

    Credential(CredentialSet& owner, const proto::Credential& serializedCred);
    Credential(CredentialSet& owner, const NymParameters& nymParameters);

    virtual bool VerifyInternally() const;

    bool AddMasterSignature();

public:
    const String& MasterID() const
    {
        return master_id_;
    }
    const String& NymID() const
    {
        return nym_id_;
    }
    proto::CredentialRole Role() const
    {
        return role_;
    }
    serializedSignature MasterSignature() const;
    serializedSignature SelfSignature(
        CredentialModeFlag version = PUBLIC_VERSION) const;
    serializedSignature SourceSignature() const;
    proto::CredentialType Type() const;

    bool isPrivate() const;
    bool isPublic() const;

    std::string asString(const bool asPrivate = false) const;
    virtual serializedCredential asSerialized(
        SerializationModeFlag asPrivate,
        SerializationSignatureFlag asSigned) const;

    // Inherited from opentxs::Contract
    EXPORT virtual void CalculateContractID(Identifier& newID) const;
    virtual void ReleaseSignatures(const bool onlyPrivate);
    using ot_super::SaveContract;
    virtual bool SaveContract() override;
    virtual bool VerifyContract() const;

    virtual bool GetContactData(proto::ContactData& contactData) const;
    virtual bool GetVerificationSet(
        std::shared_ptr<proto::VerificationSet>& verificationSet) const;
    virtual void Release();
    void Release_Credential();
    virtual bool Sign(
        Contract& theContract,
        const OTPasswordData* pPWData = nullptr) const;
    virtual bool Sign(
        const OTData& plaintext,
        proto::Signature& sig,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* exportPassword = nullptr,
        const proto::SignatureRole role = proto::SIGROLE_ERROR,
        proto::KeyRole key = proto::KEYROLE_SIGN) const;
    virtual bool Verify(
        const OTData& plaintext,
        proto::Signature& sig,
        proto::KeyRole key = proto::KEYROLE_SIGN) const;
    virtual bool Verify(const Credential& credential) const;

    virtual ~Credential();
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_CREDENTIAL_HPP
