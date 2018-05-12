/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_UI_CONTACTSUBSECTION_HPP
#define OPENTXS_UI_CONTACTSUBSECTION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/ui/ListRow.hpp"
#include "opentxs/Proto.hpp"

#include <string>

#ifdef SWIG
// clang-format off
%ignore opentxs::ui::ContactSubsection::Update;
%rename(UIContactSubsection) opentxs::ui::ContactSubsection;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class ContactSubsection : virtual public ListRow
{
public:
    EXPORT virtual std::string Name(const std::string& lang) const = 0;
    EXPORT virtual const ContactItem& First() const = 0;
    EXPORT virtual const ContactItem& Next() const = 0;
#ifndef SWIG
    EXPORT virtual proto::ContactItemType Type() const = 0;
#endif
    EXPORT virtual int SubsectionType() const = 0;

    virtual void Update(const opentxs::ContactGroup& group) = 0;

    EXPORT virtual ~ContactSubsection() = default;

protected:
    ContactSubsection() = default;

private:
    ContactSubsection(const ContactSubsection&) = delete;
    ContactSubsection(ContactSubsection&&) = delete;
    ContactSubsection& operator=(const ContactSubsection&) = delete;
    ContactSubsection& operator=(ContactSubsection&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif  // OPENTXS_UI_CONTACTSUBSECTION_HPP
