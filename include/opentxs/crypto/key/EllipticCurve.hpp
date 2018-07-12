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

#ifndef OPENTXS_CORE_CRYPTO_ASYMMETRICKEYEC_HPP
#define OPENTXS_CORE_CRYPTO_ASYMMETRICKEYEC_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/crypto/key/Asymmetric.hpp"

#include <memory>

namespace opentxs
{
namespace crypto
{
namespace key
{
class EllipticCurve : virtual public Asymmetric
{
public:
    virtual const crypto::EcdsaProvider& ECDSA() const = 0;
    virtual bool GetKey(Data& key) const = 0;
    virtual bool GetKey(proto::Ciphertext& key) const = 0;
    using Asymmetric::GetPublicKey;
    virtual bool GetPublicKey(Data& key) const = 0;

    virtual bool SetKey(const Data& key) = 0;
    virtual bool SetKey(std::unique_ptr<proto::Ciphertext>& key) = 0;

    virtual ~EllipticCurve() = default;

protected:
    EllipticCurve() = default;

private:
    EllipticCurve(const EllipticCurve&) = delete;
    EllipticCurve(EllipticCurve&&) = delete;
    EllipticCurve& operator=(const EllipticCurve&) = delete;
    EllipticCurve& operator=(EllipticCurve&&) = delete;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif  // OPENTXS_CORE_CRYPTO_ASYMMETRICKEYEC_HPP
