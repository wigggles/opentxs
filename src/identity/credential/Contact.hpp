// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::identity::credential::implementation
{
class Contact final : virtual public credential::internal::Contact,
                      public credential::implementation::Base
{
public:
    bool GetContactData(
        std::unique_ptr<proto::ContactData>& contactData) const override;

    ~Contact() override = default;

private:
    friend opentxs::Factory;

    std::unique_ptr<proto::ContactData> data_;

    std::shared_ptr<Base::SerializedType> serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const override;

    Contact(
        const api::Core& api,
        identity::internal::Authority& parent,
        const proto::Credential& credential);
    Contact(
        const api::Core& api,
        identity::internal::Authority& parent,
        const NymParameters& nymParameters);
    Contact() = delete;
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    Contact& operator=(const Contact&) = delete;
    Contact& operator=(Contact&&) = delete;
};
}  // namespace opentxs::identity::credential::implementation
