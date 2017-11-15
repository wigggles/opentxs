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

#include "opentxs/stdafx.hpp"

#include "opentxs/OT.hpp"

#include "opentxs/api/implementation/Native.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/Log.hpp"

namespace opentxs
{
api::Native* OT::instance_pointer_{nullptr};
std::atomic<bool> OT::shutdown_{false};

const api::Native& OT::App()
{
    OT_ASSERT(nullptr != instance_pointer_);

    return *instance_pointer_;
}

void OT::Cleanup()
{
    if (nullptr != instance_pointer_) {
        auto ot = dynamic_cast<api::implementation::Native*>(instance_pointer_);

        if (nullptr != ot) {
            ot->shutdown();
        }

        delete instance_pointer_;
        instance_pointer_ = nullptr;
    }
}

void OT::ClientFactory(
    const std::chrono::seconds gcInterval,
    const std::string& storagePlugin,
    const std::string& backupDirectory,
    const std::string& encryptedDirectory)
{
    ClientFactory(
        false,
        "",
        "",
        gcInterval,
        storagePlugin,
        backupDirectory,
        encryptedDirectory);
}

void OT::ClientFactory(
    const bool recover,
    const std::string& words,
    const std::string& passphrase,
    const std::chrono::seconds gcInterval,
    const std::string& storagePlugin,
    const std::string& backupDirectory,
    const std::string& encryptedDirectory)
{
    OT_ASSERT(nullptr == instance_pointer_);

    instance_pointer_ = new api::implementation::Native(
        shutdown_,
        recover,
        words,
        passphrase,
        false,
        gcInterval,
        storagePlugin,
        backupDirectory,
        encryptedDirectory,
        {});

    OT_ASSERT(nullptr != instance_pointer_);

    auto ot = dynamic_cast<api::implementation::Native*>(instance_pointer_);

    OT_ASSERT(nullptr != ot);

    ot->Init();
}

void OT::Join()
{
    while (nullptr != instance_pointer_) {
        Log::Sleep(std::chrono::milliseconds(250));
    }
}

void OT::ServerFactory(
    const std::map<std::string, std::string>& serverArgs,
    const std::chrono::seconds gcInterval,
    const std::string& storagePlugin,
    const std::string& backupDirectory)
{
    OT_ASSERT(nullptr == instance_pointer_);

    instance_pointer_ = new api::implementation::Native(
        shutdown_,
        false,
        "",
        "",
        true,
        gcInterval,
        storagePlugin,
        backupDirectory,
        "",
        serverArgs);

    OT_ASSERT(nullptr != instance_pointer_);

    auto ot = dynamic_cast<api::implementation::Native*>(instance_pointer_);

    OT_ASSERT(nullptr != ot);

    ot->Init();
}

const std::atomic<bool>& OT::Shutdown() { return shutdown_; }
}  // namespace opentxs
