// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_SERVER_MANAGER_HPP
#define OPENTXS_API_SERVER_MANAGER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/api/Core.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{
namespace api
{
namespace server
{
class Manager : virtual public api::Core
{
public:
    /** Drop a specified number of incoming requests for testing purposes */
    EXPORT virtual void DropIncoming(const int count) const = 0;
    /** Drop a specified number of outgoing replies for testing purposes */
    EXPORT virtual void DropOutgoing(const int count) const = 0;
    EXPORT virtual const std::string GetCommandPort() const = 0;
    EXPORT virtual const std::string GetDefaultBindIP() const = 0;
    EXPORT virtual const std::string GetEEP() const = 0;
    EXPORT virtual const std::string GetExternalIP() const = 0;
    EXPORT virtual const std::string GetInproc() const = 0;
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
    EXPORT virtual const Identifier& NymID() const = 0;
#if OT_CASH
    EXPORT virtual void ScanMints() const = 0;
#endif  // OT_CASH
#if OT_CASH
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
#endif
