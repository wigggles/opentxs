// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::contract::peer::request::implementation
{
class Connection final : public request::Connection,
                         public peer::implementation::Request
{
public:
    Connection(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const proto::ConnectionInfoType type,
        const identifier::Server& serverID);
    Connection(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized);

    ~Connection() final = default;

private:
    friend opentxs::Factory;

    const proto::ConnectionInfoType connection_type_;

    Connection* clone() const noexcept final { return new Connection(*this); }
    SerializedType IDVersion(const Lock& lock) const final;

    Connection() = delete;
    Connection(const Connection&);
    Connection(Connection&&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection& operator=(Connection&&) = delete;
};
}  // namespace opentxs::contract::peer::request::implementation
