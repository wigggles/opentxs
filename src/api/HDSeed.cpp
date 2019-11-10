// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_WITH_BIP32
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
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
const OTPassword HDSeed::binary_secret_{};
const OTPassword HDSeed::text_secret_{};

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
    const_cast<OTPassword&>(binary_secret_).randomizeMemory(0);
    const_cast<OTPassword&>(text_secret_).randomizePassword(0);

    OT_ASSERT(binary_secret_.isMemory());
    OT_ASSERT(text_secret_.isPassword());
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
std::unique_ptr<opentxs::crypto::key::HD> HDSeed::AccountChildKey(
    const proto::HDPath& rootPath,
    const BIP44Chain internal,
    const Bip32Index index,
    const PasswordPrompt& reason) const
{
    std::string fingerprint{rootPath.root()};
    const Bip32Index change = internal ? 1 : 0;
    Path path{};

    for (const auto& child : rootPath.child()) { path.emplace_back(child); }

    path.emplace_back(change);
    path.emplace_back(index);

    return GetHDKey(fingerprint, EcdsaCurve::secp256k1, path, reason);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

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

bool HDSeed::decrypt_seed(
    const proto::Seed& seed,
    OTPassword& words,
    OTPassword& phrase,
    OTPassword& raw,
    const PasswordPrompt& reason) const
{
    if (false == proto::Validate(seed, VERBOSE)) { return false; }

    const auto& cwords = seed.words();
    const auto& cphrase = seed.passphrase();
    const auto& craw = seed.raw();
    const auto& session = (3 > seed.version()) ? cwords : craw;
    auto key = symmetric_.Key(session.key(), session.mode());
    bool decrypted{false};

    OT_ASSERT(key.get());
    OT_ASSERT(words.isPassword());
    OT_ASSERT(phrase.isPassword());
    OT_ASSERT(raw.isMemory());

    if (seed.has_words()) {
        decrypted = key->Decrypt(cwords, reason, words);

        if (false == decrypted) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt words.")
                .Flush();

            return false;
        }
    }

    if (seed.has_passphrase()) {
        decrypted = key->Decrypt(cphrase, reason, phrase);

        if (false == decrypted) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to decrypt passphrase.")
                .Flush();

            return false;
        }
    }

    if (seed.has_raw()) {
        decrypted = key->Decrypt(craw, reason, raw);

        if (false == decrypted) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt entropy.")
                .Flush();

            return false;
        }
    }

    return true;
}

std::string HDSeed::DefaultSeed() const { return storage_.DefaultSeed(); }

#if OT_CRYPTO_SUPPORTED_KEY_HD
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
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
std::shared_ptr<proto::AsymmetricKey> HDSeed::GetPaymentCode(
    std::string& fingerprint,
    const Bip32Index nym,
    const PasswordPrompt& reason) const
{
    auto key = GetHDKey(
        fingerprint,
        EcdsaCurve::secp256k1,
        {HDIndex{Bip43Purpose::PAYCODE, Bip32Child::HARDENED},
         HDIndex{Bip44Type::BITCOIN, Bip32Child::HARDENED},
         HDIndex{nym, Bip32Child::HARDENED}},
        reason);

    if (key) { return key->Serialize(); }

    return {};
}
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

OTSymmetricKey HDSeed::GetStorageKey(
    std::string& fingerprint,
    const PasswordPrompt& reason) const
{
    auto pKey = GetHDKey(
        fingerprint,
        EcdsaCurve::secp256k1,
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

std::string HDSeed::ImportRaw(
    const OTPassword& entropy,
    const PasswordPrompt& reason) const
{
    if (false == entropy.isMemory()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid entropy format").Flush();

        return {};
    }

    if (16 > entropy.getMemorySize()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Entropy too short (")(
            entropy.getMemorySize())(")")
            .Flush();

        return {};
    }

    if (64 < entropy.getMemorySize()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Entropy too long (")(
            entropy.getMemorySize())(")")
            .Flush();

        return {};
    }

    return save_seed(text_secret_, text_secret_, entropy, reason);
}

std::string HDSeed::ImportSeed(
    const OTPassword& words,
    const OTPassword& passphrase,
    const PasswordPrompt& reason) const
{
    return save_seed(words, passphrase, binary_secret_, reason);
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

    return save_seed(words, passphrase, binary_secret_, reason);
}

std::string HDSeed::Passphrase(
    const PasswordPrompt& reason,
    const std::string& fingerprint) const
{
    // TODO: make fingerprint non-const
    std::string input(fingerprint);
    Bip32Index notUsed = 0;
    auto seed = serialized_seed(input, notUsed, reason);

    if (false == bool(seed)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load seed (")(
            fingerprint)(")")
            .Flush();

        return {};
    }

    OTPassword words{text_secret_}, phrase{text_secret_}, raw{binary_secret_};

    if (!decrypt_seed(*seed, words, phrase, raw, reason)) { return ""; }

    return phrase.getPassword();
}

std::string HDSeed::save_seed(
    const OTPassword& words,
    const OTPassword& passphrase,
    const OTPassword& raw,
    const PasswordPrompt& reason) const
{
    OT_ASSERT(words.isPassword());
    OT_ASSERT(passphrase.isPassword());
    OT_ASSERT(raw.isMemory());

    auto seed{binary_secret_};
    const bool haveRaw = (0 < raw.getMemorySize());
    const bool haveWords = (0 < words.getPasswordSize());
    const bool havePhrase = (0 < passphrase.getPasswordSize());

    if (haveRaw) {
        seed = raw;
    } else {
        bip39_.WordsToSeed(words, seed, passphrase);
    }

    OT_ASSERT(0 < seed.getMemorySize());

    // the fingerprint is used as the identifier of the seed for indexing
    // purposes. Always use the secp256k1 version for this.
    auto fingerprint = bip32_.SeedToFingerprint(EcdsaCurve::secp256k1, seed);
    auto key = symmetric_.Key(reason, DEFAULT_ENCRYPTION_MODE);

    OT_ASSERT(key.get());

    proto::Seed serialized;
    serialized.set_version(DefaultVersion);
    serialized.set_index(0);
    serialized.set_fingerprint(fingerprint);
    auto empty = Data::Factory();
    bool encrypted{false};
    auto& encryptedRaw = *serialized.mutable_raw();
    encrypted = key->Encrypt(seed, empty, reason, encryptedRaw);

    if (false == encrypted) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to encrypt entropy.")
            .Flush();

        return "";
    }

    if (haveWords) {
        auto& encryptedWords = *serialized.mutable_words();
        encrypted = key->Encrypt(words, empty, reason, encryptedWords, false);

        if (false == encrypted) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to encrypt seed.")
                .Flush();

            return "";
        }
    }

    if (havePhrase) {
        auto& encryptedPassphrase = *serialized.mutable_passphrase();
        encrypted =
            key->Encrypt(passphrase, empty, reason, encryptedPassphrase, false);

        if (false == encrypted) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to encrypt passphrase.")
                .Flush();

            return "";
        }
    }

    const bool stored = storage_.Store(serialized, fingerprint);

    if (false == stored) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to store seed.").Flush();

        return "";
    }

    return fingerprint;
}

std::shared_ptr<OTPassword> HDSeed::Seed(
    std::string& fingerprint,
    Bip32Index& index,
    const PasswordPrompt& reason) const
{
    auto output = factory_.BinarySecret();

    OT_ASSERT(output);

    auto serialized = serialized_seed(fingerprint, index, reason);

    if (false == bool(serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load seed (")(
            fingerprint)(")")
            .Flush();

        return std::move(output);
    }

    auto seed = factory_.BinarySecret();

    OT_ASSERT(seed);

    OTPassword words{text_secret_}, phrase{text_secret_}, raw{binary_secret_};

    if (false == decrypt_seed(*serialized, words, phrase, raw, reason)) {
        return {};
    }

    bool extracted = seed_to_data(words, phrase, raw, *seed);

    if (extracted) { output.reset(seed.release()); }

    return std::move(output);
}

bool HDSeed::seed_to_data(
    const OTPassword& words,
    const OTPassword& passphrase,
    const OTPassword& raw,
    OTPassword& output) const
{
    OT_ASSERT(words.isPassword());
    OT_ASSERT(passphrase.isPassword());
    OT_ASSERT(raw.isMemory());

    if (0 < raw.getMemorySize()) {
        output = raw;
    } else {
        bip39_.WordsToSeed(words, output, passphrase);
    }

    return true;
}

std::shared_ptr<proto::Seed> HDSeed::serialized_seed(
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

        if (false == defaultFingerprint.empty()) {
            serialized = serialized_seed(defaultFingerprint, index, reason);
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
    auto serialized = serialized_seed(seed, oldIndex, reason);

    if (false == bool(serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load seed (")(seed)(")")
            .Flush();

        return false;
    }

    if (oldIndex > index) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Index values must always increase.")
            .Flush();

        return false;
    }

    serialized->set_index(index);

    if (DefaultVersion > serialized->version()) {
        serialized->set_version(DefaultVersion);
    }

    return storage_.Store(*serialized, seed);
}

std::string HDSeed::Words(
    const PasswordPrompt& reason,
    const std::string& fingerprint) const
{
    // TODO: make fingerprint non-const
    std::string input(fingerprint);
    Bip32Index notUsed;
    auto seed = serialized_seed(input, notUsed, reason);

    if (false == bool(seed)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load seed (")(
            fingerprint)(")")
            .Flush();

        return {};
    }

    OTPassword words{text_secret_}, phrase{text_secret_}, raw{binary_secret_};

    if (!decrypt_seed(*seed, words, phrase, raw, reason)) { return ""; }

    return words.getPassword();
}
}  // namespace opentxs::api::implementation
#endif  // OT_CRYPTO_WITH_BIP32
