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
#include <string>

namespace opentxs
{

class App;
class Nym;
class OTPasswordData;

class Identity
{
private:
    friend App;

    Identity() = default;
    Identity(const Identity&) = delete;
    Identity& operator=(const Identity&) = delete;

    void AddClaimToSection(proto::ContactData& data, const Claim& claim) const;
    bool AddInternalVerification(
        bool& changed,
        proto::VerificationSet& verifications,
        const Nym& onNym,
        const std::string& claimantNymID,
        const std::string& claimID,
        const ClaimPolarity polarity,
        const std::int64_t start = 0,
        const std::int64_t end = 0,
        const OTPasswordData* pPWData = nullptr) const;
    bool ClaimIsPrimary(const Claim& claim) const;
    void ClearPrimaryAttribute(proto::ContactItem& claim) const;
    void DeleteVerification(
        bool& changed,
        proto::VerificationIdentity& identity,
        const std::string& claimID,
        const std::int64_t start = 0,
        const std::int64_t end = 0) const;
    proto::ContactItem& GetOrCreateClaim(
        proto::ContactSection& section,
        const proto::ContactItemType type,
        const std::string& value,
        const std::int64_t start,
        const std::int64_t end) const;
    proto::VerificationGroup& GetOrCreateInternalGroup(
        proto::VerificationSet& verificationSet,
        const std::uint32_t version = 1) const;
    proto::ContactSection& GetOrCreateSection(
        proto::ContactData& data,
        proto::ContactSectionName section,
        const std::uint32_t version = 1) const;
    proto::VerificationIdentity& GetOrCreateVerificationIdentity(
        proto::VerificationGroup& verificationGroup,
        const std::string& nym,
        const std::uint32_t version = 1) const;
    bool HaveVerification(
        proto::VerificationIdentity& identity,
        const std::string& claimID,
        const ClaimPolarity polarity,
        const std::int64_t start = 0,
        const std::int64_t end = 0) const;
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
    std::unique_ptr<proto::VerificationSet> InitializeVerificationSet(
        const std::uint32_t version = 1) const;
    bool MatchVerification(
        const proto::Verification& item,
        const std::string& claimID,
        const std::int64_t start = 0,
        const std::int64_t end = 0) const;
    void PopulateClaimIDs(
        proto::ContactData& data,
        const std::string& nym) const;
    void PopulateVerificationIDs(proto::VerificationGroup& group) const;
    bool RemoveInternalVerification(
        bool& changed,
        proto::VerificationSet& verifications,
        const std::string& claimantNymID,
        const std::string& claimID,
        const std::int64_t start = 0,
        const std::int64_t end = 0) const;
    void ResetPrimary(
        proto::ContactSection& section,
        const proto::ContactItemType& type) const;
    void SetAttributesOnClaim(
        proto::ContactItem& item,
        const Claim& claim) const;

public:
    bool AddClaim(Nym& toNym, const Claim claim) const;
    std::unique_ptr<proto::ContactData> Claims(const Nym& fromNym) const;
    bool DeleteClaim(Nym& onNym, std::string& claimID) const;
    std::unique_ptr<proto::VerificationSet> Verifications(
        const Nym& fromNym) const;
    std::unique_ptr<proto::VerificationSet> Verify(
        Nym& onNym,
        bool& changed,
        const std::string& claimantNymID,
        const std::string& claimID,
        const ClaimPolarity polarity,
        const std::int64_t start = 0,
        const std::int64_t end = 0,
        const OTPasswordData* pPWData = nullptr) const;
};
} // namespace opentxs
#endif // OPENTXS_CORE_APP_IDENTITY_HPP
