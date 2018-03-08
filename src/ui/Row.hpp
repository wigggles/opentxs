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

#ifndef OPENTXS_UI_ROW_IMPLEMENTATION_HPP
#define OPENTXS_UI_ROW_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/core/Lockable.hpp"

namespace opentxs::ui::implementation
{
template <typename InterfaceType, typename ParentType, typename IdentifierType>
class Row : virtual public InterfaceType, public Lockable
{
public:
    bool Last() const override { return parent_.last(id_); }

protected:
    const ParentType& parent_;
    const network::zeromq::Context& zmq_;
    const api::ContactManager& contact_;
    const IdentifierType id_;

    Row(const ParentType& parent,
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const IdentifierType id)
        : parent_(parent)
        , zmq_(zmq)
        , contact_(contact)
        , id_(id)
    {
    }
    Row() = delete;
    Row(const Row&) = delete;
    Row(Row&&) = delete;
    Row& operator=(const Row&) = delete;
    Row& operator=(Row&&) = delete;

    virtual ~Row() = default;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_ROW_IMPLEMENTATION_HPP
