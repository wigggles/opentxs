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

/** \defgroup native Native API */

#ifndef OPENTXS_CORE_API_NATIVE_HPP
#define OPENTXS_CORE_API_NATIVE_HPP

#include "opentxs/Version.hpp"

#include "opentxs/Types.hpp"

#include <chrono>
#include <functional>
#include <string>

namespace opentxs
{
class CryptoEngine;

namespace api
{
class Activity;
class Api;
class Blockchain;
class ContactManager;
class Dht;
class Identity;
class Server;
class Settings;
class Storage;
class Wallet;
class ZMQ;

class Native
{
public:
    virtual class Activity& Activity() const = 0;
    virtual class Api& API() const = 0;
    virtual class Blockchain& Blockchain() const = 0;
    virtual class Settings& Config(
        const std::string& path = std::string("")) const = 0;
    virtual class ContactManager& Contact() const = 0;
    virtual opentxs::CryptoEngine& Crypto() const = 0;
    virtual class Storage& DB() const = 0;
    virtual class Dht& DHT() const = 0;
    virtual void HandleSignals() const = 0;
    virtual class Identity& Identity() const = 0;
    /** Adds a task to the periodic task list with the specified interval. By
     * default, schedules for immediate execution. */
    virtual void Schedule(
        const std::chrono::seconds& interval,
        const opentxs::PeriodicTask& task,
        const std::chrono::seconds& last = std::chrono::seconds(0)) const = 0;
    virtual const class Server& Server() const = 0;
    virtual bool ServerMode() const = 0;
    virtual class Wallet& Wallet() const = 0;
    virtual class ZMQ& ZMQ() const = 0;

    virtual ~Native() = default;

protected:
    Native() = default;

private:
    Native(const Native&) = delete;
    Native(Native&&) = delete;
    Native& operator=(const Native&) = delete;
    Native& operator=(Native&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_CORE_API_NATIVE_HPP
