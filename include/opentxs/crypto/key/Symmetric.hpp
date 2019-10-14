// Copyright (c) 2010-2019 The Open-Transactions developers
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
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace crypto
{
namespace key
{
class Symmetric
{
public:
    /** Generate a blank, invalid key */
    EXPORT static OTSymmetricKey Factory();

    EXPORT virtual operator bool() const = 0;

    EXPORT virtual const api::Core& api() const = 0;

    /** Decrypt ciphertext using the symmetric key
     *
     *  \param[in] ciphertext The data to be decrypted
     *  \param[out] plaintext The encrypted output
     */
    EXPORT virtual bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        std::string& plaintext) const = 0;
    EXPORT virtual bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        String& plaintext) const = 0;
    EXPORT virtual bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        Data& plaintext) const = 0;
    EXPORT virtual bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        OTPassword& plaintext) const = 0;
    /** Encrypt plaintext using the symmetric key
     *
     *  \param[in] plaintext The data to be encrypted
     *  \param[in] iv Nonce for the encryption operation
     *  \param[out] ciphertext The encrypted output
     *  \param[in] attachKey Set to true if the serialized key should be
     *                       embedded in the ciphertext
     *  \param[in] mode The symmetric algorithm to use for encryption
     */
    EXPORT virtual bool Encrypt(
        const std::string& plaintext,
        const Data& iv,
        const PasswordPrompt& reason,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) const = 0;
    EXPORT virtual bool Encrypt(
        const String& plaintext,
        const Data& iv,
        const PasswordPrompt& reason,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) const = 0;
    EXPORT virtual bool Encrypt(
        const OTPassword& plaintext,
        const Data& iv,
        const PasswordPrompt& reason,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) const = 0;
    EXPORT virtual bool Encrypt(
        const Data& plaintext,
        const Data& iv,
        const PasswordPrompt& reason,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) const = 0;
    EXPORT virtual OTIdentifier ID(const PasswordPrompt& reason) const = 0;
    EXPORT virtual bool RawKey(const PasswordPrompt& reason, OTPassword& output)
        const = 0;
    EXPORT virtual bool Serialize(proto::SymmetricKey& output) const = 0;
    EXPORT virtual bool Unlock(const PasswordPrompt& reason) const = 0;

    EXPORT virtual bool ChangePassword(
        const PasswordPrompt& reason,
        const OTPassword& newPassword) = 0;

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
