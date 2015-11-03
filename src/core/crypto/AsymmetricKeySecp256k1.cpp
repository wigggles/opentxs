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

#include <opentxs/core/crypto/AsymmetricKeySecp256k1.hpp>

#include <opentxs/core/FormattedKey.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/crypto/CryptoEngine.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/Libsecp256k1.hpp>

#include <vector>

namespace opentxs
{

AsymmetricKeySecp256k1::AsymmetricKeySecp256k1()
    : OTAsymmetricKey()
{
    m_keyType = OTAsymmetricKey::SECP256K1;
}

void AsymmetricKeySecp256k1::ReleaseKeyLowLevel_Hook() const
{
}

CryptoAsymmetric& AsymmetricKeySecp256k1::engine() const

{
    return CryptoEngine::Instance().SECP256K1();
}

bool AsymmetricKeySecp256k1::IsEmpty() const
{
    return key_.empty();
}

bool AsymmetricKeySecp256k1::SetPrivateKey(
    const FormattedKey& strCert,
    __attribute__((unused)) const String* pstrReason, // This reason is what displays on the
                              // passphrase dialog.
    __attribute__((unused)) const OTPassword* pImportPassword) // Used when importing an exported
                                       // Nym into a wallet.
{
    ReleaseKeyLowLevel(); // In case the key is already loaded, we release it
                          // here. (Since it's being replaced, it's now the
                          // wrong key anyway.)
    m_bIsPublicKey = false;
    m_bIsPrivateKey = true;

    key_.reset();
    key_.Set(strCert.Get());

    return true;
}

bool AsymmetricKeySecp256k1::SetPublicKeyFromPrivateKey(
    const FormattedKey& strCert,
    const String* pstrReason,
    __attribute__((unused)) const OTPassword* pImportPassword)
{
    ReleaseKeyLowLevel(); // In case the key is already loaded, we release it
                          // here. (Since it's being replaced, it's now the
                          // wrong key anyway.)
    m_bIsPublicKey = true;
    m_bIsPrivateKey = false;

    Libsecp256k1& engine = static_cast<Libsecp256k1&>(this->engine());
    bool havePrivkey = false;
    OTPassword privKey;

    if (nullptr != pstrReason) {
        OTPasswordData passwordData(*pstrReason);
        havePrivkey = engine.AsymmetricKeyToECDSAPrivkey(strCert, passwordData, privKey);
    } else {
        OTPasswordData passwordData("Unlock the nym's private credential.");
        havePrivkey = engine.AsymmetricKeyToECDSAPrivkey(strCert, passwordData, privKey);
    }

    if (havePrivkey) {
        secp256k1_pubkey_t pubKey;

        bool validPubkey = engine.secp256k1_pubkey_create(pubKey, privKey);

        if (validPubkey) {
            return engine.ECDSAPubkeyToAsymmetricKey(pubKey, *this);
        }
    }

    return false;
}

bool AsymmetricKeySecp256k1::GetPrivateKey(
    FormattedKey& strOutput,
    __attribute__((unused)) const OTAsymmetricKey* pPubkey,
    __attribute__((unused)) const String* pstrReason,
    __attribute__((unused)) const OTPassword* pImportPassword) const
{
    strOutput.reset();
    strOutput.Set(key_.Get());

    return true;
}

bool AsymmetricKeySecp256k1::GetPublicKey(
    __attribute__((unused)) String& strKey) const
{
    strKey.reset();
    strKey.Set(key_.Get());

    return true;
}

bool AsymmetricKeySecp256k1::GetPublicKey(
    __attribute__((unused)) FormattedKey& strKey) const
{
    strKey.reset();
    strKey.Set(key_.Get());

    return true;
}

bool AsymmetricKeySecp256k1::SetPublicKey(
    const String& strKey)
{
    ReleaseKeyLowLevel(); // In case the key is already loaded, we release it
                          // here. (Since it's being replaced, it's now the
                          // wrong key anyway.)
    m_bIsPublicKey = true;
    m_bIsPrivateKey = false;

    key_.reset();
    key_.Set(strKey.Get());

    return true;
}

bool AsymmetricKeySecp256k1::SetPublicKey(
    const FormattedKey& strKey)
{
    ReleaseKeyLowLevel(); // In case the key is already loaded, we release it
                          // here. (Since it's being replaced, it's now the
                          // wrong key anyway.)
    m_bIsPublicKey = true;
    m_bIsPrivateKey = false;

    key_.reset();
    key_.Set(strKey.Get());

    return true;
}

bool AsymmetricKeySecp256k1::ReEncryptPrivateKey(
    __attribute__((unused)) const OTPassword& theExportPassword,
    __attribute__((unused)) bool bImporting) const
{
    OT_ASSERT(IsPrivate());

    bool bReturnVal = false;

    if (!IsEmpty() > 0) {
        OTPassword pClearKey;
        bool haveClearKey = false;

        // Here's thePWData we use if we didn't have anything else:
        //
        OTPasswordData thePWData(
            bImporting ? "(Importing) Enter the exported Nym's passphrase."
                       : "(Exporting) Enter your wallet's master passphrase.");

        // If we're importing, that means we're currently stored as an EXPORTED
        // NYM (i.e. with its own
        // password, independent of the wallet.) So we use theExportedPassword.
        //
        if (bImporting) {
            haveClearKey = static_cast<Libsecp256k1&>(engine()).ImportECDSAPrivkey(key_, theExportPassword, pClearKey);
        }
        // Else if we're exporting, that means we're currently stored in the
        // wallet (i.e. using the wallet's
        // cached master key.) So we use the normal password callback.
        //
        else {
            haveClearKey = static_cast<Libsecp256k1&>(engine()).AsymmetricKeyToECDSAPrivkey(*this, thePWData, pClearKey);
        }

        if (haveClearKey) {
            otLog4
                << __FUNCTION__
                << ": Success decrypting private key.\n";

            // Okay, we have loaded up the private key, now let's save it
            // using the new passphrase.

            // If we're importing, that means we just loaded up the (previously)
            // exported Nym using theExportedPassphrase, so now we need to save it
            // again using the normal password callback (for importing it to the
            // wallet.)

            bool reencrypted = false;

            if (bImporting) {
                reencrypted = static_cast<Libsecp256k1&>(engine()).ECDSAPrivkeyToAsymmetricKey(pClearKey, thePWData, *const_cast<AsymmetricKeySecp256k1*>(this));
            }

            // Else if we're exporting, that means we just loaded up the Nym
            // from the wallet using the normal password callback, and now we
            // need to save it back again using theExportedPassphrase (for exporting
            // it outside of the wallet.)
            else {
                reencrypted = static_cast<Libsecp256k1&>(engine()).ExportECDSAPrivkey(pClearKey, theExportPassword, *const_cast<AsymmetricKeySecp256k1*>(this));
            }

            if (!reencrypted) {
                otErr << __FUNCTION__ << ": Could not encrypt private key:\n\n";
            }

            bReturnVal = reencrypted;

        }
        else
            otErr << __FUNCTION__ << ": Could not decrypt private key:\n\n"
                  << key_.Get() << "\n\n";
    }
    else
        otErr << __FUNCTION__
              << ": Key is empty.\n\n";

    return bReturnVal;
}

void AsymmetricKeySecp256k1::Release_AsymmetricKeySecp256k1()
{
}

void AsymmetricKeySecp256k1::Release()
{
    Release_AsymmetricKeySecp256k1(); // My own cleanup is performed here.

    // Next give the base class a chance to do the same...
    ot_super::Release(); // since I've overridden the base class, I call it
                         // now...
}

AsymmetricKeySecp256k1::~AsymmetricKeySecp256k1()
{
    Release_AsymmetricKeySecp256k1();

    ReleaseKeyLowLevel_Hook();
}

} // namespace opentxs
