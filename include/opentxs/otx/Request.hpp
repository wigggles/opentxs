// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_OTX_REQUEST_HPP
#define OPENTXS_OTX_REQUEST_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <memory>

namespace opentxs
{
namespace otx
{
class Request : virtual public Signable
{
public:
    EXPORT static Pimpl<opentxs::otx::Request> Factory(
        const std::shared_ptr<const opentxs::Nym> signer,
        const identifier::Server& server,
        const proto::ServerRequestType type);
    EXPORT static Pimpl<opentxs::otx::Request> Factory(
        const api::Core& api,
        const proto::ServerRequest serialized);

    EXPORT virtual proto::ServerRequest Contract() const = 0;
    EXPORT virtual const identifier::Nym& Initiator() const = 0;
    EXPORT virtual RequestNumber Number() const = 0;
    EXPORT virtual const identifier::Server& Server() const = 0;
    EXPORT virtual proto::ServerRequestType Type() const = 0;

    EXPORT virtual bool SetIncludeNym(const bool include) = 0;
    EXPORT virtual bool SetNumber(const RequestNumber number) = 0;

    EXPORT virtual ~Request() = default;

protected:
    Request() = default;

private:
    friend OTXRequest;

    virtual Request* clone() const = 0;

    Request(const Request&) = delete;
    Request(Request&&) = delete;
    Request& operator=(const Request&) = delete;
    Request& operator=(Request&&) = delete;
};
}  // namespace otx
}  // namespace opentxs
#endif
