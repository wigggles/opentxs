// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/MessagableList.cpp"

#pragma once

#include <map>
#include <utility>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/ui/MessagableList.hpp"
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
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

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
using MessagableListList = List<
    MessagableExternalInterface,
    MessagableInternalInterface,
    MessagableListRowID,
    MessagableListRowInterface,
    MessagableListRowInternal,
    MessagableListRowBlank,
    MessagableListSortKey,
    MessagableListPrimaryID>;

class MessagableList final : public MessagableListList
{
public:
    const Identifier& ID() const noexcept final;

    MessagableList(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID
#if OT_QT
        ,
        const bool qt
#endif
        ) noexcept;
    ~MessagableList() final;

private:
    const ListenerDefinitions listeners_;
    const OTIdentifier owner_contact_id_;

    void* construct_row(
        const MessagableListRowID& id,
        const MessagableListSortKey& index,
        const CustomData& custom) const noexcept final;
    bool last(const MessagableListRowID& id) const noexcept final
    {
        return MessagableListList::last(id);
    }

    void process_contact(
        const MessagableListRowID& id,
        const MessagableListSortKey& key) noexcept;
    void process_contact(const network::zeromq::Message& message) noexcept;
    void process_nym(const network::zeromq::Message& message) noexcept;
    void startup() noexcept;

    MessagableList() = delete;
    MessagableList(const MessagableList&) = delete;
    MessagableList(MessagableList&&) = delete;
    MessagableList& operator=(const MessagableList&) = delete;
    MessagableList& operator=(MessagableList&&) = delete;
};
}  // namespace opentxs::ui::implementation
