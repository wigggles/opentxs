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

#ifndef OPENTXS_CORE_CRYPTO_LIBSECP256K1_HPP
#define OPENTXS_CORE_CRYPTO_LIBSECP256K1_HPP

#include <opentxs/core/crypto/Crypto.hpp>

extern "C" {
#include "secp256k1.h"
}

namespace opentxs
{

class Libsecp256k1 : public Crypto
{
    friend class CryptoEngine;

private:
    secp256k1_context_t* context_ = nullptr;

protected:
    Libsecp256k1();
    void Init_Override() const;
    void Cleanup_Override() const;

public:
    ~Libsecp256k1();
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTCRYPTO_HPP
