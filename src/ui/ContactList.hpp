// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/ContactList.cpp"

#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "ui/List.hpp"
#include "ui/Widget.hpp"

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

namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ContactListList = List<
    ContactListExternalInterface,
    ContactListInternalInterface,
    ContactListRowID,
    ContactListRowInterface,
    ContactListRowInternal,
    ContactListRowBlank,
    ContactListSortKey,
    ContactListPrimaryID>;

class ContactList final : public ContactListList
{
public:
    auto AddContact(
        const std::string& label,
        const std::string& paymentCode,
        const std::string& nymID) const noexcept -> std::string final;
    auto ID() const noexcept -> const Identifier& final
    {
        return owner_contact_id_;
    }
#if OT_QT
    QModelIndex index(
        int row,
        int column,
        const QModelIndex& parent = QModelIndex()) const noexcept final;
#endif

    ContactList(
        const api::client::internal::Manager& api,
        const identifier::Nym& nymID,
#if OT_QT
        const bool qt,
#endif
        const SimpleCallback& cb) noexcept;
    ~ContactList() final;

private:
    struct ParsedArgs {
        OTNymID nym_id_;
        OTPaymentCode payment_code_;

        ParsedArgs(
            const api::internal::Core& api,
            const std::string& purportedID,
            const std::string& purportedPaymentCode) noexcept;

    private:
        static auto extract_nymid(
            const api::internal::Core& api,
            const std::string& purportedID,
            const std::string& purportedPaymentCode) noexcept -> OTNymID;
        static auto extract_paymentcode(
            const api::internal::Core& api,
            const std::string& purportedID,
            const std::string& purportedPaymentCode) noexcept -> OTPaymentCode;
    };

    const ListenerDefinitions listeners_;
    const OTIdentifier owner_contact_id_;
    std::shared_ptr<ContactListRowInternal> owner_;

    auto construct_row(
        const ContactListRowID& id,
        const ContactListSortKey& index,
        CustomData& custom) const noexcept -> void* final;
    auto first(const Lock& lock) const noexcept
        -> std::shared_ptr<const ContactListRowInternal> final;
    auto last(const ContactListRowID& id) const noexcept -> bool final
    {
        return ContactListList::last(id);
    }

    void add_item(
        const ContactListRowID& id,
        const ContactListSortKey& index,
        CustomData& custom) noexcept final;
    void process_contact(const network::zeromq::Message& message) noexcept;

    void startup() noexcept;

    ContactList() = delete;
    ContactList(const ContactList&) = delete;
    ContactList(ContactList&&) = delete;
    auto operator=(const ContactList&) -> ContactList& = delete;
    auto operator=(ContactList &&) -> ContactList& = delete;
};
}  // namespace opentxs::ui::implementation
