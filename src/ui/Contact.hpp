// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactType = List<
    ContactExternalInterface,
    ContactInternalInterface,
    ContactRowID,
    ContactRowInterface,
    ContactRowInternal,
    ContactRowBlank,
    ContactSortKey,
    ContactPrimaryID>;

class Contact : public ContactType
{
public:
    std::string ContactID() const noexcept final;
    std::string DisplayName() const noexcept final;
#if OT_QT
    int FindRow(const ContactRowID& id, const ContactSortKey& key) const
        noexcept final
    {
        return find_row(id, key);
    }
#endif
    std::string PaymentCode() const noexcept final;

    ~Contact();

private:
    friend opentxs::Factory;

    static const std::set<proto::ContactSectionName> allowed_types_;
    static const std::map<proto::ContactSectionName, int> sort_keys_;

    const ListenerDefinitions listeners_;
    std::string name_;
    std::string payment_code_;

    static int sort_key(const proto::ContactSectionName type) noexcept;
    static bool check_type(const proto::ContactSectionName type) noexcept;

    void* construct_row(
        const ContactRowID& id,
        const ContactSortKey& index,
        const CustomData& custom) const noexcept final;

    bool last(const ContactRowID& id) const noexcept final
    {
        return ContactType::last(id);
    }
    void update(ContactRowInterface& row, const CustomData& custom) const
        noexcept;

    void process_contact(const opentxs::Contact& contact) noexcept;
    void process_contact(const network::zeromq::Message& message) noexcept;
    void startup() noexcept;

    Contact(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const Identifier& contactID
#if OT_QT
        ,
        const bool qt
#endif
        ) noexcept;
    Contact() = delete;
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    Contact& operator=(const Contact&) = delete;
    Contact& operator=(Contact&&) = delete;
};
}  // namespace opentxs::ui::implementation
