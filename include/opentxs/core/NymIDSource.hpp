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

#ifndef OPENTXS_CORE_NYMIDSOURCE_HPP
#define OPENTXS_CORE_NYMIDSOURCE_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/core/crypto/PaymentCode.hpp"
#endif
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Proto.hpp"

#include <cstdint>
#include <memory>

namespace opentxs
{
typedef std::shared_ptr<proto::NymIDSource> serializedNymIDSource;

class NymIDSource
{
public:
    static serializedNymIDSource ExtractArmoredSource(
        const OTASCIIArmor& armoredSource);

    String asString() const;
    String Description() const;
    proto::SourceType Type() const;
    Identifier NymID() const;
    serializedNymIDSource Serialize() const;
    bool Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature) const;
    bool Sign(
        const MasterCredential& credential,
        proto::Signature& sig,
        const OTPasswordData* pPWData = nullptr) const;

    explicit NymIDSource(const proto::NymIDSource& serializedSource);
    explicit NymIDSource(const String& stringSource);
    NymIDSource(
        const NymParameters& nymParameters,
        proto::AsymmetricKey& pubkey);
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    explicit NymIDSource(const PaymentCode& source);
#endif
    NymIDSource(const NymIDSource&);

private:
    std::uint32_t version_ = 0;
    proto::SourceType type_ = proto::SOURCETYPE_ERROR;
    std::shared_ptr<OTAsymmetricKey> pubkey_;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    OTPaymentCode payment_code_;
#endif

    OTData asData() const;

    static std::unique_ptr<proto::AsymmetricKey> ExtractKey(
        const proto::Credential& credential,
        const proto::KeyRole role);

    NymIDSource() = delete;
    NymIDSource(NymIDSource&&) = delete;
    NymIDSource& operator=(const NymIDSource&);
    NymIDSource& operator=(NymIDSource&&);
};
}  // namespace opentxs

#endif  // OPENTXS_CORE_NYMIDSOURCE_HPP
