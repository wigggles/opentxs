/************************************************************
 *
 *  OTSocket.cpp
 *
 */

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

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
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
 **************************************************************/

#include <opentxs/core/stdafx.hpp>

#include <opentxs/ext/OTSocket.hpp>
#include <opentxs/core/Log.hpp>

namespace opentxs
{

OTSocket::OTSocket(bool connect)
    : m_lLatencySendMs(5000)
    , m_lLatencyReceiveMs(5000)
    , m_bConnected(false)
    , m_bListening(false)
    , endpoint_()
    , context_zmq(new zmq::context_t(1, 31))
    , socket_zmq(
          new zmq::socket_t(*context_zmq.get(), connect ? ZMQ_REQ : ZMQ_REP))
{
    int linger = 1000;
    socket_zmq->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
}

bool OTSocket::Connect(const std::string& endpoint)
{
    if (m_bListening) return false;

    endpoint_ = endpoint;

    try {
        socket_zmq->connect(endpoint_.c_str());
    }
    catch (const std::exception& e) {
        Log::vError("%s: Exception Caught: %s \n", __FUNCTION__, e.what());
        OT_FAIL;
    }

    m_bConnected = true;

    return true;
}

bool OTSocket::Listen(const std::string& endpoint)
{
    if (m_bConnected) return false;

    endpoint_ = endpoint;

    try {
        socket_zmq->bind(endpoint_.c_str());
    }
    catch (const std::exception& e) {
        Log::vError("%s: Exception Caught: %s \n", __FUNCTION__, e.what());
        OT_FAIL;
    }

    m_bListening = true;

    return true;
}

bool OTSocket::Send(const char* data, std::size_t length)
{
    if (!m_bConnected && !m_bListening) return false;

    if (!socket_zmq->send(data, length)) {
        HandleSendingError();
        return false;
    }
    return true;
}

bool OTSocket::Send(const char* data, std::size_t length,
                    const std::string& endpoint)
{
    if (endpoint_ != endpoint) Connect(endpoint);

    return Send(data, length);
}

bool OTSocket::Receive(std::string& serverReply)
{
    if (!m_bConnected && !m_bListening) return false;

    zmq::message_t zmq_message;
    if (!socket_zmq->recv(&zmq_message)) {
        HandleReceivingError();
        return false;
    }
    serverReply.assign(static_cast<const char*>(zmq_message.data()),
                       zmq_message.size());
    return true;
}

void OTSocket::HandleSendingError()
{
    switch (errno) {
    // Non-blocking mode was requested and the message cannot be sent at the
    // moment.
    case EAGAIN:
        Log::vOutput(0, "OTSocket::HandleSendingError: Non-blocking mode was "
                        "requested and the message cannot be sent at the "
                        "moment. Re-trying...\n");
        return;
    // The zmq_send() operation is not supported by this socket type.
    case ENOTSUP:
        Log::Error("OTSocket::HandleSendingError: failure: The zmq_send() "
                   "operation is not supported by this socket type.\n");
        return;
    // The zmq_send() operation cannot be performed on this socket at the moment
    // due to the socket not being in the appropriate state. This error may
    // occur with socket types that switch between several states, such as
    // ZMQ_REP. See the messaging patterns section of zmq_socket(3) for more
    // information.
    case EFSM:
        Log::vOutput(0, "OTSocket::HandleSendingError: The zmq_send() "
                        "operation cannot be performed on this socket at the "
                        "moment due to the socket not being in the "
                        "appropriate state. Deleting socket and "
                        "re-trying...\n");
        return;
    // The ØMQ context associated with the specified socket was terminated.
    case ETERM:
        Log::Error("OTSocket::HandleSendingError: The ØMQ context associated "
                   "with the specified socket was terminated. (Deleting and "
                   "re-creating the context and the socket, and trying "
                   "again.)\n");
        return;
    // The provided socket was invalid.
    case ENOTSOCK:
        Log::Error("OTSocket::HandleSendingError: The provided socket was "
                   "invalid. (Deleting socket and re-trying...)\n");
        return;
    // The operation was interrupted by delivery of a signal before the message
    // was sent. Re-trying...
    case EINTR:
        Log::Error("OTSocket::HandleSendingError: The operation was "
                   "interrupted by delivery of a signal before the message "
                   "was sent. (Re-trying...)\n");
        return;
    // Invalid message.
    case EFAULT:
        Log::Error("OTSocket::HandleSendingError: Failure: The provided "
                   "pollitems were not valid (nullptr).\n");
        return;
    default:
        Log::Error(
            "OTSocket::HandleSendingError: Default case. Re-trying...\n");
        return;
    }
}

void OTSocket::HandleReceivingError()
{
    switch (errno) {
    // Non-blocking mode was requested and no messages are available at the
    // moment.
    case EAGAIN:
        Log::vOutput(0, "OTSocket::HandleReceivingError: Non-blocking mode "
                        "was requested and no messages are available at the "
                        "moment. Re-trying...\n");
        return;
    // The zmq_recv() operation is not supported by this socket type.
    case ENOTSUP:
        Log::Error("OTSocket::HandleReceivingError: Failure: The zmq_recv() "
                   "operation is not supported by this socket type.\n");
        return;
    // The zmq_recv() operation cannot be performed on this socket at the moment
    // due to the socket not being in the appropriate state. This error may
    // occur with socket types that switch between several states, such as
    // ZMQ_REP. See the messaging patterns section of zmq_socket(3) for more
    // information.
    case EFSM:
        Log::vOutput(0, "OTSocket::HandleReceivingError: The zmq_recv() "
                        "operation cannot be performed on this socket at the "
                        "moment due to the socket not being in the "
                        "appropriate state. (Deleting socket and "
                        "re-trying...)\n");
        return;
    // The ØMQ context associated with the specified socket was terminated.
    case ETERM:
        Log::Error("OTSocket::HandleReceivingError: The ØMQ context "
                   "associated with the specified socket was terminated. "
                   "(Re-creating the context, and trying again...)\n");
        return;
    // The provided socket was invalid.
    case ENOTSOCK:
        Log::Error("OTSocket::HandleReceivingError: The provided socket was "
                   "invalid. (Deleting socket and re-trying.)\n");
        return;
    // The operation was interrupted by delivery of a signal before a message
    // was available.
    case EINTR:
        Log::Error("OTSocket::HandleSendingError: The operation was "
                   "interrupted by delivery of a signal before the message "
                   "was sent. (Re-trying...)\n");
        return;
    // The message passed to the function was invalid.
    case EFAULT:
        Log::Error("OTSocket::HandleReceivingError: Failure: The message "
                   "passed to the function was invalid.\n");
        return;
    default:
        Log::Error(
            "OTSocket::HandleReceivingError: Default case. Re-trying...\n");
        return;
    }
}

} // namespace opentxs
