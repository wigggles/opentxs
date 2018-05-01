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

#ifndef OPENTXS_API_CONTACT_MANAGER_IMPLEMENTATION_HPP
#define OPENTXS_API_CONTACT_MANAGER_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/api/ContactManager.hpp"

#include <map>
#include <mutex>
#include <tuple>

namespace opentxs::api::implementation
{
class ContactManager : virtual public opentxs::api::ContactManager
{
public:
    OTIdentifier BlockchainAddressToContact(
        const std::string& address,
        const proto::ContactItemType currency =
            proto::CITEMTYPE_BTC) const override;
    std::shared_ptr<const class Contact> Contact(
        const Identifier& id) const override;
    OTIdentifier ContactID(const Identifier& nymID) const override;
    ObjectList ContactList() const override;
    std::string ContactName(const Identifier& contactID) const override;
    std::shared_ptr<const class Contact> Merge(
        const Identifier& parent,
        const Identifier& child) const override;
    std::unique_ptr<Editor<class Contact>> mutable_Contact(
        const Identifier& id) const override;
    std::shared_ptr<const class Contact> NewContact(
        const std::string& label) const override;
    std::shared_ptr<const class Contact> NewContact(
        const std::string& label,
        const Identifier& nymID,
        const PaymentCode& paymentCode) const override;
    std::shared_ptr<const class Contact> NewContactFromAddress(
        const std::string& address,
        const std::string& label,
        const proto::ContactItemType currency =
            proto::CITEMTYPE_BTC) const override;
    std::shared_ptr<const class Contact> Update(
        const proto::CredentialIndex& nym) const override;

    ~ContactManager() = default;

private:
    friend class implementation::Native;

    using ContactLock = std::pair<std::mutex, std::shared_ptr<class Contact>>;
    using Address = std::pair<proto::ContactItemType, std::string>;
    using ContactMap = std::map<Identifier, ContactLock>;
    using ContactNameMap = std::map<Identifier, std::string>;

    const api::storage::Storage& storage_;
    const api::client::Wallet& wallet_;
    mutable std::recursive_mutex lock_{};
    mutable ContactMap contact_map_{};
    mutable ContactNameMap contact_name_map_;
    OTZMQPublishSocket publisher_;

    static ContactNameMap build_name_map(const api::storage::Storage& storage);

    void check_identifiers(
        const Identifier& inputNymID,
        const PaymentCode& paymentCode,
        bool& haveNymID,
        bool& havePaymentCode,
        Identifier& outputNymID) const;
    bool verify_write_lock(const rLock& lock) const;

    // takes ownership
    ContactMap::iterator add_contact(const rLock& lock, class Contact* contact)
        const;
    OTIdentifier address_to_contact(
        const rLock& lock,
        const std::string& address,
        const proto::ContactItemType currency) const;
    std::shared_ptr<const class Contact> contact(
        const rLock& lock,
        const std::string& label) const;
    std::shared_ptr<const class Contact> contact(
        const rLock& lock,
        const Identifier& id) const;
    void import_contacts(const rLock& lock);
    void init_nym_map(const rLock& lock);
    ContactMap::iterator load_contact(const rLock& lock, const Identifier& id)
        const;
    std::unique_ptr<Editor<class Contact>> mutable_contact(
        const rLock& lock,
        const Identifier& id) const;
    ContactMap::iterator obtain_contact(const rLock& lock, const Identifier& id)
        const;
    std::shared_ptr<const class Contact> new_contact(
        const rLock& lock,
        const std::string& label,
        const Identifier& nymID,
        const PaymentCode& paymentCode) const;
    void refresh_indices(const rLock& lock, class Contact& contact) const;
    void save(class Contact* contact) const;
    void start();
    std::shared_ptr<const class Contact> update_existing_contact(
        const rLock& lock,
        const std::string& label,
        const PaymentCode& code,
        const Identifier& contactID) const;
    void update_nym_map(
        const rLock& lock,
        const Identifier nymID,
        class Contact& contact,
        const bool replace = false) const;

    ContactManager(
        const api::storage::Storage& storage,
        const api::client::Wallet& wallet,
        const opentxs::network::zeromq::Context& context);
    ContactManager() = delete;
    ContactManager(const ContactManager&) = delete;
    ContactManager(ContactManager&&) = delete;
    ContactManager& operator=(const ContactManager&) = delete;
    ContactManager& operator=(ContactManager&&) = delete;
};
}  // namespace opentxs::api::implementation
#endif  // OPENTXS_API_CONTACT_MANAGER_IMPLEMENTATION_HPP
