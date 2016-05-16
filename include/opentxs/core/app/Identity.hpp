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

#ifndef OPENTXS_CORE_APP_IDENTITY_HPP
#define OPENTXS_CORE_APP_IDENTITY_HPP

#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"

#include <cstdint>
#include <memory>

namespace opentxs
{

class App;
class Nym;

class Identity
{
private:
    friend App;

    Identity() = default;
    Identity(const Identity&) = delete;
    Identity& operator=(const Identity&) = delete;

    void AddClaimToSection(proto::ContactData& data, const Claim& claim) const;
    bool ClaimIsPrimary(const Claim& claim) const;
    void ClearPrimaryAttribute(proto::ContactItem& claim) const;
    proto::ContactItem& GetOrCreateClaim(
        proto::ContactSection& section,
        const proto::ContactItemType type,
        const std::string& value,
        const std::int64_t start,
        const std::int64_t end) const;
    proto::ContactSection& GetOrCreateSection(
        proto::ContactData& data,
        proto::ContactSectionName section,
        const std::uint32_t version = 1) const;
    std::unique_ptr<proto::ContactData> InitializeContactData(
        const std::uint32_t version = 1) const;
    void InitializeContactItem(
        proto::ContactItem& item,
        const proto::ContactItemType type,
        const std::string& value,
        const std::int64_t start,
        const std::int64_t end) const;
    void InitializeContactSection(
        proto::ContactSection& section,
        const proto::ContactSectionName name,
        const std::uint32_t version = 1) const;
    void ResetPrimary(
        proto::ContactSection& section,
        const proto::ContactItemType& type) const;
    void SetAttributesOnClaim(
        proto::ContactItem& item,
        const Claim& claim) const;

public:
    bool AddClaim(Nym& toNym, const Claim claim) const;
};
} // namespace opentxs
#endif // OPENTXS_CORE_APP_IDENTITY_HPP
