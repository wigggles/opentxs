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

#ifndef OPENTXS_CORE_CRYPTO_CRYPTOSYMMETRICENGINE_HPP
#define OPENTXS_CORE_CRYPTO_CRYPTOSYMMETRICENGINE_HPP

#include "opentxs/Version.hpp"

#include "opentxs/api/crypto/Symmetric.hpp"

namespace opentxs
{

class CryptoSymmetricNew;

namespace api
{
namespace implementation
{
class Crypto;
}  // namespace implementation

namespace crypto
{
namespace implementation
{

class Symmetric : virtual public api::crypto::Symmetric
{
public:
    std::unique_ptr<SymmetricKey> Key(
        const OTPasswordData& password,
        const proto::SymmetricMode mode =
            proto::SMODE_CHACHA20POLY1305) const override;
    std::unique_ptr<SymmetricKey> Key(
        const proto::SymmetricKey& serialized,
        const proto::SymmetricMode mode) const override;
    std::unique_ptr<SymmetricKey> Key(
        const OTPassword& seed,
        const std::uint64_t operations = 0,
        const std::uint64_t difficulty = 0,
        const proto::SymmetricMode mode = proto::SMODE_CHACHA20POLY1305,
        const proto::SymmetricKeyType type =
            proto::SKEYTYPE_ARGON2) const override;

    ~Symmetric() = default;

private:
    friend class api::implementation::Crypto;

    CryptoSymmetricNew& sodium_;

    CryptoSymmetricNew* GetEngine(const proto::SymmetricMode mode) const;

    Symmetric(CryptoSymmetricNew& sodium);
    Symmetric() = delete;
    Symmetric(const Symmetric&) = delete;
    Symmetric(Symmetric&&) = delete;
    Symmetric& operator=(const Symmetric&) = delete;
    Symmetric& operator=(Symmetric&&) = delete;
};
}  // namespace implementation
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_CORE_CRYPTO_CRYPTOSYMMETRICENGINE_HPP
