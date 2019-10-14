// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class Contacts final : public client::internal::Contacts
{
public:
#if OT_CRYPTO_SUPPORTED_KEY_HD
    OTIdentifier BlockchainAddressToContact(
        const std::string& address,
        const proto::ContactItemType currency =
            proto::CITEMTYPE_BTC) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    std::shared_ptr<const class Contact> Contact(
        const Identifier& id,
        const PasswordPrompt& reason) const final;
    OTIdentifier ContactID(const identifier::Nym& nymID) const final;
    ObjectList ContactList() const final;
    std::string ContactName(const Identifier& contactID) const final;
    std::shared_ptr<const class Contact> Merge(
        const Identifier& parent,
        const Identifier& child,
        const PasswordPrompt& reason) const final;
    std::unique_ptr<Editor<class Contact>> mutable_Contact(
        const Identifier& id,
        const PasswordPrompt& reason) const final;
    std::shared_ptr<const class Contact> NewContact(
        const std::string& label) const final;
    std::shared_ptr<const class Contact> NewContact(
        const std::string& label,
        const identifier::Nym& nymID
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        ,
        const PaymentCode& paymentCode
#endif
        ,
        const PasswordPrompt& reason) const final;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    std::shared_ptr<const class Contact> NewContactFromAddress(
        const std::string& address,
        const std::string& label,
        const PasswordPrompt& reason,
        const proto::ContactItemType currency) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    OTIdentifier NymToContact(
        const identifier::Nym& nymID,
        const PasswordPrompt& reason) const final;
    std::shared_ptr<const class Contact> Update(
        const proto::Nym& nym,
        const PasswordPrompt& reason) const final;

    ~Contacts() final = default;

private:
    friend opentxs::Factory;

    using ContactLock = std::pair<std::mutex, std::shared_ptr<class Contact>>;
    using Address = std::pair<proto::ContactItemType, std::string>;
    using ContactMap = std::map<OTIdentifier, ContactLock>;
    using ContactNameMap = std::map<OTIdentifier, std::string>;

    const api::client::Manager& api_;
    mutable std::recursive_mutex lock_{};
    mutable ContactMap contact_map_{};
    mutable ContactNameMap contact_name_map_;
    OTZMQPublishSocket publisher_;

    static ContactNameMap build_name_map(const api::storage::Storage& storage);

    void check_identifiers(
        const Identifier& inputNymID,
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        const PaymentCode& paymentCode,
#endif
        bool& haveNymID,
        bool& havePaymentCode,
        identifier::Nym& outputNymID) const;
    bool verify_write_lock(const rLock& lock) const;

    // takes ownership
    ContactMap::iterator add_contact(const rLock& lock, class Contact* contact)
        const;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    OTIdentifier address_to_contact(
        const rLock& lock,
        const std::string& address,
        const proto::ContactItemType currency) const;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    std::shared_ptr<const class Contact> contact(
        const rLock& lock,
        const std::string& label) const;
    std::shared_ptr<const class Contact> contact(
        const PasswordPrompt& reason,
        const rLock& lock,
        const Identifier& id) const;
    void import_contacts(const rLock& lock, const PasswordPrompt& reason);
    void init_nym_map(const PasswordPrompt& reason, const rLock& lock);
    ContactMap::iterator load_contact(
        const PasswordPrompt& reason,
        const rLock& lock,
        const Identifier& id) const;
    std::unique_ptr<Editor<class Contact>> mutable_contact(
        const PasswordPrompt& reason,
        const rLock& lock,
        const Identifier& id) const;
    ContactMap::iterator obtain_contact(
        const PasswordPrompt& reason,
        const rLock& lock,
        const Identifier& id) const;
    std::shared_ptr<const class Contact> new_contact(
        const rLock& lock,
        const PasswordPrompt& reason,
        const std::string& label,
        const identifier::Nym& nymID
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        ,
        const PaymentCode& paymentCode
#endif
        ) const;
    void refresh_indices(
        const PasswordPrompt& reason,
        const rLock& lock,
        class Contact& contact) const;
    void save(const PasswordPrompt& reason, class Contact* contact) const;
    void start(const PasswordPrompt& reason) final;
    std::shared_ptr<const class Contact> update_existing_contact(
        const PasswordPrompt& reason,
        const rLock& lock,
        const std::string& label,
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        const PaymentCode& code,
#endif
        const Identifier& contactID) const;
    void update_nym_map(
        const PasswordPrompt& reason,
        const rLock& lock,
        const identifier::Nym& nymID,
        class Contact& contact,
        const bool replace = false) const;

    Contacts(const api::client::Manager& api);
    Contacts() = delete;
    Contacts(const Contacts&) = delete;
    Contacts(Contacts&&) = delete;
    Contacts& operator=(const Contacts&) = delete;
    Contacts& operator=(Contacts&&) = delete;
};
}  // namespace opentxs::api::client::implementation
