// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACT_SECTION_IMPLEMENTATION_HPP
#define OPENTXS_UI_CONTACT_SECTION_IMPLEMENTATION_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactSectionList = List<
    ContactSectionExternalInterface,
    ContactSectionInternalInterface,
    ContactSectionRowID,
    ContactSectionRowInterface,
    ContactSectionRowBlank,
    ContactSectionSortKey>;
using ContactSectionRow =
    RowType<ContactRowInterface, ContactInternalInterface, ContactRowID>;

class ContactSection : public ContactSectionList, public ContactSectionRow
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

    static int sort_key(const ContactSectionRowID type);
    static bool check_type(const ContactSectionRowID type);
    static const opentxs::ContactGroup& recover(const void* input);

    void construct_row(
        const ContactSectionRowID& id,
        const ContactSectionSortKey& index,
        const CustomData& custom) const override;

    bool last(const ContactSectionRowID& id) const override
    {
        return ContactSectionList::last(id);
    }
    std::set<ContactSectionRowID> process_section(
        const opentxs::ContactSection& section);
    void startup(const opentxs::ContactSection section);
    void update(ContactSectionRowInterface& row, const CustomData& custom)
        const override;

    ContactSection(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
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
