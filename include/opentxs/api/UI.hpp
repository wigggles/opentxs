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

#ifndef OPENTXS_API_UI_HPP
#define OPENTXS_API_UI_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

namespace opentxs
{
namespace api
{
class UI
{
public:
    EXPORT virtual const ui::ActivitySummary& ActivitySummary(
        const Identifier& nymID) const = 0;
    EXPORT virtual const ui::ActivityThread& ActivityThread(
        const Identifier& nymID,
        const Identifier& threadID) const = 0;
    EXPORT virtual const ui::ContactList& ContactList(
        const Identifier& nymID) const = 0;
    EXPORT virtual const ui::MessagableList& MessagableList(
        const Identifier& nymID) const = 0;
#ifndef SWIG
    EXPORT virtual const ui::PayableList& PayableList(
        const Identifier& nymID,
        proto::ContactItemType currency) const = 0;
#endif
    EXPORT virtual const ui::PayableList& PayableList(
        const Identifier& nymID,
        std::uint32_t currency) const = 0;

    virtual ~UI() = default;

protected:
    UI() = default;

private:
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_UI_HPP
