// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip39.hpp"

#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

#include "HDSeed.hpp"

#define OT_METHOD "opentxs::api::implementation::HDSeed::"

namespace opentxs
{
api::HDSeed* Factory::HDSeed(
    const api::Factory& factory,
    const api::crypto::Asymmetric& asymmetric,
    const api::crypto::Symmetric& symmetric,
    const api::storage::Storage& storage,
    const crypto::Bip32& bip32,
    const crypto::Bip39& bip39)
{
    return new api::implementation::HDSeed(
        factory, asymmetric, symmetric, storage, bip32, bip39);
}
}  // namespace opentxs

namespace opentxs::api::implementation
{
const proto::SymmetricMode HDSeed::DEFAULT_ENCRYPTION_MODE =
    proto::SMODE_CHACHA20POLY1305;
const std::string HDSeed::DEFAULT_PASSPHRASE = "";

HDSeed::HDSeed(
    const api::Factory& factory,
    const api::crypto::Asymmetric& asymmetric,
    const api::crypto::Symmetric& symmetric,
    const api::storage::Storage& storage,
    const opentxs::crypto::Bip32& bip32,
    const opentxs::crypto::Bip39& bip39)
    : factory_(factory)
    , asymmetric_(asymmetric)
    , symmetric_(symmetric)
    , storage_(storage)
    , bip32_(bip32)
    , bip39_(bip39)
{
}

std::shared_ptr<proto::AsymmetricKey> HDSeed::AccountChildKey(
    const proto::HDPath& rootPath,
    const BIP44Chain internal,
    const Bip32Index index,
    const PasswordPrompt& reason) const
{
    std::string fingerprint{rootPath.root()};
    const Bip32Index change = internal ? 1 : 0;
    Path path{};

    for (const auto& index : rootPath.child()) { path.emplace_back(index); }

    path.emplace_back(change);
    path.emplace_back(index);

    auto key = GetHDKey(fingerprint, EcdsaCurve::SECP256K1, path, reason);

    if (key) { return key->Serialize(); }

    return {};
}

std::string HDSeed::Bip32Root(
    const PasswordPrompt& reason,
    const std::string& fingerprint) const
{
    // TODO: make fingerprint non-const
    std::string input(fingerprint);
    Bip32Index notUsed = 0;
    auto seed = Seed(input, notUsed, reason);

    if (!seed) { return ""; }

    auto start = static_cast<const unsigned char*>(seed->getMemory());
    const auto end = start + seed->getMemorySize();

    std::vector<unsigned char> bytes(start, end);
    std::ostringstream stream;
    stream << std::hex << std::setfill('0');

    for (int byte : bytes) { stream << std::setw(2) << byte; }

    return stream.str();
}

bool HDSeed::DecryptSeed(
    const proto::Seed& seed,
    OTPassword& words,
    OTPassword& phrase,
    const PasswordPrompt& reason) const
{
    if (!proto::Validate(seed, VERBOSE)) { return false; }

    const auto& cwords = seed.words();
    const auto& cphrase = seed.passphrase();

    auto key = symmetric_.Key(cwords.key(), cwords.mode());

    OT_ASSERT(key.get());
    OT_ASSERT(words.isPassword());

    const bool haveWords = key->Decrypt(seed.words(), reason, words);

    if (!haveWords) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt words.")
            .Flush();

        return false;
    }

    OT_ASSERT(phrase.isPassword());

    if (seed.has_passphrase()) {
        const bool havePassphrase = key->Decrypt(cphrase, reason, phrase);

        if (!havePassphrase) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to decrypt passphrase.")
                .Flush();

            return false;
        }
    }

    return true;
}

std::string HDSeed::DefaultSeed() const { return storage_.DefaultSeed(); }

std::unique_ptr<opentxs::crypto::key::HD> HDSeed::GetHDKey(
    std::string& fingerprint,
    const EcdsaCurve& curve,
    const Path& path,
    const PasswordPrompt& reason,
    const proto::KeyRole role,
    const VersionNumber version) const
{
    Bip32Index notUsed{0};
    auto seed = Seed(fingerprint, notUsed, reason);

    if (false == bool(seed)) { return {}; }

    return asymmetric_.NewHDKey(
        fingerprint, *seed, curve, path, reason, role, version);
}

std::shared_ptr<proto::AsymmetricKey> HDSeed::GetPaymentCode(
    std::string& fingerprint,
    const Bip32Index nym,
    const PasswordPrompt& reason) const
{
    auto key = GetHDKey(
        fingerprint,
        EcdsaCurve::SECP256K1,
        {HDIndex{Bip43Purpose::PAYCODE, Bip32Child::HARDENED},
         HDIndex{Bip44Type::BITCOIN, Bip32Child::HARDENED},
         HDIndex{nym, Bip32Child::HARDENED}},
        reason);

    if (key) { return key->Serialize(); }

    return {};
}

OTSymmetricKey HDSeed::GetStorageKey(
    std::string& fingerprint,
    const PasswordPrompt& reason) const
{
    auto pKey = GetHDKey(
        fingerprint,
        EcdsaCurve::SECP256K1,
        {HDIndex{Bip43Purpose::FS, Bip32Child::HARDENED},
         HDIndex{Bip32Child::ENCRYPT_KEY, Bip32Child::HARDENED}},
        reason);

    if (false == bool(pKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive storage key.")
            .Flush();

        return OTSymmetricKey{opentxs::Factory::SymmetricKey()};
    }

    const auto& key = *pKey;
    const auto privateKey = key.PrivateKey(reason);
    OTPassword keySource{};
    keySource.setMemory(privateKey->data(), privateKey->size());

    return symmetric_.Key(keySource);
}

std::string HDSeed::SaveSeed(
    const OTPassword& words,
    const OTPassword& passphrase,
    const PasswordPrompt& reason) const
{
    OT_ASSERT(words.isPassword() && passphrase.isPassword());

    auto seed = factory_.BinarySecret();

    OT_ASSERT(seed);

    bip39_.WordsToSeed(words, *seed, passphrase);

    OT_ASSERT(1 < seed->getMemorySize());

    // the fingerprint is used as the identifier of the seed for indexing
    // purposes. Always use the secp256k1 version for this.
    auto fingerprint = bip32_.SeedToFingerprint(EcdsaCurve::SECP256K1, *seed);
    auto key = symmetric_.Key(reason, DEFAULT_ENCRYPTION_MODE);

    OT_ASSERT(key.get());

    proto::Seed serialized;
    serialized.set_version(2);
    serialized.set_index(0);
    auto& encryptedWords = *serialized.mutable_words();
    auto& encryptedPassphrase = *serialized.mutable_passphrase();
    serialized.set_fingerprint(fingerprint);
    auto empty = Data::Factory();
    const bool haveWords = key->Encrypt(words, empty, reason, encryptedWords);

    if (false == haveWords) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to encrypt seed.").Flush();

        return "";
    }

    bool havePassphrase =
        key->Encrypt(passphrase, empty, reason, encryptedPassphrase, false);

    if (!havePassphrase) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to encrypt passphrase.")
            .Flush();

        return "";
    }

    const bool stored = storage_.Store(serialized, fingerprint);

    if (!stored) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to store seed.").Flush();

        return "";
    }

    return fingerprint;
}

bool HDSeed::SeedToData(
    const OTPassword& words,
    const OTPassword& passphrase,
    OTPassword& output) const
{
    OT_ASSERT(words.isPassword());
    OT_ASSERT(passphrase.isPassword());

    bip39_.WordsToSeed(words, output, passphrase);

    return true;
}

std::string HDSeed::ImportSeed(
    const OTPassword& words,
    const OTPassword& passphrase,
    const PasswordPrompt& reason) const
{
    return SaveSeed(words, passphrase, reason);
}

std::string HDSeed::NewSeed(const PasswordPrompt& reason) const
{
    auto entropy = factory_.BinarySecret();

    OT_ASSERT(entropy);

    entropy->randomizeMemory(256 / 8);
    OTPassword words, passphrase;
    passphrase.setPassword(DEFAULT_PASSPHRASE);

    if (false == bip39_.SeedToWords(*entropy, words)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to convert entropy to word list.")
            .Flush();

        return {};
    }

    return SaveSeed(words, passphrase, reason);
}

std::string HDSeed::Passphrase(
    const PasswordPrompt& reason,
    const std::string& fingerprint) const
{
    // TODO: make fingerprint non-const
    std::string input(fingerprint);
    Bip32Index notUsed = 0;
    auto seed = SerializedSeed(input, notUsed, reason);

    if (!seed) { return ""; }

    OTPassword words, phrase;

    if (!DecryptSeed(*seed, words, phrase, reason)) { return ""; }

    return phrase.getPassword();
}

std::shared_ptr<OTPassword> HDSeed::Seed(
    std::string& fingerprint,
    Bip32Index& index,
    const PasswordPrompt& reason) const
{
    auto output = factory_.BinarySecret();

    OT_ASSERT(output);

    auto serialized = SerializedSeed(fingerprint, index, reason);

    if (serialized) {
        auto seed = factory_.BinarySecret();

        OT_ASSERT(seed);

        OTPassword words, phrase;

        if (false == DecryptSeed(*serialized, words, phrase, reason)) {
            return {};
        }

        bool extracted = SeedToData(words, phrase, *seed);

        if (extracted) {
            output.reset(seed.release());
        } else {
            OT_FAIL;
        }
    }

    return output;
}

std::shared_ptr<proto::Seed> HDSeed::SerializedSeed(
    std::string& fingerprint,
    Bip32Index& index,
    const PasswordPrompt& reason) const
{
    const bool wantDefaultSeed = fingerprint.empty();
    std::shared_ptr<proto::Seed> serialized;
    index = 0;

    if (wantDefaultSeed) {
        std::string defaultFingerprint = storage_.DefaultSeed();
        bool haveDefaultSeed = !defaultFingerprint.empty();

        if (false == haveDefaultSeed) { defaultFingerprint = NewSeed(reason); }

        if (!defaultFingerprint.empty()) {
            serialized = SerializedSeed(defaultFingerprint, index, reason);
        } else {
            OT_FAIL;
        }

        // Update to correct value
        fingerprint = defaultFingerprint;
    } else {  // want an explicitly identified seed
        storage_.Load(fingerprint, serialized);
    }

    if (serialized) { index = serialized->index(); }

    return serialized;
}

bool HDSeed::UpdateIndex(
    std::string& seed,
    const Bip32Index index,
    const PasswordPrompt& reason) const
{
    Bip32Index oldIndex = 0;
    auto serialized = SerializedSeed(seed, oldIndex, reason);

    if (oldIndex > index) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Index values must always increase.")
            .Flush();

        return false;
    }

    serialized->set_index(index);

    if (serialized->version() < 2) { serialized->set_version(2); }

    return storage_.Store(*serialized, seed);
}

std::string HDSeed::Words(
    const PasswordPrompt& reason,
    const std::string& fingerprint) const
{
    // TODO: make fingerprint non-const
    std::string input(fingerprint);
    Bip32Index notUsed;
    auto seed = SerializedSeed(input, notUsed, reason);

    if (!seed) { return ""; }

    OTPassword words, phrase;

    if (!DecryptSeed(*seed, words, phrase, reason)) { return ""; }

    return words.getPassword();
}
}  // namespace opentxs::api::implementation
#endif
