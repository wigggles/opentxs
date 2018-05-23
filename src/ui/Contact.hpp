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

#ifndef OPENTXS_UI_CONTACT_IMPLEMENTATION_HPP
#define OPENTXS_UI_CONTACT_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactPimpl = std::unique_ptr<opentxs::ui::ContactSection>;
using ContactIDType = proto::ContactSectionName;
using ContactSortKey = int;
using ContactInner = std::map<ContactIDType, ContactPimpl>;
using ContactOuter = std::map<ContactSortKey, ContactInner>;
using ContactReverse = std::map<ContactIDType, ContactSortKey>;
using ContactType = List<
    opentxs::ui::Contact,
    ContactParent,
    opentxs::ui::ContactSection,
    ContactIDType,
    ContactPimpl,
    ContactInner,
    ContactSortKey,
    ContactOuter,
    ContactOuter::const_iterator,
    ContactReverse>;

class Contact : virtual public ContactType
{
public:
    std::string ContactID() const override;
    std::string DisplayName() const override;
    std::string PaymentCode() const override;

    ~Contact() = default;

private:
    friend Factory;

    static const std::set<proto::ContactSectionName> allowed_types_;
    static const std::map<proto::ContactSectionName, int> sort_keys_;

    std::string name_;
    std::string payment_code_;
    OTZMQListenCallback contact_subscriber_callback_;
    OTZMQSubscribeSocket contact_subscriber_;

    static int sort_key(const proto::ContactSectionName type);
    static bool check_type(const proto::ContactSectionName type);
    static const opentxs::ContactSection& recover(const void* input);

    ContactIDType blank_id() const override
    {
        return proto::CONTACTSECTION_ERROR;
    }
    void construct_item(
        const ContactIDType& id,
        const ContactSortKey& index,
        const CustomData& custom) const override;

    bool last(const ContactIDType& id) const override
    {
        return ContactType::last(id);
    }
    ContactOuter::const_iterator outer_first() const override
    {
        return items_.begin();
    }
    ContactOuter::const_iterator outer_end() const override
    {
        return items_.end();
    }
    void update(ContactPimpl& row, const CustomData& custom) const override;

    void process_contact(const opentxs::Contact& contact);
    void process_contact(const network::zeromq::MultipartMessage& message);
    void startup();

    Contact(
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const Identifier& nymID);
    Contact() = delete;
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    Contact& operator=(const Contact&) = delete;
    Contact& operator=(Contact&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACT_IMPLEMENTATION_HPP
