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

#include "opentxs/stdafx.hpp"

#include "ContactManager.hpp"

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/Proto.hpp"

#include <functional>

#define OT_METHOD "opentxs::api::implementation::ContactManager"

namespace opentxs::api::implementation
{
ContactManager::ContactManager(
    const api::storage::Storage& storage,
    const api::client::Wallet& wallet,
    const opentxs::network::zeromq::Context& context)
    : storage_(storage)
    , wallet_(wallet)
    , lock_()
    , contact_map_()
    , contact_name_map_(build_name_map(storage))
    , publisher_(context.PublishSocket())
{
    publisher_->Start(opentxs::network::zeromq::Socket::ContactUpdateEndpoint);
}

ContactManager::ContactMap::iterator ContactManager::add_contact(
    const rLock& lock,
    class Contact* contact) const
{
    OT_ASSERT(nullptr != contact);

    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto& id = contact->ID();
    auto& it = contact_map_[id];
    it.second.reset(contact);

    return contact_map_.find(id);
}

Identifier ContactManager::address_to_contact(
    const rLock& lock,
    const std::string& address,
    const proto::ContactItemType currency) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto contact = storage_.BlockchainAddressOwner(currency, address);

    return Identifier(contact);
}

Identifier ContactManager::BlockchainAddressToContact(
    const std::string& address,
    const proto::ContactItemType currency) const
{
    rLock lock(lock_);

    return address_to_contact(lock, address, currency);
}

ContactManager::ContactNameMap ContactManager::build_name_map(
    const api::storage::Storage& storage)
{
    ContactNameMap output{};

    for (const auto & [ id, alias ] : storage.ContactList()) {
        output.emplace(id, alias);
    }

    return output;
}

void ContactManager::check_identifiers(
    const Identifier& inputNymID,
    const PaymentCode& paymentCode,
    bool& haveNymID,
    bool& havePaymentCode,
    Identifier& outputNymID) const
{
    if (paymentCode.VerifyInternally()) {
        havePaymentCode = true;
    }

    if (false == inputNymID.empty()) {
        haveNymID = true;
        outputNymID = inputNymID;
    } else if (havePaymentCode) {
        haveNymID = true;
        outputNymID = paymentCode.ID();
    }
}

std::shared_ptr<const class Contact> ContactManager::contact(
    const rLock& lock,
    const Identifier& id) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto it = obtain_contact(lock, id);

    if (contact_map_.end() != it) {

        return it->second.second;
    }

    return {};
}

std::shared_ptr<const class Contact> ContactManager::contact(
    const rLock& lock,
    const std::string& label) const
{
    std::unique_ptr<class Contact> contact(new class Contact(wallet_, label));

    if (false == bool(contact)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to create new contact."
              << std::endl;

        return {};
    }

    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto& contactID = contact->ID();

    OT_ASSERT(0 == contact_map_.count(contactID));

    auto it = add_contact(lock, contact.release());
    auto& output = it->second.second;

    if (false == storage_.Store(*output)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to create save contact."
              << std::endl;
        contact_map_.erase(it);

        return {};
    }

    return output;
}

std::shared_ptr<const class Contact> ContactManager::Contact(
    const Identifier& id) const
{
    rLock lock(lock_);

    return contact(lock, id);
}

Identifier ContactManager::ContactID(const Identifier& nymID) const
{
    return Identifier(storage_.ContactOwnerNym(String(nymID).Get()));
}

ObjectList ContactManager::ContactList() const
{
    return storage_.ContactList();
}

std::string ContactManager::ContactName(const Identifier& contactID) const
{
    rLock lock(lock_);
    auto it = contact_name_map_.find(contactID);

    if (contact_name_map_.end() == it) {

        return {};
    }

    return it->second;
}

void ContactManager::import_contacts(const rLock& lock)
{
    auto nyms = wallet_.NymList();

    for (const auto& it : nyms) {
        const Identifier nymID(it.first);
        const auto contactID = storage_.ContactOwnerNym(String(nymID).Get());

        if (contactID.empty()) {
            const auto nym = wallet_.Nym(nymID);

            if (false == bool(nym)) {
                throw std::runtime_error("Unable to load nym");
            }

            switch (nym->Claims().Type()) {
                case proto::CITEMTYPE_INDIVIDUAL:
                case proto::CITEMTYPE_ORGANIZATION:
                case proto::CITEMTYPE_BUSINESS:
                case proto::CITEMTYPE_GOVERNMENT: {
                    PaymentCode code(nym->PaymentCode());
                    new_contact(lock, nym->Alias(), nymID, code);
                } break;
                case proto::CITEMTYPE_ERROR:
                case proto::CITEMTYPE_SERVER:
                default: {
                }
            }
        }
    }
}

void ContactManager::init_nym_map(const rLock& lock)
{
    otErr << OT_METHOD << __FUNCTION__ << ": Upgrading indices" << std::endl;

    for (const auto& it : storage_.ContactList()) {
        const auto& contactID = Identifier(it.first);
        auto loaded = load_contact(lock, contactID);

        if (contact_map_.end() == loaded) {

            throw std::runtime_error("failed to load contact");
        }

        auto& contact = loaded->second.second;

        if (false == bool(contact)) {

            throw std::runtime_error("null contact pointer");
        }

        const auto type = contact->Type();

        if (proto::CITEMTYPE_ERROR == type) {
            otErr << OT_METHOD << __FUNCTION__ << ": Invalid contact "
                  << it.first << std::endl;
            storage_.DeleteContact(it.first);
        }

        const auto nyms = contact->Nyms();

        for (const auto& nym : nyms) {
            update_nym_map(lock, nym, *contact);
        }
    }

    storage_.ContactSaveIndices();
}

ContactManager::ContactMap::iterator ContactManager::load_contact(
    const rLock& lock,
    const Identifier& id) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    std::shared_ptr<proto::Contact> serialized{nullptr};
    const auto loaded = storage_.Load(String(id).Get(), serialized, SILENT);

    if (false == loaded) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load contact "
              << String(id) << std::endl;

        return contact_map_.end();
    }

    OT_ASSERT(serialized);

    std::unique_ptr<class Contact> contact(
        new class Contact(wallet_, *serialized));

    if (false == bool(contact)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to instantate serialized contact." << std::endl;

        return contact_map_.end();
    }

    return add_contact(lock, contact.release());
}

std::shared_ptr<const class Contact> ContactManager::Merge(
    const Identifier& parent,
    const Identifier& child) const
{
    rLock lock(lock_);
    auto childContact = contact(lock, child);

    if (false == bool(childContact)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Child contact "
              << String(child).Get() << " can not be loaded." << std::endl;

        return {};
    }

    const auto& childID = childContact->ID();

    if (childID != child) {
        otErr << OT_METHOD << __FUNCTION__ << ": Child contact "
              << String(child).Get() << " is already merged into "
              << String(childID).Get() << std::endl;

        return {};
    }

    auto parentContact = contact(lock, parent);

    if (false == bool(parentContact)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Parent contact "
              << String(parent).Get() << " can not be loaded." << std::endl;

        return {};
    }

    const auto& parentID = parentContact->ID();

    if (parentID != parent) {
        otErr << OT_METHOD << __FUNCTION__ << ": Parent contact "
              << String(parent).Get() << " is merged into "
              << String(parentID).Get() << std::endl;

        return {};
    }

    OT_ASSERT(childContact);
    OT_ASSERT(parentContact);

    auto& lhs = const_cast<class Contact&>(*parentContact);
    auto& rhs = const_cast<class Contact&>(*childContact);
    lhs += rhs;

    if (false == storage_.Store(rhs)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to create save child contact." << std::endl;

        OT_FAIL;
    }

    if (false == storage_.Store(lhs)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to create save parent contact." << std::endl;

        OT_FAIL;
    }

    contact_map_.erase(child);

    return parentContact;
}

std::unique_ptr<Editor<class Contact>> ContactManager::mutable_contact(
    const rLock& lock,
    const Identifier& id) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    std::unique_ptr<Editor<class Contact>> output{nullptr};

    auto it = contact_map_.find(id);

    if (contact_map_.end() == it) {
        it = load_contact(lock, id);
    }

    if (contact_map_.end() == it) {

        return {};
    }

    std::function<void(class Contact*)> callback =
        [&](class Contact* in) -> void { this->save(in); };
    output.reset(new Editor<class Contact>(it->second.second.get(), callback));

    return output;
}

std::unique_ptr<Editor<class Contact>> ContactManager::mutable_Contact(
    const Identifier& id) const
{
    rLock lock(lock_);
    auto output = mutable_contact(lock, id);
    lock.unlock();

    return output;
}

std::shared_ptr<const class Contact> ContactManager::new_contact(
    const rLock& lock,
    const std::string& label,
    const Identifier& nymID,
    const PaymentCode& code) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    bool haveNymID{false};
    bool havePaymentCode{false};
    Identifier inputNymID{};
    check_identifiers(nymID, code, haveNymID, havePaymentCode, inputNymID);

    if (haveNymID) {
        const auto contactID = storage_.ContactOwnerNym(String(nymID).Get());

        if (false == contactID.empty()) {

            return update_existing_contact(
                lock, label, code, Identifier(contactID));
        }
    }

    auto newContact = contact(lock, label);

    if (false == bool(newContact)) {

        return {};
    }

    Identifier contactID = newContact->ID();
    newContact.reset();
    auto output = mutable_contact(lock, contactID);

    if (false == bool(output)) {

        return {};
    }

    auto& mContact = output->It();

    if (false == nymID.empty()) {
        auto nym = wallet_.Nym(nymID);

        if (nym) {
            mContact.AddNym(nym, true);
        } else {
            mContact.AddNym(nymID, true);
        }

        update_nym_map(lock, nymID, mContact, true);
    }

    if (code.VerifyInternally()) {
        mContact.AddPaymentCode(code, true);
    }

    output.reset();

    return contact(lock, contactID);
}

std::shared_ptr<const class Contact> ContactManager::NewContact(
    const std::string& label) const
{
    rLock lock(lock_);

    return contact(lock, label);
}

std::shared_ptr<const class Contact> ContactManager::NewContact(
    const std::string& label,
    const Identifier& nymID,
    const PaymentCode& paymentCode) const
{
    rLock lock(lock_);

    return new_contact(lock, label, nymID, paymentCode);
}

std::shared_ptr<const class Contact> ContactManager::NewContactFromAddress(
    const std::string& address,
    const std::string& label,
    const proto::ContactItemType currency) const
{
    rLock lock(lock_);

    const auto existingID = address_to_contact(lock, address, currency);

    if (false == existingID.empty()) {

        return contact(lock, existingID);
    }

    auto newContact = contact(lock, label);

    OT_ASSERT(newContact);

    const auto newContactID = newContact->ID();
    auto& it = contact_map_.at(newContactID);
    auto& contact = *it.second;

    if (false == contact.AddBlockchainAddress(address, currency)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to add address to contact." << std::endl;

        OT_FAIL;
    }

    if (false == storage_.Store(contact)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to save contact."
              << std::endl;

        OT_FAIL;
    }

    return newContact;
}

ContactManager::ContactMap::iterator ContactManager::obtain_contact(
    const rLock& lock,
    const Identifier& id) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    auto it = contact_map_.find(id);

    if (contact_map_.end() != it) {

        return it;
    }

    return load_contact(lock, id);
}

void ContactManager::refresh_indices(const rLock& lock, class Contact& contact)
    const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto nyms = contact.Nyms();

    for (const auto& nymid : nyms) {
        update_nym_map(lock, nymid, contact, true);
    }

    const auto& id = contact.ID();
    contact_name_map_[id] = contact.Label();
    const std::string rawID{String(id).Get()};
    publisher_->Publish(rawID);
}

void ContactManager::save(class Contact* contact) const
{
    OT_ASSERT(nullptr != contact);

    if (false == storage_.Store(*contact)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to create or save contact." << std::endl;

        OT_FAIL;
    }

    const auto& id = contact->ID();

    if (false == storage_.SetContactAlias(String(id).Get(), contact->Label())) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to create or save contact." << std::endl;

        OT_FAIL;
    }

    rLock lock(lock_);
    refresh_indices(lock, *contact);
}

void ContactManager::start()
{
    const auto level = storage_.ContactUpgradeLevel();

    switch (level) {
        case 0:
        case 1: {
            rLock lock(lock_);
            init_nym_map(lock);
            import_contacts(lock);
        }
        case 2:
        default: {
        }
    }
}

std::shared_ptr<const class Contact> ContactManager::Update(
    const proto::CredentialIndex& serialized) const
{
    auto nym = wallet_.Nym(serialized);

    if (false == bool(nym)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid nym." << std::endl;

        return {};
    }

    auto& data = nym->Claims();

    switch (data.Type()) {
        case proto::CITEMTYPE_INDIVIDUAL:
        case proto::CITEMTYPE_ORGANIZATION:
        case proto::CITEMTYPE_BUSINESS:
        case proto::CITEMTYPE_GOVERNMENT: {
            break;
        }
        default: {
            return {};
        }
    }

    const auto& nymID = nym->ID();
    rLock lock(lock_);
    const auto contactIdentifier =
        storage_.ContactOwnerNym(String(nymID).Get());
    const auto contactID = Identifier(contactIdentifier);

    if (contactIdentifier.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << String(nymID)
              << " is not associated with a contact. Creating a new contact."
              << std::endl;

        return new_contact(
            lock,
            Contact::ExtractLabel(*nym),
            nymID,
            PaymentCode(nym->PaymentCode()));
    }

    {
        auto contact = mutable_contact(lock, contactID);
        contact->It().Update(serialized);
        contact.reset();
    }

    auto contact = obtain_contact(lock, contactID);

    OT_ASSERT(contact_map_.end() != contact);

    auto& output = contact->second.second;

    OT_ASSERT(output);

    storage_.RelabelThread(String(output->ID()).Get(), output->Label());

    return output;
}

std::shared_ptr<const class Contact> ContactManager::update_existing_contact(
    const rLock& lock,
    const std::string& label,
    const PaymentCode& code,
    const Identifier& contactID) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    auto it = obtain_contact(lock, contactID);

    OT_ASSERT(contact_map_.end() != it);

    auto& contactMutex = it->second.first;
    auto& contact = it->second.second;

    OT_ASSERT(contact);

    Lock contactLock(contactMutex);
    const auto& existingLabel = contact->Label();

    if ((existingLabel != label) && (false == label.empty())) {
        contact->SetLabel(label);
    }

    contact->AddPaymentCode(code, true);
    save(contact.get());

    return contact;
}

void ContactManager::update_nym_map(
    const rLock& lock,
    const Identifier nymID,
    class Contact& contact,
    const bool replace) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto contactIdentifier =
        storage_.ContactOwnerNym(String(nymID).Get());
    const bool exists = (false == contactIdentifier.empty());
    const auto& incomingID = contact.ID();
    const auto contactID = Identifier(contactIdentifier);
    const bool same = (incomingID == contactID);

    if (exists && (false == same)) {
        if (replace) {
            auto it = load_contact(lock, contactID);

            if (contact_map_.end() != it) {

                throw std::runtime_error("contact not found");
            }

            auto& oldContact = it->second.second;

            if (false == bool(oldContact)) {
                throw std::runtime_error("null contact pointer");
            }

            oldContact->RemoveNym(nymID);

            if (false == storage_.Store(*oldContact)) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Unable to create or save contact." << std::endl;

                OT_FAIL;
            }
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Duplicate nym found."
                  << std::endl;
            contact.RemoveNym(nymID);

            if (false == storage_.Store(contact)) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Unable to create or save contact." << std::endl;

                OT_FAIL;
            }

            return;
        }
    }
}

bool ContactManager::verify_write_lock(const rLock& lock) const
{
    if (lock.mutex() != &lock_) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect mutex." << std::endl;

        return false;
    }

    if (false == lock.owns_lock()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock not owned." << std::endl;

        return false;
    }

    return true;
}
}  // namespace opentxs::api::implementation
