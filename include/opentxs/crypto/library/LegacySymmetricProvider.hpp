// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_LIBRARY_LEGACYSYMMETRICPROVIDER_HPP
#define OPENTXS_CRYPTO_LIBRARY_LEGACYSYMMETRICPROVIDER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/String.hpp"

#include <memory>
#include <mutex>
#include <tuple>

namespace opentxs
{
namespace crypto
{
using BinarySecret = std::shared_ptr<OTPassword>;

class LegacySymmetricProvider
{
public:
    enum Mode : std::int32_t {
        ERROR_MODE,
        AES_128_CBC,
        AES_256_CBC,
        AES_256_ECB,
        AES_128_GCM,
        AES_256_GCM
    };

    EXPORT static std::uint32_t IVSize(const Mode Mode);
    EXPORT static std::uint32_t KeySize(const Mode Mode);
    EXPORT static OTString ModeToString(const Mode Mode);
    EXPORT static Mode StringToMode(const String& Mode);
    EXPORT static std::uint32_t TagSize(const Mode Mode);

    EXPORT virtual bool Decrypt(
        const OTPassword& theRawSymmetricKey,
        const char* szInput,
        std::uint32_t lInputLength,
        const Data& theIV,
        CryptoSymmetricDecryptOutput& theDecryptedOutput) const = 0;
    EXPORT virtual bool Decrypt(
        const LegacySymmetricProvider::Mode cipher,
        const OTPassword& key,
        const char* ciphertext,
        std::uint32_t ciphertextLength,
        CryptoSymmetricDecryptOutput& plaintext) const = 0;
    EXPORT virtual bool Decrypt(
        const LegacySymmetricProvider::Mode cipher,
        const OTPassword& key,
        const Data& iv,
        const char* ciphertext,
        std::uint32_t ciphertextLength,
        CryptoSymmetricDecryptOutput& plaintext) const = 0;
    EXPORT virtual bool Decrypt(
        const LegacySymmetricProvider::Mode cipher,
        const OTPassword& key,
        const Data& iv,
        const Data& tag,
        const char* ciphertext,
        const std::uint32_t ciphertextLength,
        CryptoSymmetricDecryptOutput& plaintext) const = 0;
    EXPORT virtual OTPassword* DeriveNewKey(
        const OTPassword& userPassword,
        const Data& dataSalt,
        std::uint32_t uIterations,
        Data& dataCheckHash) const = 0;
    EXPORT virtual bool Encrypt(
        const OTPassword& theRawSymmetricKey,
        const char* szInput,
        std::uint32_t lInputLength,
        const Data& theIV,
        Data& theEncryptedOutput) const = 0;
    EXPORT virtual bool Encrypt(
        const LegacySymmetricProvider::Mode cipher,
        const OTPassword& key,
        const char* plaintext,
        std::uint32_t plaintextLength,
        Data& ciphertext) const = 0;
    EXPORT virtual bool Encrypt(
        const LegacySymmetricProvider::Mode cipher,
        const OTPassword& key,
        const Data& iv,
        const char* plaintext,
        std::uint32_t plaintextLength,
        Data& ciphertext) const = 0;
    EXPORT virtual bool Encrypt(
        const LegacySymmetricProvider::Mode cipher,
        const OTPassword& key,
        const Data& iv,
        const char* plaintext,
        std::uint32_t plaintextLength,
        Data& ciphertext,
        Data& tag) const = 0;
    EXPORT virtual OTPassword* InstantiateBinarySecret() const = 0;
    EXPORT virtual BinarySecret InstantiateBinarySecretSP() const = 0;

    EXPORT virtual ~LegacySymmetricProvider() = default;

protected:
    LegacySymmetricProvider() = default;

private:
    LegacySymmetricProvider(const LegacySymmetricProvider&) = delete;
    LegacySymmetricProvider(LegacySymmetricProvider&&) = delete;
    LegacySymmetricProvider& operator=(const LegacySymmetricProvider&) = delete;
    LegacySymmetricProvider& operator=(LegacySymmetricProvider&&) = delete;
};
}  // namespace crypto
}  // namespace opentxs
#endif
