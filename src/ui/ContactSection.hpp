// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactSectionList = List<
    ContactSectionExternalInterface,
    ContactSectionInternalInterface,
    ContactSectionRowID,
    ContactSectionRowInterface,
    ContactSectionRowInternal,
    ContactSectionRowBlank,
    ContactSectionSortKey,
    ContactSectionPrimaryID>;
using ContactSectionRow =
    RowType<ContactRowInternal, ContactInternalInterface, ContactRowID>;

class ContactSection final : public ContactSectionList, public ContactSectionRow
{
public:
    std::string ContactID() const override { return primary_id_->str(); }
    std::string Name(const std::string& lang) const override
    {
        return proto::TranslateSectionName(row_id_, lang);
    }
    proto::ContactSectionName Type() const override { return row_id_; }

    void reindex(
        const implementation::ContactSortKey& key,
        const implementation::CustomData& custom) override;

    ~ContactSection() = default;

private:
    friend opentxs::Factory;

    static const std::
        map<proto::ContactSectionName, std::set<proto::ContactItemType>>
            allowed_types_;
    static const std::
        map<proto::ContactSectionName, std::map<proto::ContactItemType, int>>
            sort_keys_;

    static int sort_key(const ContactSectionRowID type);
    static bool check_type(const ContactSectionRowID type);

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
    void startup(const CustomData custom);

    ContactSection(
        const ContactInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ContactRowID& rowID,
        const ContactSortKey& key,
        const CustomData& custom
#if OT_QT
        ,
        const bool qt
#endif
    );
    ContactSection() = delete;
    ContactSection(const ContactSection&) = delete;
    ContactSection(ContactSection&&) = delete;
    ContactSection& operator=(const ContactSection&) = delete;
    ContactSection& operator=(ContactSection&&) = delete;
};
}  // namespace opentxs::ui::implementation
