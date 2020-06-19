// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/client/Contacts.cpp"

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "internal/api/client/Client.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"

namespace opentxs
{
namespace api
{
namespace storage
{
class Storage;
}  // namespace storage
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class Contact;
class Nym;
}  // namespace proto

class Contact;
class Factory;
class PaymentCode;
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
class Contacts final : public client::internal::Contacts
{
public:
    auto BlockchainAddressToContact(
        const std::string& address,
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC) const
        -> OTIdentifier final;
    auto Contact(const Identifier& id) const
        -> std::shared_ptr<const opentxs::Contact> final;
    auto ContactID(const identifier::Nym& nymID) const -> OTIdentifier final;
    auto ContactList() const -> ObjectList final;
    auto ContactName(const Identifier& contactID) const -> std::string final;
    auto Merge(const Identifier& parent, const Identifier& child) const
        -> std::shared_ptr<const opentxs::Contact> final;
    auto mutable_Contact(const Identifier& id) const
        -> std::unique_ptr<Editor<opentxs::Contact>> final;
    auto NewContact(const std::string& label) const
        -> std::shared_ptr<const opentxs::Contact> final;
    auto NewContact(
        const std::string& label,
        const identifier::Nym& nymID,
        const PaymentCode& paymentCode) const
        -> std::shared_ptr<const opentxs::Contact> final;
    auto NewContactFromAddress(
        const std::string& address,
        const std::string& label,
        const proto::ContactItemType currency) const
        -> std::shared_ptr<const opentxs::Contact> final;
    auto NymToContact(const identifier::Nym& nymID) const -> OTIdentifier final;
    auto Update(const proto::Nym& nym) const
        -> std::shared_ptr<const opentxs::Contact> final;

    ~Contacts() final = default;

private:
    friend opentxs::Factory;

    using ContactLock =
        std::pair<std::mutex, std::shared_ptr<opentxs::Contact>>;
    using Address = std::pair<proto::ContactItemType, std::string>;
    using ContactMap = std::map<OTIdentifier, ContactLock>;
    using ContactNameMap = std::map<OTIdentifier, std::string>;

    const api::client::internal::Manager& api_;
    mutable std::recursive_mutex lock_{};
    mutable ContactMap contact_map_{};
    mutable ContactNameMap contact_name_map_;
    OTZMQPublishSocket publisher_;

    static auto build_name_map(const api::storage::Storage& storage)
        -> ContactNameMap;

    void check_identifiers(
        const Identifier& inputNymID,
        const PaymentCode& paymentCode,
        bool& haveNymID,
        bool& havePaymentCode,
        identifier::Nym& outputNymID) const;
    auto verify_write_lock(const rLock& lock) const -> bool;

    // takes ownership
    auto add_contact(const rLock& lock, opentxs::Contact* contact) const
        -> ContactMap::iterator;
    auto address_to_contact(
        const rLock& lock,
        const std::string& address,
        const proto::ContactItemType currency) const -> OTIdentifier;
    auto contact(const rLock& lock, const std::string& label) const
        -> std::shared_ptr<const opentxs::Contact>;
    auto contact(const rLock& lock, const Identifier& id) const
        -> std::shared_ptr<const opentxs::Contact>;
    void import_contacts(const rLock& lock);
    void init_nym_map(const rLock& lock);
    auto load_contact(const rLock& lock, const Identifier& id) const
        -> ContactMap::iterator;
    auto mutable_contact(const rLock& lock, const Identifier& id) const
        -> std::unique_ptr<Editor<opentxs::Contact>>;
    auto obtain_contact(const rLock& lock, const Identifier& id) const
        -> ContactMap::iterator;
    auto new_contact(
        const rLock& lock,
        const std::string& label,
        const identifier::Nym& nymID,
        const PaymentCode& paymentCode) const
        -> std::shared_ptr<const opentxs::Contact>;
    void refresh_indices(const rLock& lock, opentxs::Contact& contact) const;
    void save(opentxs::Contact* contact) const;
    void start() final;
    auto update_existing_contact(
        const rLock& lock,
        const std::string& label,
        const PaymentCode& code,
        const Identifier& contactID) const
        -> std::shared_ptr<const opentxs::Contact>;
    void update_nym_map(
        const rLock& lock,
        const identifier::Nym& nymID,
        opentxs::Contact& contact,
        const bool replace = false) const;

    Contacts(const api::client::internal::Manager& api);
    Contacts() = delete;
    Contacts(const Contacts&) = delete;
    Contacts(Contacts&&) = delete;
    auto operator=(const Contacts&) -> Contacts& = delete;
    auto operator=(Contacts &&) -> Contacts& = delete;
};
}  // namespace opentxs::api::client::implementation
