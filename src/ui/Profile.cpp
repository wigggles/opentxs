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

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/Profile.hpp"
#include "opentxs/ui/ProfileSection.hpp"

#include "List.hpp"
#include "ProfileParent.hpp"
#include "ProfileSectionBlank.hpp"

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <tuple>
#include <vector>

#include "Profile.hpp"

template struct std::pair<int, std::string>;

#define OT_METHOD "opentxs::ui::implementation::Profile::"

namespace opentxs
{
ui::Profile* Factory::ProfileWidget(
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const api::client::Wallet& wallet,
    const Identifier& nymID)
{
    return new ui::implementation::Profile(zmq, contact, wallet, nymID);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
const std::set<proto::ContactSectionName> Profile::allowed_types_{
    proto::CONTACTSECTION_COMMUNICATION,
    proto::CONTACTSECTION_PROFILE};

const std::map<proto::ContactSectionName, int> Profile::sort_keys_{
    {proto::CONTACTSECTION_COMMUNICATION, 0},
    {proto::CONTACTSECTION_PROFILE, 1}};

Profile::Profile(
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const api::client::Wallet& wallet,
    const Identifier& nymID)
    : ProfileType(
          zmq,
          contact,
          proto::CONTACTSECTION_ERROR,
          nymID,
          new ProfileSectionBlank)
    , wallet_(wallet)
    , name_(nym_name(wallet, nymID))
    , payment_code_()
    , nym_subscriber_callback_(network::zeromq::ListenCallback::Factory(
          [this](const network::zeromq::Message& message) -> void {
              this->process_nym(message);
          }))
    , nym_subscriber_(zmq_.SubscribeSocket(nym_subscriber_callback_.get()))
{
    OT_ASSERT(blank_p_)

    init();
    const auto& endpoint = network::zeromq::Socket::NymDownloadEndpoint;
    otWarn << OT_METHOD << __FUNCTION__ << ": Connecting to " << endpoint
           << std::endl;
    const auto listening = nym_subscriber_->Start(endpoint);

    OT_ASSERT(listening)

    startup_.reset(new std::thread(&Profile::startup, this));

    OT_ASSERT(startup_)
}

bool Profile::AddClaim(
    const proto::ContactSectionName section,
    const proto::ContactItemType type,
    const std::string& value,
    const bool primary,
    const bool active) const
{
    auto nym = wallet_.mutable_Nym(nym_id_);

    switch (section) {
        case proto::CONTACTSECTION_SCOPE: {

            return nym.SetScope(type, value, primary);
        } break;
        case proto::CONTACTSECTION_COMMUNICATION: {
            switch (type) {
                case proto::CITEMTYPE_EMAIL: {

                    return nym.AddEmail(value, primary, active);
                } break;
                case proto::CITEMTYPE_PHONE: {

                    return nym.AddPhoneNumber(value, primary, active);
                } break;
                case proto::CITEMTYPE_OPENTXS: {

                    return nym.AddPreferredOTServer(value, primary);
                } break;
                default: {
                }
            }
        } break;
        case proto::CONTACTSECTION_PROFILE: {

            return nym.AddSocialMediaProfile(value, type, primary, active);
        } break;
        case proto::CONTACTSECTION_CONTRACT: {

            return nym.AddContract(value, type, primary, active);
        } break;
        case proto::CONTACTSECTION_PROCEDURE: {
            return nym.AddPaymentCode(value, type, primary, active);
        } break;
        default: {
        }
    }

    Claim claim{};
    auto& [id, claimSection, claimType, claimValue, start, end, attributes] =
        claim;
    id = "";
    claimSection = section;
    claimType = type;
    claimValue = value;
    start = 0;
    end = 0;

    if (primary) { attributes.emplace(proto::CITEMATTR_PRIMARY); }

    if (primary || active) { attributes.emplace(proto::CITEMATTR_ACTIVE); }

    return nym.AddClaim(claim);
}

Profile::ItemTypeList Profile::AllowedItems(
    const proto::ContactSectionName section,
    const std::string& lang) const
{
    return ui::ProfileSection::AllowedItems(section, lang);
}

Profile::SectionTypeList Profile::AllowedSections(const std::string& lang) const
{
    SectionTypeList output{};

    for (const auto& type : allowed_types_) {
        output.emplace_back(type, proto::TranslateSectionName(type, lang));
    }

    return output;
}

bool Profile::check_type(const proto::ContactSectionName type)
{
    return 1 == allowed_types_.count(type);
}

void Profile::construct_item(
    const ProfileIDType& id,
    const ContactSortKey& index,
    const CustomData& custom) const
{
    OT_ASSERT(1 == custom.size())

    names_.emplace(id, index);
    items_[index].emplace(
        id,
        Factory::ProfileSectionWidget(
            zmq_, contact_manager_, wallet_, *this, recover(custom[0])));
}

bool Profile::Delete(
    const int sectionType,
    const int type,
    const std::string& claimID) const
{
    Lock lock(lock_);
    auto& section = find_by_id(lock, static_cast<ProfileIDType>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.Delete(type, claimID);
}

const Identifier& Profile::NymID() const { return nym_id_; }

std::string Profile::DisplayName() const
{
    Lock lock(lock_);

    return name_;
}

std::string Profile::nym_name(
    const api::client::Wallet& wallet,
    const Identifier& nymID)
{
    for (const auto& [id, name] : wallet.NymList()) {
        if (nymID.str() == id) { return name; }
    }

    return {};
}

std::string Profile::PaymentCode() const
{
    Lock lock(lock_);

    return payment_code_;
}

void Profile::process_nym(const Nym& nym)
{
    Lock lock(lock_);
    name_ = nym.Alias();
    payment_code_ = nym.PaymentCode();
    lock.unlock();
    UpdateNotify();
    std::set<ProfileIDType> active{};

    for (const auto& section : nym.Claims()) {
        auto& type = section.first;

        if (check_type(type)) {
            add_item(type, sort_key(type), {section.second.get()});
            active.emplace(type);
        }
    }

    delete_inactive(active);
    UpdateNotify();
}

void Profile::process_nym(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const Identifier nymID(id);

    OT_ASSERT(false == nymID.empty())

    if (nymID != nym_id_) { return; }

    const auto nym = wallet_.Nym(nymID);

    OT_ASSERT(nym)

    process_nym(*nym);
}

const opentxs::ContactSection& Profile::recover(const void* input)
{
    OT_ASSERT(nullptr != input)

    return *static_cast<const opentxs::ContactSection*>(input);
}

bool Profile::SetActive(
    const int sectionType,
    const int type,
    const std::string& claimID,
    const bool active) const
{
    Lock lock(lock_);
    auto& section = find_by_id(lock, static_cast<ProfileIDType>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.SetActive(type, claimID, active);
}

bool Profile::SetPrimary(
    const int sectionType,
    const int type,
    const std::string& claimID,
    const bool primary) const
{
    Lock lock(lock_);
    auto& section = find_by_id(lock, static_cast<ProfileIDType>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.SetPrimary(type, claimID, primary);
}

bool Profile::SetValue(
    const int sectionType,
    const int type,
    const std::string& claimID,
    const std::string& value) const
{
    Lock lock(lock_);
    auto& section = find_by_id(lock, static_cast<ProfileIDType>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.SetValue(type, claimID, value);
}

int Profile::sort_key(const proto::ContactSectionName type)
{
    return sort_keys_.at(type);
}

void Profile::startup()
{
    otErr << OT_METHOD << __FUNCTION__ << ": Loading nym " << nym_id_->str()
          << std::endl;
    const auto nym = wallet_.Nym(nym_id_);

    OT_ASSERT(nym)

    process_nym(*nym);
    startup_complete_->On();
}

void Profile::update(ProfilePimpl& row, const CustomData& custom) const
{
    OT_ASSERT(row)
    OT_ASSERT(1 == custom.size())

    row->Update(recover(custom[0]));
}
}  // namespace opentxs::ui::implementation
