// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"        // IWYU pragma: associated
#include "1_Internal.hpp"      // IWYU pragma: associated
#include "ui/PayableList.hpp"  // IWYU pragma: associated

#include <list>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "ui/List.hpp"

#define OT_METHOD "opentxs::ui::implementation::PayableList::"

#if OT_QT
namespace opentxs::ui
{
QT_PROXY_MODEL_WRAPPER(PayableListQt, implementation::PayableList)
}  // namespace opentxs::ui
#endif

namespace opentxs::factory
{
auto PayableListModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const proto::ContactItemType& currency
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<ui::implementation::PayableList>
{
    using ReturnType = ui::implementation::PayableList;

    return std::make_unique<ReturnType>(
        api,
        publisher,
        nymID,
        currency
#if OT_QT
        ,
        qt
#endif
    );
}

#if OT_QT
auto PayableListQtModel(ui::implementation::PayableList& parent) noexcept
    -> std::unique_ptr<ui::PayableListQt>
{
    using ReturnType = ui::PayableListQt;

    return std::make_unique<ReturnType>(parent);
}
#endif  // OT_QT
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
PayableList::PayableList(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const proto::ContactItemType& currency
#if OT_QT
    ,
    const bool qt
#endif
    )
    : PayableListList(
          api,
          publisher,
          nymID
#if OT_QT
          ,
          qt,
          Roles{
              {PayableListQt::ContactIDRole, "id"},
              {PayableListQt::SectionRole, "section"}},
          2
#endif
          )
    , listeners_({
          {api_.Endpoints().ContactUpdate(),
           new MessageProcessor<PayableList>(&PayableList::process_contact)},
          {api_.Endpoints().NymDownload(),
           new MessageProcessor<PayableList>(&PayableList::process_nym)},
      })
    , owner_contact_id_(Identifier::Factory(last_id_))
    , currency_(currency)
{
    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&PayableList::startup, this));

    OT_ASSERT(startup_)
}

auto PayableList::construct_row(
    const PayableListRowID& id,
    const PayableListSortKey& index,
    CustomData& custom) const noexcept -> void*
{
    OT_ASSERT(1 == custom.size())

    std::unique_ptr<const std::string> paymentCode;
    paymentCode.reset(static_cast<const std::string*>(custom[0]));

    OT_ASSERT(paymentCode);
    OT_ASSERT(false == paymentCode->empty());

    names_.emplace(id, index);
    const auto [it, added] = items_[index].emplace(
        id,
        factory::PayableListItem(
            *this, api_, publisher_, id, index, *paymentCode, currency_));

    return it->second.get();
}

auto PayableList::ID() const -> const Identifier& { return owner_contact_id_; }

void PayableList::process_contact(
    const PayableListRowID& id,
    const PayableListSortKey& key)
{
    if (owner_contact_id_ == id) { return; }

    const auto contact = api_.Contacts().Contact(id);

    if (false == bool(contact)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: Contact ")(id)(
            " can not be loaded.")
            .Flush();

        return;
    }

    OT_ASSERT(contact);

    auto paymentCode =
        std::make_unique<std::string>(contact->PaymentCode(currency_));

    OT_ASSERT(paymentCode);

    if (!paymentCode->empty()) {
        auto custom = CustomData{paymentCode.release()};
        add_item(id, key, custom);
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Skipping unpayable contact ")(id)
            .Flush();
    }
}

void PayableList::process_contact(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto contactID = Identifier::Factory(id);

    OT_ASSERT(false == contactID->empty())

    const auto name = api_.Contacts().ContactName(contactID);
    process_contact(contactID, name);
}

void PayableList::process_nym(const network::zeromq::Message& message)
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

void PayableList::startup()
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

PayableList::~PayableList()
{
    for (auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
