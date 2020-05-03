// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_PEERREPLY_HPP
#define OPENTXS_CORE_CONTRACT_PEER_PEERREPLY_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/SharedPimpl.hpp"

namespace opentxs
{
namespace contract
{
namespace peer
{
class Reply;
}  // namespace peer
}  // namespace contract

using OTPeerReply = SharedPimpl<contract::peer::Reply>;
}  // namespace opentxs

namespace opentxs
{
namespace contract
{
namespace peer
{
class Reply : virtual public opentxs::contract::Signable
{
public:
    using SerializedType = proto::PeerReply;

    OPENTXS_EXPORT virtual SerializedType Contract() const = 0;
    OPENTXS_EXPORT virtual proto::PeerRequestType Type() const = 0;

    OPENTXS_EXPORT virtual ~Reply() override = default;

protected:
    Reply() noexcept = default;

private:
    friend OTPeerReply;

#ifndef _WIN32
    OPENTXS_EXPORT Reply* clone() const noexcept override = 0;
#endif

    Reply(const Reply&) = delete;
    Reply(Reply&&) = delete;
    Reply& operator=(const Reply&) = delete;
    Reply& operator=(Reply&&) = delete;
};
}  // namespace peer
}  // namespace contract
}  // namespace opentxs
#endif
