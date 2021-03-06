// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_SERVER_MANAGER_HPP
#define OPENTXS_API_SERVER_MANAGER_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <string>

#include "opentxs/api/Core.hpp"

#define OT_MINT_KEY_SIZE_DEFAULT 1536
#define OT_MINT_KEY_SIZE_TEST 288

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
    OPENTXS_EXPORT virtual void DropIncoming(const int count) const = 0;
    /** Drop a specified number of outgoing replies for testing purposes */
    OPENTXS_EXPORT virtual void DropOutgoing(const int count) const = 0;
    OPENTXS_EXPORT virtual std::string GetAdminNym() const = 0;
    OPENTXS_EXPORT virtual std::string GetAdminPassword() const = 0;
    OPENTXS_EXPORT virtual std::string GetCommandPort() const = 0;
    OPENTXS_EXPORT virtual std::string GetDefaultBindIP() const = 0;
    OPENTXS_EXPORT virtual std::string GetEEP() const = 0;
    OPENTXS_EXPORT virtual std::string GetExternalIP() const = 0;
    OPENTXS_EXPORT virtual std::string GetInproc() const = 0;
    OPENTXS_EXPORT virtual std::string GetListenCommand() const = 0;
    OPENTXS_EXPORT virtual std::string GetListenNotify() const = 0;
    OPENTXS_EXPORT virtual std::string GetOnion() const = 0;
#if OT_CASH
    OPENTXS_EXPORT virtual std::shared_ptr<blind::Mint> GetPrivateMint(
        const identifier::UnitDefinition& unitid,
        std::uint32_t series) const = 0;
    OPENTXS_EXPORT virtual std::shared_ptr<const blind::Mint> GetPublicMint(
        const identifier::UnitDefinition& unitID) const = 0;
#endif  // OT_CASH
    OPENTXS_EXPORT virtual std::string GetUserName() const = 0;
    OPENTXS_EXPORT virtual std::string GetUserTerms() const = 0;
    OPENTXS_EXPORT virtual const identifier::Server& ID() const = 0;
    OPENTXS_EXPORT virtual const identifier::Nym& NymID() const = 0;
#if OT_CASH
    OPENTXS_EXPORT virtual void ScanMints() const = 0;
#endif  // OT_CASH
    OPENTXS_EXPORT virtual opentxs::server::Server& Server() const = 0;
#if OT_CASH
    OPENTXS_EXPORT virtual void SetMintKeySize(
        const std::size_t size) const = 0;
    OPENTXS_EXPORT virtual void UpdateMint(
        const identifier::UnitDefinition& unitID) const = 0;
#endif  // OT_CASH

    OPENTXS_EXPORT virtual void Start() = 0;

    OPENTXS_EXPORT ~Manager() override = default;

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
