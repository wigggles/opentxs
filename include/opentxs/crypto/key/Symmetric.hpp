// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_KEY_SYMMETRIC_HPP
#define OPENTXS_CRYPTO_KEY_SYMMETRIC_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#include <cstdint>
#include <string>

namespace opentxs
{
namespace crypto
{
namespace key
{
class Symmetric
{
public:
    /** Generate a blank, invalid key */
    EXPORT static OTSymmetricKey Factory();

    /** Derive a new, random symmetric key
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] keyPassword Optional key password information.
     *  \param[in] mode The symmetric algorithm for which to generate an
     *                  appropriate key
     */
    EXPORT static OTSymmetricKey Factory(
        const crypto::SymmetricProvider& engine,
        const OTPasswordData& password,
        const proto::SymmetricMode mode = proto::SMODE_ERROR);

    /** Instantiate a symmetric key from serialized form
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] serialized The symmetric key in protobuf form
     */
    EXPORT static OTSymmetricKey Factory(
        const crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized);

    /** Derive a symmetric key from a seed
     *
     *  \param[in] seed A binary or text seed to be expanded into a secret key
     *  \param[in] operations The number of iterations/operations the KDF should
     *                        perform
     *  \param[in] difficulty A type-specific difficulty parameter used by the
     *                        KDF.
     *  \param[in] size       The target number of bytes for the derived secret
     *                        key
     *  \param[in] type       The KDF to be used for the derivation process
     */
    EXPORT static OTSymmetricKey Factory(
        const crypto::SymmetricProvider& engine,
        const OTPassword& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const proto::SymmetricKeyType type);

    /** Construct a symmetric key from an existing OTPassword
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] raw An existing, unencrypted binary or text secret
     */
    EXPORT static OTSymmetricKey Factory(
        const crypto::SymmetricProvider& engine,
        const OTPassword& raw);

    EXPORT virtual bool ChangePassword(
        const OTPasswordData& oldPassword,
        const OTPassword& newPassword) = 0;

    /** Decrypt ciphertext using the symmetric key
     *
     *  \param[in] ciphertext The data to be decrypted
     *  \param[in] keyPassword The password needed to decrypt the key
     *  \param[out] plaintext The encrypted output
     */
    EXPORT virtual bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const OTPasswordData& keyPassword,
        std::string& plaintext) = 0;
    EXPORT virtual bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const OTPasswordData& keyPassword,
        String& plaintext) = 0;
    EXPORT virtual bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const OTPasswordData& keyPassword,
        Data& plaintext) = 0;
    EXPORT virtual bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const OTPasswordData& keyPassword,
        OTPassword& plaintext) = 0;

    /** Encrypt plaintext using the symmetric key
     *
     *  \param[in] plaintext The data to be encrypted
     *  \param[in] iv Nonce for the encryption operation
     *  \param[in] keyPassword The password needed to decrypt the key
     *  \param[out] ciphertext The encrypted output
     *  \param[in] attachKey Set to true if the serialized key should be
     *                       embedded in the ciphertext
     *  \param[in] mode The symmetric algorithm to use for encryption
     */
    EXPORT virtual bool Encrypt(
        const std::string& plaintext,
        const Data& iv,
        const OTPasswordData& keyPassword,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) = 0;
    EXPORT virtual bool Encrypt(
        const String& plaintext,
        const Data& iv,
        const OTPasswordData& keyPassword,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) = 0;
    EXPORT virtual bool Encrypt(
        const OTPassword& plaintext,
        const Data& iv,
        const OTPasswordData& keyPassword,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) = 0;
    EXPORT virtual bool Encrypt(
        const Data& plaintext,
        const Data& iv,
        const OTPasswordData& keyPassword,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) = 0;

    EXPORT virtual OTIdentifier ID() = 0;

    EXPORT virtual bool Serialize(proto::SymmetricKey& output) const = 0;

    EXPORT virtual bool Unlock(const OTPasswordData& keyPassword) = 0;

    EXPORT virtual operator bool() const = 0;

    EXPORT virtual ~Symmetric() = default;

protected:
    Symmetric() = default;

private:
    friend OTSymmetricKey;

    virtual Symmetric* clone() const = 0;

    Symmetric(const Symmetric&) = delete;
    Symmetric(Symmetric&&) = delete;
    Symmetric& operator=(const Symmetric&) = delete;
    Symmetric& operator=(Symmetric&&) = delete;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif
