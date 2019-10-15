// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACTITEM_HPP
#define OPENTXS_UI_CONTACTITEM_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Types.hpp"

#include <string>

#include "ListRow.hpp"

#ifdef SWIG
// clang-format off
%template(OTUIContactItem) opentxs::SharedPimpl<opentxs::ui::ContactItem>;
%rename(UIContactItem) opentxs::ui::ContactItem;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class ContactItem : virtual public ListRow
{
public:
    EXPORT virtual std::string ClaimID() const noexcept = 0;
    EXPORT virtual bool IsActive() const noexcept = 0;
    EXPORT virtual bool IsPrimary() const noexcept = 0;
    EXPORT virtual std::string Value() const noexcept = 0;

    EXPORT ~ContactItem() override = default;

protected:
    ContactItem() noexcept = default;

private:
    ContactItem(const ContactItem&) = delete;
    ContactItem(ContactItem&&) = delete;
    ContactItem& operator=(const ContactItem&) = delete;
    ContactItem& operator=(ContactItem&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif
