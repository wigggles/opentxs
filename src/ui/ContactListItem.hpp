// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/ui/ContactListItem.hpp"
#include "ui/Row.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace client
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace ui
{
class ContactListItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ContactListItemRow =
    Row<ContactListRowInternal, ContactListInternalInterface, ContactListRowID>;

class ContactListItem : public ContactListItemRow
{
public:
    auto ContactID() const noexcept -> std::string final;
    auto DisplayName() const noexcept -> std::string final;
    auto ImageURI() const noexcept -> std::string final;
    auto Section() const noexcept -> std::string final;

#if OT_QT
    QVariant qt_data(const int column, const int role) const noexcept override;
#endif

    void reindex(
        const ContactListSortKey&,
        const CustomData&) noexcept override;

    ContactListItem(
        const ContactListInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ContactListRowID& rowID,
        const ContactListSortKey& key) noexcept;
    ~ContactListItem() override = default;

protected:
    ContactListSortKey key_;

private:
    ContactListItem() = delete;
    ContactListItem(const ContactListItem&) = delete;
    ContactListItem(ContactListItem&&) = delete;
    auto operator=(const ContactListItem&) -> ContactListItem& = delete;
    auto operator=(ContactListItem &&) -> ContactListItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ContactListItem>;
