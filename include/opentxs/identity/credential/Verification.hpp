// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_IDENTITY_CREDENTIAL_VERIFICATION_HPP
#define OPENTXS_IDENTITY_CREDENTIAL_VERIFICATION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/Proto.hpp"

namespace opentxs
{
namespace identity
{
namespace credential
{
class Verification : virtual public Base
{
public:
    EXPORT static proto::Verification SigningForm(
        const proto::Verification& item);
    EXPORT static std::string VerificationID(
        const api::Core& api,
        const proto::Verification& item);

    EXPORT ~Verification() override = default;

protected:
    Verification() noexcept {}  // TODO Signable

private:
    Verification(const Verification&) = delete;
    Verification(Verification&&) = delete;
    Verification& operator=(const Verification&) = delete;
    Verification& operator=(Verification&&) = delete;
};
}  // namespace credential
}  // namespace identity
}  // namespace opentxs
#endif
