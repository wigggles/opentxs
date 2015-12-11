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

#ifndef OPENTXS_CORE_CRYPTO_KEYCREDENTIAL_HPP
#define OPENTXS_CORE_CRYPTO_KEYCREDENTIAL_HPP

#include "OTKeypair.hpp"
#include "Credential.hpp"
#include <opentxs/core/crypto/NymParameters.hpp>
#include <opentxs-proto/verify/VerifyCredentials.hpp>

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

class OTAsymmetricKey;
class CredentialSet;
class OTPassword;
class OTPasswordData;

typedef std::list<OTAsymmetricKey*> listOfAsymmetricKeys;

// CONTENTS needs to be PUBLIC and PRIVATE contents, EACH being a string map.
//
// The server (or anyone else) will only be able to see my public contents, not
// my private
// contents.
//
// The credential ID comes from a hash of the credential. (Which must be signed
// before it can be hashed.)
//
// Since I will have a public version of the credential, signed, for others, and
// I will have a private version
// signed, for myself, then I will have to store both signed versions, yes? I
// can't be re-signing things because
// the public version is hashed to form my credential ID. So once signed, we
// can't be signing it again later.
//
// So I think CredentialSet will store a string containing the signed public
// version. Then it can include a copy
// of this string in the signed private version. (That way it always has both
// versions safe and signed, and it can
// always pull out its public version and send it to servers or whoever when it
// needs to.
//
// A credential can store its own signed public version, which must contain
// the master credential ID and be
// signed by that master key. If a credential is a key credential, then it must also
// be signed by itself.
//
// This is packaged up and attached to the signed private version, which
// includes the private keys, and is only
// stored on the client side.
//
// Might want also a version with IDs only.
//
// When creating a new credential, I want the ability to specify the public and
// private key information.But what if I don't specify? I should be able to pass nullptr, and
// OT should be smart enough to generate the three certs and the three private keys, without
// me having to pass anything at all.
//
// If it's a master, this credential should be signed with itself.
// If it's a normal credential (not master) then it should be signed with
// its master, but not signed by itself since it may have no key.
// If it's a child key credential (a form of credential) then it should be signed by itself
// AND by its master. And it must contain its master's ID.
// But if it's a master, it cannot contain its master's ID except maybe its own
// ID, but it is impossible for a contract to contain its own ID when its ID is a
// hash of the signed contract!
//
// I might make KeyCredential and then have ChildKeyCredential and MasterCredential both
// derive from that.
// That way the master key doesn't have to contain its own ID, while the child key credential
// can still contain
// its master's ID.

/// KeyCredential
/// A form of Credential that contains 3 key pairs: signing,
/// authentication, and encryption.
/// We won't use KeyCredential directly but only as a common base class for
/// ChildKeyCredential and MasterCredential.
///
class KeyCredential : public Credential
{
private: // Private prevents erroneous use by other classes.
    typedef Credential ot_super;
    friend class CredentialSet;
    KeyCredential() = delete;
    bool addKeytoSerializedKeyCredential(
        proto::KeyCredential& credential,
        const bool getPrivate,
        const proto::KeyRole role) const;
    bool addKeyCredentialtoSerializedCredential(
        serializedCredential credential,
        const bool addPrivate) const;
    std::shared_ptr<OTKeypair> DeriveHDKeypair(
        const uint32_t nym,
        const uint32_t credset,
        const uint32_t credindex,
        const proto::KeyRole role);

protected:
    virtual serializedCredential Serialize(bool asPrivate = false, bool asSigned = true) const;
    KeyCredential(
        CredentialSet& theOwner,
        const NymParameters& nymParameters,
        const proto::CredentialRole role);
    KeyCredential(CredentialSet& theOwner, const proto::Credential& serializedCred);
public:
    std::shared_ptr<OTKeypair> m_SigningKey; // Signing keys, for signing/verifying a "legal
                            // signature".
    std::shared_ptr<OTKeypair> m_AuthentKey; // Authentication keys, used for signing/verifying
                            // transmissions and stored files.
    std::shared_ptr<OTKeypair>  m_EncryptKey; // Encryption keys, used for sealing/opening
                            // OTEnvelopes.
    bool ReEncryptKeys(const OTPassword& theExportPassword,
                       bool bImporting); // Used when importing/exporting a Nym
                                         // to/from the wallet.
    virtual bool VerifyInternally(); // Verify that m_strNymID is the same as
                                     // the hash of m_strSourceForNymID. Also
                                     // verify that *this ==
                                     // m_pOwner->m_MasterCredential (the master
                                     // credential.) Then verify the
                                     // (self-signed) signature on *this.
    bool VerifySignedBySelf() const;
    bool Sign(Contract& theContract, const OTPasswordData* pPWData = nullptr);
    EXPORT int32_t GetPublicKeysBySignature(
        listOfAsymmetricKeys& listOutput, const OTSignature& theSignature,
        char cKeyType = '0') const; // 'S' (signing key) or
                                    // 'E' (encryption key)
                                    // or 'A'
                                    // (authentication key)
    virtual ~KeyCredential();
    virtual void Release();
    void Release_KeyCredential();

    virtual bool Sign(
        const proto::Credential& credential,
        const CryptoHash::HashType hashType,
        OTData& signature, // output
        const OTPassword* exportPassword = nullptr,
        const OTPasswordData* pPWData = nullptr) const;
    virtual bool SelfSign(
        const OTPassword* exportPassword = nullptr,
        const OTPasswordData* pPWData = nullptr,
        const bool onlyPrivate = false);

    EXPORT virtual bool VerifySig(
                                const proto::Signature& sig,
                                const OTAsymmetricKey& theKey,
                                const bool asPrivate = true,
                                const OTPasswordData* pPWData = nullptr) const;


};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_KEYCREDENTIAL_HPP
