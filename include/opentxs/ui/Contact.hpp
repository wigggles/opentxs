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

#ifndef OPENTXS_UI_CONTACT_HPP
#define OPENTXS_UI_CONTACT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/ui/Widget.hpp"
#include "opentxs/Proto.hpp"

#include <string>

#ifdef SWIG
// clang-format off
%shared_ptr(opentxs::ui::ContactSection)
%rename(UIContact) opentxs::ui::Contact;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class Contact : virtual public Widget
{
public:
    EXPORT virtual std::string ContactID() const = 0;
    EXPORT virtual std::string DisplayName() const = 0;
    EXPORT virtual std::shared_ptr<const ContactSection> First() const = 0;
    EXPORT virtual std::shared_ptr<const ContactSection> Next() const = 0;
    EXPORT virtual std::string PaymentCode() const = 0;

    EXPORT virtual ~Contact() = default;

protected:
    Contact() = default;

private:
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    Contact& operator=(const Contact&) = delete;
    Contact& operator=(Contact&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif  // OPENTXS_UI_CONTACT_HPP
