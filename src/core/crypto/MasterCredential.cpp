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

#include <opentxs/core/crypto/MasterCredential.hpp>

#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/CredentialSet.hpp>
#include <opentxs/core/crypto/Credential.hpp>
#include <opentxs/core/util/Tag.hpp>
#include <opentxs/core/FormattedKey.hpp>
#include <opentxs/core/Log.hpp>

#include <irrxml/irrXML.hpp>

// return -1 if error, 0 if nothing, and 1 if the node was processed.
//

namespace opentxs
{

int32_t MasterCredential::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    int32_t nReturnVal = ot_super::ProcessXMLNode(xml);

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do in the case of OTAccount.
    //
    if (0 != nReturnVal) return nReturnVal;
    // else it was 0 (continue...)

    const String strNodeName(xml->getNodeName());

    if (strNodeName.Compare("masterCredential")) {
        m_strNymID = xml->getAttributeValue("nymID");

        String masterType = xml->getAttributeValue("type");

        Credential::CredentialType actualCredentialType = Credential::ERROR_TYPE;

        if (masterType.Exists()) {
            actualCredentialType = StringToCredentialType(masterType.Get());
        } else {
            actualCredentialType = Credential::LEGACY; //backward compatibility
        }

        OT_ASSERT(!m_AuthentKey);
        OT_ASSERT(!m_EncryptKey);
        OT_ASSERT(!m_SigningKey);

        if (Credential::ERROR_TYPE == actualCredentialType) {
            otErr << "Invalid master credential type.\n";
            return -1;
        } else {
            m_Type = actualCredentialType;
            OTAsymmetricKey::KeyType actualKeyType = Credential::CredentialTypeToKeyType(actualCredentialType);
            m_AuthentKey = std::make_shared<OTKeypair>(actualKeyType);
            m_EncryptKey = std::make_shared<OTKeypair>(actualKeyType);
            m_SigningKey = std::make_shared<OTKeypair>(actualKeyType);
        }

        m_strMasterCredID.Release();

        otWarn << "Loading masterCredential...\n";

        nReturnVal = 1;
    }

    return nReturnVal;
}

void MasterCredential::UpdateContents()
{
    m_xmlUnsigned.Release();

    Tag tag("masterCredential");

    // a hash of the nymIDSource
    tag.add_attribute("nymID", GetNymID().Get());

    String masterType = CredentialTypeToString(this->GetType());

    if (masterType.Exists()) {
        tag.add_attribute("type", masterType.Get());
    } else {
        tag.add_attribute("type", CredentialTypeToString(Credential::LEGACY).Get()); //backward compatibility
    }

    if (GetNymIDSource().Exists()) {
        OTASCIIArmor ascSource;
        ascSource.SetString(GetNymIDSource()); // A nym should always
                                               // verify through its own
                                               // source. (Whatever that
                                               // may be.)
        tag.add_tag("nymIDSource", ascSource.Get());
    }

    // PUBLIC INFO
    //
    //  if (Credential::credPublicInfo == m_StoreAs)   // PUBLIC INFO
    // (Always save this in every state.)
    {
        UpdatePublicContentsToTag(tag);
    }

    // PRIVATE INFO
    //
    // If we're saving the private credential info...
    //
    if (Credential::credPrivateInfo == m_StoreAs) // PRIVATE INFO
    {
        UpdatePublicCredentialToTag(tag);
        UpdatePrivateContentsToTag(tag);
    }
    // -------------------------------------------------
    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned.Concatenate("%s", str_result.c_str());

    m_StoreAs = Credential::credPrivateInfo; // <=== SET IT BACK TO DEFAULT
                                                  // BEHAVIOR. Any other state
                                                  // processes ONCE, and then
                                                  // goes back to this again.
}

// Verify that m_strNymID is the same as the hash of m_strSourceForNymID. Also
// verify that
// *this == m_pOwner->GetMasterCredential() (the master credential.) Verify the
// (self-signed)
// signature on *this.
//
bool MasterCredential::VerifyInternally()
{
    // Verify that m_strNymID is the same as the hash of m_strSourceForNymID.
    //
    // We can't use super here, since Credential::VerifyInternally will
    // verify
    // m_strMasterCredID against the actual Master, which is not relevant to us
    // in
    // MasterCredential. But this means if we need anything else that
    // KeyCredential::VerifyInternally
    // was doing, we will have to duplicate that here as well...
    //  if (!ot_super::VerifyInternally())
    //      return false;
    if (!VerifyNymID()) return false;

    OT_ASSERT(nullptr != m_pOwner);
    // Verify that *this == m_pOwner->GetMasterCredential() (the master credential.)
    //
    if (this != &(m_pOwner->GetMasterCredential())) {
        otOut << __FUNCTION__ << ": Failure: Expected *this object to be the "
                                 "same as m_pOwner->GetMasterCredential(), "
                                 "but it wasn't.\n";
        return false;
    }

    // Remember this note above: ...if we need anything else that
    // KeyCredential::VerifyInternally
    // was doing, we will have to duplicate that here as well...
    // Since we aren't calling KeyCredential::VerifyInternally (the super) and
    // since that function
    // verifies that the credential is self-signed, we must do the same
    // verification here:
    //
    // Any KeyCredential (both master and child, but no other credentials)
    // must ** sign itself.**
    //
    if (!VerifySignedBySelf()) {
        otOut << __FUNCTION__ << ": Failed verifying master credential: it's "
                                 "not signed by itself (its own signing "
                                 "key.)\n";
        return false;
    }

    return true;
}

// Should actually curl the URL, or lookup the blockchain value, or verify Cert
// against
// Cert Authority, etc. Due to the network slowdown of this step, we will
// eventually make
// a separate identity verification server.
//
bool MasterCredential::VerifyAgainstSource() const
{
    // RULE: *Any* source except for a public key, will begin with a
    // protocol specifier. Such as:
    //
    // http:        (a normal URL)
    // https:       (a normal URL on https)
    // bitcoin:     (a bitcoin address)
    // namecoin:    (a namecoin address)
    // i2p:         (an i2p address)
    // tor:         (a tor address)
    // freenet:     (a freenet address)
    // cert:        (Subject and Issuer DN from the cert)
    //
    // If NO protocol specifier is found, the source is assumed
    // to be a public key.
    // Public key is the default because that's the original behavior
    // of OT anyway: the public key was hashed to form the NymID. We will
    // continue to support this as a default, but now we are additionally
    // also allowing other sources such as Namecoin, Freenet, etc. As int64_t
    // as a Nym's source hashes to its correct ID, and as long as its master
    // credentials can be verified from that same source, then all master
    // credentials can be verified (as well as child credentials) from any source
    // the user prefers.
    //

    bool bVerified = false;

    const std::string str_raw_source(m_strSourceForNymID.Get());
    std::string str_source;

    // It's a URL.
    if (str_raw_source.compare(0, 5, "http:") == 0) {
        str_source.insert(str_source.begin(), str_raw_source.begin() + 5,
                          str_raw_source.end());
        bVerified = VerifySource_HTTP(str_source.c_str());
    }
    else if (str_raw_source.compare(0, 6, "https:") == 0) {
        str_source.insert(str_source.begin(), str_raw_source.begin() + 6,
                          str_raw_source.end());
        bVerified = VerifySource_HTTPS(str_source.c_str());
    }
    // It's a Bitcoin address.
    else if (str_raw_source.compare(0, 8, "bitcoin:") == 0) {
        str_source.insert(str_source.begin(), str_raw_source.begin() + 8,
                          str_raw_source.end());
        bVerified = VerifySource_Bitcoin(str_source.c_str());
    }
    // It's a Namecoin address.
    else if (str_raw_source.compare(0, 9, "namecoin:") == 0) {
        str_source.insert(str_source.begin(), str_raw_source.begin() + 9,
                          str_raw_source.end());
        bVerified = VerifySource_Namecoin(str_source.c_str());
    }
    // It's a Freenet URL.
    else if (str_raw_source.compare(0, 8, "freenet:") == 0) {
        str_source.insert(str_source.begin(), str_raw_source.begin() + 8,
                          str_raw_source.end());
        bVerified = VerifySource_Freenet(str_source.c_str());
    }
    // It's a Tor URL.
    else if (str_raw_source.compare(0, 4, "tor:") == 0) {
        str_source.insert(str_source.begin(), str_raw_source.begin() + 4,
                          str_raw_source.end());
        bVerified = VerifySource_TOR(str_source.c_str());
    }
    // It's an I2P URL.
    else if (str_raw_source.compare(0, 4, "i2p:") == 0) {
        str_source.insert(str_source.begin(), str_raw_source.begin() + 4,
                          str_raw_source.end());
        bVerified = VerifySource_I2P(str_source.c_str());
    }
    // It's the Issuer/Subject DN info from a cert issued by a traditional
    // certificate authority.
    else if (str_raw_source.compare(0, 5, "cert:") == 0) {
        str_source.insert(str_source.begin(), str_raw_source.begin() + 5,
                          str_raw_source.end());
        bVerified = VerifySource_CA(str_source.c_str());
    }
    else // It's presumably a public key.
    {
        str_source = str_raw_source;
        bVerified = VerifySource_Pubkey(str_source.c_str());
    }

    return bVerified;
}

bool MasterCredential::VerifySource_HTTP(const String) const
{
    /*
     The source is a URL, http://blah.com/folder
     If I download files from there, will I find my own master credential inside?
     If so, then I verify.
     */

    otErr << __FUNCTION__ << ": Failure: this function has not yet been "
                             "written, so this HTTP source cannot be "
                             "verified.\n";
    //    return false;

    // Todo security
    otErr << "\nNOTE: Returning TRUE for TESTING PURPOSES, as if HTTP source "
             "had verified."
             "\n\n\n ----------------------- \n\n";

    return true;
}

bool MasterCredential::VerifySource_HTTPS(const String) const
{
    /*
     The source is a URL, https://blah.com/folder
     If I download files from there, will I find my own master credential inside?
     If so, then I verify.
     */

    otErr << __FUNCTION__ << ": Failure: this function has not yet been "
                             "written, so this HTTPS source cannot be "
                             "verified.\n";
    //    return false;

    // Todo security
    otErr << "\nNOTE: Returning TRUE for TESTING PURPOSES, as if HTTPS source "
             "had verified."
             "\n\n\n ----------------------- \n\n";

    return true;
}

bool MasterCredential::VerifySource_Bitcoin(const String) const
{
    /*
     The source is a Bitcoin address
     The last transfer from that address should have memo data with the hash of
     the master credential.
     I compare that to my own ID and they should match.
     Alternately, to support multiple master credentials, have the last transfer
     go to multiple addresses,
     and each should have a memo with the master cred ID for each credential,
     one of which should match my own.
     If so, then I verify.
     */

    otErr << __FUNCTION__ << ": Failure: this function has not yet been "
                             "written, so this Bitcoin source cannot be "
                             "verified.\n";
    //    return false;

    // Todo security
    otErr << "\nNOTE: Returning TRUE for TESTING PURPOSES, as if Bitcoin had "
             "verified."
             "\n\n\n ----------------------- \n\n";

    return true;
}

bool MasterCredential::VerifySource_Namecoin(const String) const
{
    /*
     The source is a URL, http://blah.bit/folder
     If I download files from there, will I find my own master credential inside?
     If so, then I verify.
     */

    otErr << __FUNCTION__ << ": Failure: this function has not yet been "
                             "written, so this Namecoin source cannot be "
                             "verified.\n";
    //    return false;

    // Todo security
    otErr << "\nNOTE: Returning TRUE for TESTING PURPOSES, as if Namecoin had "
             "verified."
             "\n\n\n ----------------------- \n\n";

    return true;
}

bool MasterCredential::VerifySource_Freenet(const String) const
{
    otErr << __FUNCTION__ << ": Failure: this function has not yet been "
                             "written, so this Freenet source cannot be "
                             "verified.\n";
    return false;
}

bool MasterCredential::VerifySource_TOR(const String) const
{
    otErr << __FUNCTION__ << ": Failure: this function has not yet been "
                             "written, so this Tor source cannot be "
                             "verified.\n";
    return false;
}

bool MasterCredential::VerifySource_I2P(const String) const
{
    otErr << __FUNCTION__ << ": Failure: this function has not yet been "
                             "written, so this I2P source cannot be "
                             "verified.\n";
    return false;
}

bool MasterCredential::VerifySource_CA(const String) const
{

    /*
     The Source is the DN info on the Cert.
     Therefore look at the Cert being used in this master credential.
     Does it have the same DN info? Does it verify through its CA ?
     Then it verifies.
     */

    otErr << __FUNCTION__ << ": Failure: this function has not yet been "
                             "written, so this CA source cannot be verified.\n";
    return false;
}

bool MasterCredential::VerifySource_Pubkey(const String) const
{
    // Verify signed by self.
    //
    // Note: Whenever VerifyAgainstSource is called, VerifyInternally is also
    // called.
    // And VerifyInternally, for all KeyCredentials, verifies already that the
    // credential has been signed by its own private signing key.
    // Since the credential is already verified as having signed itself, there's
    // no
    // reason to verify that redundantly here, so we just return true.
    //
    return true;
}

MasterCredential::MasterCredential(CredentialSet& theOwner)
    : ot_super(theOwner)
{
    m_strContractType = "MASTER KEY CREDENTIAL";
    m_Role = proto::CREDROLE_MASTERKEY;
}

MasterCredential::MasterCredential(CredentialSet& theOwner, const Credential::CredentialType masterType)
    : ot_super(theOwner, masterType)
{
    m_strContractType = "MASTER KEY CREDENTIAL";
    m_Role = proto::CREDROLE_MASTERKEY;
}

MasterCredential::MasterCredential(CredentialSet& theOwner, serializedCredential serializedCred)
: ot_super(theOwner, serializedCred)
{
    m_strContractType = "MASTER KEY CREDENTIAL";
    m_Role = proto::CREDROLE_MASTERKEY;

    m_strSourceForNymID = serializedCred->publiccredential().masterdata().source().raw();
}

MasterCredential::MasterCredential(CredentialSet& theOwner, const NymParameters& nymParameters)
    : ot_super(theOwner, nymParameters)
{
    m_strContractType = "MASTER KEY CREDENTIAL";
    m_Role = proto::CREDROLE_MASTERKEY;

    if (nymParameters.Source().size() > 0) {
        m_strSourceForNymID = nymParameters.Source();
    }
    else {
        FormattedKey sourceForNymID;
        m_SigningKey->GetPublicKey(sourceForNymID);
        m_strSourceForNymID = sourceForNymID;
    }
}

MasterCredential::~MasterCredential()
{
}

serializedCredential MasterCredential::Serialize(bool asPrivate, bool asSigned) const
{
    serializedCredential serializedCredential =
        this->ot_super::Serialize(asPrivate, asSigned);

    proto::MasterCredentialParameters* parameters = new proto::MasterCredentialParameters;
    proto::nymIDSource* source = new proto::nymIDSource;

    std::string* legacySource = new std::string;
    *legacySource = m_strSourceForNymID.Get();

    source->set_version(1);
    source->set_type(proto::SOURCETYPE_SELF);
    source->set_allocated_raw(legacySource);

    parameters->set_version(1);
    parameters->set_allocated_source(source);

    // Only the public credential gets MasterCredentialParameters
    if (serializedCredential->has_publiccredential()) {
        proto::KeyCredential* keyCredential = serializedCredential->mutable_publiccredential();
        keyCredential->set_allocated_masterdata(parameters);

    }

    serializedCredential->set_role(proto::CREDROLE_MASTERKEY);

    return serializedCredential;
}

} // namespace opentxs
