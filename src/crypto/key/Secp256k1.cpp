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

#include "Internal.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/crypto/library/Secp256k1.hpp"
#endif
#include "opentxs/OT.hpp"

#include "Secp256k1.hpp"

namespace opentxs
{
crypto::key::Secp256k1* Factory::Secp256k1Key(const proto::AsymmetricKey& input)
{
    return new crypto::key::implementation::Secp256k1(input);
}

crypto::key::Secp256k1* Factory::Secp256k1Key(const String& input)
{
    return new crypto::key::implementation::Secp256k1(input);
}

crypto::key::Secp256k1* Factory::Secp256k1Key(const proto::KeyRole input)
{
    return new crypto::key::implementation::Secp256k1(input);
}
}  // namespace opentxs

namespace opentxs::crypto::key::implementation
{
Secp256k1::Secp256k1()
    : ot_super(proto::AKEYTYPE_SECP256K1, proto::KEYROLE_ERROR)
{
}

Secp256k1::Secp256k1(const proto::KeyRole role)
    : ot_super(proto::AKEYTYPE_SECP256K1, role)
{
}

Secp256k1::Secp256k1(const proto::AsymmetricKey& serializedKey)
    : ot_super(serializedKey)
{
}

Secp256k1::Secp256k1(const String& publicKey)
    : ot_super(proto::AKEYTYPE_SECP256K1, publicKey)
{
}

const crypto::EcdsaProvider& Secp256k1::ECDSA() const
{
    return dynamic_cast<const crypto::EcdsaProvider&>(engine());
}

const crypto::AsymmetricProvider& Secp256k1::engine() const

{
    return OT::App().Crypto().SECP256K1();
}
}  // namespace opentxs::crypto::key::implementation
#endif
