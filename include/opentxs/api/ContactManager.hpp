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

#include "opentxs/api/Editor.hpp"
#include "opentxs/core/Identifier.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <tuple>

namespace opentxs
{

class Contact;
class OT;
class PaymentCode;
class Storage;
class Wallet;

class ContactManager
{
public:
    Identifier ContactID(const Identifier& nymID) const;
    ObjectList ContactList() const;

    std::shared_ptr<const class Contact> Contact(const Identifier& id);
    std::unique_ptr<Editor<class Contact>> mutable_Contact(
        const Identifier& id);
    std::shared_ptr<const class Contact> NewContact(const std::string& label);
    std::shared_ptr<const class Contact> NewContact(
        const std::string& label,
        const Identifier& nymID,
        const PaymentCode& paymentCode);
    std::shared_ptr<const class Contact> Update(
        const proto::CredentialIndex& nym);

    ~ContactManager() = default;

private:
    friend class OT;

    typedef std::pair<std::mutex, std::shared_ptr<class Contact>> ContactLock;
    typedef std::map<Identifier, ContactLock> ContactMap;
    typedef std::map<Identifier, Identifier> NymMap;

    Storage& storage_;
    Wallet& wallet_;
    mutable std::mutex lock_{};
    ContactMap contact_map_{};
    NymMap nym_contact_map_{};

    void check_identifiers(
        const Identifier& inputNymID,
        const PaymentCode& paymentCode,
        bool& haveNymID,
        bool& havePaymentCode,
        Identifier& outputNymID) const;
    bool verify_write_lock(const Lock& lock) const;

    // takes ownership
    ContactMap::iterator add_contact(const Lock& lock, class Contact* contact);
    std::shared_ptr<const class Contact> contact(
        const Lock& lock,
        const std::string& label);
    std::shared_ptr<const class Contact> contact(
        const Lock& lock,
        const Identifier& id);
    void import_contacts(const Lock& lock);
    void init_nym_map(const Lock& lock);
    ContactMap::iterator load_contact(const Lock& lock, const Identifier& id);
    std::unique_ptr<Editor<class Contact>> mutable_contact(
        const Lock& lock,
        const Identifier& id);
    ContactMap::iterator obtain_contact(const Lock& lock, const Identifier& id);
    std::shared_ptr<const class Contact> new_contact(
        const Lock& lock,
        const std::string& label,
        const Identifier& nymID,
        const PaymentCode& paymentCode);
    void save(class Contact* contact);
    std::shared_ptr<const class Contact> update_existing_contact(
        const Lock& lock,
        const std::string& label,
        const PaymentCode& code,
        NymMap::iterator& existing);
    void update_nym_map(
        const Lock& lock,
        const Identifier nymID,
        class Contact& contact,
        const bool replace = false);

    ContactManager(Storage& storage, Wallet& wallet);
    ContactManager() = delete;
    ContactManager(const ContactManager&) = delete;
    ContactManager(ContactManager&&) = delete;
    ContactManager operator=(const ContactManager&) = delete;
    ContactManager operator=(ContactManager&&) = delete;
};
}  // namespace opentxs
#endif  // OPENTXS_API_CONTACT_MANAGER_HPP
