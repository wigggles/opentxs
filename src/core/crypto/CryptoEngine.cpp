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

namespace opentxs
{

CryptoEngine* CryptoEngine::pInstance_ = nullptr;

CryptoEngine::CryptoEngine()
    : pSSL_(new SSLImplementation)
{
    Init();
}

void CryptoEngine::Init()
{
    pSSL_->Init();

}

CryptoUtil& CryptoEngine::Util()
{
    OT_ASSERT(nullptr != pSSL_);

    return *pSSL_;
}

#ifdef OT_CRYPTO_SUPPORTED_KEY_RSA
CryptoAsymmetric& CryptoEngine::RSA()
{
    OT_ASSERT(nullptr != pSSL_);

    return *pSSL_;
}

#endif
#ifdef OT_CRYPTO_SUPPORTED_KEY_RSA
CryptoSymmetric& CryptoEngine::AES()
{
    OT_ASSERT(nullptr != pSSL_);

    return *pSSL_;
}
#endif
CryptoEngine& CryptoEngine::Instance()
{
    if (nullptr == pInstance_)
    {
        pInstance_ = new CryptoEngine;
    }

    return *pInstance_;
}

void CryptoEngine::Cleanup()
{
    pSSL_->Cleanup();
}

CryptoEngine::~CryptoEngine()
{
    Cleanup();
}

} // namespace opentxs
