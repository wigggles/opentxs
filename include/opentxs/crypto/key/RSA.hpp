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

#ifndef OPENTXS_CRYPTO_KEY_RSA_HPP
#define OPENTXS_CRYPTO_KEY_RSA_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/crypto/key/Asymmetric.hpp"

namespace opentxs
{
namespace crypto
{
namespace key
{
class RSA : virtual public Asymmetric
{
public:
    virtual bool GetPrivateKey(
        String& strOutput,
        const Asymmetric* pPubkey,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr) const = 0;
    /** Don't ever call this. It's only here because it's impossible to get rid
     * of unless and until RSA key support is removed entirely. */
    virtual bool SaveCertToString(
        String& strOutput,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr) const = 0;

    virtual bool SetPrivateKey(
        const String& strCert,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr) = 0;
    virtual bool SetPublicKey(const String& strKey) = 0;
    virtual bool SetPublicKeyFromPrivateKey(
        const String& strCert,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr) = 0;

    virtual ~RSA() = default;

protected:
    RSA() = default;

private:
    RSA(const RSA&) = delete;
    RSA(RSA&&) = delete;
    RSA& operator=(const RSA&) = delete;
    RSA& operator=(RSA&&) = delete;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#endif  // OPENTXS_CRYPTO_KEY_RSA_HPP
