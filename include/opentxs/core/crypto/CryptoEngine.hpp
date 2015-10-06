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

#ifndef OPENTXS_CORE_CRYPTO_CRYPTOENGINE_HPP
#define OPENTXS_CORE_CRYPTO_CRYPTOENGINE_HPP

#ifdef OT_CRYPTO_USING_OPENSSL
#include <opentxs/core/crypto/OTCryptoOpenSSL.hpp>
#else // Apparently NO crypto engine is defined!
// Perhaps error out here...
#endif

namespace opentxs
{
class OTCrypto;

// Choose your OTCrypto implementation here.
#ifdef OT_CRYPTO_USING_OPENSSL
typedef OTCrypto_OpenSSL RSAImplementation;
#else // Apparently NO crypto engine is defined!
// Perhaps error out here...
#endif

class CryptoEngine
{
public:
    EXPORT static OTCrypto* Util();
    //Asymmetric encryption engines
    EXPORT static OTCrypto* RSA();
    //Symmetric encryption engines
    EXPORT static OTCrypto* AES();

    EXPORT static void Init();
    EXPORT static void Cleanup();
};

}  // namespace opentxs
#endif // OPENTXS_CORE_CRYPTO_CRYPTOENGINE_HPP
