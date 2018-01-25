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

#define OPENTXS_ARG_BACKUP_DIRECTORY "backupdirectory"
#define OPENTXS_ARG_BINDIP "bindip"
#define OPENTXS_ARG_COMMANDPORT "commandport"
#define OPENTXS_ARG_EEP "eep"
#define OPENTXS_ARG_ENCRYPTED_DIRECTORY "encrypteddirectory"
#define OPENTXS_ARG_EXTERNALIP "externalip"
#define OPENTXS_ARG_GC "gc"
#define OPENTXS_ARG_INIT "only-init"
#define OPENTXS_ARG_LISTENCOMMAND "listencommand"
#define OPENTXS_ARG_LISTENNOTIFY "listennotify"
#define OPENTXS_ARG_NAME "name"
#define OPENTXS_ARG_NOTIFICATIONPORT "notificationport"
#define OPENTXS_ARG_ONION "onion"
#define OPENTXS_ARG_PASSPHRASE "passphrase"
#define OPENTXS_ARG_STORAGE_PLUGIN "storageplugin"
#define OPENTXS_ARG_TERMS "terms"
#define OPENTXS_ARG_VERSION "version"
#define OPENTXS_ARG_WORDS "words"

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
        const ArgList& args,
        const std::chrono::seconds gcInterval = std::chrono::seconds(0),
        const bool recover = false);
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
