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

#ifndef OPENTXS_API_UI_IMPLEMENTATION_HPP
#define OPENTXS_API_UI_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/api/UI.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/Types.hpp"

#include <map>
#include <memory>

namespace opentxs::api::implementation
{
class UI : virtual public opentxs::api::UI, Lockable
{
public:
    const ui::ContactList& ContactList(const Identifier& nymID) const override;

    ~UI();

private:
    friend class implementation::Native;
    using ContactListMap =
        std::map<Identifier, std::unique_ptr<ui::ContactList>>;

    const opentxs::network::zeromq::Context& zmq_;
    const api::ContactManager& contact_;
    mutable ContactListMap contact_lists_{};

    UI(const opentxs::network::zeromq::Context& zmq,
       const api::ContactManager& contact);
    UI() = delete;
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&) = delete;
};
}  // namespace opentxs::api::implementation
#endif  // OPENTXS_API_UI_IMPLEMENTATION_HPP
