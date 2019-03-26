// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_NYMDATA_HPP
#define OPENTXS_CLIENT_NYMDATA_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Nym.hpp"

#include <cstdint>
#include <memory>
#include <string>

#ifdef SWIG
#include <algorithm>
#include <set>
#include <vector>

// clang-format off
%template(ProtoTypeList) std::vector<int>;
%extend opentxs::NymData {
    std::string BestSocialMediaProfile(const int type) const
    {
        return $self->BestSocialMediaProfile(
            static_cast<opentxs::proto::ContactItemType>(type));
    }
    bool HaveContract(
        const std::string& id,
        const int currency,
        const bool primary,
        const bool active) const
    {
        return $self->HaveContract(
            opentxs::identifier::UnitDefinition::Factory(id),
            static_cast<opentxs::proto::ContactItemType>(currency),
            primary,
            active);
    }
    std::string PaymentCode(const int currency) const
    {
        return $self->PaymentCode(
            static_cast<opentxs::proto::ContactItemType>(currency));
    }
    std::string SocialMediaProfiles(
        const int type,
        bool active = true) const
    {
        return $self->SocialMediaProfiles(
            static_cast<opentxs::proto::ContactItemType>(type), active);
    }
    const std::vector<int> SocialMediaProfileTypes() const
    {
        const auto& types = $self->Nym().SocialMediaProfileTypes();

        std::vector<int> output;
        std::transform(
            types.begin(),
            types.end(),
            std::inserter(output, output.end()),
            [](opentxs::proto::ContactItemType type) -> int {
                return static_cast<int>(type);
            });

        return output;
    }
    int Type() const
    {
        return static_cast<int>($self->Type());
    }
    bool AddContract(
        const std::string& instrumentDefinitionID,
        const int currency,
        const bool primary,
        const bool active)
    {
        return $self->AddContract(
            instrumentDefinitionID,
            static_cast<opentxs::proto::ContactItemType>(currency),
            primary,
            active);
    }
    bool AddPaymentCode(
        const std::string& code,
        const int currency,
        const bool primary,
        const bool active)
    {
        return $self->AddPaymentCode(
            code,
            static_cast<opentxs::proto::ContactItemType>(currency),
            primary,
            active);
    }
    bool AddSocialMediaProfile(
        const std::string& value,
        const int type,
        const bool primary,
        const bool active)
    {
        return $self->AddSocialMediaProfile(
            value,
            static_cast<opentxs::proto::ContactItemType>(type),
            primary,
            active);
    }
    bool SetScope(
        const int type,
        const std::string& name,
        const bool primary)
    {
        return $self->SetScope(
            static_cast<opentxs::proto::ContactItemType>(type), name, primary);
    }
}

%ignore opentxs::NymData::AddChildKeyCredential;
%ignore opentxs::NymData::AddClaim;
%ignore opentxs::NymData::AddContract;
%ignore opentxs::NymData::AddPaymentCode;
%ignore opentxs::NymData::AddSocialMediaProfile;
%ignore opentxs::NymData::asPublicNym;
%ignore opentxs::NymData::BestSocialMediaProfile;
%ignore opentxs::NymData::Claims;
%ignore opentxs::NymData::DeleteClaim;
%ignore opentxs::NymData::HaveContract;
%ignore opentxs::NymData::Nym;
%ignore opentxs::NymData::PaymentCode;
%ignore opentxs::NymData::SetContactData;
%ignore opentxs::NymData::SetScope;
%ignore opentxs::NymData::SetVerificationSet;
%ignore opentxs::NymData::SocialMediaProfiles;
%ignore opentxs::NymData::SocialMediaProfileTypes;
%ignore opentxs::NymData::Type;
%ignore opentxs::NymData::VerificationSet;
%ignore opentxs::NymData::NymData(NymData&&);
// clang-format on
#endif

namespace opentxs
{
namespace api
{
namespace implementation
{
class Wallet;
}  // namespace implementation
}  // namespace api

class NymData
{
public:
    NymData(const NymData&);
    NymData(NymData&&);

    const proto::CredentialIndex asPublicNym() const;
    std::string BestEmail() const;
    std::string BestPhoneNumber() const;
    std::string BestSocialMediaProfile(const proto::ContactItemType type) const;
    const class ContactData& Claims() const;
    bool DeleteClaim(const Identifier& id);
    std::string EmailAddresses(bool active = true) const;
    bool HaveContract(
        const identifier::UnitDefinition& id,
        const proto::ContactItemType currency,
        const bool primary,
        const bool active) const;
    std::string Name() const;
    const opentxs::Nym& Nym() const;
    std::string PaymentCode(const proto::ContactItemType currency) const;
    std::string PhoneNumbers(bool active = true) const;
    std::string PreferredOTServer() const;
    std::string PrintContactData() const;
    std::string SocialMediaProfiles(
        const proto::ContactItemType type,
        bool active = true) const;
    std::set<proto::ContactItemType> SocialMediaProfileTypes() const;
    proto::ContactItemType Type() const;
    bool Valid() const;
    std::unique_ptr<proto::VerificationSet> VerificationSet() const;

    std::string AddChildKeyCredential(
        const Identifier& strMasterID,
        const NymParameters& nymParameters);
    bool AddClaim(const Claim& claim);
    bool AddContract(
        const std::string& instrumentDefinitionID,
        const proto::ContactItemType currency,
        const bool primary,
        const bool active);
    bool AddEmail(
        const std::string& value,
        const bool primary,
        const bool active);
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    bool AddPaymentCode(
        const std::string& code,
        const proto::ContactItemType currency,
        const bool primary,
        const bool active);
#endif
    bool AddPhoneNumber(
        const std::string& value,
        const bool primary,
        const bool active);
    bool AddPreferredOTServer(const std::string& id, const bool primary);
    bool AddSocialMediaProfile(
        const std::string& value,
        const proto::ContactItemType type,
        const bool primary,
        const bool active);
    void Release();
    bool SetCommonName(const std::string& name);
    bool SetContactData(const proto::ContactData& data);
    bool SetScope(
        const proto::ContactItemType type,
        const std::string& name,
        const bool primary);
    bool SetVerificationSet(const proto::VerificationSet& data);

    ~NymData();

private:
    friend api::implementation::Wallet;

    typedef std::unique_lock<std::mutex> Lock;
    typedef std::function<void(NymData*, Lock&)> LockedSave;

    const api::Factory& factory_;
    std::unique_ptr<Lock> object_lock_;
    std::unique_ptr<LockedSave> locked_save_callback_;

    std::shared_ptr<opentxs::Nym> nym_;

    const ContactData& data() const;

    const opentxs::Nym& nym() const;
    opentxs::Nym& nym();

    void release();

    NymData(
        const api::Factory& factory,
        std::mutex& objectMutex,
        const std::shared_ptr<opentxs::Nym>& nym,
        LockedSave save);
    NymData() = delete;
    NymData& operator=(const NymData&) = delete;
    NymData& operator=(NymData&&) = delete;
};
}  // namespace opentxs
#endif
