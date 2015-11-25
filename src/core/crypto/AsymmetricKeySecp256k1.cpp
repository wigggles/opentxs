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

#include <opentxs/core/Log.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/crypto/CryptoEngine.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/Libsecp256k1.hpp>

#include <vector>

namespace opentxs
{

AsymmetricKeySecp256k1::AsymmetricKeySecp256k1()
    : OTAsymmetricKey(OTAsymmetricKey::SECP256K1)
{
}

AsymmetricKeySecp256k1::AsymmetricKeySecp256k1(const proto::AsymmetricKey& serializedKey)
    : OTAsymmetricKey(serializedKey)
{
    m_keyType = OTAsymmetricKey::SECP256K1;

    OTData theKey(serializedKey.key().c_str(), serializedKey.key().size());
    if (proto::KEYMODE_PUBLIC == serializedKey.mode()) {
        SetKey(theKey, false);
    } else if (proto::KEYMODE_PRIVATE == serializedKey.mode()){
        SetKey(theKey, true);
    }
}

AsymmetricKeySecp256k1::AsymmetricKeySecp256k1(const String& publicKey)
    : OTAsymmetricKey()
{
    m_keyType = OTAsymmetricKey::SECP256K1;

    OTData dataKey;
    CryptoEngine::Instance().Util().Base58CheckDecode(publicKey, dataKey);

    SetKey(dataKey, true);
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
    if (!key_) {
        return true;
    }
    return false;
}

bool AsymmetricKeySecp256k1::SetKey(const OTData& key, bool isPrivate)
{
    ReleaseKeyLowLevel(); // In case the key is already loaded, we release it
                          // here. (Since it's being replaced, it's now the
                          // wrong key anyway.)
    m_bIsPublicKey = !isPrivate;
    m_bIsPrivateKey = isPrivate;

    key_.reset(new OTData(key));

    return true;
}

bool AsymmetricKeySecp256k1::GetKey(OTData& key) const
{
    if (key_) {
        key.Assign(*key_);

        return true;
    }

    return false;
}

bool AsymmetricKeySecp256k1::GetPublicKey(
    String& strKey) const
{
    strKey.reset();
    strKey.Set(CryptoEngine::Instance().Util().Base58CheckEncode(*key_));

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
            haveClearKey = static_cast<Libsecp256k1&>(engine()).ImportECDSAPrivkey(*key_, theExportPassword, pClearKey);
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
            otErr << __FUNCTION__ << ": Could not decrypt private key:\n\n";
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

serializedAsymmetricKey AsymmetricKeySecp256k1::Serialize() const

{
    serializedAsymmetricKey serializedKey = ot_super::Serialize();

    if (IsPrivate()) {
        serializedKey->set_mode(proto::KEYMODE_PRIVATE);
    } else {
        serializedKey->set_mode(proto::KEYMODE_PUBLIC);
    }

    serializedKey->set_key(key_->GetPointer(), key_->GetSize());

    return serializedKey;

}

} // namespace opentxs
