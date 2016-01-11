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

#include <chrono>
#include <functional>
#include <iostream>
#include <string>

#include <opentxs/core/app/App.hpp>

#include <opentxs/core/Log.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/app/Settings.hpp>
#include <opentxs/core/util/OTDataFolder.hpp>
#include <opentxs/core/util/OTFolders.hpp>

namespace opentxs
{

App* App::instance_pointer_ = nullptr;

App::App(const bool serverMode)
    : server_mode_(serverMode)
{
    Init();
}

void App::Init()
{
    crypto_ = &CryptoEngine::It();

    Digest hash = std::bind(
        static_cast<bool(CryptoHash::*)(
            const uint32_t,
            const std::string&,
            std::string&)>(&CryptoHash::Digest),
        &(Crypto().Hash()),
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3);

    Random random = std::bind(&CryptoUtil::RandomFilename, &(Crypto().Util()));

    std::shared_ptr<OTDB::StorageFS> storage(OTDB::StorageFS::Instantiate());
    std::string root_path = OTFolders::Common().Get();
    std::string path;

    if (0 <= storage->ConstructAndCreatePath(
            path,
            OTFolders::Common().Get(),
            ".temp")) {
        path.erase(path.end() - 5, path.end());
    }

    storage_ = &Storage::It(
        hash,
        random,
        path);

    dht_ = &Dht::It(server_mode_ ? 4223 : 4221);

    String strConfigFilePath;
    OTDataFolder::GetConfigFilePath(strConfigFilePath);
    config_ = new Settings(strConfigFilePath);

    periodic_thread_ = new std::thread(&App::Periodic, this);
}

void App::Periodic()
{
    for (;;) {
        // Collect garbage, if necessary
        if (nullptr != storage_) {
            storage_->RunGC();
        }

        Log::Sleep(std::chrono::milliseconds(250));
    }
}

App& App::Me(const bool serverMode)
{
    if (nullptr == instance_pointer_)
    {
        instance_pointer_ = new App(serverMode);
    }

    return *instance_pointer_;
}

Settings& App::Config()
{
    OT_ASSERT(nullptr != config_)

    return *config_;
}

CryptoEngine& App::Crypto()
{
    OT_ASSERT(nullptr != crypto_)

    return *crypto_;
}

Storage& App::DB()
{
    OT_ASSERT(nullptr != storage_)

    return *storage_;
}

Dht& App::DHT()
{
    OT_ASSERT(nullptr != dht_)

    return *dht_;
}

void App::Cleanup()
{
    delete storage_;
    storage_ = nullptr;

    delete dht_;
    dht_ = nullptr;

    delete crypto_;
    crypto_ = nullptr;

    delete config_;
    config_ = nullptr;
}

App::~App()
{
    if ((nullptr != periodic_thread_) && periodic_thread_->joinable()) {
        periodic_thread_->join();
        delete periodic_thread_;
        periodic_thread_ = nullptr;
    }
    Cleanup();
}

} // namespace opentxs
