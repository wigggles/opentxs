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

#include "opentxs/Version.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

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

#define DEFAULT_PROCESS_INBOX_ITEMS 5

namespace opentxs
{
class ContactGroup;
class Nym;
class OT_API;
class OT_ME;
class OTAPI_Exec;
class ServerContext;

namespace api
{
class ContactManager;
class Identity;
class Settings;

namespace client
{
class ServerAction;
class Wallet;
}  // namespace client

namespace implementation
{
class Api;
}  // namespace implementation

namespace crypto
{
class Encode;
}  // namespace crypto
}  // namespace api

class OTME_too
{
public:
    bool AcceptIncoming(
        const Identifier& nymID,
        const Identifier& accountID,
        const Identifier& serverID,
        const std::size_t max = DEFAULT_PROCESS_INBOX_ITEMS) const;
    Messagability CanMessage(
        const std::string& senderNymID,
        const std::string& recipientContactID) const;
    Identifier FindNym(const std::string& nymID, const std::string& serverHint)
        const;
    Identifier FindServer(const std::string& serverID) const;
    const Identifier& GetIntroductionServer() const;
    std::string ImportNym(const std::string& armored) const;
    Identifier MessageContact(
        const std::string& senderNymID,
        const std::string& contactID,
        const std::string& message) const;
    void Refresh(const std::string& wallet = "") const;
    std::uint64_t RefreshCount() const;
    void RegisterIntroduction(const Identifier& nymID) const;
    bool RegisterNym(
        const std::string& nymID,
        const std::string& server,
        const bool setContactData) const;
    Identifier RegisterNym_async(
        const std::string& nymID,
        const std::string& server,
        const bool setContactData) const;
    void SetInterval(const std::string& server, const std::uint64_t interval)
        const;
    std::string SetIntroductionServer(const std::string& contract) const;
    ThreadStatus Status(const Identifier& thread) const;

    ~OTME_too();

private:
    friend class api::implementation::Api;

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
    const api::Settings& config_;
    const api::ContactManager& contacts_;
    const OT_API& ot_api_;
    const OTAPI_Exec& exec_;
    const OT_ME& otme_;
    const api::client::ServerAction& action_;
    const api::client::Wallet& wallet_;
    const api::crypto::Encode& encoding_;
    const api::Identity& identity_;

    mutable std::atomic<bool> refreshing_{false};
    mutable std::atomic<bool> shutdown_{false};
    mutable std::atomic<bool> introduction_server_set_{false};
    mutable std::atomic<bool> need_server_nyms_{false};
    mutable std::atomic<std::uint64_t> refresh_count_{0};
    mutable std::mutex thread_lock_;
    mutable std::mutex messagability_lock_;
    mutable std::mutex refresh_interval_lock_;
    mutable std::mutex introduction_server_lock_;
    mutable std::unique_ptr<std::thread> refresh_thread_;
    mutable std::map<Identifier, Thread> threads_;
    mutable MessagabilityMap messagability_map_;
    mutable std::map<std::string, std::uint64_t> refresh_interval_;
    mutable Identifier introduction_server_{};

    std::pair<bool, std::size_t> accept_incoming(
        const rLock& lock,
        const std::size_t max,
        const Identifier& accountID,
        ServerContext& context) const;
    Identifier add_background_thread(BackgroundThread thread) const;
    void add_checknym_tasks(const nymAccountMap nyms, serverTaskMap& tasks)
        const;
    void build_account_list(serverTaskMap& output) const;
    void build_nym_list(std::list<std::string>& output) const;
    Messagability can_message(
        const std::string& sender,
        const std::string& recipient,
        std::string& server) const;
    bool check_nym_revision(const std::string& nymID, const std::string& server)
        const;
    bool check_server_registration(
        const std::string& nym,
        const std::string& server,
        const bool force,
        const bool publish) const;
    void clean_background_threads();
    bool do_i_download_server_nym() const;
    bool download_nym(
        const std::string& localNym,
        const std::string& remoteNym,
        const std::string& server = "") const;
    void establish_mailability(
        const std::string& sender,
        const std::string& recipient,
        std::atomic<bool>* running,
        std::atomic<bool>* exitStatus) const;
    std::set<std::string> extract_message_servers(
        const std::string& nymID) const;
    void fill_registered_servers(
        std::string& introductionNym,
        std::set<std::string>& serverList,
        std::list<std::pair<std::string, std::string>>& serverNymList) const;
    void fill_viable_servers(
        std::list<std::pair<std::string, std::string>>& servers) const;
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
    void get_admin(
        const Identifier& nymID,
        const Identifier& serverID,
        const std::string& password) const;
    std::string get_introduction_server(const Lock& lock) const;
    std::time_t get_time(const std::string& alias) const;
    std::string import_default_introduction_server(const Lock& lock) const;
    std::uint64_t legacy_contact_count() const;
    void load_introduction_server() const;
    void mailability(const std::string& sender, const std::string& recipient)
        const;
    void message_contact(
        const std::string& server,
        const std::string& senderNymID,
        const std::string& contactID,
        const std::string& message,
        std::atomic<bool>* running,
        std::atomic<bool>* exitStatus) const;
    bool need_to_refresh(const std::string& serverID) const;
    std::string obtain_server_id(
        const std::string& ownerNym,
        const Nym& bridgeNym) const;
    std::string obtain_server_id(const std::string& nym) const;
    void parse_contact_section(const std::uint64_t index);
    bool publish_server_registration(
        const std::string& nymID,
        const std::string& server,
        const bool forcePrimary) const;
    void refresh_contacts(nymAccountMap& nymsToCheck) const;
    void refresh_thread() const;
    void register_nym(
        const std::string& nym,
        const std::string& server,
        std::atomic<bool>* running,
        std::atomic<bool>* exitStatus) const;
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
    std::string set_introduction_server(
        const Lock& lock,
        const std::string& contract) const;
    void scan_contacts();
    void Shutdown();
    std::set<std::string> unique_servers(const ContactGroup& group) const;
    bool update_nym_revision(
        const std::string& nymID,
        const std::string& server) const;
    bool yield() const;
    bool verify_lock(const Lock& lock, const std::mutex& mutex) const;

    OTME_too(
        std::recursive_mutex& lock,
        const api::Settings& config,
        const api::ContactManager& contacts,
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const OT_ME& otme,
        const api::client::ServerAction& action,
        const api::client::Wallet& wallet,
        const api::crypto::Encode& encoding,
        const api::Identity& identity);
    OTME_too() = delete;
    OTME_too(const OTME_too&) = delete;
    OTME_too(OTME_too&&) = delete;
    OTME_too& operator=(const OTME_too&) = delete;
    OTME_too& operator=(OTME_too&&) = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CLIENT_OTME_TOO_HPP
