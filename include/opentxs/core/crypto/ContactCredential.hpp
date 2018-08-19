// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_CONTACTCREDENTIAL_HPP
#define OPENTXS_CORE_CRYPTO_CONTACTCREDENTIAL_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>

namespace opentxs
{
class ContactCredential : public Credential
{
private:
    typedef Credential ot_super;
    friend class Credential;

    std::unique_ptr<proto::ContactData> data_;

    serializedCredential serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const override;

    ContactCredential(
        const api::Core& api,
        CredentialSet& parent,
        const proto::Credential& credential);
    ContactCredential(
        const api::Core& api,
        CredentialSet& parent,
        const NymParameters& nymParameters);
    ContactCredential() = delete;
    ContactCredential(const ContactCredential&) = delete;
    ContactCredential(ContactCredential&&) = delete;
    ContactCredential& operator=(const ContactCredential&) = delete;
    ContactCredential& operator=(ContactCredential&&) = delete;

public:
    static std::string ClaimID(
        const std::string& nymid,
        const std::uint32_t section,
        const proto::ContactItem& item);
    static std::string ClaimID(
        const std::string& nymid,
        const proto::ContactSectionName section,
        const proto::ContactItemType type,
        const std::int64_t start,
        const std::int64_t end,
        const std::string& value);
    static OTIdentifier ClaimID(const proto::Claim& preimage);
    static Claim asClaim(
        const String& nymid,
        const std::uint32_t section,
        const proto::ContactItem& item);

    bool GetContactData(
        std::unique_ptr<proto::ContactData>& contactData) const override;

    virtual ~ContactCredential() = default;
};
}  // namespace opentxs
#endif
