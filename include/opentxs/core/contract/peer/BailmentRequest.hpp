// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_BAILMENTREQUEST_HPP
#define OPENTXS_CORE_CONTRACT_PEER_BAILMENTREQUEST_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/peer/PeerRequest.hpp"

namespace opentxs
{
using OTBailmentRequest = SharedPimpl<contract::peer::request::Bailment>;

namespace contract
{
namespace peer
{
namespace request
{
class Bailment : virtual public peer::Request
{
public:
    OPENTXS_EXPORT ~Bailment() override = default;

protected:
    Bailment() noexcept = default;

private:
    friend OTBailmentRequest;

#ifndef _WIN32
    OPENTXS_EXPORT Bailment* clone() const noexcept override = 0;
#endif

    Bailment(const Bailment&) = delete;
    Bailment(Bailment&&) = delete;
    Bailment& operator=(const Bailment&) = delete;
    Bailment& operator=(Bailment&&) = delete;
};
}  // namespace request
}  // namespace peer
}  // namespace contract
}  // namespace opentxs
#endif
