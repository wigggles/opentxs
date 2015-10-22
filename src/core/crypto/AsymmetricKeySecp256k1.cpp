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
#include <opentxs/core/String.hpp>
#include <opentxs/core/crypto/CryptoEngine.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>

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
    return true;
}

bool AsymmetricKeySecp256k1::SetPrivateKey(
    __attribute__((unused)) const FormattedKey& strCert,
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

    return false;
}

bool AsymmetricKeySecp256k1::SetPublicKeyFromPrivateKey(
    __attribute__((unused)) const FormattedKey& strCert,
    __attribute__((unused)) const String* pstrReason,
    __attribute__((unused)) const OTPassword* pImportPassword)
{
    ReleaseKeyLowLevel(); // In case the key is already loaded, we release it
                          // here. (Since it's being replaced, it's now the
                          // wrong key anyway.)
    m_bIsPublicKey = false;
    m_bIsPrivateKey = true;

    return false;
}

bool AsymmetricKeySecp256k1::GetPrivateKey(
    __attribute__((unused)) FormattedKey& strOutput,
    __attribute__((unused)) const OTAsymmetricKey* pPubkey,
    __attribute__((unused)) const String* pstrReason,
    __attribute__((unused)) const OTPassword* pImportPassword) const
{
    return false;
}

bool AsymmetricKeySecp256k1::GetPublicKey(
    __attribute__((unused)) String& strKey) const
{
    return false;
}

bool AsymmetricKeySecp256k1::GetPublicKey(
    __attribute__((unused)) FormattedKey& strKey) const
{
    return false;
}

bool AsymmetricKeySecp256k1::SetPublicKey(
    __attribute__((unused)) const String& strKey)
{
    ReleaseKeyLowLevel(); // In case the key is already loaded, we release it
                          // here. (Since it's being replaced, it's now the
                          // wrong key anyway.)
    m_bIsPublicKey = true;
    m_bIsPrivateKey = false;

    return false;
}

bool AsymmetricKeySecp256k1::SetPublicKey(
    __attribute__((unused)) const FormattedKey& strKey)
{
    ReleaseKeyLowLevel(); // In case the key is already loaded, we release it
                          // here. (Since it's being replaced, it's now the
                          // wrong key anyway.)
    m_bIsPublicKey = true;
    m_bIsPrivateKey = false;

    return false;
}

bool AsymmetricKeySecp256k1::ReEncryptPrivateKey(
    __attribute__((unused)) const OTPassword& theExportPassword,
    __attribute__((unused)) bool bImporting) const
{
    return false;
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
