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

#include <opentxs/core/crypto/CryptoEngine.hpp>
#include <opentxs/core/crypto/OTCrypto.hpp>

namespace opentxs
{

OTCrypto* CryptoEngine::Util()
{
  return RSA();
}

OTCrypto* CryptoEngine::RSA()
{
  static RSAImplementation s_theSingleton; // For now we're only allowing a single
                                           // instance.
  return &s_theSingleton;
}

OTCrypto* CryptoEngine::AES()
{
  return RSA();
}

void CryptoEngine::Init()
{
    RSA()->Init();
}

void CryptoEngine::Cleanup()
{
    RSA()->Cleanup();
}

} // namespace opentxs
