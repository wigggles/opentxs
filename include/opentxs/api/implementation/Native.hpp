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

#ifndef OPENTXS_CORE_API_IMPLEMENTATION_NATIVE_HPP
#define OPENTXS_CORE_API_IMPLEMENTATION_NATIVE_HPP

#include "opentxs/Version.hpp"

#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/Types.hpp"

#include <atomic>
#include <cstdint>
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
class OT;
class Signals;
class StorageConfig;
class String;
class SymmetricKey;

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace api
{
namespace implementation
{

/** \brief Singlton class for providing an interface to process-level resources.
 *  \ingroup native
 */
class Native : virtual public api::Native
{
public:
    api::Activity& Activity() const override;
    api::Api& API() const override;
    api::Blockchain& Blockchain() const override;
    api::Settings& Config(
        const std::string& path = std::string("")) const override;
    api::ContactManager& Contact() const override;
    api::Crypto& Crypto() const override;
    api::storage::Storage& DB() const override;
    api::network::Dht& DHT() const override;
    void HandleSignals() const override;
    api::Identity& Identity() const override;
    /** Adds a task to the periodic task list with the specified interval. By
     * default, schedules for immediate execution. */
    void Schedule(
        const std::chrono::seconds& interval,
        const PeriodicTask& task,
        const std::chrono::seconds& last =
            std::chrono::seconds(0)) const override;
    const api::Server& Server() const override;
    bool ServerMode() const override;
    api::client::Wallet& Wallet() const override;
    api::network::ZMQ& ZMQ() const override;

private:
    friend class opentxs::OT;

    /** Last performed, Interval, Task */
    typedef std::tuple<time64_t, time64_t, PeriodicTask> TaskItem;
    typedef std::list<TaskItem> TaskList;
    typedef std::map<std::string, std::unique_ptr<api::Settings>> ConfigMap;

    const bool recover_{false};
    const bool server_mode_{false};
    std::int64_t nym_publish_interval_{0};
    std::int64_t nym_refresh_interval_{0};
    std::int64_t server_publish_interval_{0};
    std::int64_t server_refresh_interval_{0};
    std::int64_t unit_publish_interval_{0};
    std::int64_t unit_refresh_interval_{0};
    const std::chrono::seconds gc_interval_{0};
    OTPassword word_list_{};
    OTPassword passphrase_{};
    std::string primary_storage_plugin_{};
    std::string archive_directory_{};
    std::string encrypted_directory_{};
    mutable std::mutex config_lock_;
    mutable std::mutex task_list_lock_;
    mutable std::mutex signal_handler_lock_;
    mutable TaskList periodic_task_list;
    std::atomic<bool>& shutdown_;
    std::unique_ptr<api::Activity> activity_;
    std::unique_ptr<api::Api> api_;
    std::unique_ptr<api::Blockchain> blockchain_;
    mutable ConfigMap config_;
    std::unique_ptr<api::ContactManager> contacts_;
    std::unique_ptr<api::Crypto> crypto_;
    std::unique_ptr<api::network::Dht> dht_;
    std::unique_ptr<api::Identity> identity_;
    std::unique_ptr<api::storage::Storage> storage_;
    std::unique_ptr<api::client::Wallet> wallet_;
    std::unique_ptr<api::network::ZMQ> zeromq_;
    std::unique_ptr<std::thread> periodic_;
    std::unique_ptr<SymmetricKey> storage_encryption_key_;
    std::unique_ptr<api::Server> server_;
    std::unique_ptr<opentxs::network::zeromq::Context> zmq_context_p_;
    opentxs::network::zeromq::Context& zmq_context_;
    mutable std::unique_ptr<Signals> signal_handler_;
    const ArgList server_args_;

    explicit Native(
        const ArgList& args,
        std::atomic<bool>& shutdown,
        const bool recover,
        const bool serverMode,
        const std::chrono::seconds gcInterval);
    Native() = delete;
    Native(const Native&) = delete;
    Native(Native&&) = delete;
    Native& operator=(const Native&) = delete;
    Native& operator=(Native&&) = delete;

    String get_primary_storage_plugin(
        const StorageConfig& config,
        bool& migrate,
        String& previous) const;

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

    ~Native() = default;
};
}  // namespace implementation
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_CORE_API_IMPLEMENTATION_NATIVE_HPP
