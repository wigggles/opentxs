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

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/OTData.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"

#include <memory>

namespace opentxs
{

typedef std::shared_ptr<proto::PaymentCode> SerializedPaymentCode;

class Credential;
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
    PaymentCode(const std::string& base58);
    PaymentCode(const proto::PaymentCode& paycode);
    PaymentCode(
        const uint32_t nym,
        const bool bitmessage = false,
        const uint8_t bitmessageVersion = 0,
        const uint8_t bitmessageStream = 0);

    bool operator==(const proto::PaymentCode& rhs) const;

    const Identifier ID() const;
    const std::string asBase58() const;
    SerializedPaymentCode Serialize() const;
    bool VerifyInternally() const;
    bool Verify(const MasterCredential& credential) const;
    bool Sign(
        const uint32_t nym,
        const Credential& credential,
        proto::Signature& sig,
        const OTPasswordData* pPWData = nullptr) const;
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_PAYMENTCODE_HPP
