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
#include <opentxs/core/verify/Verify.hpp>

#include <opentxs/core/crypto/Credential.hpp>

#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/CredentialSet.hpp>
#include <opentxs/core/util/Tag.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/OTStorage.hpp>

#include <irrxml/irrXML.hpp>

#include <map>

// Contains 3 key pairs: signing, authentication, and encryption.
// This is stored as an Contract, and it must be signed by the
// master key. (which is also an Credential.)
//

namespace opentxs
{

void Credential::SetOwner(CredentialSet& theOwner)
{
    m_pOwner = &theOwner;
}

Credential::Credential(CredentialSet& theOwner)
    : Contract()
    , m_StoreAs(Credential::credPrivateInfo)
    , m_Type(Credential::ERROR_TYPE)
    , m_pOwner(&theOwner)
{
    m_strContractType = "CREDENTIAL";
}

Credential::Credential(CredentialSet& theOwner, Credential::CredentialType type, proto::KeyMode mode)
    : Contract()
    , m_StoreAs(Credential::credPrivateInfo)
    , m_Type(type)
    , m_mode(mode)
    , m_pOwner(&theOwner)
{
    m_strContractType = "CREDENTIAL";
}

Credential::Credential(CredentialSet& theOwner, serializedCredential serializedCred)
    : Credential(theOwner, Credential::LEGACY, serializedCred->mode())
{
    m_Role = serializedCred->role();
    m_strNymID = serializedCred->nymid();
    SetIdentifier(serializedCred->id());
}

Credential::~Credential()
{
    Release_Credential();
}

// virtual
void Credential::Release()
{
    Release_Credential(); // My own cleanup is done here.

    // Next give the base class a chance to do the same...
    Contract::Release(); // since I've overridden the base class, I call it
                         // now...
}

void Credential::Release_Credential()
{
    // Release any dynamically allocated members here. (Normally.)
}

// virtual
bool Credential::SetPublicContents(const String::Map& mapPublic)
{
    m_mapPublicInfo = mapPublic;
    return true;
}

// virtual
bool Credential::SetPrivateContents(const String::Map& mapPrivate,
                                         const OTPassword*) // if not nullptr,
                                                            // it means to
                                                            // use this
                                                            // password by
                                                            // default.)
{
    m_mapPrivateInfo = mapPrivate;
    return true;
}

void Credential::SetMasterCredID(const String& strMasterCredID)
{
    m_strMasterCredID = strMasterCredID;
}

void Credential::SetNymIDandSource(const String& strNymID,
                                        const String& strSourceForNymID)
{
    m_strNymID = strNymID;
    m_strSourceForNymID = strSourceForNymID;
}

void Credential::UpdatePublicContentsToTag(Tag& parent) // Used in
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

void Credential::UpdatePublicCredentialToTag(
    Tag& parent) // Used in UpdateContents.
{
    if (GetContents().Exists()) {
        OTASCIIArmor ascContents(GetContents());
        if (ascContents.Exists()) {
            parent.add_tag("publicCredential", ascContents.Get());
        }
    }
}

void Credential::UpdatePrivateContentsToTag(Tag& parent) // Used in
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

void Credential::UpdateContents()
{
    m_xmlUnsigned.Release();

    Tag tag(CredentialObjectName);

    // a hash of the nymIDSource
    tag.add_attribute("nymID", GetNymID().Get());
    // Hash of the master credential that signed
    // this child credential.
    tag.add_attribute("masterID", GetMasterCredID().Get());

    if (GetNymIDSource().Exists()) {
        OTASCIIArmor ascSource;
        ascSource.SetString(GetNymIDSource()); // A nym should always
                                               // verify through its own
                                               // source. (Whatever that
                                               // may be.)
        tag.add_tag("nymIDSource", ascSource.Get());
    }

    //  if (Credential::credPublicInfo == m_StoreAs)  // (Always saving
    // public info.)
    {
        // PUBLIC INFO
        UpdatePublicContentsToTag(tag);
    }

    // If we're saving the private credential info...
    //
    if (Credential::credPrivateInfo == m_StoreAs) {
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

// return -1 if error, 0 if nothing, and 1 if the node was processed.
//
int32_t Credential::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
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
    // if (nReturnVal = Contract::ProcessXMLNode(xml))
    //      return nReturnVal;

    if (strNodeName.Compare(CredentialObjectName) || strNodeName.Compare(CredentialObjectNameOld)) {
        m_strNymID = xml->getAttributeValue("nymID");
        m_strMasterCredID = xml->getAttributeValue("masterID");

        otWarn << "Loading child credential...\n";

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
                // (The Contract::LoadEncodedTextField call below will read
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
                      << ": Child credential expected to load " << nCount
                      << " publicInfo objects, "
                         "but actually loaded " << mapPublic.size()
                      << ". (Mismatch, failure loading.)\n";
                return (-1); // error condition
            }

            if (false ==
                SetPublicContents(mapPublic)) // <==============  Success.
            {
                otErr << __FUNCTION__ << ", " << __FILE__ << ", " << __LINE__
                      << ": Child credential failed setting public contents while "
                         "loading.\n";
                return (-1); // error condition
            }

        } // if strCount.Exists() && nCount > 0

        otInfo << "Loaded publicContents for child credential.\n";

        nReturnVal = 1;
    }
    else if (strNodeName.Compare("publicCredential")) {
        if (!Contract::LoadEncodedTextField(
                xml, m_strContents)) // <========= m_strContents.
        {
            otErr << "Error in " << __FILE__ << " line " << __LINE__
                  << ": failed loading expected public credential while "
                     "loading private child credential.\n";
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
                // (The Contract::LoadEncodedTextField call below will read
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
                      << ": Child credential expected to load " << nCount
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
                      << ": Child credential failed setting private contents "
                         "while loading.\n";
                return (-1); // error condition
            }

        } // if strCount.Exists() && nCount > 0

        otInfo << "Loaded privateContents for child credential.\n";

        nReturnVal = 1;
    }

    return nReturnVal;
}

// VERIFICATION

// Verify that m_strNymID is the same as the hash of m_strSourceForNymID.
//
bool Credential::VerifyNymID() const
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
// m_pOwner->GetMasterCredential().GetPubCredential() (the master credential.) Verify
// that
// m_pOwner->GetMasterCredential() and *this have the same NymID. Then verify the
// signature of m_pOwner->GetMasterCredential().
//
bool Credential::VerifyInternally()
{
    OT_ASSERT(nullptr != m_pOwner);

    // Verify that m_strNymID is the same as the hash of m_strSourceForNymID.
    //
    if (!VerifyNymID()) return false;

    // Verify that m_pOwner->GetMasterCredential() and *this have the same NymID.
    //
    if (!m_strNymID.Compare(m_pOwner->GetMasterCredential().GetNymID())) {
        otOut << __FUNCTION__
              << ": Failure: The actual master credential's NymID doesn't "
                 "match the NymID on this child credential.\n"
                 "    This NymID: " << m_strNymID
              << "\nMaster's NymID: " << m_pOwner->GetMasterCredential().GetNymID()
              << "\n My Master Cred ID: " << m_strMasterCredID << "\n";
        return false;
    }

    // Verify m_strMasterCredID against the hash of
    // m_pOwner->GetMasterCredential().GetPubCredential()
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

    // Then verify the signature of m_pOwner->GetMasterCredential()...
    // Let's get a few things straight:
    // * MasterCredential is a key (derived from KeyCredential, derived from
    // Credential) and it can only sign itself.
    // * The only further verification a master credential can get is if its hash is
    // posted at the source. Or, if the source
    //   is a public key, then the master key must be signed by the
    // corresponding private key. (Again: itself.)
    // * Conversely to a master credential which can ONLY sign itself, all key credentials must
    // ALSO sign themselves.
    //
    // * Thus: Any KeyCredential (both master and child, but no other
    // credentials) must ** sign itself.**
    // * Whereas m_strMasterSigned is only used by ChildKeyCredential, and thus only must
    // be verified there.
    // * Any Credential must also be signed by its master. (Except masters,
    // which already sign themselves.)
    // * Any MasterCredential must (at some point, and/or regularly) verify against
    // its own source.

    // * Any Credential must also be signed by its master. (Except masters,
    // which already sign themselves.)
    //
    if (!VerifySignedByMaster()) {
        otOut << __FUNCTION__ << ": Failure: This child credential hasn't been "
                                 "signed by its master credential.\n";
        return false;
    }

    return true;
}

bool Credential::VerifySignedByMaster()
{
    OT_ASSERT(nullptr != m_pOwner);
    OT_ASSERT(m_pOwner->GetMasterCredential().m_SigningKey);
    return VerifyWithKey(m_pOwner->GetMasterCredential().m_SigningKey->GetPublicKey());
}

bool Credential::VerifyContract()
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

// Overriding from Contract.
void Credential::CalculateContractID(Identifier& newID) const
{
    if (!newID.CalculateDigest(GetPubCredential()))
        otErr << __FUNCTION__ << ": Error calculating credential digest.\n";
}

// I needed this for exporting a Nym (with credentials) from the wallet.
const String& Credential::GetPriCredential() const
{
    OT_ASSERT_MSG(!m_mapPrivateInfo.empty(), "ASSERT: GetPriCredential can "
                                             "only be called on private "
                                             "child credentials.");

    return m_strRawFile;
}

// We don't want to have to figure this out each time we need the public
// credential, so we just
// call this function wherever we need to get the public credential.
//
const String& Credential::GetPubCredential() const // More intelligent
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

String Credential::CredentialTypeToString(Credential::CredentialType credentialType)

{
    String credentialString;

    switch (credentialType) {
        case Credential::LEGACY :
            credentialString="legacy";
            break;
        case Credential::SECP256K1 :
            credentialString="secp256k1";
            break;
        case Credential::URL :
            credentialString="url";
            break;
        default :
            credentialString="error";
    }
    return credentialString;
}

Credential::CredentialType Credential::StringToCredentialType(const String & credentialType)

{
    if (credentialType.Compare("legacy"))
        return Credential::LEGACY;
    else if (credentialType.Compare("secp256k1"))
        return Credential::SECP256K1;
    else if (credentialType.Compare("url"))
        return Credential::URL;
    return Credential::ERROR_TYPE;
}

OTAsymmetricKey::KeyType Credential::CredentialTypeToKeyType(Credential::CredentialType credentialType)

{
    OTAsymmetricKey::KeyType newKeyType;

    switch (credentialType) {
        case Credential::LEGACY :
            newKeyType = OTAsymmetricKey::LEGACY;
            break;
        case Credential::SECP256K1 :
            newKeyType = OTAsymmetricKey::SECP256K1;
            break;
        case Credential::URL :
            newKeyType = OTAsymmetricKey::NULL_TYPE;
            break;
        default :
            newKeyType = OTAsymmetricKey::ERROR_TYPE;
    }
    return newKeyType;
}

Credential::CredentialType Credential::GetType() const

{
    return m_Type;
}

serializedCredential Credential::Serialize(bool asPrivate, bool asSigned) const

{
    serializedCredential serializedCredential = std::make_shared<proto::Credential>();

    serializedCredential->set_version(1);
    serializedCredential->set_type(proto::CREDTYPE_LEGACY);

    if (asPrivate) {
        OT_ASSERT(proto::KEYMODE_PRIVATE == m_mode);
        serializedCredential->set_mode(m_mode);

    } else {
        serializedCredential->set_mode(proto::KEYMODE_PUBLIC);
    }

    if (asSigned) {
        serializedSignature publicSig;
        serializedSignature privateSig;

        proto::Signature* pPrivateSig;
        proto::Signature* pPublicSig;

        if (asPrivate) {
            privateSig = GetSelfSignature(true);

            OT_ASSERT(nullptr != privateSig);
            if (nullptr != privateSig) {
                pPrivateSig = serializedCredential->add_signature();
                *pPrivateSig = *privateSig;
            }
        }

        publicSig = GetSelfSignature(false);

        OT_ASSERT(nullptr != publicSig);
        if (nullptr != publicSig) {
            pPublicSig = serializedCredential->add_signature();
            *pPublicSig = *GetSelfSignature(false);
        }
    }

    String credID;
    GetIdentifier(credID);

    serializedCredential->set_id(credID.Get());
    serializedCredential->set_nymid(GetNymID().Get());

    return serializedCredential;
}

serializedCredential Credential::SerializeForPublicSignature() const
{
    serializedCredential pubsigVersion = Serialize(false, false);

    return pubsigVersion;
}

serializedCredential Credential::SerializeForPrivateSignature() const
{
    serializedCredential privsigVersion = Serialize(true, false);

    return privsigVersion;
}

serializedCredential Credential::SerializeForIdentifier() const
{
    serializedCredential idVersion = SerializeForPublicSignature();

    if (idVersion->has_id()) {
        idVersion->clear_id();
    }

    return idVersion;
}

OTData Credential::SerializeCredToData(const proto::Credential& serializedCred) const
{
    int size = serializedCred.ByteSize();
    char* protoArray = new char [size];
    serializedCred.SerializeToArray(protoArray, size);

    OTData serializedData(protoArray, size);
    delete[] protoArray;

    return serializedData;
}


serializedSignature Credential::GetSelfSignature(bool privateVersion) const
{
    proto::SignatureRole targetRole;

    if (privateVersion) {
        targetRole = proto::SIGROLE_PRIVCREDENTIAL;
    } else {
        targetRole = proto::SIGROLE_PUBCREDENTIAL;
    }

    // Perhaps someday this method will return a list of matching signatures
    // For now, it only returns the first one.
    for (auto& it : m_listSerializedSignatures) {
        if (it->role() == targetRole) {
            return it;
        }
    }

    return nullptr;
}

} // namespace opentxs
