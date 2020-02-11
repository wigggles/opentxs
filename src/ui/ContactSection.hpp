// Copyright (c) 2010-2020 The Open-Transactions developers
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

class ContactSection final
    : public Combined<ContactSectionList, ContactSectionRow, ContactSortKey>
{
public:
    std::string ContactID() const noexcept final { return primary_id_->str(); }
#if OT_QT
    int FindRow(const ContactSectionRowID& id, const ContactSectionSortKey& key)
        const noexcept final
    {
        return find_row(id, key);
    }
#endif
    std::string Name(const std::string& lang) const noexcept final
    {
        return proto::TranslateSectionName(row_id_, lang);
    }
    proto::ContactSectionName Type() const noexcept final { return row_id_; }

    void reindex(
        const implementation::ContactSortKey& key,
        const implementation::CustomData& custom) noexcept final;

    ~ContactSection() = default;

private:
    friend opentxs::Factory;

    static const std::
        map<proto::ContactSectionName, std::set<proto::ContactItemType>>
            allowed_types_;
    static const std::
        map<proto::ContactSectionName, std::map<proto::ContactItemType, int>>
            sort_keys_;

    static int sort_key(const ContactSectionRowID type) noexcept;
    static bool check_type(const ContactSectionRowID type) noexcept;

    void* construct_row(
        const ContactSectionRowID& id,
        const ContactSectionSortKey& index,
        const CustomData& custom) const noexcept final;

    bool last(const ContactSectionRowID& id) const noexcept final
    {
        return ContactSectionList::last(id);
    }
    std::set<ContactSectionRowID> process_section(
        const opentxs::ContactSection& section) noexcept;
    void startup(const CustomData custom) noexcept;

    ContactSection(
        const ContactInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ContactRowID& rowID,
        const ContactSortKey& key,
        const CustomData& custom
#if OT_QT
        ,
        const bool qt
#endif
        ) noexcept;
    ContactSection() = delete;
    ContactSection(const ContactSection&) = delete;
    ContactSection(ContactSection&&) = delete;
    ContactSection& operator=(const ContactSection&) = delete;
    ContactSection& operator=(ContactSection&&) = delete;
};
}  // namespace opentxs::ui::implementation
