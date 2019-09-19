// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/ui/Contact.hpp"
#include "opentxs/ui/ContactSection.hpp"

#include "internal/ui/UI.hpp"
#include "List.hpp"

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <tuple>
#include <vector>

#include "Contact.hpp"

#define OT_METHOD "opentxs::ui::implementation::Contact::"

#if OT_QT
namespace opentxs::ui
{
QT_MODEL_WRAPPER(ContactQt, Contact)

QString ContactQt::displayName() const noexcept
{
    return parent_->DisplayName().c_str();
}
QString ContactQt::contactID() const noexcept
{
    return parent_->ContactID().c_str();
}
QString ContactQt::paymentCode() const noexcept
{
    return parent_->PaymentCode().c_str();
}
}  // namespace opentxs::ui
#endif

namespace opentxs::ui::implementation
{
const std::set<proto::ContactSectionName> Contact::allowed_types_{
    proto::CONTACTSECTION_COMMUNICATION,
    proto::CONTACTSECTION_PROFILE};

const std::map<proto::ContactSectionName, int> Contact::sort_keys_{
    {proto::CONTACTSECTION_COMMUNICATION, 0},
    {proto::CONTACTSECTION_PROFILE, 1}};

Contact::Contact(
    const api::client::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const Identifier& contactID
#if OT_QT
    ,
    const bool qt,
    const RowCallbacks insertCallback,
    const RowCallbacks removeCallback
#endif
    ) noexcept
    : ContactType(
          api,
          publisher,
          contactID
#if OT_QT
          ,
          qt,
          insertCallback,
          removeCallback
#endif
          )
    , listeners_({
          {api_.Endpoints().ContactUpdate(),
           new MessageProcessor<Contact>(&Contact::process_contact)},
      })
    , name_(api_.Contacts().ContactName(contactID))
    , payment_code_()
{
    // NOTE nym_id_ is actually the contact id
    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&Contact::startup, this));

    OT_ASSERT(startup_)
}

bool Contact::check_type(const proto::ContactSectionName type) noexcept
{
    return 1 == allowed_types_.count(type);
}

void Contact::construct_row(
    const ContactRowID& id,
    const ContactSortKey& index,
    const CustomData& custom) const noexcept
{
    names_.emplace(id, index);
    items_[index].emplace(
        id,
        Factory::ContactSectionWidget(
            *this,
            api_,
            publisher_,
            id,
            index,
            custom
#if OT_QT
            ,
            enable_qt_,
            insert_callbacks_,
            remove_callbacks_
#endif
            ));
}

std::string Contact::ContactID() const noexcept { return primary_id_->str(); }

std::string Contact::DisplayName() const noexcept
{
    Lock lock(lock_);

    return name_;
}

std::string Contact::PaymentCode() const noexcept
{
    Lock lock(lock_);

    return payment_code_;
}

void Contact::process_contact(const opentxs::Contact& contact) noexcept
{
    Lock lock(lock_);
    name_ = contact.Label();
    payment_code_ = contact.PaymentCode();
    lock.unlock();
    UpdateNotify();
    std::set<ContactRowID> active{};
    const auto data = contact.Data();

    if (data) {
        for (const auto& section : *data) {
            auto& type = section.first;

            if (check_type(type)) {
                CustomData custom{new opentxs::ContactSection(*section.second)};
                add_item(type, sort_key(type), custom);
                active.emplace(type);
            }
        }
    } else {
        items_.clear();
    }

    delete_inactive(active);
}

void Contact::process_contact(const network::zeromq::Message& message) noexcept
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto contactID = Identifier::Factory(id);

    OT_ASSERT(false == contactID->empty())

    if (contactID != primary_id_) { return; }

    const auto contact = api_.Contacts().Contact(contactID, reason);

    OT_ASSERT(contact)

    process_contact(*contact);
}

int Contact::sort_key(const proto::ContactSectionName type) noexcept
{
    return sort_keys_.at(type);
}

void Contact::startup() noexcept
{
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Loading contact ")(primary_id_)
        .Flush();
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    const auto contact = api_.Contacts().Contact(primary_id_, reason);

    OT_ASSERT(contact)

    process_contact(*contact);
    finish_startup();
}

Contact::~Contact()
{
    for (auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
