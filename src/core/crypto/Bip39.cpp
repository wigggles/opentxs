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
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/core/crypto/Bip39.hpp"

#include "opentxs/core/OTData.hpp"
#include "opentxs/core/app/App.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHash.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/storage/Storage.hpp"

#include <memory>
#include <string>

namespace opentxs
{

const CryptoSymmetric::Mode Bip39::DEFAULT_ENCRYPTION_MODE =
    CryptoSymmetric::AES_256_CBC;
const std::string Bip39::DEFAULT_PASSPHRASE = "opentxs";

bool Bip39::DecryptSeed(proto::Seed& seed) const
{
    if (proto::Check(seed, 0, 0xFFFFFFFF)) {
        auto key = CryptoSymmetric::GetMasterKey("Decrypting a new BIP39 seed");

        OT_ASSERT(key);

        OTData decryptedWords, decryptedPassphrase;

        OTData iv;
        bool haveIV = App::Me().Crypto().Hash().Digest(
            proto::HASHTYPE_SHA256,
            OTData(seed.fingerprint().c_str(), seed.fingerprint().length()),
            iv);

        if (!haveIV) { return false; }

        bool haveWords =
            App::Me().Crypto().AES().Decrypt(
                *key,
                seed.words().c_str(),
                seed.words().length(),
                iv,
                decryptedWords);

        if (1 > decryptedWords.GetSize()) { return false; }

        bool havePassphrase =
            App::Me().Crypto().AES().Decrypt(
                *key,
                seed.passphrase().c_str(),
                seed.passphrase().length(),
                iv,
                decryptedPassphrase);

        if (1 > decryptedPassphrase.GetSize()) { return false; }

        if (haveWords && havePassphrase) {
            const std::string words(
                static_cast<const char*>(decryptedWords.GetPointer()),
                decryptedWords.GetSize());
            const std::string passphrase(
                static_cast<const char*>(decryptedPassphrase.GetPointer()),
                decryptedPassphrase.GetSize());
            seed.set_words(words);
            seed.set_passphrase(passphrase);

            return true;
        }
    }

    return false;
}

std::string Bip39::SaveSeed(
    const std::string& words,
    const std::string& passphrase) const
{
    OT_ASSERT(!words.empty() && !passphrase.empty());

    auto seed = App::Me().Crypto().AES().InstantiateBinarySecretSP();
    WordsToSeed(words, *seed, passphrase);

    OT_ASSERT(1 < seed->getMemorySize());

    // the fingerprint is used as the identifier of the seed for indexing
    // purposes. Always use the secp256k1 version for this.
    auto fingerprint = App::Me().Crypto().BIP32().SeedToFingerprint(
        EcdsaCurve::SECP256K1, *seed);
    auto key = CryptoSymmetric::GetMasterKey("Generating a new BIP39 seed");

    OT_ASSERT(key);

    OTData encryptedWords, encryptedPassphrase, iv;

    bool haveIV = App::Me().Crypto().Hash().Digest(
        proto::HASHTYPE_SHA256,
        OTData(fingerprint.c_str(), fingerprint.length()),
        iv);

    if (haveIV) {
        bool haveWords =
            App::Me().Crypto().AES().Encrypt(
                *key,
                words.c_str(),
                words.length(),
                iv,
                encryptedWords);

        OT_ASSERT(haveWords);

        bool havePassphrase =
            App::Me().Crypto().AES().Encrypt(
                *key,
                passphrase.c_str(),
                passphrase.length(),
                iv,
                encryptedPassphrase);

        OT_ASSERT(havePassphrase);

        if (haveWords && havePassphrase) {
            proto::Seed serialized;
            serialized.set_version(1);
            serialized.set_words(
                static_cast<const char*>(encryptedWords.GetPointer()),
                encryptedWords.GetSize());
            serialized.set_passphrase(
                static_cast<const char*>(encryptedPassphrase.GetPointer()),
                encryptedPassphrase.GetSize());
            serialized.set_fingerprint(fingerprint);

            bool stored = App::Me().DB().Store(serialized, fingerprint);

            if (stored) {

                return fingerprint;
            }
        }
    }

    return "";
}

bool Bip39::SeedToData(const proto::Seed& seed, OTPassword& output) const
{
    WordsToSeed(
        seed.words(),
        output,
        seed.passphrase());

    return true;
}

std::string Bip39::ImportSeed(
    const std::string& words,
    const std::string& passphrase) const
{
    return SaveSeed(words, passphrase);
}

std::string Bip39::NewSeed() const
{
    auto entropy = App::Me().Crypto().AES().InstantiateBinarySecretSP();

    if (entropy) {
        entropy->randomizeMemory(256/8);
        std::string words = toWords(*entropy);

        return SaveSeed(words, Bip39::DEFAULT_PASSPHRASE);
    }

    return "";
}

std::string Bip39::Passphrase(const std::string& fingerprint) const
{
    auto seed = SerializedSeed(fingerprint);

    if (!seed) { return ""; }

    return seed->passphrase();
}

std::shared_ptr<OTPassword> Bip39::Seed(const std::string& fingerprint) const
{
    auto output = App::Me().Crypto().AES().InstantiateBinarySecretSP();

    OT_ASSERT(output);

        auto serialized = SerializedSeed(fingerprint);

        if (serialized) {
            std::unique_ptr<OTPassword>
                seed(App::Me().Crypto().AES().InstantiateBinarySecret());

            OT_ASSERT(seed);

            bool extracted = SeedToData(*serialized, *seed);

            if (extracted) {
                output.reset(seed.release());
            }
        }

    return output;
}

std::shared_ptr<proto::Seed> Bip39::SerializedSeed(
    const std::string& fingerprint) const
{
    const bool wantDefaultSeed = fingerprint.empty();
    std::shared_ptr<proto::Seed> serialized;

    if (wantDefaultSeed) {
        std::string defaultFingerprint = App::Me().DB().DefaultSeed();
        bool haveDefaultSeed = !defaultFingerprint.empty();

        if (!haveDefaultSeed) {
            defaultFingerprint = NewSeed();
        }

        if (!defaultFingerprint.empty()) {
            serialized = SerializedSeed(defaultFingerprint);
        }

    } else { // want an explicitly identified seed
        const bool loaded = App::Me().DB().Load(fingerprint, serialized);

        if (loaded && serialized) {
            DecryptSeed(*serialized);
        }
    }

    return serialized;
}

std::string Bip39::Words(const std::string& fingerprint) const
{
    auto seed = SerializedSeed(fingerprint);

    if (!seed) { return ""; }

    return seed->words();
}
} // namespace opentxs
#endif
