// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_IDENTITY_HPP
#define OPENTXS_API_IDENTITY_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{
namespace api
{
class Identity
{
public:
    virtual std::unique_ptr<proto::VerificationSet> Verifications(
        const identity::Nym& fromNym) const = 0;
    virtual std::unique_ptr<proto::VerificationSet> Verify(
        NymData& onNym,
        bool& changed,
        const std::string& claimantNymID,
        const std::string& claimID,
        const ClaimPolarity polarity,
        const std::int64_t start = 0,
        const std::int64_t end = 0,
        const OTPasswordData* pPWData = nullptr) const = 0;

    virtual ~Identity() = default;

protected:
    Identity() = default;

private:
    Identity(const Identity&) = delete;
    Identity(Identity&&) = delete;
    Identity& operator=(const Identity&) = delete;
    Identity& operator=(Identity&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
