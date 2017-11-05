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

/** \defgroup native Native API */

#ifndef OPENTXS_CORE_API_OT_HPP
#define OPENTXS_CORE_API_OT_HPP

#include "opentxs/Version.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/util/Common.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>

namespace opentxs
{

class CryptoEngine;
class Signals;
class SymmetricKey;

namespace api
{

class Activity;
class Api;
class Blockchain;
class ContactManager;
class Dht;
class Identity;
class Server;
class Settings;
class Storage;
class Wallet;
class ZMQ;

}  // namespace api

/** \brief Singlton class for providing an interface to process-level resources.
 *  \ingroup native
 */
class OT
{
public:
    typedef std::function<void()> PeriodicTask;

    /** Native API accessor
     *
     *  Returns a reference to the native API singleton after it has been
     *  initialized.
     */
    static const OT& App();

    /** OT shutdown method
     *
     *  Call this when the application is closing, after all OT operations
     *  are complete.
     */
    static void Cleanup();
    static void ClientFactory(
        const std::chrono::seconds gcInterval = std::chrono::seconds(0),
        const std::string& storagePlugin = "",
        const std::string& backupDirectory = "",
        const std::string& encryptedDirectory = "");
    static void ClientFactory(
        const bool recover,
        const std::string& words,
        const std::string& passphrase,
        const std::chrono::seconds gcInterval = std::chrono::seconds(0),
        const std::string& storagePlugin = "",
        const std::string& backupDirectory = "",
        const std::string& encryptedDirectory = "");
    static void Join();
    static void ServerFactory(
        const std::map<std::string, std::string>& serverArgs,
        const std::chrono::seconds gcInterval = std::chrono::seconds(0),
        const std::string& storagePlugin = "",
        const std::string& backupDirectory = "");

    api::Activity& Activity() const;
    api::Api& API() const;
    api::Blockchain& Blockchain() const;
    api::Settings& Config(const std::string& path = std::string("")) const;
    api::ContactManager& Contact() const;
    CryptoEngine& Crypto() const;
    api::Storage& DB() const;
    api::Dht& DHT() const;
    void HandleSignals() const;
    api::Identity& Identity() const;

    /** Adds a task to the periodic task list with the specified interval. By
     * default, schedules for immediate execution. */
    void Schedule(
        const time64_t& interval,
        const PeriodicTask& task,
        const time64_t& last = 0) const;
    const api::Server& Server() const;
    bool ServerMode() const;
    api::Wallet& Wallet() const;
    api::ZMQ& ZMQ() const;

private:
    /** Last performed, Interval, Task */
    typedef std::tuple<time64_t, time64_t, PeriodicTask> TaskItem;
    typedef std::list<TaskItem> TaskList;
    typedef std::map<std::string, std::unique_ptr<api::Settings>> ConfigMap;

    static OT* instance_pointer_;

    const bool recover_{false};
    const bool server_mode_{false};
    std::int64_t nym_publish_interval_{0};
    std::int64_t nym_refresh_interval_{0};
    std::int64_t server_publish_interval_{0};
    std::int64_t server_refresh_interval_{0};
    std::int64_t unit_publish_interval_{0};
    std::int64_t unit_refresh_interval_{0};
    const std::chrono::seconds gc_interval_{0};
    const OTPassword word_list_{};
    const OTPassword passphrase_{};
    const std::string primary_storage_plugin_{};
    const std::string archive_directory_{};
    std::string encrypted_directory_{};
    mutable std::mutex config_lock_;
    mutable std::mutex task_list_lock_;
    mutable std::mutex signal_handler_lock_;
    mutable TaskList periodic_task_list;
    mutable std::atomic<bool> shutdown_{false};
    std::unique_ptr<api::Activity> activity_;
    std::unique_ptr<api::Api> api_;
    std::unique_ptr<api::Blockchain> blockchain_;
    mutable ConfigMap config_;
    std::unique_ptr<api::ContactManager> contacts_;
    std::unique_ptr<CryptoEngine> crypto_;
    std::unique_ptr<api::Dht> dht_;
    std::unique_ptr<api::Identity> identity_;
    std::unique_ptr<api::Storage> storage_;
    std::unique_ptr<api::Wallet> wallet_;
    std::unique_ptr<api::ZMQ> zeromq_;
    std::unique_ptr<std::thread> periodic_;
    std::unique_ptr<SymmetricKey> storage_encryption_key_;
    std::unique_ptr<api::Server> server_;
    mutable std::unique_ptr<Signals> signal_handler_;
    const std::map<std::string, std::string> server_args_{};

    explicit OT(
        const bool recover,
        const std::string& words,
        const std::string& passphrase,
        const bool serverMode,
        const std::chrono::seconds gcInterval,
        const std::string& storagePlugin,
        const std::string& backupDirectory,
        const std::string& encryptedDirectory,
        const std::map<std::string, std::string>& serverArgs);
    OT() = delete;
    OT(const OT&) = delete;
    OT(OT&&) = delete;
    OT& operator=(const OT&) = delete;
    OT& operator=(OT&&) = delete;

    void Init_Activity();
    void Init_Api();
    void Init_Blockchain();
    void Init_Config();
    void Init_Contacts();
    void Init_Contracts();
    void Init_Crypto();
    void Init_Dht();
    void Init_Identity();
    void Init_Log();
    void Init_Periodic();
    void Init_Server();
    void Init_Storage();
    void Init_StorageBackup();
    void Init_ZMQ();
    void Init();
    void Periodic();
    void recover();
    void set_storage_encryption();
    void shutdown();
    void start();

    ~OT() = default;
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_API_OT_HPP
