// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/PayableList.hpp"

#include "internal/ui/UI.hpp"
#include "List.hpp"

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
    const Identifier& ID() const final;

#if OT_QT
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
        noexcept final;
#endif
    ~PayableList();

private:
    friend opentxs::Factory;

    const ListenerDefinitions listeners_;
    const OTIdentifier owner_contact_id_;
    const proto::ContactItemType currency_;

    void construct_row(
        const PayableListRowID& id,
        const PayableListSortKey& index,
        const CustomData& custom) const noexcept final;
    bool last(const PayableListRowID& id) const noexcept final
    {
        return PayableListList::last(id);
    }

    void process_contact(
        const PayableListRowID& id,
        const PayableListSortKey& key);
    void process_contact(const network::zeromq::Message& message);
    void process_nym(const network::zeromq::Message& message);
    void startup();

    PayableList(
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const proto::ContactItemType& currency
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback,
        const RowCallbacks removeCallback
#endif
    );
    PayableList() = delete;
    PayableList(const PayableList&) = delete;
    PayableList(PayableList&&) = delete;
    PayableList& operator=(const PayableList&) = delete;
    PayableList& operator=(PayableList&&) = delete;
};
}  // namespace opentxs::ui::implementation
