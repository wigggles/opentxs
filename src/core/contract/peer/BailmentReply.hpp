// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::contract::peer::reply::implementation
{
class Bailment final : public reply::Bailment,
                       public peer::implementation::Reply
{
public:
    Bailment(
        const api::internal::Core& api,
        const Nym_p& nym,
        const SerializedType& serialized);
    Bailment(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Server& server,
        const std::string& terms);

    ~Bailment() final = default;

private:
    friend opentxs::Factory;

    Bailment* clone() const noexcept final { return new Bailment(*this); }
    SerializedType IDVersion(const Lock& lock) const final;

    Bailment() = delete;
    Bailment(const Bailment&);
    Bailment(Bailment&&) = delete;
    Bailment& operator=(const Bailment&) = delete;
    Bailment& operator=(Bailment&&) = delete;
};
}  // namespace opentxs::contract::peer::reply::implementation
