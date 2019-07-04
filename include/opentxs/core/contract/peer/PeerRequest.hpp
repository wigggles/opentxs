// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_PEERREQUEST_HPP
#define OPENTXS_CORE_CONTRACT_PEER_PEERREQUEST_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/Proto.hpp"

#include <string>

namespace opentxs
{
class PeerRequest : public Signable
{
private:
    typedef Signable ot_super;

    OTNymID initiator_;
    OTNymID recipient_;
    OTIdentifier server_;
    OTIdentifier cookie_;
    proto::PeerRequestType type_{proto::PEERREQUEST_ERROR};

    static std::unique_ptr<PeerRequest> Finish(
        std::unique_ptr<PeerRequest>& contract,
        const PasswordPrompt& reason);
    static OTIdentifier GetID(
        const api::Core& api,
        const proto::PeerRequest& contract);
    static bool FinalizeContract(
        PeerRequest& contract,
        const PasswordPrompt& reason);

    proto::PeerRequest contract(const Lock& lock) const;
    OTIdentifier GetID(const Lock& lock) const override;
    proto::PeerRequest SigVersion(const Lock& lock) const;

    bool update_signature(const Lock& lock, const PasswordPrompt& reason)
        override;

    PeerRequest() = delete;

protected:
    const api::Core& api_;

    virtual proto::PeerRequest IDVersion(const Lock& lock) const;
    bool validate(const Lock& lock, const PasswordPrompt& reason)
        const override;
    bool verify_signature(
        const Lock& lock,
        const proto::Signature& signature,
        const PasswordPrompt& reason) const override;

    PeerRequest(
        const api::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized);
    PeerRequest(
        const api::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized,
        const std::string& conditions);
    PeerRequest(
        const api::Core& api,
        const Nym_p& nym,
        VersionNumber version,
        const identifier::Nym& recipient,
        const identifier::Server& serverID,
        const proto::PeerRequestType& type);
    PeerRequest(
        const api::Core& api,
        const Nym_p& nym,
        VersionNumber version,
        const identifier::Nym& recipient,
        const identifier::Server& serverID,
        const std::string& conditions,
        const proto::PeerRequestType& type);

public:
    static std::unique_ptr<PeerRequest> Create(
        const api::Core& api,
        const Nym_p& nym,
        const proto::PeerRequestType& type,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const PasswordPrompt& reason);
    static std::unique_ptr<PeerRequest> Create(
        const api::Core& api,
        const Nym_p& sender,
        const proto::PeerRequestType& type,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const identifier::Nym& recipient,
        const Identifier& requestID,
        const std::string& txid,
        const Amount& amount,
        const PasswordPrompt& reason);
    static std::unique_ptr<PeerRequest> Create(
        const api::Core& api,
        const Nym_p& nym,
        const proto::PeerRequestType& type,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const std::uint64_t& amount,
        const std::string& terms,
        const PasswordPrompt& reason);
    static std::unique_ptr<PeerRequest> Create(
        const api::Core& api,
        const Nym_p& sender,
        const proto::PeerRequestType& type,
        const proto::ConnectionInfoType connectionType,
        const identifier::Nym& recipient,
        const identifier::Server& serverID,
        const PasswordPrompt& reason);
    static std::unique_ptr<PeerRequest> Create(
        const api::Core& api,
        const Nym_p& sender,
        const proto::PeerRequestType& type,
        const proto::SecretType secretType,
        const identifier::Nym& recipient,
        const std::string& primary,
        const std::string& secondary,
        const identifier::Server& serverID,
        const PasswordPrompt& reason);
    static std::unique_ptr<PeerRequest> Factory(
        const api::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized,
        const PasswordPrompt& reason);

    std::string Alias() const override { return Name(); }
    proto::PeerRequest Contract() const;
    const identifier::Nym& Initiator() const { return initiator_; }
    std::string Name() const override;
    const identifier::Nym& Recipient() const { return recipient_; }
    OTData Serialize() const override;
    const proto::PeerRequestType& Type() const { return type_; }
    void SetAlias(const std::string&) override {}

    ~PeerRequest() = default;
};
}  // namespace opentxs
#endif
