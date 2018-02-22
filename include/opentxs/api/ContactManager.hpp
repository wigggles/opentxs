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
 *  fellowtraveler\opentransactions.org
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

#ifndef OPENTXS_API_CONTACT_MANAGER_HPP
#define OPENTXS_API_CONTACT_MANAGER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <memory>

namespace opentxs
{
namespace api
{
class ContactManager
{
public:
    EXPORT virtual Identifier BlockchainAddressToContact(
        const std::string& address,
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC) const = 0;
    EXPORT virtual std::shared_ptr<const class Contact> Contact(
        const Identifier& id) const = 0;
    EXPORT virtual Identifier ContactID(const Identifier& nymID) const = 0;
    EXPORT virtual ObjectList ContactList() const = 0;
    EXPORT virtual std::shared_ptr<const class Contact> Merge(
        const Identifier& parent,
        const Identifier& child) const = 0;
    EXPORT virtual std::unique_ptr<Editor<class Contact>> mutable_Contact(
        const Identifier& id) const = 0;
    EXPORT virtual std::shared_ptr<const class Contact> NewContact(
        const std::string& label) const = 0;
    EXPORT virtual std::shared_ptr<const class Contact> NewContact(
        const std::string& label,
        const Identifier& nymID,
        const PaymentCode& paymentCode) const = 0;
    EXPORT virtual std::shared_ptr<const class Contact> NewContactFromAddress(
        const std::string& address,
        const std::string& label,
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC) const = 0;
    EXPORT virtual std::shared_ptr<const class Contact> Update(
        const proto::CredentialIndex& nym) const = 0;

    virtual ~ContactManager() = default;

protected:
    ContactManager() = default;

private:
    ContactManager(const ContactManager&) = delete;
    ContactManager(ContactManager&&) = delete;
    ContactManager& operator=(const ContactManager&) = delete;
    ContactManager& operator=(ContactManager&&) = delete;
};
}  // namespace api
}  // namespace opentxs

#endif  // OPENTXS_API_CONTACT_MANAGER_HPP
