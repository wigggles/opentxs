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

#include "opentxs/Version.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/core/Identifier.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <tuple>

namespace opentxs
{

class Contact;
class PaymentCode;

namespace api
{

class Storage;
class Wallet;

namespace implementation
{
class Native;
}

class ContactManager
{
public:
    Identifier BlockchainAddressToContact(
        const std::string& address,
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC) const;
    Identifier ContactID(const Identifier& nymID) const;
    ObjectList ContactList() const;

    std::shared_ptr<const class Contact> Contact(const Identifier& id);
    std::shared_ptr<const class Contact> Merge(
        const Identifier& parent,
        const Identifier& child);
    std::unique_ptr<Editor<class Contact>> mutable_Contact(
        const Identifier& id);
    std::shared_ptr<const class Contact> NewContact(const std::string& label);
    std::shared_ptr<const class Contact> NewContact(
        const std::string& label,
        const Identifier& nymID,
        const PaymentCode& paymentCode);
    std::shared_ptr<const class Contact> NewContactFromAddress(
        const std::string& address,
        const std::string& label,
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC);
    std::shared_ptr<const class Contact> Update(
        const proto::CredentialIndex& nym);

    ~ContactManager() = default;

private:
    friend class implementation::Native;

    typedef std::pair<std::mutex, std::shared_ptr<class Contact>> ContactLock;
    typedef std::pair<proto::ContactItemType, std::string> Address;
    typedef std::map<Identifier, ContactLock> ContactMap;

    Storage& storage_;
    Wallet& wallet_;
    mutable std::recursive_mutex lock_{};
    ContactMap contact_map_{};

    void check_identifiers(
        const Identifier& inputNymID,
        const PaymentCode& paymentCode,
        bool& haveNymID,
        bool& havePaymentCode,
        Identifier& outputNymID) const;
    bool verify_write_lock(const rLock& lock) const;

    // takes ownership
    ContactMap::iterator add_contact(const rLock& lock, class Contact* contact);
    Identifier address_to_contact(
        const rLock& lock,
        const std::string& address,
        const proto::ContactItemType currency) const;
    std::shared_ptr<const class Contact> contact(
        const rLock& lock,
        const std::string& label);
    std::shared_ptr<const class Contact> contact(
        const rLock& lock,
        const Identifier& id);
    void import_contacts(const rLock& lock);
    void init_nym_map(const rLock& lock);
    ContactMap::iterator load_contact(const rLock& lock, const Identifier& id);
    std::unique_ptr<Editor<class Contact>> mutable_contact(
        const rLock& lock,
        const Identifier& id);
    ContactMap::iterator obtain_contact(
        const rLock& lock,
        const Identifier& id);
    std::shared_ptr<const class Contact> new_contact(
        const rLock& lock,
        const std::string& label,
        const Identifier& nymID,
        const PaymentCode& paymentCode);
    void refresh_indices(const rLock& lock, class Contact& contact);
    void save(class Contact* contact);
    void start();
    std::shared_ptr<const class Contact> update_existing_contact(
        const rLock& lock,
        const std::string& label,
        const PaymentCode& code,
        const Identifier& contactID);
    void update_nym_map(
        const rLock& lock,
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
}  // namespace api
}  // namespace opentxs

#endif  // OPENTXS_API_CONTACT_MANAGER_HPP
