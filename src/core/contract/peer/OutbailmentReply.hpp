// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::contract::peer::reply::implementation
{
class Outbailment final : public reply::Outbailment,
                          public peer::implementation::Reply
{
public:
    Outbailment(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Server& server,
        const std::string& terms);
    Outbailment(
        const api::internal::Core& api,
        const Nym_p& nym,
        const SerializedType& serialized);

    ~Outbailment() final = default;

private:
    friend opentxs::Factory;

    Outbailment* clone() const noexcept final { return new Outbailment(*this); }
    SerializedType IDVersion(const Lock& lock) const final;

    Outbailment() = delete;
    Outbailment(const Outbailment&);
    Outbailment(Outbailment&&) = delete;
    Outbailment& operator=(const Outbailment&) = delete;
    Outbailment& operator=(Outbailment&&) = delete;
};
}  // namespace opentxs::contract::peer::reply::implementation
