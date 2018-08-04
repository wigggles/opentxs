// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_SERVER_HPP
#define OPENTXS_API_SERVER_HPP

#include "opentxs/Forward.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{
namespace api
{
namespace server
{
class Manager
{
public:
    EXPORT virtual const network::Dht& DHT() const = 0;
    EXPORT virtual const api::Factory& Factory() const = 0;
    EXPORT virtual const std::string GetCommandPort() const = 0;
    EXPORT virtual const std::string GetDefaultBindIP() const = 0;
    EXPORT virtual const std::string GetEEP() const = 0;
    EXPORT virtual const std::string GetExternalIP() const = 0;
    EXPORT virtual const std::string GetListenCommand() const = 0;
    EXPORT virtual const std::string GetListenNotify() const = 0;
    EXPORT virtual const std::string GetOnion() const = 0;
#if OT_CASH
    EXPORT virtual std::shared_ptr<Mint> GetPrivateMint(
        const Identifier& unitid,
        std::uint32_t series) const = 0;
    EXPORT virtual std::shared_ptr<const Mint> GetPublicMint(
        const Identifier& unitID) const = 0;
#endif  // OT_CASH
    EXPORT virtual const std::string GetUserName() const = 0;
    EXPORT virtual const std::string GetUserTerms() const = 0;
    EXPORT virtual const Identifier& ID() const = 0;
    EXPORT virtual int Instance() const = 0;
    EXPORT virtual const Identifier& NymID() const = 0;
#if OT_CASH
    EXPORT virtual void ScanMints() const = 0;
#endif  // OT_CASH
    /** Adds a task to the periodic task list with the specified interval. By
     * default, schedules for immediate execution. */
    EXPORT virtual void Schedule(
        const std::chrono::seconds& interval,
        const opentxs::PeriodicTask& task,
        const std::chrono::seconds& last = std::chrono::seconds(0)) const = 0;
#if OT_CRYPTO_WITH_BIP39
    EXPORT virtual const api::HDSeed& Seeds() const = 0;
#endif
#if OT_CASH
    EXPORT virtual const storage::Storage& Storage() const = 0;
    EXPORT virtual void UpdateMint(const Identifier& unitID) const = 0;
#endif  // OT_CASH

    EXPORT virtual void Start() = 0;

    EXPORT virtual ~Manager() = default;

protected:
    Manager() = default;

private:
    Manager(const Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(const Manager&) = delete;
    Manager& operator=(Manager&&) = delete;
};
}  // namespace server
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_SERVER_HPP
