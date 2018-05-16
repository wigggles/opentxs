/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_API_CLIENT_IMPLEMENTATION_ISSUER_HPP
#define OPENTXS_API_CLIENT_IMPLEMENTATION_ISSUER_HPP

#include "opentxs/Internal.hpp"

namespace opentxs::api::client::implementation
{
class Issuer : virtual public opentxs::api::client::Issuer, Lockable
{
public:
    operator std::string() const override;

    std::set<OTIdentifier> AccountList(
        const proto::ContactItemType type,
        const Identifier& unitID) const override;
    bool BailmentInitiated(const Identifier& unitID) const override;
    std::vector<BailmentDetails> BailmentInstructions(
        const Identifier& unitID,
        const bool onlyUnused = true) const override;
    std::vector<ConnectionDetails> ConnectionInfo(
        const proto::ConnectionInfoType type) const override;
    bool ConnectionInfoInitiated(
        const proto::ConnectionInfoType type) const override;
    std::set<std::tuple<OTIdentifier, OTIdentifier, bool>> GetRequests(
        const proto::PeerRequestType type,
        const RequestStatus state = RequestStatus::All) const override;
    const Identifier& IssuerID() const override;
    const Identifier& LocalNymID() const override;
    bool Paired() const override;
    const std::string& PairingCode() const override;
    OTIdentifier PrimaryServer() const override;
    std::set<proto::PeerRequestType> RequestTypes() const override;
    proto::Issuer Serialize() const override;
    bool StoreSecretComplete() const override;
    bool StoreSecretInitiated() const override;

    void AddAccount(
        const proto::ContactItemType type,
        const Identifier& unitID,
        const Identifier& accountID) override;
    bool AddReply(
        const proto::PeerRequestType type,
        const Identifier& requestID,
        const Identifier& replyID) override;
    bool AddRequest(
        const proto::PeerRequestType type,
        const Identifier& requestID) override;
    bool RemoveAccount(
        const proto::ContactItemType type,
        const Identifier& unitID,
        const Identifier& accountID) override;
    void SetPaired(const bool paired) override;
    void SetPairingCode(const std::string& code) override;
    bool SetUsed(
        const proto::PeerRequestType type,
        const Identifier& requestID,
        const bool isUsed = true) override;

    ~Issuer();

private:
    typedef std::map<Identifier, std::pair<Identifier, bool>> Workflow;
    typedef std::map<proto::PeerRequestType, Workflow> WorkflowMap;
    typedef std::pair<Identifier, Identifier> UnitAccountPair;

    friend Factory;
    const api::client::Wallet& wallet_;
    std::uint32_t version_{0};
    std::string pairing_code_{""};
    mutable OTFlag paired_;
    const Identifier& nym_id_;
    const Identifier& issuer_id_;
    std::map<proto::ContactItemType, std::set<UnitAccountPair>> account_map_{};
    WorkflowMap peer_requests_{};

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
        const api::client::Wallet& wallet,
        const Identifier& nymID,
        const proto::Issuer& serialized);
    Issuer(
        const api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& issuerID);
    Issuer() = delete;
    Issuer(const Issuer&) = delete;
    Issuer(Issuer&&) = delete;
    Issuer& operator=(const Issuer&) = delete;
    Issuer& operator=(Issuer&&) = delete;
};
}  // namespace opentxs::api::client::implementation
#endif  // OPENTXS_API_CLIENT_IMPLEMENTATION_ISSUER_HPP
