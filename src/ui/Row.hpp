// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "RowType.hpp"
#include "Widget.hpp"

namespace opentxs::ui::implementation
{
template <typename InterfaceType, typename ParentType, typename IdentifierType>
class Row : public RowType<InterfaceType, ParentType, IdentifierType>,
            public Widget,
            public Lockable
{
protected:
    Row(const ParentType& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const IdentifierType id,
        const bool valid)
        : RowType<InterfaceType, ParentType, IdentifierType>(parent, id, valid)
        , Widget(api, publisher, parent.WidgetID())
    {
    }
    Row() = delete;
    Row(const Row&) = delete;
    Row(Row&&) = delete;
    Row& operator=(const Row&) = delete;
    Row& operator=(Row&&) = delete;

    virtual ~Row() = default;
};
}  // namespace opentxs::ui::implementation
