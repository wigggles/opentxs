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

#include <opentxs/core/crypto/OTSubcredential.hpp>

#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/OTCredential.hpp>
#include <opentxs/core/util/Tag.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/OTStorage.hpp>

#include <irrxml/irrXML.hpp>

// Contains 3 key pairs: signing, authentication, and encryption.
// This is stored as an OTContract, and it must be signed by the
// master key. (which is also an OTSubcredential.)
//

namespace opentxs
{

void OTSubcredential::SetOwner(OTCredential& theOwner)
{
    m_pOwner = &theOwner;
}

OTSubcredential::OTSubcredential()
    : Contract()
    , m_StoreAs(OTSubcredential::credPrivateInfo)
    , m_pOwner(nullptr)
{
    m_strContractType = "CREDENTIAL";
}

OTSubcredential::OTSubcredential(OTCredential& theOwner)
    : Contract()
    , m_StoreAs(OTSubcredential::credPrivateInfo)
    , m_pOwner(&theOwner)
{
    m_strContractType = "CREDENTIAL";
}

OTSubcredential::~OTSubcredential()
{
    Release_Subcredential();
}

// virtual
void OTSubcredential::Release()
{
    Release_Subcredential(); // My own cleanup is done here.

    // Next give the base class a chance to do the same...
    Contract::Release(); // since I've overridden the base class, I call it
                         // now...
}

void OTSubcredential::Release_Subcredential()
{
    // Release any dynamically allocated members here. (Normally.)
}

// virtual
bool OTSubcredential::SetPublicContents(const String::Map& mapPublic)
{
    m_mapPublicInfo = mapPublic;
    return true;
}

// virtual
bool OTSubcredential::SetPrivateContents(const String::Map& mapPrivate,
                                         const OTPassword*) // if not nullptr,
                                                            // it means to
                                                            // use this
                                                            // password by
                                                            // default.)
{
    m_mapPrivateInfo = mapPrivate;
    return true;
}

void OTSubcredential::SetMasterCredID(const String& strMasterCredID)
{
    m_strMasterCredID = strMasterCredID;
}

void OTSubcredential::SetNymIDandSource(const String& strNymID,
                                        const String& strSourceForNymID)
{
    m_strNymID = strNymID;
    m_strSourceForNymID = strSourceForNymID;
}

void OTSubcredential::UpdatePublicContentsToTag(Tag& parent) // Used in
                                                             // UpdateContents.
{
    if (!m_mapPublicInfo.empty()) {
        uint64_t theSize = m_mapPublicInfo.size();

        TagPtr tagContents(new Tag("publicContents"));
        tagContents->add_attribute("count", formatUlong(theSize));

        for (auto& it : m_mapPublicInfo) {
            String strInfo(it.second);
            OTASCIIArmor ascInfo(strInfo);

            TagPtr tagInfo(new Tag("publicInfo", ascInfo.Get()));
            tagInfo->add_attribute("key", it.first);

            tagContents->add_tag(tagInfo);
        }
        parent.add_tag(tagContents);
    }
}

void OTSubcredential::UpdatePublicCredentialToTag(
    Tag& parent) // Used in UpdateContents.
{
    if (GetContents().Exists()) {
        OTASCIIArmor ascContents(GetContents());
        if (ascContents.Exists()) {
            parent.add_tag("publicCredential", ascContents.Get());
        }
    }
}

void OTSubcredential::UpdatePrivateContentsToTag(Tag& parent) // Used in
                                                              // UpdateContents.
{
    if (!m_mapPrivateInfo.empty()) {
        uint64_t theSize = m_mapPrivateInfo.size();

        TagPtr tagContents(new Tag("privateContents"));
        tagContents->add_attribute("count", formatUlong(theSize));

        for (auto& it : m_mapPrivateInfo) {
            String strInfo(it.second);
            OTASCIIArmor ascInfo(strInfo);

            TagPtr tagInfo(new Tag("privateInfo", ascInfo.Get()));
            tagInfo->add_attribute("key", it.first);

            tagContents->add_tag(tagInfo);
        }
        parent.add_tag(tagContents);
    }
}

void OTSubcredential::UpdateContents()
{
    m_xmlUnsigned.Release();

    Tag tag("subCredential");

    // a hash of the nymIDSource
    tag.add_attribute("nymID", GetNymID().Get());
    // Hash of the master credential that signed
    // this subcredential.
    tag.add_attribute("masterID", GetMasterCredID().Get());

    if (GetNymIDSource().Exists()) {
        OTASCIIArmor ascSource;
        ascSource.SetString(GetNymIDSource()); // A nym should always
                                               // verify through its own
                                               // source. (Whatever that
                                               // may be.)
        tag.add_tag("nymIDSource", ascSource.Get());
    }

    //  if (OTSubcredential::credPublicInfo == m_StoreAs)  // (Always saving
    // public info.)
    {
        // PUBLIC INFO
        UpdatePublicContentsToTag(tag);
    }

    // If we're saving the private credential info...
    //
    if (OTSubcredential::credPrivateInfo == m_StoreAs) {
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

// return -1 if error, 0 if nothing, and 1 if the node was processed.
//
int32_t OTSubcredential::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    int32_t nReturnVal = 0;

    const String strNodeName(xml->getNodeName());

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do in the case of OTAccount.
    //
    // if (nReturnVal = OTContract::ProcessXMLNode(xml))
    //      return nReturnVal;

    if (strNodeName.Compare("subCredential")) {
        m_strNymID = xml->getAttributeValue("nymID");
        m_strMasterCredID = xml->getAttributeValue("masterID");

        otWarn << "Loading subcredential...\n";

        nReturnVal = 1;
    }
    else if (strNodeName.Compare("nymIDSource")) {
        otWarn << "Loading nymIDSource...\n";

        OTASCIIArmor ascTemp;
        if (!Contract::LoadEncodedTextField(xml, ascTemp)) {
            otErr << "Error in " << __FILE__ << " line " << __LINE__
                  << ": failed loading expected nymIDSource field.\n";
            return (-1); // error condition
        }
        if (ascTemp.Exists()) ascTemp.GetString(m_strSourceForNymID);

        nReturnVal = 1;
    }
    else if (strNodeName.Compare("publicContents")) {
        String strCount;
        strCount = xml->getAttributeValue("count");
        const int32_t nCount = strCount.Exists() ? atoi(strCount.Get()) : 0;
        if (nCount > 0) {
            int32_t nTempCount = nCount;
            String::Map mapPublic;

            while (nTempCount-- > 0) {

                const char* pElementExpected = "publicInfo";
                String strPublicInfo;

                // This map contains values we will also want, when we read the
                // info...
                // (The OTContract::LoadEncodedTextField call below will read
                // all the values
                // as specified in this map.)
                //
                String::Map temp_MapAttributes;
                temp_MapAttributes.insert(std::pair<std::string, std::string>(
                    "key",
                    "")); // Value should be "A" or "E" or "S" after reading.

                if (!Contract::LoadEncodedTextFieldByName(
                        xml, strPublicInfo, pElementExpected,
                        &temp_MapAttributes)) // </publicInfo>
                {
                    otErr << __FUNCTION__ << ": Error: Expected "
                          << pElementExpected << " element with text field.\n";
                    return (-1); // error condition
                }

                auto it = temp_MapAttributes.find("key");
                if ((it != temp_MapAttributes.end())) // We expected this much.
                {
                    std::string& str_key = it->second;

                    if (str_key.size() >
                        0) // Success finding key type ('A' 'E' or 'S')
                    {

                        mapPublic.insert(std::pair<std::string, std::string>(
                            str_key, strPublicInfo.Get()));

                    }
                    // else it's empty, which is expected if nothing was there,
                    // since that's the default value
                    // that we set above for "name" in temp_MapAttributes.
                    else {
                        otErr << __FUNCTION__
                              << ": Expected key type of 'A' or 'E' or 'S'.\n";
                        return (-1); // error condition
                    }
                }
                else {
                    otErr << __FUNCTION__
                          << ": Strange error: couldn't find key type AT "
                             "ALL.\n"; // should never happen.
                    return (-1);       // error condition
                }
            } // while

            if (static_cast<int64_t>(mapPublic.size()) != nCount) {
                otErr << __FUNCTION__ << ", " << __FILE__ << ", " << __LINE__
                      << ": Subcredential expected to load " << nCount
                      << " publicInfo objects, "
                         "but actually loaded " << mapPublic.size()
                      << ". (Mismatch, failure loading.)\n";
                return (-1); // error condition
            }

            if (false ==
                SetPublicContents(mapPublic)) // <==============  Success.
            {
                otErr << __FUNCTION__ << ", " << __FILE__ << ", " << __LINE__
                      << ": Subcredential failed setting public contents while "
                         "loading.\n";
                return (-1); // error condition
            }

        } // if strCount.Exists() && nCount > 0

        otInfo << "Loaded publicContents for subcredential.\n";

        nReturnVal = 1;
    }
    else if (strNodeName.Compare("publicCredential")) {
        if (!Contract::LoadEncodedTextField(
                xml, m_strContents)) // <========= m_strContents.
        {
            otErr << "Error in " << __FILE__ << " line " << __LINE__
                  << ": failed loading expected public credential while "
                     "loading private subcredential.\n";
            return (-1); // error condition
        }

        nReturnVal = 1;
    }
    else if (strNodeName.Compare("privateContents")) {
        String strCount;
        strCount = xml->getAttributeValue("count");
        const int32_t nCount = strCount.Exists() ? atoi(strCount.Get()) : 0;
        if (nCount > 0) {
            int32_t nTempCount = nCount;
            String::Map mapPrivate;

            while (nTempCount-- > 0) {

                const char* pElementExpected = "privateInfo";
                String strPrivateInfo;

                // This map contains values we will also want, when we read the
                // info...
                // (The OTContract::LoadEncodedTextField call below will read
                // all the values
                // as specified in this map.)
                //
                String::Map temp_MapAttributes;
                temp_MapAttributes.insert(std::pair<std::string, std::string>(
                    "key",
                    "")); // Value should be "A" or "E" or "S" after reading.

                if (!Contract::LoadEncodedTextFieldByName(
                        xml, strPrivateInfo, pElementExpected,
                        &temp_MapAttributes)) // </privateInfo>
                {
                    otErr << __FUNCTION__ << ": Error: Expected "
                          << pElementExpected << " element with text field.\n";
                    return (-1); // error condition
                }

                auto it = temp_MapAttributes.find("key");
                if ((it != temp_MapAttributes.end())) // We expected this much.
                {
                    std::string& str_key = it->second;

                    if (str_key.size() >
                        0) // Success finding key type ('A' 'E' or 'S')
                    {

                        mapPrivate.insert(std::pair<std::string, std::string>(
                            str_key, strPrivateInfo.Get()));

                    }
                    // else it's empty, which is expected if nothing was there,
                    // since that's the default value
                    // that we set above for "name" in temp_MapAttributes.
                    else {
                        otErr << __FUNCTION__
                              << ": Expected key type of 'A' or 'E' or 'S'.\n";
                        return (-1); // error condition
                    }
                }
                else {
                    otErr << __FUNCTION__
                          << ": Strange error: couldn't find key type AT "
                             "ALL.\n"; // should never happen.
                    return (-1);       // error condition
                }
            } // while

            if (static_cast<int64_t>(mapPrivate.size()) != nCount) {
                otErr << __FUNCTION__ << ", " << __FILE__ << ", " << __LINE__
                      << ": Subcredential expected to load " << nCount
                      << " privateInfo objects, "
                         "but actually loaded " << mapPrivate.size()
                      << ". (Mismatch, failure loading.)\n";
                return (-1); // error condition
            }

            OT_ASSERT(nullptr != m_pOwner);

            // Sometimes we are supposed to use a specific, pre-specified
            // password (versus just
            // blindly asking the user to type a password when it's not cached
            // alrady.) For example,
            // when importing a Nym, the exported version of the Nym is not
            // encrypted to the cached
            // wallet key. It's got its own exported passphrase. So it won't be
            // cached, and we will have
            // the wallet's cached key disabled while loading it. This means we
            // have to enter the same
            // passphrase many times in a row, while OT loads up all the
            // credentials and keys for that
            // Nym. Therefore, instead, we ask the user up front to enter the
            // special passphrase for that
            // exported Nym. Then we just pass it in to all the functions that
            // need it, so none of them have
            // to ask the user to type it.
            //
            // That's what brings us here now... when that happens,
            // m_pOwner->GetImportPassword() will be set
            // with the appropriate pointer to the passphrase. Otherwise it will
            // be nullptr. Meanwhile SetPrivateContents
            // below accepts an import passphrase, which it defaults to nullptr.
            //
            // So we just pass it in either way (sometimes it's nullptr and the
            // wallet cached master key is used, and
            // sometimes an actual passphrase is passed in, so we use it.)

            if (false ==
                SetPrivateContents(mapPrivate, m_pOwner->GetImportPassword())) {
                otErr << __FUNCTION__ << ", " << __FILE__ << ", " << __LINE__
                      << ": Subcredential failed setting private contents "
                         "while loading.\n";
                return (-1); // error condition
            }

        } // if strCount.Exists() && nCount > 0

        otInfo << "Loaded privateContents for subcredential.\n";

        nReturnVal = 1;
    }

    return nReturnVal;
}

// VERIFICATION

// Verify that m_strNymID is the same as the hash of m_strSourceForNymID.
//
bool OTSubcredential::VerifyNymID() const
{

    // Verify that m_strNymID is the same as the hash of m_strSourceForNymID.
    //
    Identifier theTempID;
    const bool bCalculate = theTempID.CalculateDigest(m_strSourceForNymID);
    OT_ASSERT(bCalculate);

    const String strNymID(theTempID);
    if (!m_strNymID.Compare(strNymID)) {
        otOut << __FUNCTION__
              << ": Failure: When the NymID's source is hashed, the result "
                 "doesn't match the expected NymID.\n"
                 "Expected: " << m_strNymID << "\n   Found: " << strNymID
              << "\n  Source: " << m_strSourceForNymID << "\n";
        return false;
    }

    return true;
}

// Call VerifyNymID. Then verify m_strMasterCredID against the hash of
// m_pOwner->GetMasterkey().GetPubCredential() (the master credential.) Verify
// that
// m_pOwner->GetMasterkey() and *this have the same NymID. Then verify the
// signature of m_pOwner->GetMasterkey().
//
bool OTSubcredential::VerifyInternally()
{
    OT_ASSERT(nullptr != m_pOwner);

    // Verify that m_strNymID is the same as the hash of m_strSourceForNymID.
    //
    if (!VerifyNymID()) return false;

    // Verify that m_pOwner->GetMasterkey() and *this have the same NymID.
    //
    if (!m_strNymID.Compare(m_pOwner->GetMasterkey().GetNymID())) {
        otOut << __FUNCTION__
              << ": Failure: The actual master credential's NymID doesn't "
                 "match the NymID on this subcredential.\n"
                 "    This NymID: " << m_strNymID
              << "\nMaster's NymID: " << m_pOwner->GetMasterkey().GetNymID()
              << "\n My Master Cred ID: " << m_strMasterCredID << "\n";
        return false;
    }

    // Verify m_strMasterCredID against the hash of
    // m_pOwner->GetMasterkey().GetPubCredential()
    // (the master credentialID is a hash of the master credential.)
    //
    Identifier theActualMasterID;
    const bool bCalcMasterCredID =
        theActualMasterID.CalculateDigest(m_pOwner->GetPubCredential());
    OT_ASSERT(bCalcMasterCredID);
    const String strActualMasterID(theActualMasterID);

    if (!m_strMasterCredID.Compare(strActualMasterID)) {
        otOut << __FUNCTION__
              << ": Failure: When the actual Master Credential is hashed, the "
                 "result doesn't match the expected Master Credential ID.\n"
                 "Expected: " << m_strMasterCredID
              << "\n   Found: " << strActualMasterID << "\nMaster Cred:\n"
              << m_pOwner->GetPubCredential() << "\n";
        return false;
    }

    // Then verify the signature of m_pOwner->GetMasterkey()...
    // Let's get a few things straight:
    // * OTMasterkey is a key (derived from OTKeyCredential, derived from
    // OTSubcredential) and it can only sign itself.
    // * The only further verification a Masterkey can get is if its hash is
    // posted at the source. Or, if the source
    //   is a public key, then the master key must be signed by the
    // corresponding private key. (Again: itself.)
    // * Conversely to a master key which can ONLY sign itself, all subkeys must
    // ALSO sign themselves.
    //
    // * Thus: Any OTKeyCredential (both master and subkeys, but no other
    // credentials) must ** sign itself.**
    // * Whereas m_strMasterSigned is only used by OTSubkey, and thus only must
    // be verified there.
    // * Any OTSubcredential must also be signed by its master. (Except masters,
    // which already sign themselves.)
    // * Any OTMasterkey must (at some point, and/or regularly) verify against
    // its own source.

    // * Any OTSubcredential must also be signed by its master. (Except masters,
    // which already sign themselves.)
    //
    if (!VerifySignedByMaster()) {
        otOut << __FUNCTION__ << ": Failure: This subcredential hasn't been "
                                 "signed by its master credential.\n";
        return false;
    }

    return true;
}

bool OTSubcredential::VerifySignedByMaster()
{
    OT_ASSERT(nullptr != m_pOwner);
    return VerifyWithKey(m_pOwner->GetMasterkey().m_SigningKey.GetPublicKey());
}

bool OTSubcredential::VerifyContract()
{
    if (!VerifyContractID()) {
        otWarn << __FUNCTION__ << ": Failed verifying credential ID against "
                                  "whatever it was expected to be.\n";
        return false;
    }

    if (!VerifyInternally()) // Logs copiously.
        return false;

    return true;
}

// Overriding from OTContract.
void OTSubcredential::CalculateContractID(Identifier& newID) const
{
    if (!newID.CalculateDigest(GetPubCredential()))
        otErr << __FUNCTION__ << ": Error calculating credential digest.\n";
}

// I needed this for exporting a Nym (with credentials) from the wallet.
const String& OTSubcredential::GetPriCredential() const
{
    OT_ASSERT_MSG(!m_mapPrivateInfo.empty(), "ASSERT: GetPriCredential can "
                                             "only be called on private "
                                             "subcredentials.");

    return m_strRawFile;
}

// We don't want to have to figure this out each time we need the public
// credential, so we just
// call this function wherever we need to get the public credential.
//
const String& OTSubcredential::GetPubCredential() const // More intelligent
                                                        // version of
                                                        // GetContents. Higher
                                                        // level.
{
    // If this is a private (client-side) credential containing private keys,
    // then the public version is stored in GetContents(), and return that.
    if ((!m_mapPrivateInfo.empty()) && GetContents().Exists())
        return GetContents();

    // Otherwise this is a server-side copy of a Nym's credential, with no
    // private keys inside it.
    // In which case public info that would have been in GetContents (making
    // room so we can have
    // the private keys in m_strRawFile) is now directly found in m_strRawFile,
    // since that's all the
    // server ever loads up. There is only public info and nothing else, so it's
    // found in the main
    // normal location, m_strRawFile.
    //
    return m_strRawFile;
}

} // namespace opentxs
