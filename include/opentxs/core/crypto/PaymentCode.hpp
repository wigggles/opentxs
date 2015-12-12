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

#ifndef OPENTXS_CORE_CRYPTO_PAYMENTCODE_HPP
#define OPENTXS_CORE_CRYPTO_PAYMENTCODE_HPP

#include <memory>
#include <opentxs-proto/verify/VerifyCredentials.hpp>

#include <opentxs/core/Identifier.hpp>
#include <opentxs/core/OTData.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>

namespace opentxs
{

typedef std::shared_ptr<proto::PaymentCode> SerializedPaymentCode;

class MasterCredential;

class PaymentCode
{
private:
    const uint8_t BIP47_VERSION_BYTE = 0x47;

    uint8_t version_ = 1;
    std::shared_ptr<OTAsymmetricKey> pubkey_;
    OTData chain_code_;
    bool hasBitmessage_ = false;
    uint8_t bitmessage_version_ = 0;
    uint8_t bitmessage_stream_ = 0;

    const OTData Pubkey() const;
    void ConstructKey(const OTData& pubkey, const OTData& chaincode);
    PaymentCode() = delete;

public:
    PaymentCode(proto::PaymentCode& paycode);
    PaymentCode(
        const uint32_t nym,
        const bool bitmessage = false,
        const uint8_t bitmessageVersion = 0,
        const uint8_t bitmessageStream = 0);

    Identifier ID() const;
    const std::string asBase58() const;
    const SerializedPaymentCode Serialize() const;
    bool Verify(const MasterCredential& credential) const;

};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_PAYMENTCODE_HPP
