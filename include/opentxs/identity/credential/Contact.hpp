// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_IDENTITY_CREDENTIAL_CONTACT_HPP
#define OPENTXS_IDENTITY_CREDENTIAL_CONTACT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <string>

namespace opentxs
{
namespace identity
{
namespace credential
{
class Contact : virtual public Base
{
public:
    EXPORT static std::string ClaimID(
        const std::string& nymid,
        const std::uint32_t section,
        const proto::ContactItem& item);
    EXPORT static std::string ClaimID(
        const std::string& nymid,
        const proto::ContactSectionName section,
        const proto::ContactItemType type,
        const std::int64_t start,
        const std::int64_t end,
        const std::string& value);
    EXPORT static OTIdentifier ClaimID(const proto::Claim& preimage);
    EXPORT static Claim asClaim(
        const String& nymid,
        const std::uint32_t section,
        const proto::ContactItem& item);

    EXPORT ~Contact() override = default;

protected:
    Contact(){};  // TODO Signable

private:
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    Contact& operator=(const Contact&) = delete;
    Contact& operator=(Contact&&) = delete;
};
}  // namespace credential
}  // namespace identity
}  // namespace opentxs
#endif
