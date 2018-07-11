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

#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/LegacySymmetricProvider.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"

#include <string>

#include "Symmetric.hpp"

namespace opentxs
{
api::crypto::Symmetric* Factory::Symmetric(crypto::SymmetricProvider& sodium)
{
    return new api::crypto::implementation::Symmetric(sodium);
}
}  // namespace opentxs

namespace opentxs::api::crypto::implementation
{
Symmetric::Symmetric(opentxs::crypto::SymmetricProvider& sodium)
    : sodium_(sodium)
{
}

opentxs::crypto::SymmetricProvider* Symmetric::GetEngine(
    const proto::SymmetricMode mode) const
{
    opentxs::crypto::SymmetricProvider* engine = nullptr;

    // Add support for other crypto engines here
    switch (mode) {
        default: {
            engine = &sodium_;
        }
    }

    return engine;
}

std::unique_ptr<opentxs::crypto::key::Symmetric> Symmetric::Key(
    const OTPasswordData& password,
    const proto::SymmetricMode mode) const
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    return opentxs::crypto::key::Symmetric::Factory(*engine, password, mode);
}

std::unique_ptr<opentxs::crypto::key::Symmetric> Symmetric::Key(
    const proto::SymmetricKey& serialized,
    const proto::SymmetricMode mode) const
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    return opentxs::crypto::key::Symmetric::Factory(*engine, serialized);
}

std::unique_ptr<opentxs::crypto::key::Symmetric> Symmetric::Key(
    const OTPassword& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const proto::SymmetricMode mode,
    const proto::SymmetricKeyType type) const
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    return opentxs::crypto::key::Symmetric::Factory(
        *engine, seed, operations, difficulty, engine->KeySize(mode), type);
}
}  // namespace opentxs::api::crypto::implementation
