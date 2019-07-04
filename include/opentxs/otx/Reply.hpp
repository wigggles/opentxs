// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_OTX_REPLY_HPP
#define OPENTXS_OTX_REPLY_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <memory>

namespace opentxs
{
namespace otx
{
class Reply : virtual public Signable
{
public:
    EXPORT static const VersionNumber DefaultVersion;
    EXPORT static const VersionNumber MaxVersion;

    EXPORT static Pimpl<opentxs::otx::Reply> Factory(
        const api::Core& api,
        const Nym_p signer,
        const identifier::Nym& recipient,
        const identifier::Server& server,
        const proto::ServerReplyType type,
        const bool success,
        const PasswordPrompt& reason);
    EXPORT static Pimpl<opentxs::otx::Reply> Factory(
        const api::Core& api,
        const proto::ServerReply serialized,
        const PasswordPrompt& reason);

    EXPORT virtual proto::ServerReply Contract() const = 0;
    EXPORT virtual RequestNumber Number() const = 0;
    EXPORT virtual std::shared_ptr<proto::OTXPush> Push() const = 0;
    EXPORT virtual const identifier::Nym& Recipient() const = 0;
    EXPORT virtual const identifier::Server& Server() const = 0;
    EXPORT virtual bool Success() const = 0;
    EXPORT virtual proto::ServerReplyType Type() const = 0;

    EXPORT virtual bool SetNumber(
        const RequestNumber number,
        const PasswordPrompt& reason) = 0;
    EXPORT virtual bool SetPush(
        const proto::OTXPush& push,
        const PasswordPrompt& reason) = 0;

    EXPORT virtual ~Reply() = default;

protected:
    Reply() = default;

private:
    friend OTXReply;

    virtual Reply* clone() const = 0;

    Reply(const Reply&) = delete;
    Reply(Reply&&) = delete;
    Reply& operator=(const Reply&) = delete;
    Reply& operator=(Reply&&) = delete;
};
}  // namespace otx
}  // namespace opentxs
#endif
