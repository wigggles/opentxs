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

#ifndef OPENTXS_CLIENT_OTME_TOO_HPP
#define OPENTXS_CLIENT_OTME_TOO_HPP

#include "opentxs/core/Proto.hpp"

#include <atomic>
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <tuple>

namespace opentxs
{

class Api;
class MadeEasy;
class Nym;
class OT_API;
class OT_ME;
class OTAPI_Exec;
class Settings;

class OTME_too
{
private:
    friend class Api;

    typedef std::map<proto::ContactItemType, std::string> unitTypeMap;
    typedef std::map<std::string, proto::ContactItemType> typeUnitMap;
    typedef std::tuple<
        std::uint64_t,  // index
        std::string,    // owner nym
        std::string,    // admin password
        std::string,    // notary ID
        unitTypeMap,    // unit IDs
        unitTypeMap,    // account IDs
        bool,           // backup started
        bool,           // connected
        bool,           // rename started
        bool            // done
        > PairedNode;
    typedef std::map<std::string, PairedNode> PairedNodes;
    typedef std::map<
        std::string,
        std::tuple<
            std::string,
            std::string,
            std::string>> ServerNameData;
    typedef std::map<std::string, std::list<std::string>> nymAccountMap;
    typedef std::map<std::string, nymAccountMap> serverNymMap;

    std::recursive_mutex& api_lock_;
    Settings& config_;
    OT_API& ot_api_;
    OTAPI_Exec& exec_;
    const MadeEasy& made_easy_;
    const OT_ME& otme_;

    mutable std::atomic<bool> pairing_;
    mutable std::atomic<bool> refreshing_;
    mutable std::atomic<std::uint64_t> refresh_count_;
    mutable std::mutex pair_initiate_lock_;
    mutable std::mutex pair_lock_;
    mutable std::unique_ptr<std::thread> pairing_thread_;
    mutable std::unique_ptr<std::thread> refresh_thread_;

    PairedNodes paired_nodes_;

    void build_account_list(serverNymMap& output) const;
    bool check_accounts(PairedNode& node);
    bool check_backup(const std::string& bridgeNymID, PairedNode& node);
    bool check_bridge_nym(
        const std::string& bridgeNym,
        PairedNode& node);
    bool check_introduction_server(const std::string& withNym) const;
    bool check_nym_revision(
        const std::string& nymID,
        const std::string& server) const;
    void check_server_names();
    bool check_server_registration(
        const std::string& nym,
        const std::string& server,
        const bool force,
        const bool publish) const;
    bool download_nym(
        const std::string& localNym,
        const std::string& remoteNym,
        const std::string& server = "") const;
    std::uint64_t extract_assets(
        const proto::ContactData& claims,
        PairedNode& node);
    std::string extract_server(const proto::ContactData& claims) const;
    std::string extract_server_name(const std::string& serverNymID) const;
    void fill_existing_accounts(
        const std::string& nym,
        std::uint64_t& have,
        typeUnitMap& neededUnits,
        unitTypeMap& neededAccounts,
        PairedNode& node);
    std::unique_ptr<PairedNode> find_node(
        const std::string& identifier,
        std::string& bridgeNymId) const;
    std::unique_ptr<PairedNode> find_node(const std::string& identifier) const;
    std::string get_introduction_server() const;
    bool insert_at_index(
        const std::int64_t index,
        const std::int64_t total,
        const std::string& myNym,
        const std::string& bridgeNym,
        const std::string& password) const;
    void mark_connected(PairedNode& node);
    void mark_finished(const std::string& bridgeNymID);
    void mark_renamed(const std::string& bridgeNymID);
    std::string obtain_account(
        const std::string& nym,
        const std::string& id,
        const std::string& server) const;
    bool obtain_asset_contract(
        const std::string& nym,
        const std::string& id,
        const std::string& server) const;
    bool obtain_assets(
        const std::string& bridgeNym,
    const proto::ContactData& claims,
        PairedNode& node);
    std::unique_ptr<proto::ContactData> obtain_contact_data(
        const std::string& localNym,
        const Nym& remoteNym,
        const std::string& server) const;
    std::shared_ptr<const Nym> obtain_nym(
        const std::string& localNym,
        const std::string& remoteNym,
        const std::string& server) const;
    bool obtain_server_contract(
        const std::string& nym,
        const std::string& server,
        const bool publish) const;
    std::string obtain_server_id(
        const std::string& ownerNym,
        const std::string& bridgeNym,
        const proto::ContactData& claims) const;
    void pair(const std::string& bridgeNymID);
    void pairing_thread();
    void parse_pairing_section(std::uint64_t index);
    bool publish_server_registration(
        const std::string& nymID,
        const std::string& server,
        const bool forcePrimary) const;
    void refresh_thread();
    bool request_connection(
        const std::string& nym,
        const std::string& server,
        const std::string& bridgeNymID,
        const std::int64_t type) const;
    bool send_backup(const std::string& bridgeNymID, PairedNode& node) const;
    void send_server_name(
        const std::string& nym,
        const std::string& server,
        const std::string& password,
        const std::string& name) const;
    void set_server_names(const ServerNameData& servers);
    std::int64_t scan_incomplete_pairing(const std::string& bridgeNym);
    void scan_pairing();
    void Shutdown() const;
    bool update_accounts(const PairedNode& node);
    bool update_assets(PairedNode& node);
    bool update_notary(const std::string& id, PairedNode& node);
    bool update_nym_revision(
        const std::string& nymID,
        const std::string& server) const;
    proto::ContactItemType validate_unit(const std::int64_t type);
    void yield() const;

    OTME_too(
        std::recursive_mutex& lock,
        Settings& config,
        OT_API& otapi,
        OTAPI_Exec& exec,
        const MadeEasy& madeEasy,
        const OT_ME& otme);
    OTME_too() = delete;
    OTME_too(const OTME_too&) = delete;
    OTME_too(const OTME_too&&) = delete;
    OTME_too& operator=(const OTME_too&) = delete;
    OTME_too& operator=(const OTME_too&&) = delete;

public:
    std::string GetPairedServer(const std::string& identifier) const;
    bool NodeRenamed(const std::string& identifier) const;
    std::uint64_t PairedNodeCount() const;
    bool PairingComplete(const std::string& identifier) const;
    bool PairingStarted(const std::string& identifier) const;
    bool PairingSuccessful(const std::string& identifier) const;
    bool PairNode(
        const std::string& myNym,
        const std::string& bridgeNym,
        const std::string& password);
    void Refresh(const std::string& wallet = "");
    std::uint64_t RefreshCount() const;
    bool RegisterNym(
        const std::string& nymID,
        const std::string& server,
        const bool setContactData) const;
    bool RequestConnection(
        const std::string& nym,
        const std::string& node,
        const std::int64_t type) const;
    std::string SetIntroductionServer(const std::string& contract) const;
    void UpdatePairing(const std::string& wallet = "");

    ~OTME_too();
};
} // namespace opentxs

#endif // OPENTXS_CLIENT_OTME_TOO_HPP
