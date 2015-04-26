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

#ifndef OPENTXS_CORE_CRYPTO_OTKEYCREDENTIAL_HPP
#define OPENTXS_CORE_CRYPTO_OTKEYCREDENTIAL_HPP

#include "OTKeypair.hpp"
#include "OTSubcredential.hpp"

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

class OTAsymmetricKey;
class OTCredential;
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
// So I think OTCredential will store a string containing the signed public
// version. Then it can include a copy
// of this string in the signed private version. (That way it always has both
// versions safe and signed, and it can
// always pull out its public version and send it to servers or whoever when it
// needs to.
//
// A subcredential can store its own signed public version, which must contain
// the master credential ID and be
// signed by that master key. If a subcredential is a subkey, then it must also
// be signed by itself.
//
// This is packaged up and attached to the signed private version, which
// includes the private keys, and is only
// stored on the client side.
//
// Might want also a version with IDs only.
//
// When creating a new credential, I want the ability to specify the public and
// private key information.
// But what if I don't specify? I should be able to pass nullptr, and OT should
// be
// smart enough to generate
// the three certs and the three private keys, without me having to pass
// anything at all.
//
// If it's a master, this subcredential should be signed with itself.
// If it's a normal subcredential (not master) then it should be signed with
// its master, but not signed by itself since it may have no key.
// If it's a subkey (a form of subcredential) then it should be signed by itself
// AND by its master. And it must contain its master's ID.
// But if it's a master, it cannot contain its master's ID except maybe its own
// ID,
// but it is impossible for a contract to contain its own ID when its ID is a
// hash
// of the signed contract!
//
// I might make OTKeycredential and then have OTSubkey and OTMasterkey both
// derive from that.
// That way the master key doesn't have to contain its own ID, while the subkey
// can still contain
// its master's ID.

/// OTKeyCredential
/// A form of OTSubcredential that contains 3 key pairs: signing,
/// authentication, and encryption.
/// We won't use OTKeyCredential directly but only as a common base class for
/// OTSubkey and OTMasterkey.
///
class OTKeyCredential : public OTSubcredential
{
private: // Private prevents erroneous use by other classes.
    typedef OTSubcredential ot_super;

protected:
    virtual bool SetPublicContents(const String::Map& mapPublic);
    virtual bool SetPrivateContents(
        const String::Map& mapPrivate,
        const OTPassword* pImportPassword = nullptr); // if not nullptr, it
                                                      // means to
                                                      // use
    // this password by default.
public:
    OTKeypair m_SigningKey; // Signing keys, for signing/verifying a "legal
                            // signature".
    OTKeypair m_AuthentKey; // Authentication keys, used for signing/verifying
                            // transmissions and stored files.
    OTKeypair m_EncryptKey; // Encryption keys, used for sealing/opening
                            // OTEnvelopes.
    bool GenerateKeys(int32_t nBits = 1024); // Gotta start somewhere.
    bool ReEncryptKeys(const OTPassword& theExportPassword,
                       bool bImporting); // Used when importing/exporting a Nym
                                         // to/from the wallet.
    virtual bool VerifyInternally(); // Verify that m_strNymID is the same as
                                     // the hash of m_strSourceForNymID. Also
                                     // verify that *this ==
                                     // m_pOwner->m_MasterKey (the master
                                     // credential.) Then verify the
                                     // (self-signed) signature on *this.
    bool VerifySignedBySelf();
    virtual void SetMetadata();
    OTKeyCredential();
    OTKeyCredential(OTCredential& theOwner);
    bool Sign(Contract& theContract, const OTPasswordData* pPWData = nullptr);
    EXPORT int32_t GetPublicKeysBySignature(
        listOfAsymmetricKeys& listOutput, const OTSignature& theSignature,
        char cKeyType = '0') const; // 'S' (signing key) or
                                    // 'E' (encryption key)
                                    // or 'A'
                                    // (authentication key)
    virtual ~OTKeyCredential();
    virtual void Release();
    void Release_Subkey();
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTKEYCREDENTIAL_HPP
