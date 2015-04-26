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

#ifndef OPENTXS_CORE_CRYPTO_OTKEYPAIR_HPP
#define OPENTXS_CORE_CRYPTO_OTKEYPAIR_HPP

#include <list>
#include <cstdint>

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

class OTASCIIArmor;
class OTAsymmetricKey;
class Contract;
class Identifier;
class OTPassword;
class OTPasswordData;
class OTSignature;
class OTSignatureMetadata;
class String;

typedef std::list<OTAsymmetricKey*> listOfAsymmetricKeys;

// Encapsulates public/private key (though often there may only be
// a public key present, unless the nym belongs to you.)
//
class OTKeypair
{
    friend class OTLowLevelKeyData;

    OTAsymmetricKey* m_pkeyPublic;  // This nym's public key
    OTAsymmetricKey* m_pkeyPrivate; // This nym's private key

public:
    EXPORT bool MakeNewKeypair(int32_t nBits = 1024);
    EXPORT bool ReEncrypt(const OTPassword& theExportPassword, bool bImporting,
                          String& strOutput); // Used when importing/exporting
                                              // a Nym to/from the wallet.
    EXPORT bool HasPublicKey() const;
    EXPORT bool HasPrivateKey() const;
    EXPORT const OTAsymmetricKey& GetPublicKey() const;
    EXPORT const OTAsymmetricKey& GetPrivateKey() const;
    EXPORT bool CalculateID(Identifier& theOutput) const;
    EXPORT bool SaveCertToString(
        String& strOutput, const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr) const;
    EXPORT bool SavePrivateKeyToString(
        String& strOutput, const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr) const;
    EXPORT bool SaveCertAndPrivateKeyToString(
        String& strOutput, const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr);
    // Load from local storage.
    EXPORT bool LoadPrivateKey(const String& strFoldername,
                               const String& strFilename,
                               const String* pstrReason = nullptr,
                               const OTPassword* pImportPassword = nullptr);
    EXPORT bool LoadPublicKey(const String& strFoldername,
                              const String& strFilename);
    // LoadPrivateKeyFromCertString
    //
    // "escaped" means pre-pended with "- " as in:   - -----BEGIN
    // CERTIFICATE....
    //
    EXPORT bool LoadPrivateKeyFromCertString(
        const String& strCert, bool bEscaped = true,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr);
    // Load Public Key from Cert (file or string)
    //
    EXPORT bool LoadPublicKeyFromCertString(
        const String& strCert, bool bEscaped = true,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr); // DOES handle bookends,
                                                      // AND escapes.
    EXPORT bool LoadPublicKeyFromCertFile(
        const String& strFoldername, const String& strFilename,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr); // DOES handle bookends.
    EXPORT bool LoadCertAndPrivateKeyFromString(
        const String& strInput, const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr);
    // LOAD BOTH KEYS FROM CERT FILE
    //
    EXPORT bool LoadBothKeysFromCertFile(
        const String& strFoldername, const String& strFilename,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr);

    EXPORT bool SaveAndReloadBothKeysFromTempFile(
        String* pstrOutputCert = nullptr, const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr);
    // PUBLIC KEY

    // * Get the public key in ASCII-armored format WITH bookends   -- OTString
    //       - ------- BEGIN PUBLIC KEY --------
    //       Notice the "- " before the rest of the bookend starts.
    EXPORT bool GetPublicKey(String& strKey, bool bEscaped = true) const;
    // (Below) Decodes a public key from ASCII armor into an actual key pointer
    // and sets that as the m_pKey on this object.
    EXPORT bool SetPublicKey(const OTASCIIArmor& strKey);
    EXPORT bool SetPublicKey(const String& strKey, bool bEscaped = false);
    // (Above) Decodes a public key from bookended key string into an actual key
    // pointer, and sets that as the m_pPublicKey on this object.
    // This is the version that will handle the bookends ( -----BEGIN PUBLIC
    // KEY-----)

    // PRIVATE KEY
    // Get the private key in ASCII-armored format with bookends
    // - ------- BEGIN ENCRYPTED PRIVATE KEY --------
    // Notice the "- " before the rest of the bookend starts.
    EXPORT bool GetPrivateKey(String& strKey, bool bEscaped = true) const;
    EXPORT bool GetPrivateKey(OTASCIIArmor& strKey) const; // Get the private
                                                           // key in
                                                           // ASCII-armored
                                                           // format
    // Decodes a private key from ASCII armor into an actual key pointer
    // and sets that as the m_pPrivateKey on this object.
    // This is the version that will handle the bookends ( -----BEGIN ENCRYPTED
    // PRIVATE KEY-----)
    EXPORT bool SetPrivateKey(const String& strKey, bool bEscaped = false);
    EXPORT bool SetPrivateKey(const OTASCIIArmor& strKey); // Decodes a private
                                                           // key from ASCII
                                                           // armor into an
                                                           // actual key pointer
                                                           // and sets that as
                                                           // the m_pKey on this
                                                           // object.
    // Only works if a private key is present.
    //
    EXPORT bool SignContract(Contract& theContract,
                             const OTPasswordData* pPWData = nullptr);
    EXPORT void SetMetadata(const OTSignatureMetadata& theMetadata);
    EXPORT int32_t GetPublicKeyBySignature(
        listOfAsymmetricKeys& listOutput, // inclusive means, return keys when
                                          // theSignature has no metadata.
        const OTSignature& theSignature, bool bInclusive = false) const;
    EXPORT OTKeypair();
    EXPORT ~OTKeypair();
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTKEYPAIR_HPP
