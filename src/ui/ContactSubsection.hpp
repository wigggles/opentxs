// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactSubsectionList = List<
    ContactSubsectionExternalInterface,
    ContactSubsectionInternalInterface,
    ContactSubsectionRowID,
    ContactSubsectionRowInterface,
    ContactSubsectionRowInternal,
    ContactSubsectionRowBlank,
    ContactSubsectionSortKey,
    ContactSubsectionPrimaryID>;
using ContactSubsectionRow = RowType<
    ContactSectionRowInternal,
    ContactSectionInternalInterface,
    ContactSectionRowID>;

class ContactSubsection final : public ContactSubsectionList,
                                public ContactSubsectionRow
{
public:
    std::string Name(const std::string& lang) const override;
    proto::ContactItemType Type() const override { return row_id_.second; }

    void reindex(const ContactSectionSortKey& key, const CustomData& custom)
        override;

    ~ContactSubsection() = default;

private:
    friend opentxs::Factory;

    static bool check_type(const ContactSubsectionRowID type);

    void construct_row(
        const ContactSubsectionRowID& id,
        const ContactSubsectionSortKey& index,
        const CustomData& custom) const override;

    bool last(const ContactSubsectionRowID& id) const override
    {
        return ContactSubsectionList::last(id);
    }
    std::set<ContactSubsectionRowID> process_group(
        const opentxs::ContactGroup& group);
    int sort_key(const ContactSubsectionRowID type) const;
    void startup(const CustomData custom);

    ContactSubsection(
        const ContactSectionInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ContactSectionRowID& rowID,
        const ContactSectionSortKey& key,
        const CustomData& custom
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback,
        const RowCallbacks removeCallback
#endif
    );
    ContactSubsection() = delete;
    ContactSubsection(const ContactSubsection&) = delete;
    ContactSubsection(ContactSubsection&&) = delete;
    ContactSubsection& operator=(const ContactSubsection&) = delete;
    ContactSubsection& operator=(ContactSubsection&&) = delete;
};
}  // namespace opentxs::ui::implementation
