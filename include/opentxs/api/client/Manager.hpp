// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_MANAGER_HPP
#define OPENTXS_API_CLIENT_MANAGER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/api/Core.hpp"

#include <string>

namespace opentxs
{
namespace api
{
namespace client
{
class Manager : virtual public api::Core
{
public:
    EXPORT virtual const api::client::Activity& Activity() const = 0;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    EXPORT virtual const api::client::Blockchain& Blockchain() const = 0;
#endif
    EXPORT virtual const client::Cash& Cash() const = 0;
    EXPORT virtual const api::client::Contacts& Contacts() const = 0;
    EXPORT virtual const api::Crypto& Crypto() const = 0;
    EXPORT virtual const network::Dht& DHT() const = 0;
    EXPORT virtual const OTAPI_Exec& Exec(
        const std::string& wallet = "") const = 0;
    EXPORT virtual const api::Factory& Factory() const = 0;
    EXPORT virtual int Instance() const = 0;
    EXPORT virtual std::recursive_mutex& Lock(
        const Identifier& nymID,
        const Identifier& serverID) const = 0;
    EXPORT virtual const OT_API& OTAPI(
        const std::string& wallet = "") const = 0;
    EXPORT virtual const client::Pair& Pair() const = 0;
    /** Adds a task to the periodic task list with the specified interval. By
     * default, schedules for immediate execution. */
    EXPORT virtual void Schedule(
        const std::chrono::seconds& interval,
        const opentxs::PeriodicTask& task,
        const std::chrono::seconds& last = std::chrono::seconds(0)) const = 0;
#if OT_CRYPTO_WITH_BIP39
    EXPORT virtual const api::HDSeed& Seeds() const = 0;
#endif
    EXPORT virtual const client::ServerAction& ServerAction() const = 0;
    EXPORT virtual const storage::Storage& Storage() const = 0;
    EXPORT virtual const client::Sync& Sync() const = 0;
    EXPORT virtual const api::client::UI& UI() const = 0;
    EXPORT virtual const api::Wallet& Wallet() const = 0;
    EXPORT virtual const client::Workflow& Workflow() const = 0;
    EXPORT virtual const network::ZMQ& ZMQ() const = 0;

    EXPORT virtual ~Manager() = default;

protected:
    Manager() = default;

private:
    Manager(const Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(const Manager&) = delete;
    Manager& operator=(Manager&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
