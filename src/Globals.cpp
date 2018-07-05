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

#include "stdafx.hpp"

#include "opentxs/Types.hpp"

namespace opentxs
{
std::string storage_box_name(StorageBox box)
{
    std::string name = "Unknown";

    switch (box) {
        case StorageBox::SENTPEERREQUEST:
            name = "Send Peer Request";
            break;
        case StorageBox::INCOMINGPEERREQUEST:
            name = "Incoming Peer Request";
            break;
        case StorageBox::SENTPEERREPLY:
            name = "Sent Reply";
            break;
        case StorageBox::INCOMINGPEERREPLY:
            name = "Incoming Peer Reply";
            break;
        case StorageBox::FINISHEDPEERREQUEST:
            name = "Finished Peer Request";
            break;
        case StorageBox::FINISHEDPEERREPLY:
            name = "Finished Peer Reply";
            break;
        case StorageBox::PROCESSEDPEERREQUEST:
            name = "Processed Peer Request";
            break;
        case StorageBox::PROCESSEDPEERREPLY:
            name = "Processed Peer Reply";
            break;
        case StorageBox::MAILINBOX:
            name = "Mail Inbox";
            break;
        case StorageBox::MAILOUTBOX:
            name = "Mail Outbox";
            break;
        case StorageBox::INCOMINGBLOCKCHAIN:
            name = "Incoming Blockchain";
            break;
        case StorageBox::OUTGOINGBLOCKCHAIN:
            name = "Outgoing Blockchain";
            break;
        case StorageBox::INCOMINGCHEQUE:
            name = "Incoming Cheque";
            break;
        case StorageBox::OUTGOINGCHEQUE:
            name = "Outgoing Cheque";
            break;
        case StorageBox::DRAFT:
            name = "Draft";
            break;
        case StorageBox::UNKNOWN:
            [[fallthrough]];
        default:
            break;
    }

    return name;
}

}  // namespace opentxs
