// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ISSUER_ITEM_PARENT_HPP
#define OPENTXS_UI_ISSUER_ITEM_PARENT_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
class IssuerItemParent
{
public:
    virtual bool last(const IssuerItemRowID& id) const = 0;
    virtual void reindex_item(
        const IssuerItemRowID& id,
        const IssuerItemSortKey& newIndex) const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~IssuerItemParent() = default;

protected:
    IssuerItemParent() = default;
    IssuerItemParent(const IssuerItemParent&) = delete;
    IssuerItemParent(IssuerItemParent&&) = delete;
    IssuerItemParent& operator=(const IssuerItemParent&) = delete;
    IssuerItemParent& operator=(IssuerItemParent&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_ISSUER_ITEM_PARENT_HPP
