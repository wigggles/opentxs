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

#include "Symmetric.hpp"

#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/CryptoSymmetricNew.hpp"
#include "opentxs/core/crypto/SymmetricKey.hpp"

#include <string>

namespace opentxs::api::crypto::implementation
{
Symmetric::Symmetric(CryptoSymmetricNew& sodium)
    : sodium_(sodium)
{
}

CryptoSymmetricNew* Symmetric::GetEngine(const proto::SymmetricMode mode) const
{
    CryptoSymmetricNew* engine = nullptr;

    // Add support for other crypto engines here
    switch (mode) {
        default: {
            engine = &sodium_;
        }
    }

    return engine;
}

std::unique_ptr<SymmetricKey> Symmetric::Key(
    const OTPasswordData& password,
    const proto::SymmetricMode mode) const
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    return SymmetricKey::Factory(*engine, password, mode);
}

std::unique_ptr<SymmetricKey> Symmetric::Key(
    const proto::SymmetricKey& serialized,
    const proto::SymmetricMode mode) const
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    return SymmetricKey::Factory(*engine, serialized);
}

std::unique_ptr<SymmetricKey> Symmetric::Key(
    const OTPassword& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const proto::SymmetricMode mode,
    const proto::SymmetricKeyType type) const
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    return SymmetricKey::Factory(
        *engine, seed, operations, difficulty, engine->KeySize(mode), type);
}
}  // namespace opentxs::api::crypto::implementation
