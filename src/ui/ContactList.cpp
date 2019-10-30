// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/ui/ContactListItem.hpp"

#include "internal/api/client/Client.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "ContactList.hpp"

#define OT_METHOD "opentxs::ui::implementation::ContactList::"

namespace opentxs
{
ui::implementation::ContactList* Factory::ContactListModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID
#if OT_QT
    ,
    const bool qt
#endif
)
{
    return new ui::implementation::ContactList(
        api,
        publisher,
        nymID
#if OT_QT
        ,
        qt
#endif
    );
}

#if OT_QT
ui::ContactListQt* Factory::ContactListQtModel(
    ui::implementation::ContactList& parent)
{
    using ReturnType = ui::ContactListQt;

    return new ReturnType(parent);
}
#endif  // OT_QT
}  // namespace opentxs

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
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept
    : ContactListList(
          api,
          publisher,
          nymID
#if OT_QT
          ,
          qt,
          Roles{{ContactListQt::ContactIDRole, "id"},
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
    owner_.reset(Factory::ContactListItem(
        *this, api, publisher_, owner_contact_id_, "Owner"));

    OT_ASSERT(owner_)

    last_id_ = owner_contact_id_;

    OT_ASSERT(false == owner_contact_id_->empty())

    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&ContactList::startup, this));

    OT_ASSERT(startup_)
}

std::string ContactList::AddContact(
    const std::string& label,
    const std::string& paymentCode,
    const std::string& nymID) const noexcept
{
    auto reason = api_.Factory().PasswordPrompt("Adding a new contact");
    const auto contact = api_.Contacts().NewContact(
        label,
        identifier::Nym::Factory(nymID),
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        api_.Factory().PaymentCode(paymentCode, reason),
#endif
        reason);
    const auto& id = contact->ID();
    api_.OTX().CanMessage(primary_id_, id, true);

    return id.str();
}

void ContactList::add_item(
    const ContactListRowID& id,
    const ContactListSortKey& index,
    const CustomData& custom) noexcept
{
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Widget ID: ")(WidgetID()).Flush();

    if (owner_contact_id_ == id) {
        owner_->reindex(index, custom);
        UpdateNotify();

        return;
    }

    ContactListList::add_item(id, index, custom);
}

void ContactList::construct_row(
    const ContactListRowID& id,
    const ContactListSortKey& index,
    const CustomData&) const noexcept
{
    names_.emplace(id, index);
    items_[index].emplace(
        id, Factory::ContactListItem(*this, api_, publisher_, id, index));
}

/** Returns owner contact. Sets up iterators for next row */
std::shared_ptr<const ContactListRowInternal> ContactList::first(
    const Lock& lock) const noexcept
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
    add_item(contactID, name, {});
}

void ContactList::startup() noexcept
{
    const auto contacts = api_.Contacts().ContactList();
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Loading ")(contacts.size())(
        " contacts.")
        .Flush();

    for (const auto& [id, alias] : contacts) {
        add_item(Identifier::Factory(id), alias, {});
    }

    finish_startup();
}

ContactList::~ContactList()
{
    for (auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
