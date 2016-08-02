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

#include "opentxs/core/Proto.hpp"

#include <cstddef>
#include <cstdint>

namespace opentxs
{

class CryptoEngine;
class CryptoSymmetricNew;
class SymmetricKey;
class OTPassword;
class OTPasswordData;

// Singlton class for providing an interface to symmetric key methods
class CryptoSymmetricEngine
{
private:
    friend class CryptoEngine;

    CryptoSymmetricNew& sodium_;

    CryptoSymmetricNew* GetEngine(const proto::SymmetricMode mode);

    CryptoSymmetricEngine(CryptoEngine& parent);
    CryptoSymmetricEngine(const CryptoSymmetricEngine&) = delete;
    CryptoSymmetricEngine& operator=(const CryptoSymmetricEngine&) = delete;

public:
    std::unique_ptr<SymmetricKey> Key(
        const OTPasswordData& password,
        const proto::SymmetricMode mode = proto::SMODE_CHACHA20POLY1305);
    std::unique_ptr<SymmetricKey> Key(
        const proto::SymmetricKey serialized,
        const proto::SymmetricMode mode);
    std::unique_ptr<SymmetricKey> Key(
        const OTPassword& seed,
        const std::uint64_t operations = 0,
        const std::uint64_t difficulty = 0,
        const std::size_t size = 0,
        const proto::SymmetricMode mode = proto::SMODE_CHACHA20POLY1305,
        const proto::SymmetricKeyType type = proto::SKEYTYPE_ARGON2);

    ~CryptoSymmetricEngine() = default;
};
}  // namespace opentxs
#endif // OPENTXS_CORE_CRYPTO_CRYPTOSYMMETRICENGINE_HPP
