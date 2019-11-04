// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::contract::peer::request::implementation
{
class BailmentNotice final : public request::BailmentNotice,
                             public peer::implementation::Request
{
public:
    BailmentNotice(
        const api::internal::Core& api,
        const Nym_p& nym,
        const SerializedType& serialized);
    BailmentNotice(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const Identifier& requestID,
        const std::string& txid,
        const Amount& amount);

    ~BailmentNotice() final = default;

private:
    friend opentxs::Factory;

    const OTUnitID unit_;
    const OTServerID server_;
    const OTIdentifier requestID_;
    const std::string txid_;
    const Amount amount_;

    BailmentNotice* clone() const noexcept final
    {
        return new BailmentNotice(*this);
    }
    SerializedType IDVersion(const Lock& lock) const final;

    BailmentNotice() = delete;
    BailmentNotice(const BailmentNotice&);
    BailmentNotice(BailmentNotice&&) = delete;
    BailmentNotice& operator=(const BailmentNotice&) = delete;
    BailmentNotice& operator=(BailmentNotice&&) = delete;
};
}  // namespace opentxs::contract::peer::request::implementation
