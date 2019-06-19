// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::identity::credential::implementation
{
class Verification final : virtual public credential::internal::Verification,
                           public credential::implementation::Base
{
public:
    bool GetVerificationSet(std::unique_ptr<proto::VerificationSet>&
                                verificationSet) const override;

    virtual ~Verification() = default;

private:
    friend opentxs::Factory;

    std::unique_ptr<proto::VerificationSet> data_;

    std::shared_ptr<Base::SerializedType> serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const override;
    bool verify_internally(const Lock& lock, const PasswordPrompt& reason)
        const override;

    Verification(
        const api::Core& api,
        identity::internal::Authority& parent,
        const proto::Credential& credential);
    Verification(
        const api::Core& api,
        identity::internal::Authority& parent,
        const NymParameters& nymParameters,
        const VersionNumber version);
    Verification() = delete;
    Verification(const Verification&) = delete;
    Verification(Verification&&) = delete;
    Verification& operator=(const Verification&) = delete;
    Verification& operator=(Verification&&) = delete;
};
}  // namespace opentxs::identity::credential::implementation
