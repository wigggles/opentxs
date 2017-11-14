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

#ifndef OPENTXS_CORE_API_OT_HPP
#define OPENTXS_CORE_API_OT_HPP

#include "opentxs/Version.hpp"

#include <atomic>
#include <chrono>
#include <map>
#include <string>

namespace opentxs
{
namespace api
{
class Native;
}

/** \brief Static methods for starting up the native api.
 *  \ingroup native
 */
class OT
{
public:
    /** Native API accessor
     *
     *  Returns a reference to the native API singleton after it has been
     *  initialized.
     */
    static const api::Native& App();
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
    static const std::atomic<bool>& Shutdown();

private:
    static api::Native* instance_pointer_;
    static std::atomic<bool> shutdown_;

    OT() = delete;
    OT(const OT&) = delete;
    OT(OT&&) = delete;
    OT& operator=(const OT&) = delete;
    OT& operator=(OT&&) = delete;
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_API_OT_HPP
