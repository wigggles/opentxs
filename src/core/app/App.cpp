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

#include <functional>

#include <opentxs/core/app/App.hpp>

#include <opentxs/core/util/OTFolders.hpp>

namespace opentxs
{

App* App::instance_pointer_ = nullptr;

App::App()
{
    Init();
}

void App::Init()
{
    CryptoEngine::Instance();

    Digest hash = std::bind(
        static_cast<bool(CryptoHash::*)(
            const uint32_t,
            const std::string&,
            std::string&)>(&CryptoHash::Digest),
        &(Crypto().Hash()),
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3);

    storage_ = &Storage::Factory(
        hash,
        OTFolders::Common().Get(),
        Storage::Type::FS);
}

App& App::Me()
{
    if (nullptr == instance_pointer_)
    {
        instance_pointer_ = new App;
    }

    return *instance_pointer_;
}

CryptoEngine& App::Crypto() const
{
    return CryptoEngine::Instance();
}

Storage& App::Store() const
{
    OT_ASSERT(nullptr != storage_)

    return *storage_;
}

void App::Cleanup()
{
    CryptoEngine::Instance().Cleanup();
}

App::~App()
{
    Cleanup();
}

} // namespace opentxs
