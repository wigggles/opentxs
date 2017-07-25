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

#include "opentxs/api/Activity.hpp"

#include "opentxs/api/ContactManager.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/storage/Storage.hpp"

#define OT_METHOD "opentxs::Activity::"

namespace opentxs
{
Activity::Activity(ContactManager& contact, Storage& storage, Wallet& wallet)
    : contact_(contact)
    , storage_(storage)
    , wallet_(wallet)
{
}

std::shared_ptr<const Contact> Activity::nym_to_contact(const std::string& id)
{
    const Identifier nymID(id);
    auto contactID = contact_.ContactID(nymID);

    if (false == contactID.empty()) {

        return contact_.Contact(contactID);
    }

    // Contact does not yet exist. Create it.
    std::string label{};
    auto nym = wallet_.Nym(nymID);
    std::unique_ptr<PaymentCode> code;

    if (nym) {
        label = nym->Claims().Name();
        code.reset(new PaymentCode(nym->PaymentCode()));
    }

    if (false == bool(code)) {
        code.reset(new PaymentCode(""));
    }

    OT_ASSERT(code);

    return contact_.NewContact(label, nymID, *code);
}

std::unique_ptr<Message> Activity::Mail(
    const Identifier& nym,
    const Identifier& id,
    const StorageBox& box) const
{
    std::string raw, alias;
    const bool loaded = storage_.Load(
        String(nym).Get(), String(id).Get(), box, raw, alias, true);

    std::unique_ptr<Message> output;

    if (!loaded) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load Message"
              << std::endl;

        return output;
    }

    if (raw.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Empty message" << std::endl;

        return output;
    }

    output.reset(new Message);

    OT_ASSERT(output);

    if (false == output->LoadContractFromString(String(raw.c_str()))) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to deserialized Message"
              << std::endl;

        output.reset();
    }

    return output;
}

std::string Activity::Mail(
    const Identifier& nym,
    const Message& mail,
    const StorageBox box)
{
    const std::string nymID = String(nym).Get();
    Identifier id{};
    mail.CalculateContractID(id);
    const std::string output = String(id).Get();
    const String data(mail);
    std::string participantNymID{};
    const String localName(nym);

    if (localName == mail.m_strNymID2) {
        // This is an incoming message. The contact id is the sender's id.
        participantNymID = mail.m_strNymID.Get();
    } else {
        // This is an outgoing message. The contact id is the recipient's id.
        participantNymID = mail.m_strNymID2.Get();
    }

    const auto contact = nym_to_contact(participantNymID);

    OT_ASSERT(contact);

    std::string alias = contact->Label();
    const std::string contactID = String(contact->ID()).Get();
    const auto& threadID = contactID;
    const auto threadList = storage_.ThreadList(nymID);
    bool threadExists = false;

    for (const auto it : threadList) {
        const auto& id = it.first;

        if (id == threadID) {
            threadExists = true;
            break;
        }
    }

    if (false == threadExists) {
        storage_.CreateThread(nymID, threadID, {contactID});
    }

    const bool saved = storage_.Store(
        localName.Get(),
        threadID,
        output,
        mail.m_lTime,
        alias,
        data.Get(),
        box);

    if (saved) {

        return output;
    }

    return "";
}

ObjectList Activity::Mail(const Identifier& nym, const StorageBox box) const
{
    return storage_.NymBoxList(String(nym).Get(), box);
}

bool Activity::MailRemove(
    const Identifier& nym,
    const Identifier& id,
    const StorageBox box) const
{
    const std::string nymid = String(nym).Get();
    const std::string mail = String(id).Get();

    return storage_.RemoveNymBoxItem(nymid, box, mail);
}

void Activity::MigrateLegacyThreads() const
{
    std::set<std::string> contacts{};

    for (const auto& it : contact_.ContactList()) {
        contacts.insert(it.first);
    }

    const auto nymlist = storage_.NymList();

    for (const auto& it1 : nymlist) {
        const auto& nymID = it1.first;
        const auto threadList = storage_.ThreadList(nymID);

        for (const auto& it2 : threadList) {
            const auto& originalThreadID = it2.first;
            const bool isContactID = (1 == contacts.count(originalThreadID));

            if (isContactID) {

                continue;
            }

            auto contactID = contact_.ContactID(Identifier(originalThreadID));

            if (false == contactID.empty()) {
                storage_.RenameThread(
                    nymID, originalThreadID, String(contactID).Get());
            } else {
                std::shared_ptr<proto::StorageThread> thread;
                storage_.Load(nymID, originalThreadID, thread);

                OT_ASSERT(thread);

                const auto nymCount = thread->participant().size();

                if (1 == nymCount) {
                    auto newContact = contact_.NewContact(
                        "", Identifier(originalThreadID), PaymentCode(""));

                    OT_ASSERT(newContact);

                    storage_.RenameThread(
                        nymID,
                        originalThreadID,
                        String(newContact->ID()).Get());
                } else {
                    // Multi-party chats were not implemented prior to the
                    // update to contact IDs, so there is no need to handle
                    // this case
                }
            }
        }
    }
}

std::shared_ptr<proto::StorageThread> Activity::Thread(
    const Identifier& nymID,
    const Identifier& threadID) const
{
    std::shared_ptr<proto::StorageThread> output;
    storage_.Load(String(nymID).Get(), String(threadID).Get(), output);

    return output;
}

ObjectList Activity::Threads(const Identifier& nym) const
{
    const std::string nymID = String(nym).Get();
    auto output = storage_.ThreadList(nymID);

    for (auto& it : output) {
        const auto& threadID = it.first;
        auto& label = it.second;
        auto contact = contact_.Contact(Identifier(threadID));

        if (contact) {
            const auto& name = contact->Label();

            if (label != name) {
                storage_.SetThreadAlias(nymID, threadID, name);
                label = name;
            }
        }
    }

    return output;
}
}  // namespace opentxs
