// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "ui/MessagableList.hpp"  // IWYU pragma: associated

#include <list>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "ui/List.hpp"

#define OT_METHOD "opentxs::ui::implementation::MessagableList::"

namespace opentxs::factory
{
auto MessagableListModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<ui::implementation::MessagableList>
{
    using ReturnType = ui::implementation::MessagableList;

    return std::make_unique<ReturnType>(
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
auto MessagableListQtModel(ui::implementation::MessagableList& parent) noexcept
    -> std::unique_ptr<ui::MessagableListQt>
{
    using ReturnType = ui::MessagableListQt;

    return std::make_unique<ReturnType>(parent);
}
#endif  // OT_QT
}  // namespace opentxs::factory

#if OT_QT
namespace opentxs::ui
{
QT_PROXY_MODEL_WRAPPER(MessagableListQt, implementation::MessagableList)
}  // namespace opentxs::ui
#endif

namespace opentxs::ui::implementation
{
MessagableList::MessagableList(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept
    : MessagableListList(
          api,
          publisher,
          nymID
#if OT_QT
          ,
          qt,
          Roles{
              {MessagableListQt::ContactIDRole, "id"},
              {MessagableListQt::SectionRole, "section"}},
          1
#endif
          )
    , listeners_({
          {api_.Endpoints().ContactUpdate(),
           new MessageProcessor<MessagableList>(
               &MessagableList::process_contact)},
          {api_.Endpoints().NymDownload(),
           new MessageProcessor<MessagableList>(&MessagableList::process_nym)},
      })
    , owner_contact_id_(api_.Contacts().ContactID(nymID))
{
    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&MessagableList::startup, this));

    OT_ASSERT(startup_)
}

auto MessagableList::construct_row(
    const MessagableListRowID& id,
    const MessagableListSortKey& index,
    CustomData&) const noexcept -> void*
{
    names_.emplace(id, index);
    const auto [it, added] = items_[index].emplace(
        id, factory::ContactListItem(*this, api_, publisher_, id, index));

    return it->second.get();
}

auto MessagableList::ID() const noexcept -> const Identifier&
{
    return owner_contact_id_;
}

void MessagableList::process_contact(
    const MessagableListRowID& id,
    const MessagableListSortKey& key) noexcept
{
    if (owner_contact_id_ == id) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Skipping owner contact ")(id)(
            " (")(key)(")")
            .Flush();

        return;
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Incoming contact ")(id)(" (")(
            key)(") is not owner contact: (")(owner_contact_id_)(")")
            .Flush();
    }

    switch (api_.OTX().CanMessage(primary_id_, id, false)) {
        case Messagability::READY:
        case Messagability::MISSING_RECIPIENT:
        case Messagability::UNREGISTERED: {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Messagable contact ")(id)(
                " (")(key)(")")
                .Flush();
            auto custom = CustomData{};
            add_item(id, key, custom);
        } break;
        case Messagability::MISSING_SENDER:
        case Messagability::INVALID_SENDER:
        case Messagability::NO_SERVER_CLAIM:
        case Messagability::CONTACT_LACKS_NYM:
        case Messagability::MISSING_CONTACT:
        default: {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Skipping non-messagable contact ")(id)(" (")(key)(")")
                .Flush();

            if (0 < names_.count(id)) {
                Lock lock(lock_);
                delete_item(lock, id);
            }
        }
    }
}

void MessagableList::process_contact(
    const network::zeromq::Message& message) noexcept
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto contactID = Identifier::Factory(id);

    OT_ASSERT(false == contactID->empty())

    const auto name = api_.Contacts().ContactName(contactID);
    process_contact(contactID, name);
}

void MessagableList::process_nym(
    const network::zeromq::Message& message) noexcept
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto nymID = identifier::Nym::Factory(id);

    OT_ASSERT(false == nymID->empty())

    const auto contactID = api_.Contacts().ContactID(nymID);
    const auto name = api_.Contacts().ContactName(contactID);
    process_contact(contactID, name);
}

void MessagableList::startup() noexcept
{
    const auto contacts = api_.Contacts().ContactList();
    LogDetail(OT_METHOD)(__FUNCTION__)(": Loading ")(contacts.size())(
        " contacts.")
        .Flush();

    for (const auto& [id, alias] : contacts) {
        process_contact(Identifier::Factory(id), alias);
    }

    finish_startup();
}

MessagableList::~MessagableList()
{
    for (auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
