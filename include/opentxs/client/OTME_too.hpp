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

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"

#include <atomic>
#include <cstdint>
#include <ctime>
#include <functional>
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
class ContactGroup;
class ContactManager;
class CryptoEncodingEngine;
class Identity;
class MadeEasy;
class Nym;
class OT_API;
class OT_ME;
class OTAPI_Exec;
class Settings;
class Wallet;

class OTME_too
{
private:
    friend class Api;

    typedef std::unique_lock<std::mutex> Lock;
    typedef std::unique_lock<std::recursive_mutex> rLock;
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
        >
        PairedNode;
    typedef std::map<std::string, PairedNode> PairedNodes;
    typedef std::
        map<std::string, std::tuple<std::string, std::string, std::string>>
            ServerNameData;
    typedef std::map<std::string, std::list<std::string>> nymAccountMap;
    typedef std::map<
        std::string,
        std::pair<
            nymAccountMap,          // accounts to refresh
            std::list<std::string>  // ids for checkNym
            >>
        serverTaskMap;
    typedef std::tuple<
        std::atomic<bool>,             // running
        std::unique_ptr<std::thread>,  // thread handle
        std::atomic<bool>              // exit status
        >
        Thread;
    typedef std::function<void(std::atomic<bool>*, std::atomic<bool>*)>
        BackgroundThread;
    typedef std::map<std::pair<std::string, std::string>, std::atomic<bool>>
        MessagabilityMap;

    class Cleanup
    {
    private:
        std::atomic<bool>& run_;
        Cleanup() = delete;

    public:
        Cleanup(std::atomic<bool>& run);
        ~Cleanup();
    };

    static const std::string DEFAULT_INTRODUCTION_SERVER;

    std::recursive_mutex& api_lock_;
    Settings& config_;
    ContactManager& contacts_;
    OT_API& ot_api_;
    OTAPI_Exec& exec_;
    const MadeEasy& made_easy_;
    const OT_ME& otme_;
    Wallet& wallet_;
    CryptoEncodingEngine& encoding_;
    Identity& identity_;

    mutable std::atomic<bool> pairing_{false};
    mutable std::atomic<bool> refreshing_{false};
    mutable std::atomic<bool> shutdown_{false};
    mutable std::atomic<bool> introduction_server_set_{false};
    mutable std::atomic<std::uint64_t> refresh_count_{0};
    mutable std::mutex pair_initiate_lock_;
    mutable std::mutex pair_lock_;
    mutable std::mutex thread_lock_;
    mutable std::mutex messagability_lock_;
    mutable std::mutex refresh_interval_lock_;
    mutable std::mutex introduction_server_lock_;
    mutable std::unique_ptr<std::thread> pairing_thread_;
    mutable std::unique_ptr<std::thread> refresh_thread_;
    std::map<Identifier, Thread> threads_;
    MessagabilityMap messagability_map_;
    PairedNodes paired_nodes_;
    mutable std::map<std::string, std::uint64_t> refresh_interval_;
    mutable Identifier introduction_server_{};

    Identifier add_background_thread(BackgroundThread thread);
    void add_checknym_tasks(const nymAccountMap nyms, serverTaskMap& tasks);
    void build_account_list(serverTaskMap& output) const;
    void build_nym_list(std::list<std::string>& output) const;
    Messagability can_message(
        const std::string& sender,
        const std::string& recipient,
        std::string& server);
    bool check_accounts(PairedNode& node);
    bool check_backup(const std::string& bridgeNymID, PairedNode& node);
    bool check_bridge_nym(const std::string& bridgeNym, PairedNode& node);
    bool check_pairing(const std::string& bridgeNym, std::string& password);
    bool check_introduction_server(const std::string& withNym) const;
    bool check_nym_revision(const std::string& nymID, const std::string& server)
        const;
    void check_server_names();
    bool check_server_registration(
        const std::string& nym,
        const std::string& server,
        const bool force,
        const bool publish) const;
    void clean_background_threads();
    bool download_nym(
        const std::string& localNym,
        const std::string& remoteNym,
        const std::string& server = "") const;
    void establish_mailability(
        const std::string& sender,
        const std::string& recipient,
        std::atomic<bool>* running,
        std::atomic<bool>* exitStatus);
    std::uint64_t extract_assets(
        const proto::ContactData& claims,
        PairedNode& node);
    std::string extract_server_name(const std::string& serverNymID) const;
    std::set<std::string> extract_message_servers(
        const std::string& nymID) const;
    void fill_existing_accounts(
        const std::string& nym,
        std::uint64_t& have,
        typeUnitMap& neededUnits,
        unitTypeMap& neededAccounts,
        PairedNode& node);
    void fill_paired_servers(
        std::set<std::string>& serverList,
        std::list<std::pair<std::string, std::string>>& serverNymList) const;
    void fill_registered_servers(
        std::string& introductionNym,
        std::set<std::string>& serverList,
        std::list<std::pair<std::string, std::string>>& serverNymList) const;
    void fill_viable_servers(
        std::list<std::pair<std::string, std::string>>& servers) const;
    std::unique_ptr<PairedNode> find_node(
        const std::string& identifier,
        std::string& bridgeNymId) const;
    std::unique_ptr<PairedNode> find_node(const std::string& identifier) const;
    void find_nym(
        const std::string& nymID,
        const std::string& serverIDhint,
        std::atomic<bool>* running,
        std::atomic<bool>* exitStatus) const;
    void find_nym_if_necessary(const std::string& nymID, Identifier& task);
    void find_server(
        const std::string& serverID,
        std::atomic<bool>* running,
        std::atomic<bool>* exitStatus) const;
    std::string get_introduction_server(const Lock& lock) const;
    std::time_t get_time(const std::string& alias) const;
    std::string import_default_introduction_server(const Lock& lock) const;
    bool insert_at_index(
        const std::int64_t index,
        const std::int64_t total,
        const std::string& myNym,
        const std::string& bridgeNym,
        std::string& password) const;
    std::uint64_t legacy_contact_count() const;
    void load_introduction_server() const;
    void mailability(const std::string& sender, const std::string& recipient);
    void mark_connected(PairedNode& node);
    void mark_finished(const std::string& bridgeNymID);
    void mark_renamed(const std::string& bridgeNymID);
    void message_contact(
        const std::string& server,
        const std::string& senderNymID,
        const std::string& contactID,
        const std::string& message,
        std::atomic<bool>* running,
        std::atomic<bool>* exitStatus);
    bool need_to_refresh(const std::string& serverID);
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
        const Nym& bridgeNym) const;
    std::string obtain_server_id(const std::string& nym) const;
    void pair(const std::string& bridgeNymID);
    void pairing_thread();
    void parse_contact_section(const std::uint64_t index);
    void parse_pairing_section(std::uint64_t index);
    bool publish_server_registration(
        const std::string& nymID,
        const std::string& server,
        const bool forcePrimary) const;
    void refresh_contacts(nymAccountMap& nymsToCheck);
    void refresh_thread();
    void register_nym(
        const std::string& nym,
        const std::string& server,
        std::atomic<bool>* running,
        std::atomic<bool>* exitStatus);
    bool request_connection(
        const std::string& nym,
        const std::string& server,
        const std::string& bridgeNymID,
        const std::int64_t type) const;
    void resend_bailment(
        const Identifier& nymID,
        const proto::PeerRequest& request) const;
    void resend_bailment_notification(
        const Identifier& nymID,
        const proto::PeerRequest& request) const;
    void resend_connection_info(
        const Identifier& nymID,
        const proto::PeerRequest& request) const;
    void resend_outbailment(
        const Identifier& nymID,
        const proto::PeerRequest& request) const;
    void resend_store_secret(
        const Identifier& nymID,
        const proto::PeerRequest& request) const;
    void resend_peer_request(
        const Identifier& nymID,
        const Identifier& requestID) const;
    void resend_peer_requests() const;
    bool send_backup(const std::string& bridgeNymID, PairedNode& node) const;
    bool send_server_name(
        const std::string& nym,
        const std::string& server,
        const std::string& password,
        const std::string& name) const;
    std::string set_introduction_server(
        const Lock& lock,
        const std::string& contract) const;
    void set_server_names(const ServerNameData& servers);
    void scan_contacts();
    std::int64_t scan_incomplete_pairing(const std::string& bridgeNym);
    void scan_pairing();
    void Shutdown();
    std::set<std::string> unique_servers(const ContactGroup& group) const;
    bool update_accounts(const PairedNode& node);
    bool update_assets(PairedNode& node);
    bool update_notary(const std::string& id, PairedNode& node);
    bool update_nym_revision(
        const std::string& nymID,
        const std::string& server) const;
    proto::ContactItemType validate_unit(const std::int64_t type);
    bool yield() const;
    bool verify_lock(const Lock& lock, const std::mutex& mutex) const;

    OTME_too(
        std::recursive_mutex& lock,
        Settings& config,
        ContactManager& contacts,
        OT_API& otapi,
        OTAPI_Exec& exec,
        const MadeEasy& madeEasy,
        const OT_ME& otme,
        Wallet& wallet,
        CryptoEncodingEngine& encoding,
        Identity& identity);
    OTME_too() = delete;
    OTME_too(const OTME_too&) = delete;
    OTME_too(OTME_too&&) = delete;
    OTME_too& operator=(const OTME_too&) = delete;
    OTME_too& operator=(OTME_too&&) = delete;

public:
    bool AcceptIncoming(
        const Identifier& nymID,
        const Identifier& accountID,
        const Identifier& serverID);
    Messagability CanMessage(
        const std::string& senderNymID,
        const std::string& recipientContactID);
    Identifier FindNym(const std::string& nymID, const std::string& serverHint);
    Identifier FindServer(const std::string& serverID);
    const Identifier& GetIntroductionServer() const;
    std::string GetPairedServer(const std::string& identifier) const;
    std::string ImportNym(const std::string& armored) const;
    Identifier MessageContact(
        const std::string& senderNymID,
        const std::string& contactID,
        const std::string& message);
    bool NodeRenamed(const std::string& identifier) const;
    std::uint64_t PairedNodeCount() const;
    bool PairingComplete(const std::string& identifier) const;
    bool PairingStarted(const std::string& identifier) const;
    std::string PairingStatus(const std::string& identifier) const;
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
    Identifier RegisterNym_async(
        const std::string& nymID,
        const std::string& server,
        const bool setContactData);
    bool RequestConnection(
        const std::string& nym,
        const std::string& node,
        const std::int64_t type) const;
    void SetInterval(const std::string& server, const std::uint64_t interval)
        const;
    std::string SetIntroductionServer(const std::string& contract) const;
    ThreadStatus Status(const Identifier& thread);
    void UpdatePairing(const std::string& wallet = "");

    ~OTME_too();
};
}  // namespace opentxs

#endif  // OPENTXS_CLIENT_OTME_TOO_HPP
