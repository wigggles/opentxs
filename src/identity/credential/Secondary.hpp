// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::identity::credential::implementation
{
class Secondary final : virtual public credential::internal::Secondary,
                        credential::implementation::Key
{
public:
    ~Secondary() override = default;

private:
    friend opentxs::Factory;

    std::shared_ptr<Base::SerializedType> serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const override;

    Secondary(
        const api::internal::Core& api,
        const identity::internal::Authority& other,
        const identity::Source& source,
        const internal::Primary& master,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const PasswordPrompt& reason) noexcept(false);
    Secondary(
        const api::internal::Core& api,
        const PasswordPrompt& reason,
        const identity::internal::Authority& other,
        const identity::Source& source,
        const internal::Primary& master,
        const proto::Credential& serializedCred) noexcept(false);
    Secondary() = delete;
    Secondary(const Secondary&) = delete;
    Secondary(Secondary&&) = delete;
    Secondary& operator=(const Secondary&) = delete;
    Secondary& operator=(Secondary&&) = delete;
};
}  // namespace opentxs::identity::credential::implementation
