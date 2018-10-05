// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Contract.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSignature.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTStringXML.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/HashingProvider.hpp"
#include "opentxs/Proto.hpp"

#include <irrxml/irrXML.hpp>

#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <utility>

using namespace irr;
using namespace io;

#define OT_METHOD "opentxs::Contract::"

namespace opentxs
{
OTString trim(const String& str)
{
    std::string s(str.Get(), str.GetLength());
    return String::Factory(String::trim(s));
}

Contract::Contract(const api::Core& core)
    : Contract(
          core,
          String::Factory(),
          String::Factory(),
          String::Factory(),
          String::Factory())
{
}

Contract::Contract(
    const api::Core& core,
    const String& name,
    const String& foldername,
    const String& filename,
    const String& strID)
    : api_{core}
    , m_strName(name)
    , m_strFoldername(foldername)
    , m_strFilename(filename)
    , m_ID(Identifier::Factory(strID))
    , m_xmlUnsigned()
    , m_strRawFile(String::Factory())
    , m_strSigHashType(proto::HASHTYPE_ERROR)
    , m_strContractType(String::Factory("CONTRACT"))
    , m_mapNyms()
    , m_listSignatures()
    , m_strVersion(String::Factory("2.0"))
    , m_strEntityShortName(String::Factory())
    , m_strEntityLongName(String::Factory())
    , m_strEntityEmail(String::Factory())
    , m_mapConditions()
{
}

Contract::Contract(const api::Core& core, const String& strID)
    : Contract(
          core,
          String::Factory(),
          String::Factory(),
          String::Factory(),
          strID)
{
}

Contract::Contract(const api::Core& core, const Identifier& theID)
    : Contract(core, String::Factory(theID))
{
}

// static
bool Contract::DearmorAndTrim(
    const String& strInput,
    String& strOutput,
    String& strFirstLine)
{

    if (!strInput.Exists()) {
        otErr << __FUNCTION__ << ": Input string is empty.\n";
        return false;
    }

    strOutput.Set(strInput);

    if (false == strOutput.DecodeIfArmored(false))  // bEscapedIsAllowed=true by
                                                    // default.
    {
        LogInsane(OT_METHOD)(__FUNCTION__)(
            ": Input string apparently was encoded and then failed decoding. "
            "Contents: \n")(strInput)
            .Flush();

        return false;
    }

    strOutput.reset();  // for sgets

    // At this point, strOutput contains the actual contents, whether they
    // were originally ascii-armored OR NOT. (And they are also now trimmed,
    // either way.)

    std::array<char, 75> buf{};
    bool bGotLine = strOutput.sgets(buf.data(), 70);

    if (!bGotLine) return false;

    strFirstLine.Set(buf.data());
    strOutput.reset();  // set the "file" pointer within this string back to
                        // index 0.

    // Now I feel pretty safe -- the string I'm examining is within
    // the first 70 characters of the beginning of the contract, and
    // it will NOT contain the escape "- " sequence. From there, if
    // it contains the proper sequence, I will instantiate that type.
    if (!strFirstLine.Exists() || strFirstLine.Contains("- -")) return false;

    return true;
}

void Contract::SetIdentifier(const Identifier& theID)
{
    m_ID = Identifier::Factory(theID);
}

// The name, filename, version, and ID loaded by the wallet
// are NOT released here, since they are used immediately after
// the Release() call in LoadContract(). Really I just want to
// "Release" the stuff that is about to be loaded, not the stuff
// that I need to load it!
void Contract::Release_Contract()
{
    m_strSigHashType = proto::HASHTYPE_ERROR;
    m_xmlUnsigned.Release();
    m_strRawFile->Release();

    ReleaseSignatures();

    m_mapConditions.clear();

    m_mapNyms.clear();
}

void Contract::Release()
{
    Release_Contract();

    // No call to ot_super::Release() here, since Contract
    // is the base class.
}

Contract::~Contract() { Release_Contract(); }

bool Contract::SaveToContractFolder()
{
    OTString strFoldername(String::Factory(OTFolders::Contract().Get())),
        strFilename = String::Factory();

    GetIdentifier(strFilename);

    // These are already set in SaveContract(), called below.
    //    m_strFoldername    = strFoldername;
    //    m_strFilename    = strFilename;

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Saving asset contract to ")(
        "disk... ")
        .Flush();

    return SaveContract(strFoldername->Get(), strFilename->Get());
}

void Contract::GetFilename(String& strFilename) const
{
    String::Factory(strFilename.Get()) = m_strFilename;
}

void Contract::GetIdentifier(Identifier& theIdentifier) const
{
    theIdentifier.SetString(m_ID->str());
}

void Contract::GetIdentifier(String& theIdentifier) const
{
    m_ID->GetString(theIdentifier);
}

// Make sure this contract checks out. Very high level.
// Verifies ID, existence of public key, and signature.
//
bool Contract::VerifyContract() const
{
    // Make sure that the supposed Contract ID that was set is actually
    // a hash of the contract file, signatures and all.
    if (!VerifyContractID()) {
        otWarn << __FUNCTION__ << ": Failed verifying contract ID.\n";
        return false;
    }

    // Make sure we are able to read the official "contract" public key out of
    // this contract.
    auto pNym = GetContractPublicNym();

    if (nullptr == pNym) {
        otOut << __FUNCTION__
              << ": Failed retrieving public nym from contract.\n";
        return false;
    }

    if (!VerifySignature(*pNym)) {
        const auto theNymID = Identifier::Factory(*pNym);
        const auto strNymID = String::Factory(theNymID);
        otOut << __FUNCTION__
              << ": Failed verifying the contract's signature "
                 "against the public key that was retrieved "
                 "from the contract, with key ID: "
              << strNymID << "\n";
        return false;
    }

    otWarn << "\nVerified -- The Contract ID from the wallet matches the "
              "newly-calculated hash of the contract file.\n"
              "Verified -- A standard \"contract\" Public Key or x509 Cert WAS "
              "found inside the contract.\n"
              "Verified -- And the **SIGNATURE VERIFIED** with THAT key.\n\n";
    return true;
}

void Contract::CalculateContractID(Identifier& newID) const
{
    // may be redundant...
    std::string str_Trim(m_strRawFile->Get());
    std::string str_Trim2 = String::trim(str_Trim);

    auto strTemp = String::Factory(str_Trim2.c_str());

    if (!newID.CalculateDigest(strTemp))
        otErr << __FUNCTION__ << ": Error calculating Contract digest.\n";
}

void Contract::CalculateAndSetContractID(Identifier& newID)
{
    CalculateContractID(newID);
    SetIdentifier(newID);
}

bool Contract::VerifyContractID() const
{
    auto newID = Identifier::Factory();
    CalculateContractID(newID);

    // newID now contains the Hash aka Message Digest aka Fingerprint
    // aka thumbprint aka "IDENTIFIER" of the Contract.
    //
    // Now let's compare that identifier to the one already loaded by the wallet
    // for this contract and make sure they MATCH.

    // I use the == operator here because there is no != operator at this time.
    // That's why you see the ! outside the parenthesis.
    //
    if (!(m_ID == newID)) {
        auto str1 = String::Factory(m_ID), str2 = String::Factory(newID);

        otOut << "\nHashes do NOT match in Contract::VerifyContractID.\n "
                 "Expected: "
              << str1 << "\n   Actual: " << str2
              << "\n"
                 //               "\nRAW FILE:\n--->" << m_strRawFile << "<---"
                 "\n";
        return false;
    } else {
        auto str1 = String::Factory();
        newID->GetString(str1);
        otWarn << "\nContract ID *SUCCESSFUL* match to "
               << " hash of contract file: " << str1 << "\n\n";
        return true;
    }
}

ConstNym Contract::GetContractPublicNym() const
{
    for (auto& it : m_mapNyms) {
        ConstNym pNym = it.second;
        OT_ASSERT_MSG(
            nullptr != pNym,
            "nullptr pseudonym pointer in Contract::GetContractPublicNym.\n");

        // We favor the new "credential" system over the old "public key"
        // system.
        // No one will ever actually put BOTH in a single contract. But if they
        // do,
        // we favor the new version over the old.
        if (it.first == "signer") {
            return pNym;
        }
        // TODO have a place for hardcoded values like this.
        else if (it.first == "contract") {
            // We're saying here that every contract has to have a key tag
            // called "contract"
            // where the official public key can be found for it and for any
            // contract.
            return pNym;
        }
    }

    return nullptr;
}

// This is the one that you will most likely want to call.
// It actually attaches the resulting signature to this contract.
// If you want the signature to remain on the contract and be handled
// internally, then this is what you should call.
//
bool Contract::SignContract(const Nym& theNym, const OTPasswordData* pPWData)
{
    OTSignature* pSig = new OTSignature;
    OT_ASSERT_MSG(
        nullptr != pSig,
        "Contract::SignContract: Error allocating memory for Signature.\n");

    bool bSigned = SignContract(theNym, *pSig, pPWData);

    if (bSigned)
        m_listSignatures.push_back(pSig);
    else {
        otErr << __FUNCTION__
              << ": Failure while calling "
                 "SignContract(theNym, *pSig, pPWData)\n";
        delete pSig;
        pSig = nullptr;
    }

    return bSigned;
}

// Signs using authentication key instead of signing key.
//
bool Contract::SignContractAuthent(
    const Nym& theNym,
    const OTPasswordData* pPWData)
{
    OTSignature* pSig = new OTSignature;
    OT_ASSERT_MSG(
        nullptr != pSig,
        "Contract::SignContractAuthent: Error "
        "allocating memory for Signature.\n");

    bool bSigned = SignContractAuthent(theNym, *pSig, pPWData);

    if (bSigned)
        m_listSignatures.push_back(pSig);
    else {
        otErr << __FUNCTION__
              << ": Failure while calling "
                 "SignContractAuthent(theNym, *pSig, "
                 "pPWData)\n";
        delete pSig;
        pSig = nullptr;
    }

    return bSigned;
}

// The output signature will be in theSignature.
// It is NOT attached to the contract.  This is just a utility function.
bool Contract::SignContract(
    const Nym& theNym,
    OTSignature& theSignature,
    const OTPasswordData* pPWData)
{
    const auto& key = theNym.GetPrivateSignKey();
    m_strSigHashType = key.SigHashType();

    return SignContract(key, theSignature, m_strSigHashType, pPWData);
}

// Uses authentication key instead of signing key.
bool Contract::SignContractAuthent(
    const Nym& theNym,
    OTSignature& theSignature,
    const OTPasswordData* pPWData)
{
    const auto& key = theNym.GetPrivateAuthKey();
    m_strSigHashType = key.SigHashType();

    return SignContract(key, theSignature, m_strSigHashType, pPWData);
}

// Normally you'd use Contract::SignContract(const OTPseudonym& theNym)...
// Normally you WOULDN'T use this function SignWithKey.
// But this is here anyway for those peculiar places where you need it. For
// example,
// when first creating a Nym, you generate the master credential as part of
// creating
// the Nym, and the master credential has to sign itself, and it therefore needs
// to be
// able to "sign a contract" at a high level using purely the key, without
// having the Nym
// ready yet to signing anything with.
//
bool Contract::SignWithKey(
    const crypto::key::Asymmetric& theKey,
    const OTPasswordData* pPWData)
{
    OTSignature* pSig = new OTSignature;
    OT_ASSERT_MSG(
        nullptr != pSig,
        "Contract::SignWithKey: Error allocating memory for Signature.\n");

    m_strSigHashType = theKey.SigHashType();
    bool bSigned = SignContract(theKey, *pSig, m_strSigHashType, pPWData);

    if (bSigned)
        m_listSignatures.push_back(pSig);
    else {
        otErr << __FUNCTION__
              << ": Failure while calling SignContract(theNym, *pSig).\n";
        delete pSig;
        pSig = nullptr;
    }

    return bSigned;
}

// Done: When signing a contract, need to record the metadata into the signature
// object here.

// We will know if the key is signing, authentication, or encryption key
// because?
// Because we used the Nym to choose it! In which case we should have a default
// option,
// and also some other function with a new name that calls SignContract and
// CHANGES that default
// option.
// For example, SignContract(bool bUseAuthenticationKey=false)
// Then: SignContractAuthentication() { return SignContract(true); }
//
// In most cases we actually WILL want the signing key, since we are actually
// signing contracts
// such as cash withdrawals, etc. But when the Nym stores something for himself
// locally, or when
// sending messages, those will use the authentication key.
//
// We'll also have the ability to SWITCH the key which is important because it
// raises the
// question, how do we CHOOSE the key? On my phone I might use a different key
// than on my iPad.
// theNym should either know already (GetPrivateKey being intelligent) or it
// must be passed in
// (Into the below versions of SignContract.)
//
// If theKey knows its type (A|E|S) the next question is, does it know its other
// metadata?
// It certainly CAN know, can't it? Especially if it's being loaded from
// credentials in the
// first place. And if not, well then the data's not there and it's not added to
// the signature.
// (Simple.) So I will put the Signature Metadata into its own class, so not
// only a signature
// can use it, but also the crypto::key::Asymmetric class can use it and also
// Credential can use it.
// Then Contract just uses it if it's there. Also we don't have to pass it in
// here as separate
// parameters. At most we have to figure out which private key to get above, in
// theNym.GetPrivateKey()
// Worst case maybe put a loop, and see which of the private keys inside that
// Nym, in its credentials,
// is actually loaded and available. Then just have GetPrivateKey return THAT
// one. Similarly, later
// on, in VerifySignature, we'll pass the signature itself into the Nym so that
// the Nym can use it
// to help search for the proper public key to use for verifying, based on that
// metadata.
//
// This is all great because it means the only real change I need to do here now
// is to see if
// theKey.HasMetadata and if so, just copy it directly over to theSignature's
// Metadata.
//

// The output signature will be in theSignature.
// It is NOT attached to the contract.  This is just a utility function.
//
bool Contract::SignContract(
    const crypto::key::Asymmetric& theKey,
    OTSignature& theSignature,
    const proto::HashType hashType,
    const OTPasswordData* pPWData)
{
    // We assume if there's any important metadata, it will already
    // be on the key, so we just copy it over to the signature.
    const auto* metadata = theKey.GetMetadata();

    if (nullptr != metadata) { theSignature.getMetaData() = *(metadata); }

    // Update the contents, (not always necessary, many contracts are read-only)
    // This is where we provide an overridable function for the child classes
    // that
    // need to update their contents at this point.
    // But the Contract version of this function is actually empty, since the
    // default behavior is that contract contents don't change.
    // (Accounts and Messages being two big exceptions.)
    //
    UpdateContents();

    const auto& engine = theKey.engine();

    if (false ==
        engine.SignContract(
            trim(m_xmlUnsigned), theKey, theSignature, hashType, pPWData)) {
        otErr << "Contract::SignContract: "
                 "engine.SignContract returned false.\n";
        return false;
    }

    return true;
}

bool Contract::VerifySigAuthent(
    const Nym& theNym,
    const OTPasswordData* pPWData) const
{
    auto strNymID = String::Factory();
    theNym.GetIdentifier(strNymID);
    char cNymID = '0';
    std::uint32_t uIndex = 3;
    const bool bNymID = strNymID->At(uIndex, cNymID);

    for (auto& it : m_listSignatures) {
        OTSignature* pSig = it;
        OT_ASSERT(nullptr != pSig);

        if (bNymID && pSig->getMetaData().HasMetadata()) {
            // If the signature has metadata, then it knows the fourth character
            // of the NymID that signed it. We know the fourth character of the
            // NymID
            // who's trying to verify it. Thus, if they don't match, we can skip
            // this
            // signature without having to try to verify it at all.
            //
            if (pSig->getMetaData().FirstCharNymID() != cNymID) continue;
        }

        if (VerifySigAuthent(theNym, *pSig, pPWData)) return true;
    }

    return false;
}

bool Contract::VerifySignature(const Nym& theNym, const OTPasswordData* pPWData)
    const
{
    auto strNymID = String::Factory(theNym.ID());
    char cNymID = '0';
    std::uint32_t uIndex = 3;
    const bool bNymID = strNymID->At(uIndex, cNymID);

    for (auto& it : m_listSignatures) {
        OTSignature* pSig = it;
        OT_ASSERT(nullptr != pSig);

        if (bNymID && pSig->getMetaData().HasMetadata()) {
            // If the signature has metadata, then it knows the first character
            // of the NymID that signed it. We know the first character of the
            // NymID
            // who's trying to verify it. Thus, if they don't match, we can skip
            // this
            // signature without having to try to verify it at all.
            //
            if (pSig->getMetaData().FirstCharNymID() != cNymID) continue;
        }

        if (VerifySignature(theNym, *pSig, pPWData)) return true;
    }

    return false;
}

bool Contract::VerifyWithKey(
    const crypto::key::Asymmetric& theKey,
    const OTPasswordData* pPWData) const
{
    for (auto& it : m_listSignatures) {
        OTSignature* pSig = it;

        OT_ASSERT(nullptr != pSig);

        const auto* metadata = theKey.GetMetadata();

        if ((nullptr != metadata) && metadata->HasMetadata() &&
            pSig->getMetaData().HasMetadata()) {
            // Since key and signature both have metadata, we can use it
            // to skip signatures which don't match this key.
            //
            if (pSig->getMetaData() != *(metadata)) continue;
        }

        OTPasswordData thePWData("Contract::VerifyWithKey");

        if (VerifySignature(
                theKey,
                *pSig,
                m_strSigHashType,
                (nullptr != pPWData) ? pPWData : &thePWData))
            return true;
    }

    return false;
}

// Like VerifySignature, except it uses the authentication key instead of the
// signing key.
// (Like for sent messages or stored files, where you want a signature but you
// don't want
// a legally binding signature, just a technically secure signature.)
//
bool Contract::VerifySigAuthent(
    const Nym& theNym,
    const OTSignature& theSignature,
    const OTPasswordData* pPWData) const
{

    OTPasswordData thePWData("Contract::VerifySigAuthent 1");
    crypto::key::Keypair::Keys listOutput;

    const std::int32_t nCount = theNym.GetPublicKeysBySignature(
        listOutput, theSignature, 'A');  // 'A' for authentication key.

    if (nCount > 0)  // Found some (potentially) matching keys...
    {
        for (auto& it : listOutput) {
            auto pKey = it;
            OT_ASSERT(nullptr != pKey);

            if (VerifySignature(
                    *pKey,
                    theSignature,
                    m_strSigHashType,
                    (nullptr != pPWData) ? pPWData : &thePWData))
                return true;
        }
    } else {
        auto strNymID = String::Factory();
        theNym.GetIdentifier(strNymID);
        otWarn << __FUNCTION__
               << ": Tried to grab a list of keys from this Nym (" << strNymID
               << ") which might match this signature, "
                  "but recovered none. Therefore, will attempt to verify using "
                  "the Nym's default public "
                  "AUTHENTICATION key.\n";
    }
    // else found no keys.

    return VerifySignature(
        theNym.GetPublicAuthKey(),
        theSignature,
        m_strSigHashType,
        (nullptr != pPWData) ? pPWData : &thePWData);
}

// The only different between calling this with a Nym and calling it with an
// Asymmetric Key is that
// the key gives you the choice of hash algorithm, whereas the nym version uses
// m_strHashType to decide
// for you.  Choose the function you prefer, you can do it either way.
//
bool Contract::VerifySignature(
    const Nym& theNym,
    const OTSignature& theSignature,
    const OTPasswordData* pPWData) const
{

    OTPasswordData thePWData("Contract::VerifySignature 1");
    crypto::key::Keypair::Keys listOutput;

    const std::int32_t nCount = theNym.GetPublicKeysBySignature(
        listOutput, theSignature, 'S');  // 'S' for signing key.

    if (nCount > 0)  // Found some (potentially) matching keys...
    {
        for (auto& it : listOutput) {
            auto pKey = it;
            OT_ASSERT(nullptr != pKey);

            if (VerifySignature(
                    *pKey,
                    theSignature,
                    m_strSigHashType,
                    (nullptr != pPWData) ? pPWData : &thePWData))
                return true;
        }
    } else {
        auto strNymID = String::Factory();
        theNym.GetIdentifier(strNymID);
        otWarn << __FUNCTION__
               << ": Tried to grab a list of keys from this Nym (" << strNymID
               << ") which might match this signature, "
                  "but recovered none. Therefore, will attempt to verify using "
                  "the Nym's default public "
                  "SIGNING key.\n";
    }
    // else found no keys.

    return VerifySignature(
        theNym.GetPublicSignKey(),
        theSignature,
        m_strSigHashType,
        (nullptr != pPWData) ? pPWData : &thePWData);
}

bool Contract::VerifySignature(
    const crypto::key::Asymmetric& theKey,
    const OTSignature& theSignature,
    const proto::HashType hashType,
    const OTPasswordData* pPWData) const
{
    const auto* metadata = theKey.GetMetadata();

    // See if this key could possibly have even signed this signature.
    // (The metadata may eliminate it as a possibility.)
    if ((nullptr != metadata) && metadata->HasMetadata() &&
        theSignature.getMetaData().HasMetadata()) {
        if (theSignature.getMetaData() != *(metadata)) return false;
    }

    OTPasswordData thePWData("Contract::VerifySignature 2");
    const auto& engine = theKey.engine();

    if (false == engine.VerifyContractSignature(
                     trim(m_xmlUnsigned),
                     theKey,
                     theSignature,
                     hashType,
                     (nullptr != pPWData) ? pPWData : &thePWData)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(
            ": engine.VerifyContractSignature returned false.")
            .Flush();

        return false;
    }

    return true;
}

void Contract::ReleaseSignatures()
{

    while (!m_listSignatures.empty()) {
        OTSignature* pSig = m_listSignatures.front();
        m_listSignatures.pop_front();
        delete pSig;
    }
}

bool Contract::DisplayStatistics(String& strContents) const
{
    // Subclasses may override this.
    strContents.Concatenate(
        const_cast<char*>("ERROR:  Contract::DisplayStatistics was called "
                          "instead of a subclass...\n"));

    return false;
}

bool Contract::SaveContractWallet(Tag&) const
{
    // Subclasses may use this.

    return false;
}

bool Contract::SaveContents(std::ofstream& ofs) const
{
    ofs << m_xmlUnsigned;

    return true;
}

// Saves the unsigned XML contents to a string
bool Contract::SaveContents(String& strContents) const
{
    strContents.Concatenate(m_xmlUnsigned);

    return true;
}

// Save the contract member variables into the m_strRawFile variable
bool Contract::SaveContract()
{
    auto strTemp = String::Factory();
    bool bSuccess = RewriteContract(strTemp);

    if (bSuccess) {
        m_strRawFile->Set(strTemp);

        // RewriteContract() already does this.
        //
        //        std::string str_Trim(strTemp.Get());
        //        std::string str_Trim2 = OTString::trim(str_Trim);
        //        m_strRawFile.Set(str_Trim2.c_str());
    }

    return bSuccess;
}

void Contract::UpdateContents()
{
    // Deliberately left blank.
    //
    // Some child classes may need to perform work here
    // (OTAccount and OTMessage, for example.)
    //
    // This function is called just prior to the signing of a contract.

    // Update: MOST child classes actually use this.
    // The server and asset contracts are not meant to ever change after
    // they are signed. However, many other contracts are meant to change
    // and be re-signed. (You cannot change something without signing it.)
    // (So most child classes override this method.)
}

// CreateContract is great if you already know what kind of contract to
// instantiate
// and have already done so. Otherwise this function will take ANY flat text and
// use
// a generic Contract instance to sign it and then write it to strOutput. This
// is
// due to the fact that OT was never really designed for signing flat text, only
// contracts.
//
// static
bool Contract::SignFlatText(
    String& strFlatText,
    const String& strContractType,
    const Nym& theSigner,
    String& strOutput)
{
    const char* szFunc = "Contract::SignFlatText";

    // Trim the input to remove any extraneous whitespace
    //
    std::string str_Trim(strFlatText.Get());
    std::string str_Trim2 = String::trim(str_Trim);

    strFlatText.Set(str_Trim2.c_str());

    char cNewline = 0;
    const std::uint32_t lLength = strFlatText.GetLength();

    if ((3 > lLength) || !strFlatText.At(lLength - 1, cNewline)) {
        otErr << szFunc
              << ": Invalid input: text is less than 3 bytes "
                 "std::int64_t, or unable to read a byte from the end where "
                 "a newline is meant to be.\n";
        return false;
    }

    // ADD a newline, if necessary.
    // (The -----BEGIN part needs to start on its OWN LINE...)
    //
    // If length is 10, then string goes from 0..9.
    // Null terminator will be at 10.
    // Therefore the final newline should be at 9.
    // Therefore if char_at_index[lLength-1] != '\n'
    // Concatenate one!

    auto strInput = String::Factory();
    if ('\n' == cNewline)  // It already has a newline
        strInput = strFlatText;
    else
        strInput->Format("%s\n", strFlatText.Get());

    OTSignature theSignature;
    OTPasswordData thePWData("Signing flat text (need private key)");

    auto& key = theSigner.GetPrivateSignKey();
    auto& engine = key.engine();

    if (false == engine.SignContract(
                     trim(strInput),
                     theSigner.GetPrivateSignKey(),
                     theSignature,  // the output
                     key.SigHashType(),
                     &thePWData)) {
        otErr << szFunc << ": SignContract failed. Contents:\n\n"
              << strInput << "\n\n\n";
        return false;
    }

    listOfSignatures listSignatures;
    listSignatures.push_back(&theSignature);

    const bool bBookends = Contract::AddBookendsAroundContent(
        strOutput,  // the output (other params are input.)
        strInput,
        strContractType,
        key.SigHashType(),
        listSignatures);

    return bBookends;
}

// Saves the raw (pre-existing) contract text to any string you want to pass in.
bool Contract::SaveContractRaw(String& strOutput) const
{
    strOutput.Concatenate("%s", m_strRawFile->Get());

    return true;
}

// static
bool Contract::AddBookendsAroundContent(
    String& strOutput,
    const String& strContents,
    const String& strContractType,
    const proto::HashType hashType,
    const listOfSignatures& listSignatures)
{
    auto strTemp = String::Factory();
    auto strHashType = crypto::HashingProvider::HashTypeToString(hashType);

    strTemp->Concatenate(
        "-----BEGIN SIGNED %s-----\nHash: %s\n\n",
        strContractType.Get(),
        strHashType->Get());

    strTemp->Concatenate("%s", strContents.Get());

    for (const auto& it : listSignatures) {
        OTSignature* pSig = it;
        OT_ASSERT(nullptr != pSig);

        strTemp->Concatenate(
            "-----BEGIN %s SIGNATURE-----\n"
            "Version: Open Transactions %s\n"
            "Comment: "
            "http://opentransactions.org\n",
            strContractType.Get(),
            Log::Version());

        if (pSig->getMetaData().HasMetadata())
            strTemp->Concatenate(
                "Meta:    %c%c%c%c\n",
                pSig->getMetaData().GetKeyType(),
                pSig->getMetaData().FirstCharNymID(),
                pSig->getMetaData().FirstCharMasterCredID(),
                pSig->getMetaData().FirstCharChildCredID());

        strTemp->Concatenate(
            "%s",
            pSig->Get());  // <=== *** THE SIGNATURE ITSELF ***
        strTemp->Concatenate(
            "\n-----END %s SIGNATURE-----\n\n", strContractType.Get());
    }

    std::string str_Trim(strTemp->Get());
    std::string str_Trim2 = String::trim(str_Trim);
    strOutput.Set(str_Trim2.c_str());

    return true;
}

// Takes the pre-existing XML contents (WITHOUT signatures) and re-writes
// into strOutput the appearance of m_strRawData, adding the pre-existing
// signatures along with new signature bookends.. (The caller actually passes
// m_strRawData into this function...)
//
bool Contract::RewriteContract(String& strOutput) const
{
    auto strContents = String::Factory();
    SaveContents(strContents);

    return Contract::AddBookendsAroundContent(
        strOutput,
        strContents,
        m_strContractType,
        m_strSigHashType,
        m_listSignatures);
}

bool Contract::SaveContract(const char* szFoldername, const char* szFilename)
{
    OT_ASSERT_MSG(
        nullptr != szFilename,
        "Null filename sent to Contract::SaveContract\n");
    OT_ASSERT_MSG(
        nullptr != szFoldername,
        "Null foldername sent to Contract::SaveContract\n");

    m_strFoldername->Set(szFoldername);
    m_strFilename->Set(szFilename);

    return WriteContract(szFoldername, szFilename);
}

bool Contract::WriteContract(
    const std::string& folder,
    const std::string& filename) const
{
    OT_ASSERT(folder.size() > 2);
    OT_ASSERT(filename.size() > 2);

    if (!m_strRawFile->Exists()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error saving file (contract contents are empty): " << folder
              << Log::PathSeparator() << filename << std::endl;

        return false;
    }

    auto strFinal = String::Factory();
    auto ascTemp = Armored::Factory(m_strRawFile);

    if (false ==
        ascTemp->WriteArmoredString(strFinal, m_strContractType->Get())) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error saving file (failed writing armored string): "
              << folder << Log::PathSeparator() << filename << std::endl;

        return false;
    }

    const bool bSaved = OTDB::StorePlainString(
        strFinal->Get(), api_.DataFolder(), folder, filename, "", "");

    if (!bSaved) {
        otErr << OT_METHOD << __FUNCTION__ << "Error saving file: " << folder
              << Log::PathSeparator() << filename << std::endl;

        return false;
    }

    return true;
}

// assumes m_strFilename is already set.
// Then it reads that file into a string.
// Then it parses that string into the object.
bool Contract::LoadContract()
{
    Release();
    LoadContractRawFile();  // opens m_strFilename and reads into m_strRawFile

    return ParseRawFile();  // Parses m_strRawFile into the various member
                            // variables.
}

// The entire Raw File, signatures and all, is used to calculate the hash
// value that becomes the ID of the contract. If you change even one letter,
// then you get a different ID.
// This applies to all contracts except accounts, since their contents must
// change periodically, their ID is not calculated from a hash of the file,
// but instead is chosen at random when the account is created.
bool Contract::LoadContractRawFile()
{
    const char* szFoldername = m_strFoldername->Get();
    const char* szFilename = m_strFilename->Get();

    if (!m_strFoldername->Exists() || !m_strFilename->Exists()) return false;

    if (!OTDB::Exists(api_.DataFolder(), szFoldername, szFilename, "", "")) {
        otErr << __FUNCTION__ << ": File does not exist: " << szFoldername
              << Log::PathSeparator() << szFilename << "\n";
        return false;
    }

    auto strFileContents = String::Factory(OTDB::QueryPlainString(
        api_.DataFolder(), szFoldername, szFilename, "", ""));  // <===
                                                                // LOADING
                                                                // FROM DATA
                                                                // STORE.

    if (!strFileContents->Exists()) {
        otErr << __FUNCTION__ << ": Error reading file: " << szFoldername
              << Log::PathSeparator() << szFilename << "\n";
        return false;
    }

    if (false == strFileContents->DecodeIfArmored())  // bEscapedIsAllowed=true
                                                      // by default.
    {
        otErr << __FUNCTION__
              << ": Input string apparently was encoded and "
                 "then failed decoding. Contents: \n"
              << strFileContents << "\n";
        return false;
    }

    // At this point, strFileContents contains the actual contents, whether they
    // were originally ascii-armored OR NOT. (And they are also now trimmed,
    // either way.)
    //
    m_strRawFile->Set(strFileContents);

    return m_strRawFile->Exists();
}

bool Contract::LoadContract(const char* szFoldername, const char* szFilename)
{
    Release();

    m_strFoldername->Set(szFoldername);
    m_strFilename->Set(szFilename);

    // opens m_strFilename and reads into m_strRawFile
    if (LoadContractRawFile())
        return ParseRawFile();  // Parses m_strRawFile into the various member
                                // variables.
    else {
        otErr << "Failed loading raw contract file: " << m_strFoldername
              << Log::PathSeparator() << m_strFilename << "\n";
    }
    return false;
}

// Just like it says. If you have a contract in string form, pass it in
// here to import it.
bool Contract::LoadContractFromString(const String& theStr)
{
    Release();

    if (!theStr.Exists()) {
        otErr << __FUNCTION__ << ": ERROR: Empty string passed in...\n";
        return false;
    }

    auto strContract = String::Factory(theStr.Get());

    if (false == strContract->DecodeIfArmored())  // bEscapedIsAllowed=true by
                                                  // default.
    {
        otErr << __FUNCTION__
              << ": ERROR: Input string apparently was encoded "
                 "and then failed decoding. "
                 "Contents: \n"
              << theStr << "\n";
        return false;
    }

    m_strRawFile->Set(strContract);

    // This populates m_xmlUnsigned with the contents of m_strRawFile (minus
    // bookends, signatures, etc. JUST the XML.)
    bool bSuccess =
        ParseRawFile();  // It also parses into the various member variables.

    // Removed:
    // This was the bug where the version changed from 75 to 75c, and suddenly
    // contract ID was wrong...
    //
    // If it was a success, save back to m_strRawFile again so
    // the format is consistent and hashes will calculate properly.
    //    if (bSuccess)
    //    {
    //        // Basically we take the m_xmlUnsigned that we parsed out of the
    // raw file before,
    //        // then we use that to generate the raw file again, re-attaching
    // the signatures.
    //        // This function does that.
    //        SaveContract();
    //    }

    return bSuccess;
}

bool Contract::ParseRawFile()
{
    char buffer1[2100];  // a bit bigger than 2048, just for safety reasons.
    OTSignature* pSig = nullptr;

    std::string line;

    bool bSignatureMode = false;           // "currently in signature mode"
    bool bContentMode = false;             // "currently in content mode"
    bool bHaveEnteredContentMode = false;  // "have yet to enter content mode"

    if (!m_strRawFile->GetLength()) {
        otErr << "Empty m_strRawFile in Contract::ParseRawFile. Filename: "
              << m_strFoldername << Log::PathSeparator() << m_strFilename
              << ".\n";
        return false;
    }

    // This is redundant (I thought) but the problem hasn't cleared up yet.. so
    // trying to really nail it now.
    std::string str_Trim(m_strRawFile->Get());
    std::string str_Trim2 = String::trim(str_Trim);
    m_strRawFile->Set(str_Trim2.c_str());

    bool bIsEOF = false;
    m_strRawFile->reset();

    do {
        // Just a fresh start at the top of the loop block... probably
        // unnecessary.
        memset(buffer1, 0, 2100);  // todo remove this in optimization. (might
                                   // be removed already...)

        // the call returns true if there's more to read, and false if there
        // isn't.
        bIsEOF = !(m_strRawFile->sgets(buffer1, 2048));

        line = buffer1;
        const char* pBuf = line.c_str();

        if (line.length() < 2) {
            if (bSignatureMode) continue;
        }

        // if we're on a dashed line...
        else if (line.at(0) == '-') {
            if (bSignatureMode) {
                // we just reached the end of a signature
                //    otErr << "%s\n", pSig->Get());
                pSig = nullptr;
                bSignatureMode = false;
                continue;
            }

            // if I'm NOT in signature mode, and I just hit a dash, that means
            // there
            // are only four options:

            // a. I have not yet even entered content mode, and just now
            // entering it for the first time.
            if (!bHaveEnteredContentMode) {
                if ((line.length() > 3) &&
                    (line.find("BEGIN") != std::string::npos) &&
                    line.at(1) == '-' && line.at(2) == '-' &&
                    line.at(3) == '-') {
                    //                    otErr << "\nProcessing contract...
                    // \n";
                    bHaveEnteredContentMode = true;
                    bContentMode = true;
                    continue;
                } else {
                    continue;
                }

            }

            // b. I am now entering signature mode!
            else if (
                line.length() > 3 &&
                line.find("SIGNATURE") != std::string::npos &&
                line.at(1) == '-' && line.at(2) == '-' && line.at(3) == '-') {
                // if (bContentMode)
                //    otLog3 << "Finished reading contract.\n\nReading a
                // signature at the bottom of the contract...\n");
                // else
                //    otLog3 << "Reading another signature...\n");

                bSignatureMode = true;
                bContentMode = false;

                pSig = new OTSignature;

                OT_ASSERT_MSG(
                    nullptr != pSig,
                    "Error allocating memory for "
                    "Signature in "
                    "Contract::ParseRawFile\n");

                m_listSignatures.push_back(pSig);

                continue;
            }
            // c. There is an error in the file!
            else if (
                line.length() < 3 || line.at(1) != ' ' || line.at(2) != '-') {
                otOut
                    << "Error in contract " << m_strFilename
                    << ": a dash at the beginning of the "
                       "line should be followed by a space and another dash:\n"
                    << m_strRawFile << "\n";
                return false;
            }
            // d. It is an escaped dash, and therefore kosher, so I merely
            // remove the escape and add it.
            // I've decided not to remove the dashes but to keep them as part of
            // the signed content.
            // It's just much easier to deal with that way. The input code will
            // insert the extra dashes.
            // pBuf += 2;
        }

        // Else we're on a normal line, not a dashed line.
        else {
            if (bHaveEnteredContentMode) {
                if (bSignatureMode) {
                    if (line.length() < 2) {
                        LogDebug(OT_METHOD)(__FUNCTION__)(
                            ": Skipping short line...")
                            .Flush();

                        if (bIsEOF || !m_strRawFile->sgets(buffer1, 2048)) {
                            otOut << "Error in signature for contract "
                                  << m_strFilename
                                  << ": Unexpected EOF after short line.\n";
                            return false;
                        }

                        continue;
                    } else if (line.compare(0, 8, "Version:") == 0) {
                        LogDebug(OT_METHOD)(__FUNCTION__)(
                            ": Skipping version section...")
                            .Flush();

                        if (bIsEOF || !m_strRawFile->sgets(buffer1, 2048)) {
                            otOut << "Error in signature for contract "
                                  << m_strFilename
                                  << ": Unexpected EOF after \"Version:\"\n";
                            return false;
                        }

                        continue;
                    } else if (line.compare(0, 8, "Comment:") == 0) {
                        LogDebug(OT_METHOD)(__FUNCTION__)(
                            ": Skipping comment section..")
                            .Flush();

                        if (bIsEOF || !m_strRawFile->sgets(buffer1, 2048)) {
                            otOut << "Error in signature for contract "
                                  << m_strFilename
                                  << ": Unexpected EOF after \"Comment:\"\n";
                            return false;
                        }

                        continue;
                    }
                    if (line.compare(0, 5, "Meta:") == 0) {
                        LogDebug(OT_METHOD)(__FUNCTION__)(
                            ": Collecting signature metadata...")
                            .Flush();
                        ;

                        if (line.length() != 13)  // "Meta:    knms" (It will
                                                  // always be exactly 13
                        // characters std::int64_t.) knms represents the
                        // first characters of the Key type, NymID,
                        // Master Cred ID, and ChildCred ID. Key type is
                        // (A|E|S) and the others are base62.
                        {
                            otOut << "Error in signature for contract "
                                  << m_strFilename
                                  << ": Unexpected length for "
                                     "\"Meta:\" comment.\n";
                            return false;
                        }

                        OT_ASSERT(nullptr != pSig);
                        if (false == pSig->getMetaData().SetMetadata(
                                         line.at(9),
                                         line.at(10),
                                         line.at(11),
                                         line.at(12)))  // "knms" from "Meta:
                                                        // knms"
                        {
                            otOut << "Error in signature for contract "
                                  << m_strFilename
                                  << ": Unexpected metadata in the \"Meta:\" "
                                     "comment.\nLine: "
                                  << line << "\n";
                            return false;
                        }

                        if (bIsEOF || !m_strRawFile->sgets(buffer1, 2048)) {
                            otOut << "Error in signature for contract "
                                  << m_strFilename
                                  << ": Unexpected EOF after \"Meta:\"\n";
                            return false;
                        }

                        continue;
                    }
                }
                if (bContentMode) {
                    if (line.compare(0, 6, "Hash: ") == 0) {
                        LogDebug(OT_METHOD)(__FUNCTION__)(
                            ": Collecting message digest algorithm from "
                            " contract header...")
                            .Flush();

                        std::string strTemp = line.substr(6);
                        auto strHashType = String::Factory(strTemp.c_str());
                        strHashType->ConvertToUpperCase();

                        m_strSigHashType =
                            crypto::HashingProvider::StringToHashType(
                                strHashType);

                        if (bIsEOF || !m_strRawFile->sgets(buffer1, 2048)) {
                            otOut << "Error in contract " << m_strFilename
                                  << ": Unexpected EOF after \"Hash:\"\n";
                            return false;
                        }

                        continue;
                    }
                }
            }
        }

        if (bSignatureMode) {
            OT_ASSERT_MSG(
                nullptr != pSig,
                "Error: Null Signature pointer WHILE "
                "processing signature, in "
                "Contract::ParseRawFile");

            pSig->Concatenate("%s\n", pBuf);
        } else if (bContentMode)
            m_xmlUnsigned.Concatenate("%s\n", pBuf);
    } while (!bIsEOF);

    if (!bHaveEnteredContentMode) {
        otErr << "Error in Contract::ParseRawFile: Found no BEGIN for signed "
                 "content.\n";
        return false;
    } else if (bContentMode) {
        otErr << "Error in Contract::ParseRawFile: EOF while reading xml "
                 "content.\n";
        return false;
    } else if (bSignatureMode) {
        otErr << "Error in Contract::ParseRawFile: EOF while reading "
                 "signature.\n";
        return false;
    } else if (!LoadContractXML()) {
        otErr << "Error in Contract::ParseRawFile: unable to load XML "
                 "portion of contract into memory.\n";
        return false;
    } else if (proto::HASHTYPE_ERROR == m_strSigHashType) {
        otErr << __FUNCTION__ << ": Failed to set hash type," << std::endl;

        return false;
    } else {

        return true;
    }
}

// This function assumes that m_xmlUnsigned is ready to be processed.
// This function only processes that portion of the contract.
bool Contract::LoadContractXML()
{
    std::int32_t retProcess = 0;

    if (!m_xmlUnsigned.Exists()) { return false; }

    m_xmlUnsigned.reset();

    IrrXMLReader* xml = irr::io::createIrrXMLReader(m_xmlUnsigned);
    OT_ASSERT_MSG(
        nullptr != xml,
        "Memory allocation issue with xml reader in "
        "Contract::LoadContractXML()\n");
    std::unique_ptr<IrrXMLReader> xmlAngel(xml);

    // parse the file until end reached
    while (xml->read()) {
        auto strNodeType = String::Factory();

        switch (xml->getNodeType()) {
            case EXN_NONE:
                strNodeType->Set("EXN_NONE");
                goto switch_log;
            case EXN_COMMENT:
                strNodeType->Set("EXN_COMMENT");
                goto switch_log;
            case EXN_ELEMENT_END:
                strNodeType->Set("EXN_ELEMENT_END");
                goto switch_log;
            case EXN_CDATA:
                strNodeType->Set("EXN_CDATA");
                goto switch_log;

            switch_log:
                //                otErr << "SKIPPING %s element in
                // Contract::LoadContractXML: "
                //                              "type: %d, name: %s, value:
                //                              %s\n",
                //                              strNodeType.Get(),
                // xml->getNodeType(), xml->getNodeName(), xml->getNodeData());

                break;

            case EXN_TEXT: {
                // unknown element type
                //                otErr << "SKIPPING unknown text element type
                //                in
                // Contract::LoadContractXML: %s, value: %s\n",
                //                              xml->getNodeName(),
                // xml->getNodeData());
            } break;
            case EXN_ELEMENT: {
                retProcess = ProcessXMLNode(xml);

                // an error was returned. file format or whatever.
                if ((-1) == retProcess) {
                    otErr << "Contract::LoadContractXML: (Cancelling this "
                             "contract load; an error occurred.)\n";
                    return false;
                }
                // No error, but also the node wasn't found...
                else if (0 == retProcess) {
                    // unknown element type
                    otErr
                        << "UNKNOWN element type in Contract::LoadContractXML: "
                        << xml->getNodeName()
                        << ", value: " << xml->getNodeData() << "\n";

                    otErr << m_xmlUnsigned << "\n";
                }
                // else if 1 was returned, that means the node was processed.
            } break;
            default: {
                //                otErr << "SKIPPING (default case) element in
                // Contract::LoadContractXML: %d, value: %s\n",
                //                              xml->getNodeType(),
                // xml->getNodeData());
            }
                continue;
        }
    }

    return true;
}

// static
bool Contract::SkipToElement(IrrXMLReader*& xml)
{
    OT_ASSERT_MSG(
        nullptr != xml, "Contract::SkipToElement -- assert: nullptr != xml");

    const char* szFunc = "Contract::SkipToElement";

    while (xml->read() && (xml->getNodeType() != EXN_ELEMENT)) {
        //      otOut << szFunc << ": Looping to skip non-elements: currently
        // on: " << xml->getNodeName() << " \n";

        if (xml->getNodeType() == EXN_NONE) {
            otOut << "*** " << szFunc << ": EXN_NONE  (skipping)\n";
            continue;
        }  // SKIP
        else if (xml->getNodeType() == EXN_COMMENT) {
            otOut << "*** " << szFunc << ": EXN_COMMENT  (skipping)\n";
            continue;
        }  // SKIP
        else if (xml->getNodeType() == EXN_ELEMENT_END)
        //        { otOut << "*** Contract::SkipToElement: EXN_ELEMENT_END
        // (ERROR)\n";  return false; }
        {
            otWarn << "*** " << szFunc << ": EXN_ELEMENT_END  (skipping "
                   << xml->getNodeName() << ")\n";
            continue;
        } else if (xml->getNodeType() == EXN_CDATA) {
            otOut << "*** " << szFunc
                  << ": EXN_CDATA (ERROR -- unexpected CData)\n";
            return false;
        } else if (xml->getNodeType() == EXN_TEXT) {
            otErr << "*** " << szFunc << ": EXN_TEXT\n";
            return false;
        } else if (xml->getNodeType() == EXN_ELEMENT) {
            otOut << "*** " << szFunc << ": EXN_ELEMENT\n";
            break;
        }  // (Should never happen due to while() second condition.) Still
           // returns true.
        else {
            otErr << "*** " << szFunc
                  << ": SHOULD NEVER HAPPEN  (Unknown element type!)\n";
            return false;
        }  // Failure / Error
    }

    return true;
}

// static
bool Contract::SkipToTextField(IrrXMLReader*& xml)
{
    OT_ASSERT_MSG(
        nullptr != xml, "Contract::SkipToTextField -- assert: nullptr != xml");

    const char* szFunc = "Contract::SkipToTextField";

    while (xml->read() && (xml->getNodeType() != EXN_TEXT)) {
        if (xml->getNodeType() == EXN_NONE) {
            otOut << "*** " << szFunc << ": EXN_NONE  (skipping)\n";
            continue;
        }  // SKIP
        else if (xml->getNodeType() == EXN_COMMENT) {
            otOut << "*** " << szFunc << ": EXN_COMMENT  (skipping)\n";
            continue;
        }  // SKIP
        else if (xml->getNodeType() == EXN_ELEMENT_END)
        //        { otOut << "*** Contract::SkipToTextField:
        // EXN_ELEMENT_END  (skipping)\n";  continue; }     // SKIP
        // (debugging...)
        {
            otOut << "*** " << szFunc << ": EXN_ELEMENT_END  (ERROR)\n";
            return false;
        } else if (xml->getNodeType() == EXN_CDATA) {
            otOut << "*** " << szFunc
                  << ": EXN_CDATA (ERROR -- unexpected CData)\n";
            return false;
        } else if (xml->getNodeType() == EXN_ELEMENT) {
            otOut << "*** " << szFunc << ": EXN_ELEMENT\n";
            return false;
        } else if (xml->getNodeType() == EXN_TEXT) {
            otErr << "*** " << szFunc << ": EXN_TEXT\n";
            break;
        }  // (Should never happen due to while() second condition.) Still
           // returns true.
        else {
            otErr << "*** " << szFunc
                  << ": SHOULD NEVER HAPPEN  (Unknown element type!)\n";
            return false;
        }  // Failure / Error
    }

    return true;
}

// AFTER you read an element or text field, there is some whitespace, and you
// just want to bring your cursor back to wherever it should be for the next
// guy.
// So you call this function..
//
// static
bool Contract::SkipAfterLoadingField(IrrXMLReader*& xml)
{
    OT_ASSERT_MSG(
        nullptr != xml,
        "Contract::SkipAfterLoadingField -- assert: nullptr != xml");

    if (EXN_ELEMENT_END != xml->getNodeType())  // If we're not ALREADY on the
    // ending element, then go there.
    {
        const char* szFunc = "Contract::SkipAfterLoadingField";
        // move to the next node which SHOULD be the expected element_end.
        //
        while (xml->read()) {
            if (xml->getNodeType() == EXN_NONE) {
                otOut << "*** " << szFunc << ": EXN_NONE  (skipping)\n";
                continue;
            }  // SKIP
            else if (xml->getNodeType() == EXN_COMMENT) {
                otOut << "*** " << szFunc << ": EXN_COMMENT  (skipping)\n";
                continue;
            }  // SKIP
            else if (xml->getNodeType() == EXN_ELEMENT_END) {
                LogInsane(OT_METHOD)(__FUNCTION__)(
                    ": EXN_ELEMENT_END  (success)")
                    .Flush();
                break;
            }  // Success...
            else if (xml->getNodeType() == EXN_CDATA) {
                otOut << "*** " << szFunc << ": EXN_CDATA  (Unexpected!)\n";
                return false;
            }  // Failure / Error
            else if (xml->getNodeType() == EXN_ELEMENT) {
                otOut << "*** " << szFunc << ": EXN_ELEMENT  (Unexpected!)\n";
                return false;
            }  // Failure / Error
            else if (xml->getNodeType() == EXN_TEXT) {
                otErr << "*** " << szFunc << ": EXN_TEXT  (Unexpected!)\n";
                return false;
            }  // Failure / Error
            else {
                otErr << "*** " << szFunc
                      << ": SHOULD NEVER HAPPEN  (Unknown element type!)\n";
                return false;
            }  // Failure / Error
        }
    }

    // else ... (already on the ending element.)
    //

    return true;
}

// Loads it up and also decodes it to a string.
//
// static
bool Contract::LoadEncodedTextField(IrrXMLReader*& xml, String& strOutput)
{
    auto ascOutput = Armored::Factory();

    if (Contract::LoadEncodedTextField(xml, ascOutput) &&
        ascOutput->GetLength() > 2) {
        return ascOutput->GetString(strOutput, true);  // linebreaks = true
    }

    return false;
}

// static
bool Contract::LoadEncodedTextField(IrrXMLReader*& xml, Armored& ascOutput)
{
    OT_ASSERT_MSG(
        nullptr != xml,
        "Contract::LoadEncodedTextField -- assert: nullptr != xml");

    const char* szFunc = "Contract::LoadEncodedTextField";

    // If we're not ALREADY on a text field, maybe there is some whitespace, so
    // let's skip ahead...
    //
    if (EXN_TEXT != xml->getNodeType()) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Skipping non-text field...")
            .Flush();

        // move to the next node which SHOULD be the expected text field.
        if (!SkipToTextField(xml)) {
            otOut << szFunc
                  << ": Failure: Unable to find expected text field.\n";
            return false;
        }

        LogTrace(OT_METHOD)(__FUNCTION__)(
            ": Finished skipping non-text field. (Successfully.)")
            .Flush();
    }

    if (EXN_TEXT == xml->getNodeType())  // SHOULD always be true, in fact this
                                         // could be an assert().
    {
        auto strNodeData = String::Factory(xml->getNodeData());

        // Sometimes the XML reads up the data with a prepended newline.
        // This screws up my own objects which expect a consistent in/out
        // So I'm checking here for that prepended newline, and removing it.
        //
        char cNewline;
        if (strNodeData->Exists() && strNodeData->GetLength() > 2 &&
            strNodeData->At(0, cNewline)) {
            if ('\n' == cNewline) {
                ascOutput.Set(strNodeData->Get() + 1);
            } else {
                ascOutput.Set(strNodeData->Get());
            }

            // SkipAfterLoadingField() only skips ahead if it's not ALREADY
            // sitting on an element_end node.
            //
            xml->read();  // THIS PUTS us on the CLOSING TAG.
                          // <========================

            // The below call won't advance any further if it's ALREADY on the
            // closing tag (e.g. from the above xml->read() call.)
            if (!SkipAfterLoadingField(xml)) {
                otOut << "*** " << szFunc
                      << ": Bad data? Expected EXN_ELEMENT_END here, but "
                         "didn't get it. Returning false.\n";
                return false;
            }

            return true;
        }
    } else
        otOut << szFunc << ": Failure: Unable to find expected text field. 2\n";

    return false;
}

// Loads it up and also decodes it to a string.
// static
bool Contract::LoadEncodedTextFieldByName(
    IrrXMLReader*& xml,
    String& strOutput,
    const char* szName,
    String::Map* pmapExtraVars)
{
    OT_ASSERT(nullptr != szName);

    auto ascOutput = Armored::Factory();

    if (Contract::LoadEncodedTextFieldByName(
            xml, ascOutput, szName, pmapExtraVars) &&
        ascOutput->GetLength() > 2) {
        return ascOutput->GetString(strOutput, true);  // linebreaks = true
    }

    return false;
}

// Loads it up and keeps it encoded in an ascii-armored object.
// static
bool Contract::LoadEncodedTextFieldByName(
    IrrXMLReader*& xml,
    Armored& ascOutput,
    const char* szName,
    String::Map* pmapExtraVars)
{
    OT_ASSERT(nullptr != szName);

    // If we're not ALREADY on an element, maybe there is some whitespace, so
    // let's skip ahead...
    // If we're not already on a node, OR
    if ((EXN_ELEMENT != xml->getNodeType()) ||
        // if the node's name doesn't match the one expected.
        strcmp(szName, xml->getNodeName()) != 0) {
        // move to the next node which SHOULD be the expected name.
        if (!SkipToElement(xml)) {
            otOut << __FUNCTION__
                  << ": Failure: Unable to find expected element: " << szName
                  << ". \n";
            return false;
        }
    }

    if (EXN_ELEMENT != xml->getNodeType())  // SHOULD always be ELEMENT...
    {
        otErr << __FUNCTION__ << ": Error: Expected " << szName
              << " element with text field.\n";
        return false;  // error condition
    }

    if (strcmp(szName, xml->getNodeName()) != 0) {
        otErr << __FUNCTION__ << ": Error: missing " << szName << " element.\n";
        return false;  // error condition
    }

    // If the caller wants values for certain
    // names expected to be on this node.
    if (nullptr != pmapExtraVars) {
        String::Map& mapExtraVars = (*pmapExtraVars);

        for (auto& it : mapExtraVars) {
            std::string first = it.first;
            auto strTemp =
                String::Factory(xml->getAttributeValue(first.c_str()));

            if (strTemp->Exists()) { mapExtraVars[first] = strTemp->Get(); }
        }
    }
    // Any attribute names passed in, now have their corresponding
    // values set on mapExtraVars (for caller.)

    if (false == Contract::LoadEncodedTextField(xml, ascOutput)) {
        otErr << __FUNCTION__ << ": Error loading " << szName << " field.\n";
        return false;
    }

    return true;
}

// Make sure you escape any lines that begin with dashes using "- "
// So "---BEGIN " at the beginning of a line would change to: "- ---BEGIN"
// This function expects that's already been done.
// This function assumes there is only unsigned contents, and not a signed
// contract.
// This function is intended to PRODUCE said signed contract.
// NOTE: This function also assumes you already instantiated a contract
// of the proper type. For example, if it's an ServerContract, then you
// INSTANTIATED an ServerContract. Because if you tried to do this using
// an Contract but then the strContract was for an ServerContract, then
// this function will fail when it tries to "LoadContractFromString()" since it
// is not actually the proper type to handle those data members.
//
// Therefore I need to make an entirely different (but similar) function which
// can be used for signing a piece of unsigned XML where the actual contract
// type
// is unknown.
//
bool Contract::CreateContract(const String& strContract, const Nym& theSigner)
{
    Release();

    char cNewline = 0;  // this is about to contain a byte read from the end of
                        // the contract.
    const std::uint32_t lLength = strContract.GetLength();

    if ((3 > lLength) || !strContract.At(lLength - 1, cNewline)) {
        otErr << __FUNCTION__
              << ": Invalid input: contract is less than 3 bytes "
                 "std::int64_t, or unable to read a byte from the end where a "
                 "newline is meant to be.\n";
        return false;
    }

    // ADD a newline, if necessary.
    // (The -----BEGIN part needs to start on its OWN LINE...)
    //
    // If length is 10, then string goes from 0..9.
    // Null terminator will be at 10.
    // Therefore the final newline should be at 9.
    // Therefore if char_at_index[lLength-1] != '\n'
    // Concatenate one!

    if ('\n' == cNewline)  // It already has a newline
        m_xmlUnsigned = strContract;
    else
        m_xmlUnsigned.Format("%s\n", strContract.Get());

    // This function assumes that m_xmlUnsigned is ready to be processed.
    // This function only processes that portion of the contract.
    //
    bool bLoaded = LoadContractXML();

    if (bLoaded) {

        // Add theSigner to the contract, if he's not already there.
        //
        if (nullptr == GetContractPublicNym()) {
            const bool bHasCredentials =
                (theSigner.HasCapability(NymCapability::SIGN_MESSAGE));

            if (!bHasCredentials) {
                otErr << __FUNCTION__ << ": Signing nym has no credentials.\n";
                return false;
            } else  // theSigner has Credentials, so we'll add him to the
                    // contract.
            {
                auto pNym = api_.Wallet().Nym(theSigner.ID());
                if (nullptr == pNym) {
                    otErr << __FUNCTION__ << ": failed to load signing nym."
                          << std::endl;
                    return false;
                }
                // Add pNym to the contract's internal list of nyms.
                m_mapNyms["signer"] = pNym;
            }
        }
        // This re-writes the contract internally based on its data members,
        // similar to UpdateContents. (Except, specifically intended for the
        // initial creation of the contract.)
        // Since theSigner was just added, he will be included here now as well,
        // just prior to the actual signing below.
        //
        CreateContents();

        OTPasswordData thePWData("Contract::CreateContract needs the private "
                                 "key to sign the contract...");

        if (!SignContract(theSigner, &thePWData)) {
            otErr << __FUNCTION__ << ": SignContract failed.\n";
            return false;
        }

        SaveContract();
        auto strTemp = String::Factory();
        SaveContractRaw(strTemp);

        if (LoadContractFromString(strTemp))  // The ultimate test is, once
        {                                     // we've created the serialized
            auto NEW_ID =
                Identifier::Factory();    // string for this contract, is
            CalculateContractID(NEW_ID);  // to then load it up from that
                                          // string.
            m_ID = NEW_ID;

            return true;
        }
    } else
        otErr << __FUNCTION__
              << ": LoadContractXML failed. strContract contents:\n\n"
              << strContract << "\n\n";

    return false;
}

// Overrides of CreateContents call this in order to add some common internals.
//
void Contract::CreateInnerContents(Tag& parent)
{
    // CONDITIONS
    //
    if (!m_mapConditions.empty()) {
        for (auto& it : m_mapConditions) {
            std::string str_condition_name = it.first;
            std::string str_condition_value = it.second;

            TagPtr pTag(new Tag("condition", str_condition_value));
            pTag->add_attribute("name", str_condition_name);
            parent.add_tag(pTag);
        }
    }
    // CREDENTIALS
    //
    if (!m_mapNyms.empty()) {
        // CREDENTIALS, based on NymID and Source, and credential IDs.
        for (auto& it : m_mapNyms) {
            std::string str_name = it.first;
            ConstNym pNym = it.second;
            OT_ASSERT_MSG(
                nullptr != pNym,
                "2: nullptr pseudonym pointer in "
                "Contract::CreateInnerContents.\n");

            if ("signer" == str_name) {
                OT_ASSERT(pNym->HasCapability(NymCapability::SIGN_MESSAGE));

                auto strNymID = String::Factory();
                pNym->GetIdentifier(strNymID);

                auto publicNym = pNym->asPublicNym();

                TagPtr pTag(new Tag(str_name));  // "signer"
                pTag->add_attribute("nymID", strNymID->Get());
                pTag->add_attribute(
                    "publicNym",
                    proto::ProtoAsArmored(
                        publicNym, String::Factory("PUBLIC NYM"))
                        ->Get());

                parent.add_tag(pTag);
            }  // "signer"
        }
    }  // if (m_mapNyms.size() > 0)
}

// Only used when first generating an asset or server contract.
// Meant for contracts which never change after that point.
// Otherwise does the same thing as UpdateContents.
// (But meant for a different purpose.)
// See ServerContract.cpp and OTUnitDefinition.cpp
//
void Contract::CreateContents()
{
    OT_FAIL_MSG("ASSERT: Contract::CreateContents should never be called, "
                "but should be overridden. (In this case, it wasn't.)");
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
std::int32_t Contract::ProcessXMLNode(IrrXMLReader*& xml)
{
    const auto strNodeName = String::Factory(xml->getNodeName());

    if (strNodeName->Compare("entity")) {
        m_strEntityShortName =
            String::Factory(xml->getAttributeValue("shortname"));
        if (!m_strName->Exists())  // only set it if it's not already set, since
            // the wallet may have already had a user label
            // set.
            m_strName = m_strEntityShortName;  // m_strName may later be changed
                                               // again in
        // OTUnitDefinition::ProcessXMLNode

        m_strEntityLongName =
            String::Factory(xml->getAttributeValue("longname"));
        m_strEntityEmail = String::Factory(xml->getAttributeValue("email"));

        otWarn << "Loaded Entity, shortname: " << m_strEntityShortName
               << "\nLongname: " << m_strEntityLongName
               << ", email: " << m_strEntityEmail << "\n----------\n";

        return 1;
    } else if (strNodeName->Compare("condition")) {
        // todo security: potentially start ascii-encoding these.
        // (Are they still "human readable" if you can easily decode them?)
        //
        auto strConditionName = String::Factory();
        auto strConditionValue = String::Factory();

        strConditionName = String::Factory(xml->getAttributeValue("name"));

        if (!SkipToTextField(xml)) {
            otOut << "Contract::ProcessXMLNode: Failure: Unable to find "
                     "expected text field for xml node named: "
                  << xml->getNodeName() << "\n";
            return (-1);  // error condition
        }

        if (EXN_TEXT == xml->getNodeType()) {
            strConditionValue = String::Factory(xml->getNodeData());
        } else {
            otErr << "Error in Contract::ProcessXMLNode: Condition without "
                     "value: "
                  << strConditionName << "\n";
            return (-1);  // error condition
        }

        // Add the conditions to a list in memory on this object.
        //
        m_mapConditions.insert(std::pair<std::string, std::string>(
            strConditionName->Get(), strConditionValue->Get()));

        otWarn << "---- Loaded condition \"" << strConditionName << "\"\n";
        //        otWarn << "Loading condition \"%s\": %s----------(END
        // DATA)----------\n", strConditionName.Get(),
        //                strConditionValue.Get());

        return 1;
    } else if (strNodeName->Compare("signer")) {
        const auto strSignerNymID =
            String::Factory(xml->getAttributeValue("nymID"));

        if (!strSignerNymID->Exists()) {
            otErr << "Error in " << __FUNCTION__
                  << ": "
                     "Expected nymID attribute on signer element.\n";
            return (-1);  // error condition
        }

        auto nymId = Identifier::Factory(strSignerNymID);
        auto pNym = api_.Wallet().Nym(nymId);

        if (nullptr == pNym) {
            otErr << __FUNCTION__ << ": Failure loading signing nym "
                  << std::endl;

            return (-1);
        }
        // Add pNym to the contract's internal list of nyms.
        m_mapNyms[strNodeName->Get() /*"signer"*/] = pNym;

        return 1;  // <==== Success!
    }
    return 0;
}
}  // namespace opentxs
