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

#include <opentxs/storage/Storage.hpp>
#include <opentxs/core/crypto/CryptoEngine.hpp>

namespace opentxs
{

//Singlton class for providing an interface to process-level resources.
class App
{
private:
    static App* instance_pointer_;

    Storage* storage_ = nullptr;
    CryptoEngine* crypto_ = nullptr;

    App();
    App(App const&) = delete;
    App& operator=(App const&) = delete;

    void Init();

public:
    static App& Me();

    CryptoEngine& Crypto() const;
    Storage& DB() const;

    void Cleanup();
    ~App();
};

}  // namespace opentxs
#endif // OPENTXS_CORE_APP_APP_HPP
