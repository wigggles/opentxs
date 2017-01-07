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

#ifndef OPENTXS_CORE_APP_APP_HPP
#define OPENTXS_CORE_APP_APP_HPP

#include "opentxs/core/util/Common.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <limits>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <tuple>

namespace opentxs
{

class Api;
class AppLoader;
class CryptoEngine;
class Dht;
class Identity;
class OT_API;
class ServerLoader;
class Settings;
class Storage;
class Wallet;
class ZMQ;

/** \brief Singlton class for providing an interface to process-level resources.
 *  \ingroup native
 */
class App
{
public:
    typedef std::function<void()> PeriodicTask;

private:
    friend class AppLoader;
    friend class OT_API;
    friend class ServerLoader;

    /** Last performed, Interval, Task */
    typedef std::tuple<time64_t, time64_t, PeriodicTask> TaskItem;
    typedef std::list<TaskItem> TaskList;

    static App* instance_pointer_;

    bool server_mode_{false};

    std::unique_ptr<Api> api_;
    std::unique_ptr<Settings> config_;
    std::unique_ptr<CryptoEngine> crypto_;
    std::unique_ptr<Dht> dht_;
    std::unique_ptr<Storage> storage_;
    std::unique_ptr<Wallet> contract_manager_;
    std::unique_ptr<class Identity> identity_;
    std::unique_ptr<class ZMQ> zeromq_;

    mutable std::mutex task_list_lock_;
    mutable std::atomic<bool> shutdown_;
    mutable TaskList periodic_task_list;

    std::int64_t nym_publish_interval_{std::numeric_limits<std::int64_t>::max()};
    std::int64_t nym_refresh_interval_{std::numeric_limits<std::int64_t>::max()};
    std::int64_t server_publish_interval_{std::numeric_limits<std::int64_t>::max()};
    std::int64_t server_refresh_interval_{std::numeric_limits<std::int64_t>::max()};
    std::int64_t unit_publish_interval_{std::numeric_limits<std::int64_t>::max()};
    std::int64_t unit_refresh_interval_{std::numeric_limits<std::int64_t>::max()};

    static void Factory(const bool serverMode);
    static void Cleanup();

    explicit App(const bool serverMode);
    App() = delete;
    App(const App&) = delete;
    App(App&&) = delete;
    App& operator=(const App&) = delete;
    App& operator=(App&&) = delete;

    void Init_Api();
    void Init_Config();
    void Init_Contracts();
    void Init_Crypto();
    void Init_Dht();
    void Init_Identity();
    void Init_Periodic();
    void Init_Storage();
    void Init_ZMQ();
    void Init();

    void Periodic();

public:
    static const App& Me();

    Api& API() const;
    Settings& Config() const;
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

    ~App();
};

// Temporary workaround for OT createmint command. Will be removed once
// createmint is incorporated into the server process as a periodic task
class AppLoader
{
public:
    AppLoader() { App::Factory(true); }
    ~AppLoader() { App::Cleanup(); }
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_APP_APP_HPP
