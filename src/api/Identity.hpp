// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::implementation
{
class Identity : virtual public api::Identity
{
public:
    std::unique_ptr<proto::VerificationSet> Verifications(
        const Nym& fromNym) const override;
    std::unique_ptr<proto::VerificationSet> Verify(
        NymData& onNym,
        bool& changed,
        const std::string& claimantNymID,
        const std::string& claimID,
        const ClaimPolarity polarity,
        const std::int64_t start = 0,
        const std::int64_t end = 0,
        const OTPasswordData* pPWData = nullptr) const override;

    ~Identity() = default;

private:
    friend opentxs::Factory;

    const api::Core& api_;

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
    void DeleteVerification(
        bool& changed,
        proto::VerificationIdentity& identity,
        const std::string& claimID,
        const std::int64_t start = 0,
        const std::int64_t end = 0) const;
    proto::VerificationGroup& GetOrCreateInternalGroup(
        proto::VerificationSet& verificationSet,
        const std::uint32_t version = VERIFICATION_CREDENTIAL_VERSION) const;
    proto::VerificationIdentity& GetOrCreateVerificationIdentity(
        proto::VerificationGroup& verificationGroup,
        const std::string& nym,
        const std::uint32_t version = VERIFICATION_CREDENTIAL_VERSION) const;
    bool HaveVerification(
        proto::VerificationIdentity& identity,
        const std::string& claimID,
        const ClaimPolarity polarity,
        const std::int64_t start = 0,
        const std::int64_t end = 0) const;
    std::unique_ptr<proto::VerificationSet> InitializeVerificationSet(
        const std::uint32_t version = VERIFICATION_CREDENTIAL_VERSION) const;
    bool MatchVerification(
        const proto::Verification& item,
        const std::string& claimID,
        const std::int64_t start = 0,
        const std::int64_t end = 0) const;
    void PopulateVerificationIDs(proto::VerificationGroup& group) const;
    bool RemoveInternalVerification(
        bool& changed,
        proto::VerificationSet& verifications,
        const std::string& claimantNymID,
        const std::string& claimID,
        const std::int64_t start = 0,
        const std::int64_t end = 0) const;
    bool Sign(
        proto::Verification& plaintext,
        const Nym& nym,
        const OTPasswordData* pPWData = nullptr) const;

    Identity(const api::Core& api);
    Identity() = delete;
    Identity(const Identity&) = delete;
    Identity(Identity&&) = delete;
    Identity& operator=(const Identity&) = delete;
    Identity& operator=(Identity&&) = delete;
};
}  // namespace opentxs::api::implementation
