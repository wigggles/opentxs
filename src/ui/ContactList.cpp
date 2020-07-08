// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"        // IWYU pragma: associated
#include "1_Internal.hpp"      // IWYU pragma: associated
#include "ui/ContactList.hpp"  // IWYU pragma: associated

#include <list>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "ui/List.hpp"

#define OT_METHOD "opentxs::ui::implementation::ContactList::"

namespace opentxs::factory
{
auto ContactListModel(
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
#if OT_QT
    const bool qt,
#endif
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::implementation::ContactList>
{
    using ReturnType = ui::implementation::ContactList;

    return std::make_unique<ReturnType>(
        api,
        nymID,
#if OT_QT
        qt,
#endif
        cb);
}

#if OT_QT
auto ContactListQtModel(ui::implementation::ContactList& parent) noexcept
    -> std::unique_ptr<ui::ContactListQt>
{
    using ReturnType = ui::ContactListQt;

    return std::make_unique<ReturnType>(parent);
}
#endif  // OT_QT
}  // namespace opentxs::factory

#if OT_QT
namespace opentxs::ui
{
QT_PROXY_MODEL_WRAPPER(ContactListQt, implementation::ContactList)

QString ContactListQt::addContact(
    const QString& label,
    const QString& paymentCode,
    const QString& nymID) const noexcept
{
    return parent_
        .AddContact(
            label.toStdString(), paymentCode.toStdString(), nymID.toStdString())
        .c_str();
}
}  // namespace opentxs::ui
#endif

namespace opentxs::ui::implementation
{
ContactList::ContactList(
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
#if OT_QT
    const bool qt,
#endif
    const SimpleCallback& cb) noexcept
    : ContactListList(
          api,
          nymID,
          cb
#if OT_QT
          ,
          qt,
          Roles{
              {ContactListQt::ContactIDRole, "id"},
              {ContactListQt::SectionRole, "section"}},
          1,
          1
#endif
          )
    , listeners_({
          {api_.Endpoints().ContactUpdate(),
           new MessageProcessor<ContactList>(&ContactList::process_contact)},
      })
    , owner_contact_id_(api_.Contacts().ContactID(nymID))
    , owner_(nullptr)
{
    owner_ = factory::ContactListItem(*this, api, owner_contact_id_, "Owner");

    OT_ASSERT(owner_)

#if OT_QT
    register_child(owner_.get());
#endif  // OT_QT
    last_id_ = owner_contact_id_;

    OT_ASSERT(false == owner_contact_id_->empty())

    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&ContactList::startup, this));

    OT_ASSERT(startup_)
}

ContactList::ParsedArgs::ParsedArgs(
    const api::internal::Core& api,
    const std::string& purportedID,
    const std::string& purportedPaymentCode) noexcept
    : nym_id_(extract_nymid(api, purportedID, purportedPaymentCode))
    , payment_code_(extract_paymentcode(api, purportedID, purportedPaymentCode))
{
}

auto ContactList::ParsedArgs::extract_nymid(
    const api::internal::Core& api,
    const std::string& purportedID,
    const std::string& purportedPaymentCode) noexcept -> OTNymID
{
    auto output = api.Factory().NymID();

    if (false == purportedID.empty()) {
        // Case 1: purportedID is a nym id
        output = api.Factory().NymID(purportedID);

        if (false == output->empty()) { return output; }

        // Case 2: purportedID is a payment code
        output = api.Factory().NymIDFromPaymentCode(purportedID);

        if (false == output->empty()) { return output; }
    }

    if (false == purportedPaymentCode.empty()) {
        // Case 3: purportedPaymentCode is a payment code
        output = api.Factory().NymIDFromPaymentCode(purportedPaymentCode);

        if (false == output->empty()) { return output; }

        // Case 4: purportedPaymentCode is a nym id
        output->SetString(purportedPaymentCode);

        if (false == output->empty()) { return output; }
    }

    // Case 5: not possible to extract a nym id

    return output;
}

auto ContactList::ParsedArgs::extract_paymentcode(
    const api::internal::Core& api,
    const std::string& purportedID,
    const std::string& purportedPaymentCode) noexcept -> OTPaymentCode
{
    if (false == purportedPaymentCode.empty()) {
        // Case 1: purportedPaymentCode is a payment code
        auto output = api.Factory().PaymentCode(purportedPaymentCode);

        if (output->Valid()) { return output; }
    }

    if (false == purportedID.empty()) {
        // Case 2: purportedID is a payment code
        auto output = api.Factory().PaymentCode(purportedID);

        if (output->Valid()) { return output; }
    }

    // Case 3: not possible to extract a payment code

    return api.Factory().PaymentCode("");
}

auto ContactList::AddContact(
    const std::string& label,
    const std::string& paymentCode,
    const std::string& nymID) const noexcept -> std::string
{
    auto args = ParsedArgs{api_, nymID, paymentCode};
    const auto contact =
        api_.Contacts().NewContact(label, args.nym_id_, args.payment_code_);
    const auto& id = contact->ID();
    api_.OTX().CanMessage(primary_id_, id, true);

    return id.str();
}

void ContactList::add_item(
    const ContactListRowID& id,
    const ContactListSortKey& index,
    CustomData& custom) noexcept
{
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Widget ID: ")(WidgetID()).Flush();

    if (owner_contact_id_ == id) {
        owner_->reindex(index, custom);
        UpdateNotify();

        return;
    }

    ContactListList::add_item(id, index, custom);
}

auto ContactList::construct_row(
    const ContactListRowID& id,
    const ContactListSortKey& index,
    CustomData&) const noexcept -> void*
{
    names_.emplace(id, index);
    const auto [it, added] = items_[index].emplace(
        id, factory::ContactListItem(*this, api_, id, index));

    return it->second.get();
}

/** Returns owner contact. Sets up iterators for next row */
auto ContactList::first(const Lock& lock) const noexcept
    -> std::shared_ptr<const ContactListRowInternal>
{
    OT_ASSERT(verify_lock(lock))

    have_items_->Set(first_valid_item(lock));
    start_->Set(!have_items_.get());
    last_id_ = owner_contact_id_;

    return owner_;
}

#if OT_QT
QModelIndex ContactList::index(int row, int column, const QModelIndex& parent)
    const noexcept
{
    const auto* pointer = get_pointer(parent);

    if (nullptr == parent.internalPointer()) {
        if (0 == row) {
            return createIndex(row, column, owner_.get());
        } else {
            return get_index(row, column);
        }
    } else {
        return pointer->qt_index(row, column);
    }
}
#endif

void ContactList::process_contact(
    const network::zeromq::Message& message) noexcept
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const auto contactID = Identifier::Factory(*message.Body().begin());

    OT_ASSERT(false == contactID->empty())

    const auto name = api_.Contacts().ContactName(contactID);
    auto custom = CustomData{};
    add_item(contactID, name, custom);
}

void ContactList::startup() noexcept
{
    const auto contacts = api_.Contacts().ContactList();
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Loading ")(contacts.size())(
        " contacts.")
        .Flush();

    for (const auto& [id, alias] : contacts) {
        auto custom = CustomData{};
        add_item(Identifier::Factory(id), alias, custom);
    }

    finish_startup();
}

ContactList::~ContactList()
{
    for (auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
