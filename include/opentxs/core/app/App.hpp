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

#ifndef OPENTXS_CORE_APP_APP_HPP
#define OPENTXS_CORE_APP_APP_HPP

#include <chrono>
#include <limits>
#include <thread>

#include <opentxs/storage/Storage.hpp>
#include <opentxs/core/app/Dht.hpp>
#include <opentxs/core/app/Settings.hpp>
#include <opentxs/core/crypto/CryptoEngine.hpp>

namespace opentxs
{

//Singlton class for providing an interface to process-level resources.
class App
{
private:
    static App* instance_pointer_;

    Settings* config_ = nullptr;
    CryptoEngine* crypto_ = nullptr;
    Dht* dht_ = nullptr;
    Storage* storage_ = nullptr;

    std::thread* periodic_thread_ = nullptr;

    bool server_mode_ = false;
    time64_t last_nym_publish_ = 0;
    int64_t nym_publish_interval_ = std::numeric_limits<int64_t>::max();

    App(const bool serverMode);
    App() = delete;
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    void Periodic();

    void Init_Config();
    void Init_Crypto();
    void Init_Storage();
    void Init_Dht();
    void Init_Periodic();
    void Init();

public:
    static App& Me(const bool serverMode = false);

    Settings& Config();
    CryptoEngine& Crypto();
    Storage& DB();
    Dht& DHT();

    void Cleanup();
    ~App();
};

}  // namespace opentxs
#endif // OPENTXS_CORE_APP_APP_HPP
