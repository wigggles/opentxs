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

#ifndef OPENTXS_API_CLIENT_IMPLEMENTATION_PAIR_HPP
#define OPENTXS_API_CLIENT_IMPLEMENTATION_PAIR_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/api/client/Pair.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/UniqueQueue.hpp"
#include "opentxs/Proto.hpp"

#include <atomic>
#include <memory>
#include <thread>
#include <tuple>

namespace opentxs::api::client::implementation
{
class Pair : virtual public opentxs::api::client::Pair, Lockable
{
public:
    bool AddIssuer(
        const Identifier& localNymID,
        const Identifier& issuerNymID,
        const std::string& pairingCode) const override;
    std::string IssuerDetails(
        const Identifier& localNymID,
        const Identifier& issuerNymID) const override;
    std::set<Identifier> IssuerList(
        const Identifier& localNymID,
        const bool onlyTrusted) const override;
    void Update() const override;

    ~Pair();

private:
    class Cleanup
    {
    private:
        Flag& run_;
        Cleanup() = delete;

    public:
        Cleanup(Flag& run);
        ~Cleanup();
    };

    enum class Status : std::uint8_t {
        Error = 0,
        Started = 1,
        Registered = 2,
    };

    friend class api::implementation::Api;
    /// local nym id, issuer nym id
    typedef std::pair<Identifier, Identifier> IssuerID;

    const Flag& running_;
    const api::client::Sync& sync_;
    const client::ServerAction& action_;
    const client::Wallet& wallet_;
    const opentxs::OT_API& ot_api_;
    const opentxs::OTAPI_Exec& exec_;
    const opentxs::network::zeromq::Context& zmq_;
    mutable std::mutex peer_lock_{};
    mutable std::mutex status_lock_{};
    mutable OTFlag pairing_;
    mutable std::atomic<std::uint64_t> last_refresh_{0};
    mutable std::unique_ptr<std::thread> pairing_thread_{nullptr};
    mutable std::unique_ptr<std::thread> refresh_thread_{nullptr};
    mutable std::map<IssuerID, std::pair<Status, bool>> pair_status_{};
    mutable UniqueQueue<bool> update_;
    OTZMQPublishSocket pair_event_;
    OTZMQPublishSocket pending_bailment_;

    void check_pairing() const;
    void check_refresh() const;
    std::map<Identifier, std::set<Identifier>> create_issuer_map() const;
    std::pair<bool, Identifier> get_connection(
        const Identifier& localNymID,
        const Identifier& issuerNymID,
        const Identifier& serverID,
        const proto::ConnectionInfoType type) const;
    std::pair<bool, Identifier> initiate_bailment(
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& issuerID,
        const Identifier& unitID) const;
    void process_connection_info(
        const Lock& lock,
        const Identifier& nymID,
        const proto::PeerReply& reply) const;
    void process_peer_replies(const Lock& lock, const Identifier& nymID) const;
    void process_peer_requests(const Lock& lock, const Identifier& nymID) const;
    void process_pending_bailment(
        const Lock& lock,
        const Identifier& nymID,
        const proto::PeerRequest& request) const;
    void process_request_bailment(
        const Lock& lock,
        const Identifier& nymID,
        const proto::PeerReply& reply) const;
    void process_request_outbailment(
        const Lock& lock,
        const Identifier& nymID,
        const proto::PeerReply& reply) const;
    void process_store_secret(
        const Lock& lock,
        const Identifier& nymID,
        const proto::PeerReply& reply) const;
    void queue_nym_download(
        const Identifier& localNymID,
        const Identifier& targetNymID) const;
    void queue_nym_registration(
        const Identifier& nymID,
        const Identifier& serverID,
        const bool setData) const;
    void queue_server_contract(
        const Identifier& nymID,
        const Identifier& serverID) const;
    void queue_unit_definition(
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& unitID) const;
    void refresh() const;
    std::pair<bool, Identifier> register_account(
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& unitID) const;
    bool need_registration(
        const Identifier& localNymID,
        const Identifier& serverID) const;
    void state_machine(
        const Identifier& localNymID,
        const Identifier& issuerNymID) const;
    std::pair<bool, Identifier> store_secret(
        const Identifier& localNymID,
        const Identifier& issuerNymID,
        const Identifier& serverID) const;
    void update_pairing() const;
    void update_peer() const;

    Pair(
        const Flag& running,
        const api::client::Sync& sync,
        const client::ServerAction& action,
        const client::Wallet& wallet,
        const opentxs::OT_API& otapi,
        const opentxs::OTAPI_Exec& exec,
        const opentxs::network::zeromq::Context& context);
    Pair() = delete;
    Pair(const Pair&) = delete;
    Pair(Pair&&) = delete;
    Pair& operator=(const Pair&) = delete;
    Pair& operator=(Pair&&) = delete;
};
}  // namespace opentxs::api::client::implementation
#endif  // OPENTXS_API_CLIENT_IMPLEMENTATION_PAIR_HPP
