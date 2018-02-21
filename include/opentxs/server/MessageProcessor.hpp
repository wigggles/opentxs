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

#ifndef OPENTXS_SERVER_MESSAGEPROCESSOR_HPP
#define OPENTXS_SERVER_MESSAGEPROCESSOR_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Flag.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace opentxs
{
namespace server
{
class MessageProcessor
{
public:
    EXPORT explicit MessageProcessor(
        Server& server,
        const network::zeromq::Context& context,
        const Flag& running);

    EXPORT void cleanup();
    EXPORT void init(const int port, const OTPassword& privkey);
    EXPORT void Start();

    EXPORT ~MessageProcessor();

private:
    Server& server_;
    const Flag& running_;
    [[maybe_unused]] const network::zeromq::Context& context_;
    OTZMQReplySocket reply_socket_;
    std::unique_ptr<std::thread> thread_{nullptr};

    bool processMessage(const std::string& messageString, std::string& reply);
    void processSocket();
    void run();
};
}  // namespace server
}  // namespace opentxs
#endif  // OPENTXS_SERVER_MESSAGEPROCESSOR_HPP
