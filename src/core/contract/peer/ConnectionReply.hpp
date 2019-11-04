// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::contract::peer::reply::implementation
{
class Connection final : public reply::Connection,
                         public peer::implementation::Reply
{
public:
    Connection(
        const api::internal::Core& api,
        const Nym_p& nym,
        const SerializedType& serialized);
    Connection(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Server& server,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key);

    ~Connection() final = default;

private:
    friend opentxs::Factory;

    const bool success_;
    const std::string url_;
    const std::string login_;
    const std::string password_;
    const std::string key_;

    Connection* clone() const noexcept final { return new Connection(*this); }
    SerializedType IDVersion(const Lock& lock) const final;

    Connection() = delete;
    Connection(const Connection&);
    Connection(Connection&&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection& operator=(Connection&&) = delete;
};
}  // namespace opentxs::contract::peer::reply::implementation
