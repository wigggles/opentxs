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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/api/ContactManager.hpp"

#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/storage/Storage.hpp"

#include <functional>

#define OT_METHOD "opentxs::ContactManager::"

namespace opentxs
{
ContactManager::ContactManager(Storage& storage, Wallet& wallet)
    : storage_(storage)
    , wallet_(wallet)
    , lock_()
    , contact_map_()
    , nym_contact_map_()
    , address_contact_map_()
{
}

ContactManager::ContactMap::iterator ContactManager::add_contact(
    const rLock& lock,
    class Contact* contact)
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

    auto it = address_contact_map_.find({currency, address});

    if (address_contact_map_.end() == it) {

        return {};
    }

    return it->second;
}

Identifier ContactManager::BlockchainAddressToContact(
    const std::string& address,
    const proto::ContactItemType currency) const
{
    rLock lock(lock_);

    return address_to_contact(lock, address, currency);
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
    const Identifier& id)
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
    const std::string& label)
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
    const Identifier& id)
{
    rLock lock(lock_);

    return contact(lock, id);
}

Identifier ContactManager::ContactID(const Identifier& nymID) const
{
    rLock lock(lock_);

    auto it = nym_contact_map_.find(nymID);

    if (nym_contact_map_.end() == it) {

        return {};
    }

    return it->second;
}

ObjectList ContactManager::ContactList() const
{
    return storage_.ContactList();
}

void ContactManager::import_contacts(const rLock& lock)
{
    auto nyms = wallet_.NymList();

    for (const auto& it : nyms) {
        const Identifier nymID(it.first);

        if (0 == nym_contact_map_.count(nymID)) {
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

        const auto addresses = contact->BlockchainAddresses();

        for (const auto& address : addresses) {
            address_contact_map_[address] = contact->ID();
        }
    }
}

ContactManager::ContactMap::iterator ContactManager::load_contact(
    const rLock& lock,
    const Identifier& id)
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

std::unique_ptr<Editor<class Contact>> ContactManager::mutable_contact(
    const rLock& lock,
    const Identifier& id)
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
    const Identifier& id)
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
    const PaymentCode& code)
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    bool haveNymID{false};
    bool havePaymentCode{false};
    Identifier inputNymID{};
    check_identifiers(nymID, code, haveNymID, havePaymentCode, inputNymID);

    if (haveNymID) {
        auto it = nym_contact_map_.find(inputNymID);

        if (nym_contact_map_.end() != it) {

            return update_existing_contact(lock, label, code, it);
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
    const std::string& label)
{
    rLock lock(lock_);

    return contact(lock, label);
}

std::shared_ptr<const class Contact> ContactManager::NewContact(
    const std::string& label,
    const Identifier& nymID,
    const PaymentCode& paymentCode)
{
    rLock lock(lock_);

    return new_contact(lock, label, nymID, paymentCode);
}

std::shared_ptr<const class Contact> ContactManager::NewContactFromAddress(
    const std::string& address,
    const std::string& label,
    const proto::ContactItemType currency)
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

    address_contact_map_[{currency, address}] = newContactID;

    if (false == storage_.Store(contact)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to save contact."
              << std::endl;

        OT_FAIL;
    }

    return newContact;
}

ContactManager::ContactMap::iterator ContactManager::obtain_contact(
    const rLock& lock,
    const Identifier& id)
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
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto nyms = contact.Nyms();

    for (const auto& nymid : nyms) {
        update_nym_map(lock, nymid, contact, true);
    }

    const auto addresses = contact.BlockchainAddresses();

    for (const auto& address : addresses) {
        address_contact_map_[address] = contact.ID();
    }
}

void ContactManager::save(class Contact* contact)
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
    rLock lock(lock_);
    init_nym_map(lock);
    import_contacts(lock);
}

std::shared_ptr<const class Contact> ContactManager::Update(
    const proto::CredentialIndex& serialized)
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
    auto it = nym_contact_map_.find(nymID);

    if (nym_contact_map_.end() == it) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << String(nymID)
              << " is not associated with a contact. Creating a new contact."
              << std::endl;

        return new_contact(
            lock,
            Contact::ExtractLabel(*nym),
            nymID,
            PaymentCode(nym->PaymentCode()));
    }

    const auto& contactID = it->second;

    {
        auto contact = mutable_contact(lock, contactID);
        contact->It().Update(serialized);
        contact.reset();
    }

    auto contact = obtain_contact(lock, contactID);

    OT_ASSERT(contact_map_.end() != contact);

    auto& output = contact->second.second;

    OT_ASSERT(output);

    return output;
}

std::shared_ptr<const class Contact> ContactManager::update_existing_contact(
    const rLock& lock,
    const std::string& label,
    const PaymentCode& code,
    ContactManager::NymMap::iterator& existing)
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto& contactID = existing->second;

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
    const bool replace)
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const bool exists = (0 < nym_contact_map_.count(nymID));
    const auto& incomingID = contact.ID();
    auto& contactID = nym_contact_map_[nymID];
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

    if (false == same) {
        contactID = incomingID;
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
}  // namespace opentxs
