// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ROW_TYPE_HPP
#define OPENTXS_UI_ROW_TYPE_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
template <typename InterfaceType, typename ParentType, typename IdentifierType>
class RowType : virtual public InterfaceType
{
public:
    bool Last() const override { return parent_.last(id_); }
    bool Valid() const override { return valid_; }

protected:
    const ParentType& parent_;
    const IdentifierType id_;
    const bool valid_{false};

    RowType(const ParentType& parent, const IdentifierType id, const bool valid)
        : parent_(parent)
        , id_(id)
        , valid_(valid)
    {
    }
    RowType() = delete;
    RowType(const RowType&) = delete;
    RowType(RowType&&) = delete;
    RowType& operator=(const RowType&) = delete;
    RowType& operator=(RowType&&) = delete;

    virtual ~RowType() = default;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_ROW_TYPE_HPP
