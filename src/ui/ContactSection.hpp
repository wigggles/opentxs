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

#ifndef OPENTXS_UI_CONTACT_SECTION_IMPLEMENTATION_HPP
#define OPENTXS_UI_CONTACT_SECTION_IMPLEMENTATION_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactSectionType = List<
    opentxs::ui::ContactSection,
    ContactSectionParent,
    opentxs::ui::ContactSubsection,
    ContactSectionIDType,
    ContactSectionPimpl,
    ContactSectionInner,
    ContactSectionSortKey,
    ContactSectionOuter,
    ContactSectionOuter::const_iterator>;
using ContactSectionRowType = RowType<
    opentxs::ui::ContactSection,
    ContactParent,
    proto::ContactSectionName>;

class ContactSection : public ContactSectionType, public ContactSectionRowType
{
public:
    std::string ContactID() const override;
    std::string Name(const std::string& lang) const override;
    proto::ContactSectionName Type() const override { return id_; }

    void Update(const opentxs::ContactSection& section) override;

    ~ContactSection() = default;

private:
    friend Factory;

    static const std::
        map<proto::ContactSectionName, std::set<proto::ContactItemType>>
            allowed_types_;
    static const std::
        map<proto::ContactSectionName, std::map<proto::ContactItemType, int>>
            sort_keys_;

    static int sort_key(const ContactSectionIDType type);
    static bool check_type(const ContactSectionIDType type);
    static const opentxs::ContactGroup& recover(const void* input);

    ContactSectionIDType blank_id() const override
    {
        return {proto::CONTACTSECTION_ERROR, proto::CITEMTYPE_ERROR};
    }
    void construct_item(
        const ContactSectionIDType& id,
        const ContactSectionSortKey& index,
        const CustomData& custom) const override;

    bool last(const ContactSectionIDType& id) const override
    {
        return ContactSectionType::last(id);
    }
    ContactSectionOuter::const_iterator outer_first() const override
    {
        return items_.begin();
    }
    ContactSectionOuter::const_iterator outer_end() const override
    {
        return items_.end();
    }
    std::set<ContactSectionIDType> process_section(
        const opentxs::ContactSection& section);
    void startup(const opentxs::ContactSection section);
    void update(ContactSectionPimpl& row, const CustomData& custom)
        const override;

    ContactSection(
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const ContactParent& parent,
        const opentxs::ContactSection& section);
    ContactSection() = delete;
    ContactSection(const ContactSection&) = delete;
    ContactSection(ContactSection&&) = delete;
    ContactSection& operator=(const ContactSection&) = delete;
    ContactSection& operator=(ContactSection&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACT_SECTION_IMPLEMENTATION_HPP
