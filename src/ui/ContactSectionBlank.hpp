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

#ifndef OPENTXS_UI_CONTACT_SECTION_BLANK_IMPLEMENTATION_HPP
#define OPENTXS_UI_CONTACT_SECTION_BLANK_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/ui/ContactSection.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs::ui::implementation
{
class ContactSectionBlank : virtual public ui::ContactSection,
                            virtual public opentxs::ui::Widget
{
public:
    std::string Name(const std::string& lang) const override { return {}; }
    const opentxs::ui::ContactSubsection& First() const override
    {
        return *static_cast<const opentxs::ui::ContactSubsection*>(nullptr);
    }
    bool Last() const override { return true; }
    const opentxs::ui::ContactSubsection& Next() const override
    {
        return *static_cast<const opentxs::ui::ContactSubsection*>(nullptr);
    }
    proto::ContactSectionName Type() const override { return {}; }
    int SectionType() const override { return {}; }
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }
    std::string WidgetName() const override { return {}; }

    void Update(const opentxs::ContactSection& section) override {}

    ~ContactSectionBlank() = default;

protected:
    ContactSectionBlank() = default;

private:
    friend opentxs::ui::implementation::Contact;

    ContactSectionBlank(const ContactSectionBlank&) = delete;
    ContactSectionBlank(ContactSectionBlank&&) = delete;
    ContactSectionBlank& operator=(const ContactSectionBlank&) = delete;
    ContactSectionBlank& operator=(ContactSectionBlank&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACT_SECTION_BLANK_IMPLEMENTATION_HPP
