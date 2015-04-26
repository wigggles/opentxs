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

// A nym contains a list of credentials
//
// Each credential contains a "master" subkey, and a list of subkeys
// signed by that master.
//
// The same class (subkey) is used because there are master credentials
// and subkey credentials, so we're using a single "subkey" class to
// encapsulate each credential, both for the master credential and
// for each subkey credential.
//
// Each subkey has 3 key pairs: encryption, signing, and authentication.
//
// Each key pair has 2 OTAsymmetricKeys (public and private.)

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/crypto/OTMasterkey.hpp>

#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/OTCredential.hpp>
#include <opentxs/core/util/Tag.hpp>
#include <opentxs/core/Log.hpp>

#include <irrxml/irrXML.hpp>

// return -1 if error, 0 if nothing, and 1 if the node was processed.
//

namespace opentxs
{

int32_t OTMasterkey::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
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

        m_strMasterCredID.Release();

        otWarn << "Loading masterCredential...\n";

        nReturnVal = 1;
    }

    return nReturnVal;
}

void OTMasterkey::UpdateContents()
{
    m_xmlUnsigned.Release();

    Tag tag("masterCredential");

    // a hash of the nymIDSource
    tag.add_attribute("nymID", GetNymID().Get());

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
    //  if (OTSubcredential::credPublicInfo == m_StoreAs)   // PUBLIC INFO
    // (Always save this in every state.)
    {
        UpdatePublicContentsToTag(tag);
    }

    // PRIVATE INFO
    //
    // If we're saving the private credential info...
    //
    if (OTSubcredential::credPrivateInfo == m_StoreAs) // PRIVATE INFO
    {
        UpdatePublicCredentialToTag(tag);
        UpdatePrivateContentsToTag(tag);
    }
    // -------------------------------------------------
    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned.Concatenate("%s", str_result.c_str());

    m_StoreAs = OTSubcredential::credPrivateInfo; // <=== SET IT BACK TO DEFAULT
                                                  // BEHAVIOR. Any other state
                                                  // processes ONCE, and then
                                                  // goes back to this again.
}

// Verify that m_strNymID is the same as the hash of m_strSourceForNymID. Also
// verify that
// *this == m_pOwner->GetMasterkey() (the master credential.) Verify the
// (self-signed)
// signature on *this.
//
bool OTMasterkey::VerifyInternally()
{
    // Verify that m_strNymID is the same as the hash of m_strSourceForNymID.
    //
    // We can't use super here, since OTSubcredential::VerifyInternally will
    // verify
    // m_strMasterCredID against the actual Master, which is not relevant to us
    // in
    // OTMasterkey. But this means if we need anything else that
    // OTKeyCredential::VerifyInternally
    // was doing, we will have to duplicate that here as well...
    //  if (!ot_super::VerifyInternally())
    //      return false;
    if (!VerifyNymID()) return false;

    OT_ASSERT(nullptr != m_pOwner);
    // Verify that *this == m_pOwner->GetMasterkey() (the master credential.)
    //
    if (this != &(m_pOwner->GetMasterkey())) {
        otOut << __FUNCTION__ << ": Failure: Expected *this object to be the "
                                 "same as m_pOwner->GetMasterkey(), "
                                 "but it wasn't.\n";
        return false;
    }

    // Remember this note above: ...if we need anything else that
    // OTKeyCredential::VerifyInternally
    // was doing, we will have to duplicate that here as well...
    // Since we aren't calling OTKeyCredential::VerifyInternally (the super) and
    // since that function
    // verifies that the credential is self-signed, we must do the same
    // verification here:
    //
    // Any OTKeyCredential (both master and subkeys, but no other credentials)
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
bool OTMasterkey::VerifyAgainstSource() const
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
    // credentials can be verified (as well as subcredentials) from any source
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

bool OTMasterkey::VerifySource_HTTP(const String) const
{
    /*
     The source is a URL, http://blah.com/folder
     If I download files from there, will I find my own masterkey inside?
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

bool OTMasterkey::VerifySource_HTTPS(const String) const
{
    /*
     The source is a URL, https://blah.com/folder
     If I download files from there, will I find my own masterkey inside?
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

bool OTMasterkey::VerifySource_Bitcoin(const String) const
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

bool OTMasterkey::VerifySource_Namecoin(const String) const
{
    /*
     The source is a URL, http://blah.bit/folder
     If I download files from there, will I find my own masterkey inside?
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

bool OTMasterkey::VerifySource_Freenet(const String) const
{
    otErr << __FUNCTION__ << ": Failure: this function has not yet been "
                             "written, so this Freenet source cannot be "
                             "verified.\n";
    return false;
}

bool OTMasterkey::VerifySource_TOR(const String) const
{
    otErr << __FUNCTION__ << ": Failure: this function has not yet been "
                             "written, so this Tor source cannot be "
                             "verified.\n";
    return false;
}

bool OTMasterkey::VerifySource_I2P(const String) const
{
    otErr << __FUNCTION__ << ": Failure: this function has not yet been "
                             "written, so this I2P source cannot be "
                             "verified.\n";
    return false;
}

bool OTMasterkey::VerifySource_CA(const String) const
{

    /*
     The Source is the DN info on the Cert.
     Therefore look at the Cert being used in this Masterkey.
     Does it have the same DN info? Does it verify through its CA ?
     Then it verifies.
     */

    otErr << __FUNCTION__ << ": Failure: this function has not yet been "
                             "written, so this CA source cannot be verified.\n";
    return false;
}

bool OTMasterkey::VerifySource_Pubkey(const String) const
{
    // Verify signed by self.
    //
    // Note: Whenever VerifyAgainstSource is called, VerifyInternally is also
    // called.
    // And VerifyInternally, for all OTKeyCredentials, verifies already that the
    // credential has been signed by its own private signing key.
    // Since the credential is already verified as having signed itself, there's
    // no
    // reason to verify that redundantly here, so we just return true.
    //
    return true;
}

OTMasterkey::OTMasterkey()
    : ot_super()
{
    m_strContractType = "MASTER KEY CREDENTIAL";
}

OTMasterkey::OTMasterkey(OTCredential& theOwner)
    : ot_super(theOwner)
{
    m_strContractType = "MASTER KEY CREDENTIAL";
}

OTMasterkey::~OTMasterkey()
{
}

} // namespace opentxs
