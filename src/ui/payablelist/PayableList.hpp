// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/PayableList.cpp"

#pragma once

#include <map>
#include <utility>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/ui/PayableList.hpp"
#include "ui/base/List.hpp"
#include "ui/base/Widget.hpp"

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
using PayableListList = List<
    PayableExternalInterface,
    PayableInternalInterface,
    PayableListRowID,
    PayableListRowInterface,
    PayableListRowInternal,
    PayableListRowBlank,
    PayableListSortKey,
    PayablePrimaryID>;

class PayableList final : public PayableListList
{
public:
    auto ID() const -> const Identifier& final;

    PayableList(
        const api::client::internal::Manager& api,
        const identifier::Nym& nymID,
        const proto::ContactItemType& currency,
        const SimpleCallback& cb);
    ~PayableList() final;

private:
    const ListenerDefinitions listeners_;
    const OTIdentifier owner_contact_id_;
    const proto::ContactItemType currency_;

    auto construct_row(
        const PayableListRowID& id,
        const PayableListSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto last(const PayableListRowID& id) const noexcept -> bool final
    {
        return PayableListList::last(id);
    }

    void process_contact(
        const PayableListRowID& id,
        const PayableListSortKey& key);
    void process_contact(const network::zeromq::Message& message);
    void process_nym(const network::zeromq::Message& message);
    void startup();

    PayableList() = delete;
    PayableList(const PayableList&) = delete;
    PayableList(PayableList&&) = delete;
    auto operator=(const PayableList&) -> PayableList& = delete;
    auto operator=(PayableList &&) -> PayableList& = delete;
};
}  // namespace opentxs::ui::implementation
