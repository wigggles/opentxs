// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/ContactItem.cpp"

#pragma once

#include <memory>
#include <string>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/ui/ContactItem.hpp"
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
class ContactItem;
}  // namespace ui

class Factory;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ContactItemRow =
    Row<ContactSubsectionRowInternal,
        ContactSubsectionInternalInterface,
        ContactSubsectionRowID>;

class ContactItem final : public ContactItemRow
{
public:
    std::string ClaimID() const noexcept final
    {
        sLock lock(shared_lock_);

        return row_id_->str();
    }
    bool IsActive() const noexcept final
    {
        sLock lock(shared_lock_);

        return item_->isActive();
    }
    bool IsPrimary() const noexcept final
    {
        sLock lock(shared_lock_);

        return item_->isPrimary();
    }
    std::string Value() const noexcept final
    {
        sLock lock(shared_lock_);

        return item_->Value();
    }

    void reindex(
        const ContactSubsectionSortKey& key,
        const CustomData& custom) noexcept final;

    ContactItem(
        const ContactSubsectionInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ContactSubsectionRowID& rowID,
        const ContactSubsectionSortKey& sortKey,
        const CustomData& custom) noexcept;
    ~ContactItem() = default;

private:
    friend opentxs::Factory;

    std::unique_ptr<opentxs::ContactItem> item_;

    ContactItem() = delete;
    ContactItem(const ContactItem&) = delete;
    ContactItem(ContactItem&&) = delete;
    ContactItem& operator=(const ContactItem&) = delete;
    ContactItem& operator=(ContactItem&&) = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ContactItem>;
