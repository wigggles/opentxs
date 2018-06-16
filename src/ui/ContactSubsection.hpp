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

#ifndef OPENTXS_UI_CONTACT_SUBSECTION_IMPLEMENTATION_HPP
#define OPENTXS_UI_CONTACT_SUBSECTION_IMPLEMENTATION_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactSubsectionType = List<
    opentxs::ui::ContactSubsection,
    ContactSubsectionParent,
    opentxs::ui::ContactItem,
    ContactSubsectionIDType,
    ContactSubsectionPimpl,
    ContactSubsectionInner,
    ContactSubsectionSortKey,
    ContactSubsectionOuter,
    ContactSubsectionOuter::const_iterator>;
using ContactSubsectionRowType = RowType<
    opentxs::ui::ContactSubsection,
    ContactSectionParent,
    std::pair<proto::ContactSectionName, proto::ContactItemType>>;

class ContactSubsection : public ContactSubsectionType,
                          public ContactSubsectionRowType
{
public:
    std::string Name(const std::string& lang) const override;
    proto::ContactItemType Type() const override { return id_.second; }

    void Update(const opentxs::ContactGroup& group) override;

    ~ContactSubsection() = default;

private:
    friend Factory;

    static bool check_type(const ContactSubsectionIDType type);
    static const opentxs::ContactItem& recover(const void* input);

    ContactSubsectionIDType blank_id() const override
    {
        return Identifier::Factory();
    }
    void construct_item(
        const ContactSubsectionIDType& id,
        const ContactSubsectionSortKey& index,
        const CustomData& custom) const override;

    bool last(const ContactSubsectionIDType& id) const override
    {
        return ContactSubsectionType::last(id);
    }
    ContactSubsectionOuter::const_iterator outer_first() const override
    {
        return items_.begin();
    }
    ContactSubsectionOuter::const_iterator outer_end() const override
    {
        return items_.end();
    }
    void process_group(const opentxs::ContactGroup& group);
    int sort_key(const ContactSubsectionIDType type) const;
    void startup(const opentxs::ContactGroup group);

    ContactSubsection(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ContactSectionParent& parent,
        const opentxs::ContactGroup& group);
    ContactSubsection() = delete;
    ContactSubsection(const ContactSubsection&) = delete;
    ContactSubsection(ContactSubsection&&) = delete;
    ContactSubsection& operator=(const ContactSubsection&) = delete;
    ContactSubsection& operator=(ContactSubsection&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACT_SUBSECTION_IMPLEMENTATION_HPP
