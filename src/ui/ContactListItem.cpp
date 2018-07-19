// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/ContactManager.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/ContactListItem.hpp"

#include <locale>

#include "ContactListParent.hpp"
#include "Row.hpp"

#include "ContactListItem.hpp"

template class opentxs::SharedPimpl<opentxs::ui::ContactListItem>;

namespace opentxs
{
ui::ContactListItem* Factory::ContactListItem(
    const ui::implementation::ContactListParent& parent,
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const Identifier& id,
    const std::string& name)
{
    return new ui::implementation::ContactListItem(
        parent, zmq, publisher, contact, id, name);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
ContactListItem::ContactListItem(
    const ContactListParent& parent,
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const Identifier& id,
    const std::string& name)
    : ContactListItemType(parent, zmq, publisher, contact, id, true)
    , name_(name)
    , contact_subscriber_callback_(network::zeromq::ListenCallback::Factory(
          [this](const network::zeromq::Message& message) -> void {
              this->process_contact(message);
          }))
    , contact_subscriber_(
          zmq_.SubscribeSocket(contact_subscriber_callback_.get()))
{
    const auto listening = contact_subscriber_->Start(
        network::zeromq::Socket::ContactUpdateEndpoint);

    OT_ASSERT(listening)

    UpdateNotify();
}

std::string ContactListItem::ContactID() const { return id_->str(); }

std::string ContactListItem::DisplayName() const
{
    Lock lock(lock_);

    return name_;
}

std::string ContactListItem::ImageURI() const
{
    // TODO

    return {};
}

void ContactListItem::process_contact(const network::zeromq::Message& message)
{
    OT_ASSERT(1 == message.Body().size());

    const OTIdentifier contactID =
        Identifier::Factory({std::string(*message.Body().begin())});

    if (id_ != contactID) { return; }

    Lock lock(lock_);
    name_ = contact_.ContactName(id_);
}

std::string ContactListItem::Section() const
{
    Lock lock(lock_);

    if (id_ == parent_.ID()) { return {"ME"}; }

    if (name_.empty()) { return {" "}; }

    std::locale loc;
    std::string output{" "};
    output[0] = std::toupper(name_[0], loc);

    return output;
}

void ContactListItem::SetName(const std::string& name)
{
    Lock lock(lock_);
    name_ = name;
}
}  // namespace opentxs::ui::implementation
