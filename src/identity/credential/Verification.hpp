// Copyright (c) 2019 The Open-Transactions developers
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
    bool GetVerificationSet(
        std::unique_ptr<proto::VerificationSet>& verificationSet) const final;

    ~Verification() final = default;

private:
    friend opentxs::Factory;

    const proto::VerificationSet data_;

    std::shared_ptr<Base::SerializedType> serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const final;
    bool verify_internally(const Lock& lock, const PasswordPrompt& reason)
        const final;

    Verification(
        const api::Core& api,
        const identity::internal::Authority& parent,
        const NymParameters& nymParameters,
        const VersionNumber version);
    Verification(
        const api::Core& api,
        const identity::internal::Authority& parent,
        const proto::Credential& credential);
    Verification() = delete;
    Verification(const Verification&) = delete;
    Verification(Verification&&) = delete;
    Verification& operator=(const Verification&) = delete;
    Verification& operator=(Verification&&) = delete;
};
}  // namespace opentxs::identity::credential::implementation
