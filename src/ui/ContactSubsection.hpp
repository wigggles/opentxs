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
using ContactSubsectionList = List<
    ContactSubsectionExternalInterface,
    ContactSubsectionInternalInterface,
    ContactSubsectionRowID,
    ContactSubsectionRowInterface,
    ContactSubsectionRowBlank,
    ContactSubsectionSortKey>;
using ContactSubsectionRow = RowType<
    ContactSectionRowInterface,
    ContactSectionInternalInterface,
    ContactSectionRowID>;

class ContactSubsection : public ContactSubsectionList,
                          public ContactSubsectionRow
{
public:
    std::string Name(const std::string& lang) const override;
    proto::ContactItemType Type() const override { return id_.second; }

    void Update(const opentxs::ContactGroup& group) override;

    ~ContactSubsection() = default;

private:
    friend Factory;

    static bool check_type(const ContactSubsectionRowID type);
    static const opentxs::ContactItem& recover(const void* input);

    void construct_row(
        const ContactSubsectionRowID& id,
        const ContactSubsectionSortKey& index,
        const CustomData& custom) const override;

    bool last(const ContactSubsectionRowID& id) const override
    {
        return ContactSubsectionList::last(id);
    }
    void process_group(const opentxs::ContactGroup& group);
    int sort_key(const ContactSubsectionRowID type) const;
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
