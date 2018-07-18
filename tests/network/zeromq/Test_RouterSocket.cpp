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

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{
class Test_RouterSocket : public ::testing::Test
{
public:
    static OTZMQContext context_;

//    const std::string testMessage_{"zeromq test message"};
};

OTZMQContext Test_RouterSocket::context_{network::zeromq::Context::Factory()};

}  // namespace

TEST(RouterSocket, RouterSocket_Factory)
{
    ASSERT_NE(nullptr, &Test_RouterSocket::context_.get());

    auto dealerSocket = network::zeromq::RouterSocket::Factory(
        Test_RouterSocket::context_,
		true,
        network::zeromq::ListenCallback::Factory());

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(SocketType::Router, dealerSocket->Type());
}

// TODO: Add tests for other public member functions: SetCurve, SetSocksProxy
