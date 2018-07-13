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

#ifndef IMPLEMENTATION_OPENTXS_CRYPTO_LIBRARY_OPENSSL_BIO_HPP
#define IMPLEMENTATION_OPENTXS_CRYPTO_LIBRARY_OPENSSL_BIO_HPP

#include "Internal.hpp"

extern "C" {
#include <openssl/bio.h>
}

namespace opentxs::crypto::implementation
{
class OpenSSL_BIO
{
private:
    BIO& m_refBIO;
    bool bCleanup;
    bool bFreeOnly;

    EXPORT static BIO* assertBioNotNull(BIO* pBIO);

public:
    EXPORT OpenSSL_BIO(BIO* pBIO);

    EXPORT ~OpenSSL_BIO();

    EXPORT operator BIO*() const;

    EXPORT void release();
    EXPORT void setFreeOnly();
};

}  // namespace opentxs

#endif  // IMPLEMENTATION_OPENTXS_CRYPTO_LIBRARY_OPENSSL_BIO_HPP
