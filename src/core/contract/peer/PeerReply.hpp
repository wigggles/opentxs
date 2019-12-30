// Copyright (c) 2010-2019 The Open-Transactions developers
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
class Reply : virtual public peer::Reply,
              public opentxs::contract::implementation::Signable
{
public:
    static bool Finish(Reply& contract, const PasswordPrompt& reason);
    static std::shared_ptr<proto::PeerRequest> LoadRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const Identifier& requestID);

    std::string Alias() const final { return Name(); }
    SerializedType Contract() const override;
    std::string Name() const final { return id_->str(); }
    OTData Serialize() const final;
    proto::PeerRequestType Type() const final { return type_; }
    void SetAlias(const std::string&) final {}

    ~Reply() override = default;

protected:
    virtual SerializedType IDVersion(const Lock& lock) const;
    bool validate(const Lock& lock) const final;
    bool verify_signature(const Lock& lock, const proto::Signature& signature)
        const final;

    Reply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const VersionNumber version,
        const identifier::Nym& initiator,
        const identifier::Server& server,
        const proto::PeerRequestType& type,
        const Identifier& request,
        const std::string& conditions = {});
    Reply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const SerializedType& serialized,
        const std::string& conditions = {});
    Reply(const Reply&) noexcept;

private:
    friend opentxs::Factory;

    const OTNymID initiator_;
    const OTNymID recipient_;
    const OTServerID server_;
    const OTIdentifier cookie_;
    const proto::PeerRequestType type_;

    static OTIdentifier GetID(
        const api::internal::Core& api,
        const SerializedType& contract);
    static bool FinalizeContract(Reply& contract, const PasswordPrompt& reason);

    SerializedType contract(const Lock& lock) const;
    OTIdentifier GetID(const Lock& lock) const final;
    SerializedType SigVersion(const Lock& lock) const;

    bool update_signature(const Lock& lock, const PasswordPrompt& reason) final;

    Reply() = delete;
    Reply(Reply&&) = delete;
    Reply& operator=(const Reply&) = delete;
    Reply& operator=(Reply&&) = delete;
};
}  // namespace opentxs::contract::peer::implementation
