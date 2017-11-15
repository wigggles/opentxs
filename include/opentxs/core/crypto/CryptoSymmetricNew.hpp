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

#ifndef OPENTXS_CORE_CRYPTO_CRYPTOSYMMETRICNEW_HPP
#define OPENTXS_CORE_CRYPTO_CRYPTOSYMMETRICNEW_HPP

#include "opentxs/Version.hpp"

#include "opentxs/Proto.hpp"

#include <cstddef>

namespace opentxs
{
namespace api
{
namespace crypto
{
namespace implementation
{
class Symmetric;
}  // namespace implementation
}  // namespace crypto
}  // namespace api

class SymmetricKey;

class CryptoSymmetricNew
{
    friend class SymmetricKey;
    friend class api::crypto::implementation::Symmetric;

protected:
    virtual bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const std::uint8_t* key,
        const std::size_t keySize,
        std::uint8_t* plaintext) const = 0;
    virtual proto::SymmetricMode DefaultMode() const = 0;
    virtual bool Derive(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* salt,
        const std::size_t saltSize,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const proto::SymmetricKeyType type,
        std::uint8_t* output,
        std::size_t outputSize) const = 0;
    virtual bool Encrypt(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* key,
        const std::size_t keySize,
        proto::Ciphertext& ciphertext) const = 0;
    virtual std::size_t KeySize(const proto::SymmetricMode mode) const = 0;
    virtual std::size_t IvSize(const proto::SymmetricMode mode) const = 0;
    virtual std::size_t SaltSize(const proto::SymmetricKeyType type) const = 0;
    virtual std::size_t TagSize(const proto::SymmetricMode mode) const = 0;

    CryptoSymmetricNew() = default;
    ~CryptoSymmetricNew() = default;
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_CRYPTO_CRYPTOSYMMETRICNEW_HPP
