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

#include "opentxs/core/crypto/AsymmetricKeyEC.hpp"

#include "opentxs/core/app/App.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHash.hpp"
#include "opentxs/core/crypto/CryptoUtil.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTData.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"

extern "C" {
#include <sodium/crypto_box.h>
}

namespace opentxs
{
AsymmetricKeyEC::AsymmetricKeyEC(
    const proto::AsymmetricKeyType keyType, const proto::KeyRole role)
        : ot_super(keyType, role)
{
}

AsymmetricKeyEC::AsymmetricKeyEC(
    const proto::AsymmetricKey& serializedKey)
        : ot_super(serializedKey)
{
    m_keyType = serializedKey.type();

    std::unique_ptr<OTData> theKey;
    theKey.reset(
        new OTData(serializedKey.key().c_str(), serializedKey.key().size()));

    OT_ASSERT(theKey);

    if (proto::KEYMODE_PUBLIC == serializedKey.mode()) {
        SetKey(theKey, false);
    } else if (proto::KEYMODE_PRIVATE == serializedKey.mode()) {
        SetKey(theKey, true);
    }
}

AsymmetricKeyEC::AsymmetricKeyEC(
    const proto::AsymmetricKeyType keyType,
    const String& publicKey)
        : AsymmetricKeyEC(keyType, proto::KEYROLE_ERROR)
{
    m_keyType = proto::AKEYTYPE_SECP256K1;

    std::unique_ptr<OTData> dataKey(new OTData());

    OT_ASSERT(dataKey);

    App::Me().Crypto().Util().Base58CheckDecode(publicKey, *dataKey);

    SetKey(dataKey, true);
}

bool AsymmetricKeyEC::GetKey(OTData& key) const
{
    if (key_) {
        key.Assign(*key_);

        return true;
    }

    return false;
}

bool AsymmetricKeyEC::GetPublicKey(
    String& strKey) const
{
    strKey.reset();
    strKey.Set(App::Me().Crypto().Util().Base58CheckEncode(*key_));

    return true;
}

bool AsymmetricKeyEC::IsEmpty() const
{
    if (!key_) {

        return true;
    }

    return false;
}

bool AsymmetricKeyEC::ReEncryptPrivateKey(
    const OTPassword& theExportPassword,
    bool bImporting) const
{
    OT_ASSERT(IsPrivate());

    bool bReturnVal = false;

    if (!IsEmpty() > 0) {
        OTPassword pClearKey;
        bool haveClearKey = false;

        // Here's thePWData we use if we didn't have anything else:
        OTPasswordData thePWData(
            bImporting ? "(Importing) Enter the exported Nym's passphrase."
                       : "(Exporting) Enter your wallet's master passphrase.");

        // If we're importing, that means we're currently stored as an EXPORTED
        // NYM (i.e. with its own password, independent of the wallet.) So we
        // use theExportedPassword.
        if (bImporting) {
            haveClearKey = ECDSA().ImportECPrivatekey(
                *key_, theExportPassword, pClearKey);
        }
        // Else if we're exporting, that means we're currently stored in the
        // wallet (i.e. using the wallet's cached master key.) So we use the
        // normal password callback.
        else {
            haveClearKey = ECDSA().AsymmetricKeyToECPrivatekey(
                *this, thePWData, pClearKey);
        }

        if (haveClearKey) {
            otLog4 << __FUNCTION__ << ": Success decrypting private key."
                   << std::endl;

            // Okay, we have loaded up the private key, now let's save it
            // using the new passphrase.

            // If we're importing, that means we just loaded up the (previously)
            // exported Nym using theExportedPassphrase, so now we need to save
            // it again using the normal password callback (for importing it to
            // the wallet.)

            bool reencrypted = false;

            if (bImporting) {
                reencrypted = ECDSA().ECPrivatekeyToAsymmetricKey(
                    pClearKey,
                    thePWData,
                    *const_cast<AsymmetricKeyEC*>(this));
            }

            // Else if we're exporting, that means we just loaded up the Nym
            // from the wallet using the normal password callback, and now we
            // need to save it back again using theExportedPassphrase (for
            // exporting it outside of the wallet.)
            else {
                reencrypted = ECDSA().ExportECPrivatekey(
                    pClearKey,
                    theExportPassword,
                    *const_cast<AsymmetricKeyEC*>(this));
            }

            if (!reencrypted) {
                otErr << __FUNCTION__ << ": Could not encrypt private key."
                      << std::endl;
            }

            bReturnVal = reencrypted;

        } else {
            otErr << __FUNCTION__ << ": Could not decrypt private key."
                  << std::endl;
        }
    } else {
        otErr << __FUNCTION__ << ": Key is empty." << std::endl;
    }

    return bReturnVal;
}

void AsymmetricKeyEC::Release()
{
    Release_AsymmetricKeyEC(); // My own cleanup is performed here.

    // Next give the base class a chance to do the same...
    ot_super::Release(); // since I've overridden the base class, I call it
                         // now...
}

serializedAsymmetricKey AsymmetricKeyEC::Serialize() const
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

bool AsymmetricKeyEC::SetKey(std::unique_ptr<OTData>& key, bool isPrivate)
{
    ReleaseKeyLowLevel();
    m_bIsPublicKey = !isPrivate;
    m_bIsPrivateKey = isPrivate;
    key_.swap(key);

    return true;
}

bool AsymmetricKeyEC::TransportKey(
    OTData& publicKey,
    OTPassword& privateKey) const
{
    if (!IsPrivate()) { return false; }

    if (!key_) { return false; }

    OTPassword seed;
    ECDSA().AsymmetricKeyToECPrivatekey(*this, "Get transport key", seed);

    return ECDSA().SeedToCurveKey(seed, privateKey, publicKey);
}
} // namespace opentxs
