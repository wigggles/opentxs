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

#include <opentxs/core/crypto/OTKeypair.hpp>

#include <opentxs/core/FormattedKey.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs/core/Contract.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/crypto/LowLevelKeyGenerator.hpp>
#include <opentxs/core/crypto/OTSignature.hpp>
#include <opentxs/core/OTStorage.hpp>

#include <memory>
// DONE: Add OTKeypair member for m_pMetadata.
// Add method to set the Metadata. Or instead of a member,
// just have the method set the public and private keys.
//
// Then a key credential can have a similar function which sets the metadata
// for its three keypairs (they are identical except for the A|E|S.)
//
// When a Nym is loaded, load up its master credentials and all their
// child credentials. Since their metadata was supposedly set properly at
// creation, verify it at load time.

// TODO: on OTNym, change GetPublicKey to GetPublicKeyForVerify or
// GetPublicKeyForEncrypt or GetPublicKeyForTransmission. Then
// rebuild so that all places using GetPublicKey are forced to choose
// one of those. Same with GetPrivateKeyForSigning, GetPrivateKeyForDecrypt,
// and GetPrivateKeyForAuthentication.

// DONE: add the methods to OTPseudonym for generating a master credential
// contract
// and a sub contract. Add ability to save / load with this data. Then go from
// there.

namespace opentxs
{

OTKeypair::OTKeypair(OTAsymmetricKey::KeyType keyType)
    : m_pkeyPublic(OTAsymmetricKey::KeyFactory(keyType))
    , m_pkeyPrivate(OTAsymmetricKey::KeyFactory(keyType))
{
}

OTKeypair::~OTKeypair()
{

    if (nullptr != m_pkeyPublic) delete m_pkeyPublic; // todo: else error
    m_pkeyPublic = nullptr;

    if (nullptr != m_pkeyPrivate) delete m_pkeyPrivate; // todo: else error
    m_pkeyPrivate = nullptr;
}

void OTKeypair::SetMetadata(const OTSignatureMetadata& theMetadata)
{
    OT_ASSERT(nullptr != m_pkeyPublic);
    OT_ASSERT(nullptr != m_pkeyPrivate);
    OT_ASSERT(nullptr != m_pkeyPublic->m_pMetadata);
    OT_ASSERT(nullptr != m_pkeyPrivate->m_pMetadata);

    // Set it for both keys.
    //
    *(m_pkeyPublic->m_pMetadata) = theMetadata;
    *(m_pkeyPrivate->m_pMetadata) = theMetadata;
}

bool OTKeypair::HasPublicKey() const
{
    OT_ASSERT(nullptr != m_pkeyPublic);

    return m_pkeyPublic->IsPublic(); // This means it actually has a public key
                                     // in it, or tried to.
}

bool OTKeypair::HasPrivateKey() const
{
    OT_ASSERT(nullptr != m_pkeyPrivate);

    return m_pkeyPrivate->IsPrivate(); // This means it actually has a private
                                       // key in it, or tried to.
}

const OTAsymmetricKey& OTKeypair::GetPublicKey() const
{
    OT_ASSERT(nullptr != m_pkeyPublic);

    return (*m_pkeyPublic);
}

const OTAsymmetricKey& OTKeypair::GetPrivateKey() const
{
    OT_ASSERT(nullptr != m_pkeyPrivate);

    return (*m_pkeyPrivate);
}

bool OTKeypair::GetPrivateKey(FormattedKey& strOutput,
                                              const String* pstrReason,
                                              const OTPassword* pImportPassword)
{
    String strCert, strPrivateKey;

    OT_ASSERT(nullptr != m_pkeyPublic);
    OT_ASSERT(nullptr != m_pkeyPrivate);

    const bool bSaved1 = m_pkeyPublic->SaveCertToString(strCert, pstrReason, pImportPassword);
    const bool bSaved2 =
        m_pkeyPrivate->SavePrivateKeyToString(strPrivateKey, pstrReason, pImportPassword);

    if (bSaved1 && bSaved2)
        strOutput.Format(const_cast<char*>("%s%s"), strPrivateKey.Get(),
                         strCert.Get());

    return (bSaved1 && bSaved2);
}

// SetPrivateKey
//
// "escaped" means pre-pended with "- " as in:   - -----BEGIN CERTIFICATE....
//
// This function sets both keys
bool OTKeypair::SetPrivateKey(const FormattedKey& strCert,
                                             bool bEscaped,
                                             const String* pstrReason,
                                             const OTPassword* pImportPassword)
{
    OT_ASSERT(nullptr != m_pkeyPrivate);
    OT_ASSERT(nullptr != m_pkeyPublic);

    bool privateSuccess, publicSuccess;

    privateSuccess = m_pkeyPrivate->LoadPrivateKeyFromCertString(
        strCert, bEscaped, pstrReason, pImportPassword);

    publicSuccess = m_pkeyPublic->LoadPublicKeyFromCertString(
        strCert, bEscaped, pstrReason, pImportPassword);

    return (privateSuccess && publicSuccess);
}

bool OTKeypair::MakeNewKeypair(const std::shared_ptr<NymParameters>& pKeyData)
{
    OT_ASSERT(nullptr != m_pkeyPrivate);
    OT_ASSERT(nullptr != m_pkeyPublic);

    if (pKeyData) {
        LowLevelKeyGenerator lowLevelKeys(pKeyData);//does not take ownership

        if (!lowLevelKeys.MakeNewKeypair()) {
            otErr << "OTKeypair::MakeNewKeypair"
                << ": Failed in a call to LowLevelKeyGenerator::MakeNewKeypair.\n";
            return false;
        }

        return lowLevelKeys.SetOntoKeypair(*this);
    } else {
        return false;
    }
    // If true is returned:
    // Success! At this point, theKeypair's public and private keys have been
    // set.
}

bool OTKeypair::SignContract(Contract& theContract,
                             const OTPasswordData* pPWData)
{
    OT_ASSERT(nullptr != m_pkeyPrivate);

    return theContract.SignWithKey(*m_pkeyPrivate, pPWData);
}

// PUBLIC KEY

// * Get the public key in ASCII-armored format WITH bookends   -- OTString
//       - ------- BEGIN PUBLIC KEY --------
//       Notice the "- " before the rest of the bookend starts.
//
bool OTKeypair::GetPublicKey(FormattedKey& strKey) const
{
    OT_ASSERT(nullptr != m_pkeyPublic);

    return m_pkeyPublic->GetPublicKey(strKey, true);
}
bool OTKeypair::GetPublicKey(String& strKey) const
{
    OT_ASSERT(nullptr != m_pkeyPublic);

    return m_pkeyPublic->GetPublicKey(strKey, false);
}

// Decodes a public key from bookended key string into an actual key
// pointer, and sets that as the m_pkeyPublic on this object.
// This is the version that will handle the bookends ( -----BEGIN PUBLIC
// KEY-----)
//
bool OTKeypair::SetPublicKey(const String& strKey, bool bEscaped)
{
    OT_ASSERT(nullptr != m_pkeyPublic);

    // the below function SetPublicKey (in the return call) expects the
    // bookends to still be there, and it will handle removing them.
    return m_pkeyPublic->SetPublicKey(strKey, bEscaped);
}

// PRIVATE KEY
// Get the private key in ASCII-armored format with bookends
// - ------- BEGIN ENCRYPTED PRIVATE KEY --------
// Notice the "- " before the rest of the bookend starts.
//
bool OTKeypair::GetPrivateKey(String& strKey, bool bEscaped) const
{
    OT_ASSERT(nullptr != m_pkeyPrivate);

    return m_pkeyPrivate->GetPrivateKey(strKey, bEscaped);
}

// Decodes a private key from ASCII armor into an actual key pointer
// and sets that as the m_pPrivateKey on this object.
// This is the version that will handle the bookends ( -----BEGIN ENCRYPTED
// PRIVATE KEY-----)
bool OTKeypair::SetPrivateKey(const String& strKey, bool bEscaped)
{
    OT_ASSERT(nullptr != m_pkeyPrivate);

    const char* szOverride = "PGP PRIVATE KEY";

    if (strKey.Contains(szOverride)) {
        OTASCIIArmor theArmor;

        if (theArmor.LoadFromString(const_cast<String&>(strKey), bEscaped,
                                    szOverride)) // szOverride == "PGP PRIVATE
                                                 // KEY"
        {
            // This function expects that the bookends are already removed.
            // The ascii-armor loading code removes them and handles the escapes
            // also.
            //            return
            // m_pkeyPrivate->LoadPrivateKeyFromPGPKey(theArmor);
            //
            otOut << "OTKeypair::SetPrivateKey 1: Failure: PGP private keys "
                     "are NOT YET SUPPORTED.\n\n";
            //            otOut << "OTKeypair::SetPrivateKey 1: Failure: PGP
            // private keys are NOT YET SUPPORTED:\n\n%s\n\n",
            //                           strKey.Get());
            return false;
        }
        else {
            otOut << "OTKeypair::SetPrivateKey 2: Failure: PGP private keys "
                     "are NOT YET SUPPORTED.\n\n";
            //            otOut << "OTKeypair::SetPrivateKey 2: Failure: PGP
            // private keys are NOT YET SUPPORTED:\n\n%s\n\n",
            //                           strKey.Get());
            return false;
        }
    }
    else // the below function SetPrivateKey (in the return call) expects the
        // bookends to still be there, and it will handle removing them. (Unlike
        // PGP code above.)
        //
        return m_pkeyPrivate->SetPrivateKey(strKey, bEscaped);
}

bool OTKeypair::CalculateID(Identifier& theOutput) const
{
    OT_ASSERT(nullptr != m_pkeyPublic);

    return m_pkeyPublic->CalculateID(theOutput); // Only works for public keys.
}

int32_t OTKeypair::GetPublicKeyBySignature(
    listOfAsymmetricKeys& listOutput, // Inclusive means, return the key even
                                      // when theSignature has no metadata.
    const OTSignature& theSignature, bool bInclusive) const
{
    OT_ASSERT(nullptr != m_pkeyPublic);
    OT_ASSERT(nullptr != m_pkeyPublic->m_pMetadata);

    // We know that EITHER exact metadata matches must occur, and the signature
    // MUST have metadata, (bInclusive=false)
    // OR if bInclusive=true, we know that metadata is still used to eliminate
    // keys where possible, but that otherwise,
    // if the signature has no metadata, then the key is still returned, "just
    // in case."
    //
    if ((false == bInclusive) &&
        (false == theSignature.getMetaData().HasMetadata()))
        return 0;

    // Below this point, metadata is used if it's available.
    // It's assumed to be "okay" if it's not available, since any non-inclusive
    // calls would have already returned by now, if that were the case.
    // (But if it IS available, then it must match, or the key won't be
    // returned.)
    //
    // If the signature has no metadata, or if m_pkeyPublic has no metadata, or
    // if they BOTH have metadata, and their metadata is a MATCH...
    if (!theSignature.getMetaData().HasMetadata() ||
        !m_pkeyPublic->m_pMetadata->HasMetadata() ||
        (m_pkeyPublic->m_pMetadata->HasMetadata() &&
         theSignature.getMetaData().HasMetadata() &&
         (theSignature.getMetaData() == *(m_pkeyPublic->m_pMetadata)))) {
        // ...Then add m_pkeyPublic as a possible match, to listOutput.
        //
        listOutput.push_back(m_pkeyPublic);
        return 1;
    }
    return 0;
}

// Used when importing/exporting a Nym to/from the wallet.
//
bool OTKeypair::ReEncrypt(const OTPassword& theExportPassword, bool bImporting,
                          String& strOutput)
{

    OT_ASSERT(nullptr != m_pkeyPublic);
    OT_ASSERT(nullptr != m_pkeyPrivate);

    OT_ASSERT(HasPublicKey());
    OT_ASSERT(HasPrivateKey());

    // If we were importing, we were in the exported format but now we're in the
    // internal format.
    // Therefore we want to use the wallet's internal cached master passphrase
    // to save. Therefore
    // strReason will be used for the import case.
    //
    // But if we were exporting, then we were in the internal format and just
    // re-encrypted to the
    // export format. So we'd want to pass the export passphrase when saving.
    //
    const String strReasonAbove(
        bImporting ? "Enter the new export passphrase. (Above "
                     "ReEncryptPrivateKey in OTKeypair::ReEncrypt)"
                   : "Enter your wallet's master passphrase. (Above "
                     "ReEncryptPrivateKey in OTKeypair::ReEncrypt)");

    const String strReasonBelow(
        bImporting ? "Enter your wallet's master passphrase. (Below "
                     "ReEncryptPrivateKey in OTKeypair::ReEncrypt)"
                   : "Enter the new export passphrase. (Below "
                     "ReEncryptPrivateKey in OTKeypair::ReEncrypt)");

    // At this point the public key was loaded from a public key, not a cert,
    // but the private key was loaded from the cert. Therefore we'll save the
    // public cert from the private key, and then use that to reload the public
    // key after ReEncrypting. (Otherwise the public key would be there, but it
    // would be missing the x509, which is only available in the cert, not the
    // pubkey alone -- and without the x509 being there, the "SaveAndReload"
    // call
    // below would fail.
    // Why don't I just stick the Cert itself into the public data, instead of
    // sticking the public key in there? Because not all key credentials will
    // use
    // certs. Some will use pubkeys from certs, and some will use pubkeys not
    // from
    // certs. But I might still just stick it in there, and code things to be
    // able to
    // load either indiscriminately. After all, that's what I'm doing already in
    // the
    // asset and server contracts. But even in those cases, there will be times
    // when
    // only a pubkey is available, not a cert, so I'll probably still find
    // myself having
    // to do this. Hmm...

    const bool bReEncrypted = m_pkeyPrivate->ReEncryptPrivateKey(
        theExportPassword, bImporting); // <==== IMPORT or EXPORT occurs here.
    bool bGotCert = false;

    const bool bSuccess = (bReEncrypted && bGotCert);

    if (!bSuccess) {
        strOutput.Release();
        otErr << __FUNCTION__ << ": Failure, either when re-encrypting, or "
                                 "when subsequently retrieving "
                                 "the public/private keys. bImporting == "
              << (bImporting ? "true" : "false") << "\n";
    }

    return bSuccess;
}

} // namespace opentxs
