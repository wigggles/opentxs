// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::contract::peer::request::implementation
{
class Outbailment final : public request::Outbailment,
                          public peer::implementation::Request
{
public:
    Outbailment(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const std::uint64_t& amount,
        const std::string& terms);
    Outbailment(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized);

    ~Outbailment() final = default;

private:
    friend opentxs::Factory;

    const OTUnitID unit_;
    const OTServerID server_;
    const Amount amount_;

    Outbailment* clone() const noexcept final { return new Outbailment(*this); }
    SerializedType IDVersion(const Lock& lock) const final;

    Outbailment() = delete;
    Outbailment(const Outbailment&);
    Outbailment(Outbailment&&) = delete;
    Outbailment& operator=(const Outbailment&) = delete;
    Outbailment& operator=(Outbailment&&) = delete;
};
}  // namespace opentxs::contract::peer::request::implementation
