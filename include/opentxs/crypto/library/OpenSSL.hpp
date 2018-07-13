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

#ifndef OPENTXS_CRYPTO_LIBRARY_OPENSSL_HPP
#define OPENTXS_CRYPTO_LIBRARY_OPENSSL_HPP

#include "Internal.hpp"

#if OT_CRYPTO_USING_OPENSSL
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#endif
#include "opentxs/crypto/library/HashingProvider.hpp"
#if OT_CRYPTO_SUPPORTED_ALGO_AES
#include "opentxs/crypto/library/LegacySymmetricProvider.hpp"
#endif

namespace opentxs::crypto
{
class OpenSSL : virtual public HashingProvider
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    ,
                virtual public AsymmetricProvider
#endif
#if OT_CRYPTO_SUPPORTED_ALGO_AES
    ,
                virtual public LegacySymmetricProvider
#endif
{
public:
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    virtual bool EncryptSessionKey(
        const mapOfAsymmetricKeys& RecipPubKeys,
        Data& plaintext,
        Data& dataOutput) const = 0;
    virtual bool DecryptSessionKey(
        Data& dataInput,
        const Nym& theRecipient,
        Data& plaintext,
        const OTPasswordData* pPWData = nullptr) const = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA

    virtual void Cleanup() = 0;
    virtual void Init() = 0;

    virtual ~OpenSSL() = default;

protected:
    OpenSSL() = default;

private:
    OpenSSL(const OpenSSL&) = delete;
    OpenSSL(OpenSSL&&) = delete;
    OpenSSL& operator=(const OpenSSL&) = delete;
    OpenSSL& operator=(OpenSSL&&) = delete;
};
}  // namespace opentxs::crypto

#endif  // OT_CRYPTO_USING_OPENSSL
#endif  // OPENTXS_CRYPTO_LIBRARY_OPENSSL_HPP
