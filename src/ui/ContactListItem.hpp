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

#ifndef OPENTXS_UI_CONTACTLISTITEM_IMPLEMENTATION_HPP
#define OPENTXS_UI_CONTACTLISTITEM_IMPLEMENTATION_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactListItemType =
    Row<opentxs::ui::ContactListItem, ContactListParent, Identifier>;

class ContactListItem : public ContactListItemType
{
public:
    std::string ContactID() const override;
    std::string DisplayName() const override;
    std::string ImageURI() const override;
    std::string Section() const override;

    void SetName(const std::string& name) override;

    virtual ~ContactListItem() = default;

protected:
    std::string name_{""};
    OTZMQListenCallback contact_subscriber_callback_;
    OTZMQSubscribeSocket contact_subscriber_;

    virtual void process_contact(const network::zeromq::Message& message);

    ContactListItem(
        const ContactListParent& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const Identifier& id,
        const std::string& name);

private:
    friend Factory;

    ContactListItem() = delete;
    ContactListItem(const ContactListItem&) = delete;
    ContactListItem(ContactListItem&&) = delete;
    ContactListItem& operator=(const ContactListItem&) = delete;
    ContactListItem& operator=(ContactListItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACTLISTITEM_IMPLEMENTATION_HPP
