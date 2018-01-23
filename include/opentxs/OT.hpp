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
#include "opentxs/Types.hpp"

#include <atomic>
#include <chrono>
#include <map>
#include <string>

#define OPENTXS_ARG_STORAGE_PLUGIN "StoragePlugin"
#define OPENTXS_ARG_BACKUP_DIRECTORY "BackupDirectory"
#define OPENTXS_ARG_ENCRYPTED_DIRECTORY "EncryptedDirectory"
#define OPENTXS_ARG_WORDS "Words"
#define OPENTXS_ARG_PASSPHRASE "Passphrase"

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
        const ArgList& args,
        const std::chrono::seconds gcInterval = std::chrono::seconds(0),
        const bool recover = false);
    static void Join();
    static void ServerFactory(
        const std::map<std::string, std::string>& serverArgs,
        const ArgList& args,
        const std::chrono::seconds gcInterval = std::chrono::seconds(0));
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
