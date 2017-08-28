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

class Activity;
class Api;
class AppLoader;
class Blockchain;
class ContactManager;
class CryptoEngine;
class Dht;
class Identity;
class OTAPI_Wrap;
class ServerLoader;
class Settings;
class Storage;
class SymmetricKey;
class Wallet;
class ZMQ;

/** \brief Singlton class for providing an interface to process-level resources.
 *  \ingroup native
 */
class OT
{
public:
    typedef std::function<void()> PeriodicTask;

private:
    friend class AppLoader;
    friend class OTAPI_Wrap;
    friend class ServerLoader;

    /** Last performed, Interval, Task */
    typedef std::tuple<time64_t, time64_t, PeriodicTask> TaskItem;
    typedef std::list<TaskItem> TaskList;
    typedef std::map<std::string, std::unique_ptr<Settings>> ConfigMap;

    static OT* instance_pointer_;

    const bool server_mode_{false};
    std::int64_t nym_publish_interval_{0};
    std::int64_t nym_refresh_interval_{0};
    std::int64_t server_publish_interval_{0};
    std::int64_t server_refresh_interval_{0};
    std::int64_t unit_publish_interval_{0};
    std::int64_t unit_refresh_interval_{0};
    const std::string primary_storage_plugin_{};
    const std::string archive_directory_{};
    const std::string encrypted_directory_{};
    mutable std::mutex config_lock_;
    mutable std::mutex task_list_lock_;
    mutable TaskList periodic_task_list;
    mutable std::atomic<bool> shutdown_;
    std::unique_ptr<class Activity> activity_;
    std::unique_ptr<Api> api_;
    std::unique_ptr<class Blockchain> blockchain_;
    mutable ConfigMap config_;
    std::unique_ptr<ContactManager> contacts_;
    std::unique_ptr<CryptoEngine> crypto_;
    std::unique_ptr<Dht> dht_;
    std::unique_ptr<class Identity> identity_;
    std::unique_ptr<Storage> storage_;
    std::unique_ptr<Wallet> wallet_;
    std::unique_ptr<class ZMQ> zeromq_;
    std::unique_ptr<std::thread> periodic_;
    std::unique_ptr<SymmetricKey> storage_encryption_key_;

    static void Factory(
        const bool serverMode,
        const std::string& storagePlugin = "",
        const std::string& backupDirectory = "",
        const std::string& encryptedDirectory = "");
    static void Cleanup();

    explicit OT(
        const bool serverMode,
        const std::string& storagePlugin,
        const std::string& backupDirectory,
        const std::string& encryptedDirectory);
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
    void Init_Storage();
    void Init_StorageBackup();
    void Init_ZMQ();
    void Init();
    void Periodic();
    void set_storage_encryption();
    void Shutdown();

    ~OT() = default;

public:
    static const OT& App();

    class Activity& Activity() const;
    Api& API() const;
    class Blockchain& Blockchain() const;
    Settings& Config(const std::string& path = std::string("")) const;
    ContactManager& Contact() const;
    Wallet& Contract() const;
    CryptoEngine& Crypto() const;
    Storage& DB() const;
    Dht& DHT() const;
    class Identity& Identity() const;
    class ZMQ& ZMQ() const;

    /** Adds a task to the periodic task list with the specified interval. By
     * default, schedules for immediate execution. */
    void Schedule(
        const time64_t& interval,
        const PeriodicTask& task,
        const time64_t& last = 0) const;
};

// Temporary workaround for OT createmint command. Will be removed once
// createmint is incorporated into the server process as a periodic task
class AppLoader
{
public:
    AppLoader() { OT::Factory(true); }
    ~AppLoader() { OT::Cleanup(); }
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_API_OT_HPP
