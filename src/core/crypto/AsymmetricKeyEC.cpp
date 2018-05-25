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

#include "stdafx.hpp"

#include "opentxs/core/crypto/AsymmetricKeyEC.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/Ecdsa.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

extern "C" {
#include <sodium/crypto_box.h>
}

#define OT_METHOD "opentxs::AsymmetricKeyEC::"

namespace opentxs
{
AsymmetricKeyEC::AsymmetricKeyEC(
    const proto::AsymmetricKeyType keyType,
    const proto::KeyRole role)
    : ot_super(keyType, role)
    , key_(Data::Factory())
    , encrypted_key_(nullptr)
    , path_(nullptr)
    , chain_code_(nullptr)
{
}

AsymmetricKeyEC::AsymmetricKeyEC(const proto::AsymmetricKey& serializedKey)
    : ot_super(serializedKey)
    , key_(Data::Factory())
    , encrypted_key_(nullptr)
    , path_(nullptr)
    , chain_code_(nullptr)
{
    m_keyType = serializedKey.type();

    if (serializedKey.has_path()) {
        path_ = std::make_shared<proto::HDPath>(serializedKey.path());
    }

    if (serializedKey.has_chaincode()) {
        chain_code_.reset(new proto::Ciphertext(serializedKey.chaincode()));
    }

    if (proto::KEYMODE_PUBLIC == serializedKey.mode()) {
        auto theKey = Data::Factory(
            serializedKey.key().c_str(), serializedKey.key().size());
        SetKey(theKey);
    } else if (proto::KEYMODE_PRIVATE == serializedKey.mode()) {
        std::unique_ptr<proto::Ciphertext> encryptedKey;
        encryptedKey.reset(new proto::Ciphertext(serializedKey.encryptedkey()));

        OT_ASSERT(encryptedKey);

        SetKey(encryptedKey);
    }
}

AsymmetricKeyEC::AsymmetricKeyEC(
    const proto::AsymmetricKeyType keyType,
    const String& publicKey)
    : AsymmetricKeyEC(keyType, proto::KEYROLE_ERROR)
{
    m_keyType = proto::AKEYTYPE_SECP256K1;
    auto key = OT::App().Crypto().Encode().DataDecode(publicKey.Get());
    auto dataKey = Data::Factory(key.data(), key.size());
    SetKey(dataKey);
}

bool AsymmetricKeyEC::GetKey(Data& key) const
{
    if (key_->empty()) { return false; }

    key.Assign(key_.get());

    return true;
}

bool AsymmetricKeyEC::GetKey(proto::Ciphertext& key) const
{
    if (encrypted_key_) {
        key.CopyFrom(*encrypted_key_);

        return true;
    }

    return false;
}

bool AsymmetricKeyEC::GetPublicKey(String& strKey) const
{
    strKey.reset();
    strKey.Set(OT::App().Crypto().Encode().DataEncode(key_.get()).c_str());

    return true;
}

bool AsymmetricKeyEC::GetPublicKey(Data& key) const
{
    if (false == key_->empty()) {
        key.Assign(key_->GetPointer(), key_->GetSize());

        return true;
    }

    if (false == bool(encrypted_key_)) { return false; }

    return ECDSA().PrivateToPublic(*encrypted_key_, key);
}

bool AsymmetricKeyEC::IsEmpty() const { return key_->empty(); }

const std::string AsymmetricKeyEC::Path() const
{
    String path = "";

    if (path_) {
        if (path_->has_root()) {
            auto root = Identifier::Factory();
            root->SetString(path_->root());
            path.Concatenate(String(root));

            for (auto& it : path_->child()) {
                path.Concatenate(" / ");
                if (it < static_cast<std::uint32_t>(Bip32Child::HARDENED)) {
                    path.Concatenate(String(std::to_string(it)));
                } else {
                    path.Concatenate(String(std::to_string(
                        it -
                        static_cast<std::uint32_t>(Bip32Child::HARDENED))));
                    path.Concatenate("'");
                }
            }
        }
    }

    return path.Get();
}

bool AsymmetricKeyEC::Path(proto::HDPath& output) const
{
    if (path_) {
        output = *path_;

        return true;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": HDPath not instantiated."
          << std::endl;

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
            thePWData.SetOverride(theExportPassword);
            haveClearKey = ECDSA().ImportECPrivatekey(
                *encrypted_key_, thePWData, pClearKey);
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
                thePWData.ClearOverride();
                reencrypted = ECDSA().ECPrivatekeyToAsymmetricKey(
                    pClearKey, thePWData, *const_cast<AsymmetricKeyEC*>(this));
            }

            // Else if we're exporting, that means we just loaded up the Nym
            // from the wallet using the normal password callback, and now we
            // need to save it back again using theExportedPassphrase (for
            // exporting it outside of the wallet.)
            else {
                thePWData.SetOverride(theExportPassword);
                reencrypted = ECDSA().ExportECPrivatekey(
                    pClearKey, thePWData, *const_cast<AsymmetricKeyEC*>(this));
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
    Release_AsymmetricKeyEC();  // My own cleanup is performed here.

    // Next give the base class a chance to do the same...
    ot_super::Release();  // since I've overridden the base class, I call it
                          // now...
}

serializedAsymmetricKey AsymmetricKeyEC::Serialize() const
{
    serializedAsymmetricKey serializedKey = ot_super::Serialize();

    if (IsPrivate()) {
        serializedKey->set_mode(proto::KEYMODE_PRIVATE);

        if (path_) { *(serializedKey->mutable_path()) = *path_; }

        if (chain_code_) { *serializedKey->mutable_chaincode() = *chain_code_; }

        if (encrypted_key_) {
            *serializedKey->mutable_encryptedkey() = *encrypted_key_;
        }
    } else {
        serializedKey->set_mode(proto::KEYMODE_PUBLIC);

        if (false == key_->empty()) {
            serializedKey->set_key(key_->GetPointer(), key_->GetSize());
        }
    }

    return serializedKey;
}

bool AsymmetricKeyEC::SetKey(const Data& key)
{
    ReleaseKeyLowLevel();
    m_bIsPublicKey = true;
    m_bIsPrivateKey = false;
    key_ = key;

    return true;
}

bool AsymmetricKeyEC::SetKey(std::unique_ptr<proto::Ciphertext>& key)
{
    ReleaseKeyLowLevel();
    m_bIsPublicKey = false;
    m_bIsPrivateKey = true;
    encrypted_key_.swap(key);

    return true;
}

bool AsymmetricKeyEC::TransportKey(Data& publicKey, OTPassword& privateKey)
    const
{
    if (!IsPrivate()) { return false; }

    if (!encrypted_key_) { return false; }

    OTPassword seed;
    ECDSA().AsymmetricKeyToECPrivatekey(*this, "Get transport key", seed);

    return ECDSA().SeedToCurveKey(seed, privateKey, publicKey);
}

AsymmetricKeyEC::~AsymmetricKeyEC() {}
}  // namespace opentxs
