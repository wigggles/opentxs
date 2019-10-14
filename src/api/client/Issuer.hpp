// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class Issuer final : virtual public opentxs::api::client::Issuer, Lockable
{
public:
    std::string toString(const PasswordPrompt& reason) const final;
    //  operator std::string() const final;

    std::set<OTIdentifier> AccountList(
        const proto::ContactItemType type,
        const identifier::UnitDefinition& unitID) const final;
    bool BailmentInitiated(
        const identifier::UnitDefinition& unitID) const final;
    std::vector<BailmentDetails> BailmentInstructions(
        const identifier::UnitDefinition& unitID,
        const bool onlyUnused = true) const final;
    std::vector<ConnectionDetails> ConnectionInfo(
        const proto::ConnectionInfoType type) const final;
    bool ConnectionInfoInitiated(
        const proto::ConnectionInfoType type) const final;
    std::set<std::tuple<OTIdentifier, OTIdentifier, bool>> GetRequests(
        const proto::PeerRequestType type,
        const RequestStatus state = RequestStatus::All) const final;
    const identifier::Nym& IssuerID() const final { return issuer_id_; }
    const identifier::Nym& LocalNymID() const final { return nym_id_; }
    bool Paired() const final;
    const std::string& PairingCode() const final;
    OTServerID PrimaryServer(const PasswordPrompt& reason) const final;
    std::set<proto::PeerRequestType> RequestTypes() const final;
    proto::Issuer Serialize() const final;
    bool StoreSecretComplete() const final;
    bool StoreSecretInitiated() const final;

    void AddAccount(
        const proto::ContactItemType type,
        const identifier::UnitDefinition& unitID,
        const Identifier& accountID) final;
    bool AddReply(
        const proto::PeerRequestType type,
        const Identifier& requestID,
        const Identifier& replyID) final;
    bool AddRequest(
        const proto::PeerRequestType type,
        const Identifier& requestID) final;
    bool RemoveAccount(
        const proto::ContactItemType type,
        const identifier::UnitDefinition& unitID,
        const Identifier& accountID) final;
    void SetPaired(const bool paired) final;
    void SetPairingCode(const std::string& code) final;
    bool SetUsed(
        const proto::PeerRequestType type,
        const Identifier& requestID,
        const bool isUsed = true) final;

    ~Issuer() final;

private:
    typedef std::map<OTIdentifier, std::pair<OTIdentifier, bool>> Workflow;
    typedef std::map<proto::PeerRequestType, Workflow> WorkflowMap;
    typedef std::pair<OTUnitID, OTIdentifier> UnitAccountPair;

    friend opentxs::Factory;
    const api::Wallet& wallet_;
    VersionNumber version_{0};
    std::string pairing_code_{""};
    mutable OTFlag paired_;
    const OTNymID nym_id_;
    const OTNymID issuer_id_;
    std::map<proto::ContactItemType, std::set<UnitAccountPair>> account_map_;
    WorkflowMap peer_requests_;

    std::pair<bool, Workflow::iterator> find_request(
        const Lock& lock,
        const proto::PeerRequestType type,
        const Identifier& requestID);
    std::set<std::tuple<OTIdentifier, OTIdentifier, bool>> get_requests(
        const Lock& lock,
        const proto::PeerRequestType type,
        const RequestStatus state = RequestStatus::All) const;

    bool add_request(
        const Lock& lock,
        const proto::PeerRequestType type,
        const Identifier& requestID,
        const Identifier& replyID);

    Issuer(
        const api::Wallet& wallet,
        const identifier::Nym& nymID,
        const proto::Issuer& serialized);
    Issuer(
        const api::Wallet& wallet,
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID);
    Issuer() = delete;
    Issuer(const Issuer&) = delete;
    Issuer(Issuer&&) = delete;
    Issuer& operator=(const Issuer&) = delete;
    Issuer& operator=(Issuer&&) = delete;
};
}  // namespace opentxs::api::client::implementation
