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
 *  fellowtraveler\opentransactions.org
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

#ifndef OPENTXS_CORE_APP_WALLET_HPP
#define OPENTXS_CORE_APP_WALLET_HPP

#include <chrono>
#include <map>
#include <mutex>
#include <string>

#include "opentxs/core/contract/ServerContract.hpp"

namespace opentxs
{

class App;

/** \brief This class manages instantiated contracts and provides easy access
 *  to them.
 *
 *  It includes functionality which was previously found in OTWallet, and adds
 *  new capabilities such as the ability to (optionally) automatically perform
 *  remote lookups for contracts which are not already present in the local
 *  database.
 */
class Wallet
{
private:
    typedef std::map<std::string, std::shared_ptr<ServerContract>> ServerMap;
    typedef std::shared_ptr<const ServerContract> ConstServerContract;

    friend App;

    ServerMap server_map_;
    std::mutex server_map_lock_;

    Wallet() = default;
    Wallet(const Wallet&) = delete;
    Wallet operator=(const Wallet&) = delete;

public:

    /**   Return a smart pointer to an instantiated server contract.
     *    The smart pointer will not be initialized if the object does not
     *    exist or is invalid.
     *
     *    If the caller is willing to accept a network lookup delay, it can
     *    specify a timeout to be used in the event that the contract can not
     *    be located in local storage and must be queried from a remote
     *    location.
     *
     *    If no timeout is specified, the remote query will still happen in the
     *    background, but this method will return immediately with a null
     *    result.
     *
     *    \param[in] id the identifier of the contract to be returned
     *    \param[in] timeout The caller can set a non-zero value here if it's
     *                     willing to wait for a network lookup. The default value
     *                     of 0 will return immediately.
     */
    ConstServerContract Server(
        const Identifier& id,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(0));

    /**   Return a smart pointer to an instantiated server contract.
     *    The smart pointer will not be initialized if the provided serialized
     *    contract is invalid.
     *
     *    \param[in] contract the serialized version of the contract
     */
    ConstServerContract Server(const proto::ServerContract& contract);

    ~Wallet() = default;
};
}  // namespace opentxs
#endif // OPENTXS_CORE_APP_WALLET_HPP
