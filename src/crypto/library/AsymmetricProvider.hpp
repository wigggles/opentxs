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

#ifndef IMPLEMENTATION_OPENTXS_CRYPTO_LIBRARY_ASYMMETRICPROVIDER_HPP
#define IMPLEMENTATION_OPENTXS_CRYPTO_LIBRARY_ASYMMETRICPROVIDER_HPP

#include "Internal.hpp"

#include "opentxs/crypto/library/AsymmetricProvider.hpp"

namespace opentxs::crypto::implementation
{
class AsymmetricProvider : virtual public crypto::AsymmetricProvider
{
public:
    bool SignContract(
        const String& strContractUnsigned,
        const key::Asymmetric& theKey,
        OTSignature& theSignature,  // output
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const override;
    bool VerifyContractSignature(
        const String& strContractToVerify,
        const key::Asymmetric& theKey,
        const OTSignature& theSignature,
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const override;

    virtual ~AsymmetricProvider() = default;

protected:
    AsymmetricProvider() = default;

private:
    AsymmetricProvider(const AsymmetricProvider&) = delete;
    AsymmetricProvider(AsymmetricProvider&&) = delete;
    AsymmetricProvider& operator=(const AsymmetricProvider&) = delete;
    AsymmetricProvider& operator=(AsymmetricProvider&&) = delete;
};
}  // namespace opentxs::crypto::implementation
#endif  // IMPLEMENTATION_OPENTXS_CRYPTO_LIBRARY_ASYMMETRICPROVIDER_HPP
