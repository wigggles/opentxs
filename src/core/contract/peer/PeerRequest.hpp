// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "core/contract/Signable.hpp"

#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"

namespace opentxs::contract::peer::implementation
{
class Request : virtual public peer::Request,
                public opentxs::contract::implementation::Signable
{
public:
    static bool Finish(Request& contract, const PasswordPrompt& reason);

    std::string Alias() const final { return Name(); }
    SerializedType Contract() const final;
    const identifier::Nym& Initiator() const final { return initiator_; }
    std::string Name() const final { return id_->str(); }
    const identifier::Nym& Recipient() const final { return recipient_; }
    OTData Serialize() const final;
    proto::PeerRequestType Type() const final { return type_; }
    void SetAlias(const std::string&) final {}

    ~Request() override = default;

protected:
    virtual SerializedType IDVersion(const Lock& lock) const;
    bool validate(const Lock& lock) const final;
    bool verify_signature(const Lock& lock, const proto::Signature& signature)
        const final;

    Request(
        const api::internal::Core& api,
        const Nym_p& nym,
        VersionNumber version,
        const identifier::Nym& recipient,
        const identifier::Server& serverID,
        const proto::PeerRequestType& type,
        const std::string& conditions = {});
    Request(
        const api::internal::Core& api,
        const Nym_p& nym,
        const SerializedType& serialized,
        const std::string& conditions = {});
    Request(const Request&) noexcept;

private:
    typedef Signable ot_super;

    const OTNymID initiator_;
    const OTNymID recipient_;
    const OTIdentifier server_;
    const OTIdentifier cookie_;
    const proto::PeerRequestType type_;

    static OTIdentifier GetID(
        const api::internal::Core& api,
        const SerializedType& contract);
    static bool FinalizeContract(
        Request& contract,
        const PasswordPrompt& reason);

    SerializedType contract(const Lock& lock) const;
    OTIdentifier GetID(const Lock& lock) const final;
    SerializedType SigVersion(const Lock& lock) const;

    bool update_signature(const Lock& lock, const PasswordPrompt& reason) final;

    Request() = delete;
};
}  // namespace opentxs::contract::peer::implementation
