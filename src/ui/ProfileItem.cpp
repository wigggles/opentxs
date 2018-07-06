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

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/ui/ProfileItem.hpp"

#include "ProfileSubsectionParent.hpp"
#include "Row.hpp"

#include "ProfileItem.hpp"

namespace opentxs
{
ui::ProfileItem* Factory::ProfileItemWidget(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const api::client::Wallet& wallet,
    const ui::implementation::ProfileSubsectionParent& parent,
    const ContactItem& item)
{
    return new ui::implementation::ProfileItem(
        zmq, publisher, contact, wallet, parent, item);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
ProfileItem::ProfileItem(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const api::client::Wallet& wallet,
    const ProfileSubsectionParent& parent,
    const opentxs::ContactItem& item)
    : ProfileItemRow(
          parent,
          zmq,
          publisher,
          contact,
          Identifier::Factory(item.ID()),
          true)
    , wallet_(wallet)
    , active_(item.isActive())
    , primary_(item.isPrimary())
    , value_(item.Value())
    , start_(item.Start())
    , end_(item.End())
{
}

bool ProfileItem::add_claim(const Claim& claim) const
{
    auto nym = wallet_.mutable_Nym(parent_.NymID());

    return nym.AddClaim(claim);
}

Claim ProfileItem::as_claim() const
{
    Claim output{};
    auto& [id, section, type, value, start, end, attributes] = output;
    id = "";
    section = parent_.Section();
    type = parent_.Type();
    value = value_;
    start = start_;
    end = end_;

    if (primary_) { attributes.emplace(proto::CITEMATTR_PRIMARY); }

    if (primary_ || active_) { attributes.emplace(proto::CITEMATTR_ACTIVE); }

    return output;
}

bool ProfileItem::Delete() const
{
    auto nym = wallet_.mutable_Nym(parent_.NymID());

    return nym.DeleteClaim(id_);
}

bool ProfileItem::SetActive(const bool& active) const
{
    Claim claim{};
    auto& attributes = std::get<6>(claim);

    if (active) {
        attributes.emplace(proto::CITEMATTR_ACTIVE);
    } else {
        attributes.erase(proto::CITEMATTR_ACTIVE);
        attributes.erase(proto::CITEMATTR_PRIMARY);
    }

    return add_claim(claim);
}

bool ProfileItem::SetPrimary(const bool& primary) const
{
    Claim claim{};
    auto& attributes = std::get<6>(claim);

    if (primary) {
        attributes.emplace(proto::CITEMATTR_PRIMARY);
        attributes.emplace(proto::CITEMATTR_ACTIVE);
    } else {
        attributes.erase(proto::CITEMATTR_PRIMARY);
    }

    return add_claim(claim);
}

bool ProfileItem::SetValue(const std::string& newValue) const
{
    Claim claim{};
    std::get<3>(claim) = newValue;

    return add_claim(claim);
}
}  // namespace opentxs::ui::implementation
