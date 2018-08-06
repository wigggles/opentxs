// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_PEERREQUEST_HPP
#define OPENTXS_CORE_CONTRACT_PEER_PEERREQUEST_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/Proto.hpp"

#include <string>

namespace opentxs
{
class PeerRequest : public Signable
{
private:
    typedef Signable ot_super;

    OTIdentifier initiator_;
    OTIdentifier recipient_;
    OTIdentifier server_;
    OTIdentifier cookie_;
    proto::PeerRequestType type_{proto::PEERREQUEST_ERROR};

    static std::unique_ptr<PeerRequest> Finish(
        std::unique_ptr<PeerRequest>& contract);
    static OTIdentifier GetID(const proto::PeerRequest& contract);
    static bool FinalizeContract(PeerRequest& contract);

    proto::PeerRequest contract(const Lock& lock) const;
    OTIdentifier GetID(const Lock& lock) const override;
    proto::PeerRequest SigVersion(const Lock& lock) const;

    bool update_signature(const Lock& lock) override;

    PeerRequest() = delete;

protected:
    const api::Wallet& wallet_;

    virtual proto::PeerRequest IDVersion(const Lock& lock) const;
    bool validate(const Lock& lock) const override;
    bool verify_signature(const Lock& lock, const proto::Signature& signature)
        const override;

    PeerRequest(
        const api::Wallet& wallet,
        const ConstNym& nym,
        const proto::PeerRequest& serialized);
    PeerRequest(
        const api::Wallet& wallet,
        const ConstNym& nym,
        const proto::PeerRequest& serialized,
        const std::string& conditions);
    PeerRequest(
        const api::Wallet& wallet,
        const ConstNym& nym,
        std::uint32_t version,
        const Identifier& recipient,
        const Identifier& serverID,
        const proto::PeerRequestType& type);
    PeerRequest(
        const api::Wallet& wallet,
        const ConstNym& nym,
        std::uint32_t version,
        const Identifier& recipient,
        const Identifier& serverID,
        const std::string& conditions,
        const proto::PeerRequestType& type);

public:
    static std::unique_ptr<PeerRequest> Create(
        const api::Wallet& wallet,
        const ConstNym& nym,
        const proto::PeerRequestType& type,
        const Identifier& unitID,
        const Identifier& serverID);
    static std::unique_ptr<PeerRequest> Create(
        const api::Wallet& wallet,
        const ConstNym& sender,
        const proto::PeerRequestType& type,
        const Identifier& unitID,
        const Identifier& serverID,
        const Identifier& recipient,
        const Identifier& requestID,
        const std::string& txid,
        const Amount& amount);
    static std::unique_ptr<PeerRequest> Create(
        const api::Wallet& wallet,
        const ConstNym& nym,
        const proto::PeerRequestType& type,
        const Identifier& unitID,
        const Identifier& serverID,
        const std::uint64_t& amount,
        const std::string& terms);
    static std::unique_ptr<PeerRequest> Create(
        const api::Wallet& wallet,
        const ConstNym& sender,
        const proto::PeerRequestType& type,
        const proto::ConnectionInfoType connectionType,
        const Identifier& recipient,
        const Identifier& serverID);
    static std::unique_ptr<PeerRequest> Create(
        const api::Wallet& wallet,
        const ConstNym& sender,
        const proto::PeerRequestType& type,
        const proto::SecretType secretType,
        const Identifier& recipient,
        const std::string& primary,
        const std::string& secondary,
        const Identifier& serverID);
    static std::unique_ptr<PeerRequest> Factory(
        const api::Wallet& wallet,
        const ConstNym& nym,
        const proto::PeerRequest& serialized);

    std::string Alias() const override { return Name(); }
    proto::PeerRequest Contract() const;
    const Identifier& Initiator() const { return initiator_; }
    std::string Name() const override;
    const Identifier& Recipient() const { return recipient_; }
    OTData Serialize() const override;
    const proto::PeerRequestType& Type() const { return type_; }
    void SetAlias(const std::string&) override {}

    ~PeerRequest() = default;
};
}  // namespace opentxs
#endif
