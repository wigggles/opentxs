// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/contact/Contact.hpp"

#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/contact/ContactSection.hpp"
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/core/crypto/PaymentCode.hpp"
#endif
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/Types.hpp"

#include <sstream>
#include <stdexcept>

#define ID_BYTES 32

#define OT_METHOD "opentxs::Contact::"

#if OT_CRYPTO_SUPPORTED_KEY_HD
using AddressStyle = opentxs::Contact::AddressStyle;

const std::map<AddressStyle, std::string> address_style_map_{
    {AddressStyle::P2PKH,
     std::to_string(static_cast<int>(AddressStyle::P2PKH))},
    {AddressStyle::P2SH, std::to_string(static_cast<int>(AddressStyle::P2SH))},
    {AddressStyle::P2WPKH,
     std::to_string(static_cast<int>(AddressStyle::P2WPKH))},
};
const std::map<std::string, AddressStyle> address_style_reverse_map_{
    opentxs::reverse_map(address_style_map_)};

AddressStyle translate_style(const std::string& in) noexcept;
AddressStyle translate_style(const std::string& in) noexcept
{
    try {

        return address_style_reverse_map_.at(in);
    } catch (...) {

        return AddressStyle::Unknown;
    }
}

std::string translate_style(const AddressStyle& in) noexcept;
std::string translate_style(const AddressStyle& in) noexcept
{
    try {

        return address_style_map_.at(in);
    } catch (...) {

        return std::to_string(static_cast<int>(AddressStyle::Unknown));
    }
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

namespace opentxs
{
Contact::Contact(
    const PasswordPrompt& reason,
    const api::client::Manager& api,
    const proto::Contact& serialized)
    : api_(api)
    , version_(check_version(serialized.version(), OT_CONTACT_VERSION))
    , label_(serialized.label())
    , lock_()
    , id_(api_.Factory().Identifier(serialized.id()))
    , parent_(api_.Factory().Identifier(serialized.mergedto()))
    , primary_nym_(api_.Factory().NymID())
    , nyms_()
    , merged_children_()
    , contact_data_(new ContactData(
          api_,
          serialized.id(),
          CONTACT_CONTACT_DATA_VERSION,
          CONTACT_CONTACT_DATA_VERSION,
          ContactData::SectionMap{}))
    , cached_contact_data_()
    , revision_(serialized.revision())
{
    if (serialized.has_contactdata()) {
        contact_data_.reset(new ContactData(
            api_,
            serialized.id(),
            CONTACT_CONTACT_DATA_VERSION,
            serialized.contactdata()));
    }

    OT_ASSERT(contact_data_);

    for (const auto& child : serialized.merged()) {
        merged_children_.emplace(api_.Factory().Identifier(child));
    }

    init_nyms(reason);
}

Contact::Contact(const api::client::Manager& api, const std::string& label)
    : api_(api)
    , version_(OT_CONTACT_VERSION)
    , label_(label)
    , lock_()
    , id_(generate_id(api_))
    , parent_(api_.Factory().Identifier())
    , primary_nym_(api_.Factory().NymID())
    , nyms_()
    , merged_children_()
    , contact_data_(nullptr)
    , cached_contact_data_()
    , revision_(1)
{
    contact_data_.reset(new ContactData(
        api_,
        String::Factory(id_)->Get(),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        ContactData::SectionMap{}));

    OT_ASSERT(contact_data_);
}

Contact::operator proto::Contact() const
{
    Lock lock(lock_);
    proto::Contact output{};
    output.set_version(version_);
    output.set_id(String::Factory(id_)->Get());
    output.set_revision(revision_);
    output.set_label(label_);

    if (contact_data_) {
        auto& data = *output.mutable_contactdata();
        data = contact_data_->Serialize();
    }

    output.set_mergedto(String::Factory(parent_)->Get());

    for (const auto& child : merged_children_) {
        output.add_merged(String::Factory(child)->Get());
    }

    return output;
}

Contact& Contact::operator+=(Contact& rhs)
{
    Lock rLock(rhs.lock_, std::defer_lock);
    Lock lock(lock_, std::defer_lock);
    std::lock(rLock, lock);

    if (label_.empty()) { label_ = rhs.label_; }

    rhs.parent_ = id_;

    if (primary_nym_->empty()) { primary_nym_ = rhs.primary_nym_; }

    for (const auto& it : rhs.nyms_) {
        const auto& id = it.first;
        const auto& nym = it.second;

        if (0 == nyms_.count(id)) { nyms_.emplace(id, nym); }
    }

    rhs.nyms_.clear();

    for (const auto& it : rhs.merged_children_) { merged_children_.insert(it); }

    merged_children_.insert(rhs.id_);
    rhs.merged_children_.clear();

    if (contact_data_) {
        if (rhs.contact_data_) {
            contact_data_.reset(
                new ContactData(*contact_data_ + *rhs.contact_data_));
        }
    } else {
        if (rhs.contact_data_) {
            contact_data_.reset(new ContactData(*rhs.contact_data_));
        }
    }

    rhs.contact_data_.reset();
    cached_contact_data_.reset();
    rhs.cached_contact_data_.reset();

    return *this;
}

bool Contact::add_claim(const std::shared_ptr<ContactItem>& item)
{
    Lock lock(lock_);

    return add_claim(lock, item);
}

bool Contact::add_claim(
    const Lock& lock,
    const std::shared_ptr<ContactItem>& item)
{
    OT_ASSERT(verify_write_lock(lock));

    if (false == bool(item)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null claim.").Flush();

        return false;
    }

    const auto version = std::make_pair(item->Version(), item->Section());
    const proto::ContactItem serialized(*item);

    if (false == proto::Validate<proto::ContactItem>(
                     serialized, VERBOSE, true, version)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid claim.").Flush();

        return false;
    }

    add_verified_claim(lock, item);

    return true;
}

bool Contact::add_nym(const Lock& lock, const Nym_p& nym, const bool primary)
{
    OT_ASSERT(verify_write_lock(lock));

    if (false == bool(nym)) { return false; }

    const auto contactType = type(lock);
    const auto nymType = ExtractType(*nym);
    const bool haveType = (proto::CITEMTYPE_ERROR != contactType) &&
                          (proto::CITEMTYPE_UNKNOWN != contactType);
    const bool typeMismatch = (contactType != nymType);

    if (haveType && typeMismatch) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong nym type.").Flush();

        return false;
    }

    const auto& id = nym->ID();
    const bool needPrimary = (0 == nyms_.size());
    const bool isPrimary = needPrimary || primary;
    nyms_[id] = nym;

    if (isPrimary) { primary_nym_ = id; }

    add_nym_claim(lock, id, isPrimary);

    return true;
}

void Contact::add_nym_claim(
    const Lock& lock,
    const identifier::Nym& nymID,
    const bool primary)
{
    OT_ASSERT(verify_write_lock(lock));

    std::set<proto::ContactItemAttribute> attr{proto::CITEMATTR_LOCAL,
                                               proto::CITEMATTR_ACTIVE};

    if (primary) { attr.emplace(proto::CITEMATTR_PRIMARY); }

    std::shared_ptr<ContactItem> claim{nullptr};
    claim.reset(new ContactItem(
        api_,
        String::Factory(id_)->Get(),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        proto::CONTACTSECTION_RELATIONSHIP,
        proto::CITEMTYPE_CONTACT,
        String::Factory(nymID)->Get(),
        attr,
        NULL_START,
        NULL_END,
        ""));

    add_claim(lock, claim);
}

void Contact::add_verified_claim(
    const Lock& lock,
    const std::shared_ptr<ContactItem>& item)
{
    OT_ASSERT(verify_write_lock(lock));
    OT_ASSERT(contact_data_);

    contact_data_.reset(new ContactData(contact_data_->AddItem(item)));

    OT_ASSERT(contact_data_);

    revision_++;
    cached_contact_data_.reset();
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
bool Contact::AddBlockchainAddress(
    const std::string& address,
    const proto::ContactItemType currency)
{
    const auto& api = api_;
    auto [bytes, style, chain] = api.Blockchain().DecodeAddress(address);
    const auto bad = bytes->empty() || (AddressStyle::Unknown == style) ||
                     (BlockchainType::Unknown == chain);

    if (bad) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decode address")
            .Flush();

        return false;
    }

    if (proto::CITEMTYPE_UNKNOWN != currency) { chain = Translate(currency); }

    return AddBlockchainAddress(style, chain, bytes);
}

bool Contact::AddBlockchainAddress(
    const api::client::blockchain::AddressStyle& style,
    const blockchain::Type chain,
    const opentxs::Data& bytes)
{
    Lock lock(lock_);

    std::shared_ptr<ContactItem> claim{nullptr};
    claim.reset(new ContactItem(
        api_,
        String::Factory(id_)->Get(),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        proto::CONTACTSECTION_ADDRESS,
        Translate(chain),
        bytes.asHex(),
        {proto::CITEMATTR_LOCAL, proto::CITEMATTR_ACTIVE},
        NULL_START,
        NULL_END,
        translate_style(style)));

    return add_claim(lock, claim);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

bool Contact::AddEmail(
    const std::string& value,
    const bool primary,
    const bool active)
{
    if (value.empty()) { return false; }

    Lock lock(lock_);

    contact_data_.reset(
        new ContactData(contact_data_->AddEmail(value, primary, active)));

    OT_ASSERT(contact_data_);

    revision_++;
    cached_contact_data_.reset();

    return true;
}

bool Contact::AddNym(const Nym_p& nym, const bool primary)
{
    Lock lock(lock_);

    return add_nym(lock, nym, primary);
}

bool Contact::AddNym(const identifier::Nym& nymID, const bool primary)
{
    Lock lock(lock_);

    const bool needPrimary = (0 == nyms_.size());
    const bool isPrimary = needPrimary || primary;

    if (isPrimary) { primary_nym_ = nymID; }

    add_nym_claim(lock, nymID, isPrimary);

    revision_++;

    return true;
}

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
bool Contact::AddPaymentCode(
    const class PaymentCode& code,
    const bool primary,
    const proto::ContactItemType currency,
    const bool active)
{
    std::set<proto::ContactItemAttribute> attr{proto::CITEMATTR_LOCAL};

    if (active) { attr.emplace(proto::CITEMATTR_ACTIVE); }

    if (primary) { attr.emplace(proto::CITEMATTR_PRIMARY); }

    const std::string value = code.asBase58();
    std::shared_ptr<ContactItem> claim{nullptr};
    claim.reset(new ContactItem(
        api_,
        String::Factory(id_)->Get(),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        proto::CONTACTSECTION_PROCEDURE,
        currency,
        value,
        attr,
        NULL_START,
        NULL_END,
        ""));

    if (false == add_claim(claim)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to add claim.").Flush();

        return false;
    }

    return true;
}
#endif

bool Contact::AddPhoneNumber(
    const std::string& value,
    const bool primary,
    const bool active)
{
    if (value.empty()) { return false; }

    Lock lock(lock_);

    contact_data_.reset(
        new ContactData(contact_data_->AddPhoneNumber(value, primary, active)));

    OT_ASSERT(contact_data_);

    revision_++;
    cached_contact_data_.reset();

    return true;
}

bool Contact::AddSocialMediaProfile(
    const std::string& value,
    const proto::ContactItemType type,
    const bool primary,
    const bool active)
{
    if (value.empty()) { return false; }

    Lock lock(lock_);

    contact_data_.reset(new ContactData(
        contact_data_->AddSocialMediaProfile(value, type, primary, active)));

    OT_ASSERT(contact_data_);

    revision_++;
    cached_contact_data_.reset();

    return true;
}

std::shared_ptr<ContactItem> Contact::Best(const ContactGroup& group)
{
    if (0 == group.Size()) { return {}; }

    const auto primary = group.PrimaryClaim();

    if (primary) { return primary; }

    for (const auto& it : group) {
        const auto& claim = it.second;

        if (claim->isActive()) { return claim; }
    }

    return group.begin()->second;
}

std::string Contact::BestEmail() const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    return data->BestEmail();
}

std::string Contact::BestPhoneNumber() const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    return data->BestPhoneNumber();
}

std::string Contact::BestSocialMediaProfile(
    const proto::ContactItemType type) const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    return data->BestSocialMediaProfile(type);
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
std::vector<Contact::BlockchainAddress> Contact::BlockchainAddresses() const
{
    auto output = std::vector<BlockchainAddress>{};
    Lock lock(lock_);
    auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    const auto& version = data->Version();
    const auto section = data->Section(proto::CONTACTSECTION_ADDRESS);

    if (false == bool(section)) { return {}; }

    for (const auto& it : *section) {
        const auto& type = it.first;
        const auto& group = it.second;

        OT_ASSERT(group);

        const bool currency = proto::ValidContactItemType(
            {version, proto::CONTACTSECTION_CONTRACT}, type);

        if (false == currency) { continue; }

        for (const auto& inner : *group) {
            const auto& item = inner.second;

            OT_ASSERT(item);

            try {
                output.push_back(
                    translate(api_, type, item->Value(), item->Subtype()));
            } catch (...) {
                continue;
            }
        }
    }

    return output;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

VersionNumber Contact::check_version(
    const VersionNumber in,
    const VersionNumber targetVersion)
{
    // Upgrade version
    if (targetVersion > in) { return targetVersion; }

    return in;
}

std::shared_ptr<ContactData> Contact::Data() const
{
    Lock lock(lock_);

    return merged_data(lock);
}

OTIdentifier Contact::generate_id(const api::Core& api)
{
    auto& encode = api.Crypto().Encode();
    auto random = Data::Factory();
    encode.Nonce(ID_BYTES, random);
    auto output = api.Factory().Identifier();
    output->CalculateDigest(random);

    return output;
}

std::string Contact::EmailAddresses(bool active) const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    return data->EmailAddresses(active);
}

std::string Contact::ExtractLabel(const identity::Nym& nym)
{
    return nym.Claims().Name();
}

proto::ContactItemType Contact::ExtractType(const identity::Nym& nym)
{
    return nym.Claims().Type();
}

const Identifier& Contact::ID() const { return id_; }

void Contact::init_nyms(const PasswordPrompt& reason)
{
    OT_ASSERT(contact_data_);

    const auto nyms = contact_data_->Group(
        proto::CONTACTSECTION_RELATIONSHIP, proto::CITEMTYPE_CONTACT);

    if (false == bool(nyms)) { return; }

    // TODO conversion
    primary_nym_ = api_.Factory().NymID(nyms->Primary().str());

    for (const auto& it : *nyms) {
        const auto& item = it.second;

        OT_ASSERT(item);

        const auto nymID = api_.Factory().NymID(item->Value());
        auto& nym = nyms_[nymID];
        nym = api_.Wallet().Nym(nymID, reason);

        if (false == bool(nym)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load nym ")(nymID)(
                ".")
                .Flush();
        }
    }
}

const std::string& Contact::Label() const { return label_; }

std::time_t Contact::LastUpdated() const
{
    OT_ASSERT(contact_data_);

    const auto group = contact_data_->Group(
        proto::CONTACTSECTION_EVENT, proto::CITEMTYPE_REFRESHED);

    if (false == bool(group)) { return {}; }

    const auto claim = group->PrimaryClaim();

    if (false == bool(claim)) { return {}; }

    try {
        if (sizeof(int) == sizeof(std::time_t)) {

            return std::stoi(claim->Value());
        } else if (sizeof(long) == sizeof(std::time_t)) {

            return std::stol(claim->Value());
        } else if (sizeof(long long) == sizeof(std::time_t)) {

            return std::stoll(claim->Value());
        } else {
            OT_FAIL;
        }

    } catch (const std::out_of_range&) {

        return {};
    } catch (const std::invalid_argument&) {

        return {};
    }
}

std::shared_ptr<ContactData> Contact::merged_data(const Lock& lock) const
{
    OT_ASSERT(contact_data_);
    OT_ASSERT(verify_write_lock(lock));

    if (cached_contact_data_) { return cached_contact_data_; }

    cached_contact_data_.reset(new ContactData(*contact_data_));
    auto& output = cached_contact_data_;

    OT_ASSERT(output);

    if (false == primary_nym_->empty()) {
        try {
            auto& primary = nyms_.at(primary_nym_);

            if (primary) {
                output.reset(new ContactData(*output + primary->Claims()));
            }
        } catch (const std::out_of_range&) {
        }
    }

    for (const auto& it : nyms_) {
        const auto& nymID = it.first;
        const auto& nym = it.second;

        if (false == bool(nym)) { continue; }

        if (nymID == primary_nym_) { continue; }

        output.reset(new ContactData(*output + nym->Claims()));
    }

    return output;
}

std::vector<opentxs::OTNymID> opentxs::Contact::Nyms(
    const bool includeInactive) const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    const auto group = data->Group(
        proto::CONTACTSECTION_RELATIONSHIP, proto::CITEMTYPE_CONTACT);

    if (false == bool(group)) { return {}; }

    std::vector<OTNymID> output{};
    const auto& primaryID = group->Primary();

    for (const auto& it : *group) {
        const auto& item = it.second;

        OT_ASSERT(item);

        const auto& itemID = item->ID();

        if (false == (includeInactive || item->isActive())) { continue; }

        if (primaryID == itemID) {
            output.emplace(output.begin(), api_.Factory().NymID(item->Value()));
        } else {
            output.emplace(output.end(), api_.Factory().NymID(item->Value()));
        }
    }

    return output;
}

std::shared_ptr<ContactGroup> Contact::payment_codes(
    const Lock& lock,
    const proto::ContactItemType currency) const
{
    const auto data = merged_data(lock);

    if (false == bool(data)) { return {}; }

    return data->Group(proto::CONTACTSECTION_PROCEDURE, currency);
}

std::string Contact::PaymentCode(
    const ContactData& data,
    const proto::ContactItemType currency)
{
    auto group = data.Group(proto::CONTACTSECTION_PROCEDURE, currency);

    if (false == bool(group)) { return {}; }

    const auto item = Best(*group);

    if (false == bool(item)) { return {}; }

    return item->Value();
}

std::string Contact::PaymentCode(const proto::ContactItemType currency) const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    return PaymentCode(*data, currency);
}

std::vector<std::string> Contact::PaymentCodes(
    const proto::ContactItemType currency) const
{
    Lock lock(lock_);
    const auto group = payment_codes(lock, currency);
    lock.unlock();

    if (false == bool(group)) { return {}; }

    std::vector<std::string> output{};

    for (const auto& it : *group) {
        OT_ASSERT(it.second);

        const auto& item = *it.second;
        output.emplace_back(item.Value());
    }

    return output;
}

std::string Contact::PhoneNumbers(bool active) const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    return data->PhoneNumbers(active);
}

std::string Contact::Print() const
{
    Lock lock(lock_);
    std::stringstream out{};
    out << "Contact: " << String::Factory(id_)->Get() << ", version "
        << version_ << "revision " << revision_ << "\n"
        << "Label: " << label_ << "\n";

    if (false == parent_->empty()) {
        out << "Merged to: " << String::Factory(parent_)->Get() << "\n";
    }

    if (false == merged_children_.empty()) {
        out << "Merged contacts:\n";

        for (const auto& id : merged_children_) {
            out << " * " << String::Factory(id)->Get() << "\n";
        }
    }

    if (0 < nyms_.size()) {
        out << "Contains nyms:\n";

        for (const auto& it : nyms_) {
            const auto& id = it.first;
            out << " * " << String::Factory(id)->Get();

            if (id == primary_nym_) { out << " (primary)"; }

            out << "\n";
        }
    }

    auto data = merged_data(lock);

    if (data) { out << std::string(*data); }

    out << std::endl;

    return out.str();
}

bool Contact::RemoveNym(const identifier::Nym& nymID)
{
    Lock lock(lock_);

    auto result = nyms_.erase(nymID);

    if (primary_nym_ == nymID) { primary_nym_ = api_.Factory().NymID(); }

    return (0 < result);
}

void Contact::SetLabel(const std::string& label)
{
    Lock lock(lock_);
    label_ = label;
    revision_++;

    for (const auto& it : nyms_) {
        const auto& nymID = it.first;
        api_.Wallet().SetNymAlias(nymID, label);
    }
}

std::string Contact::SocialMediaProfiles(
    const proto::ContactItemType type,
    bool active) const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) { return {}; }

    return data->SocialMediaProfiles(type, active);
}

const std::set<proto::ContactItemType> Contact::SocialMediaProfileTypes() const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    return data->SocialMediaProfileTypes();
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
Contact::BlockchainAddress Contact::translate(
    const api::client::Manager& api,
    const proto::ContactItemType chain,
    const std::string& value,
    const std::string& subtype) noexcept(false)
{
    auto output = BlockchainAddress{api.Factory().Data(value, StringStyle::Hex),
                                    translate_style(subtype),
                                    Translate(chain)};
    auto& [outBytes, outStyle, outChain] = output;
    const auto bad = outBytes->empty() || (AddressStyle::Unknown == outStyle) ||
                     (BlockchainType::Unknown == outChain);

    if (bad) { throw std::runtime_error("Invalid address"); }

    return output;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

proto::ContactItemType Contact::type(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    const auto data = merged_data(lock);

    if (false == bool(data)) { return proto::CITEMTYPE_ERROR; }

    return data->Type();
}

proto::ContactItemType Contact::Type() const
{
    Lock lock(lock_);

    return type(lock);
}

void Contact::Update(const proto::Nym& serialized, const PasswordPrompt& reason)
{
    auto nym = api_.Wallet().Nym(serialized, reason);

    if (false == bool(nym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid serialized nym.").Flush();

        return;
    }

    Lock lock(lock_);
    const auto& nymID = nym->ID();
    auto it = nyms_.find(nymID);

    if (nyms_.end() == it) {
        add_nym(lock, nym, false);
    } else {
        it->second = nym;
    }

    update_label(lock, *nym);
    std::shared_ptr<ContactItem> claim(new ContactItem(
        api_,
        String::Factory(id_)->Get(),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        proto::CONTACTSECTION_EVENT,
        proto::CITEMTYPE_REFRESHED,
        std::to_string(std::time(nullptr)),
        {proto::CITEMATTR_PRIMARY,
         proto::CITEMATTR_ACTIVE,
         proto::CITEMATTR_LOCAL},
        NULL_START,
        NULL_END,
        ""));
    add_claim(lock, claim);
}

void Contact::update_label(const Lock& lock, const identity::Nym& nym)
{
    OT_ASSERT(verify_write_lock(lock));

    if (false == label_.empty()) { return; }

    label_ = ExtractLabel(nym);
}

bool Contact::verify_write_lock(const Lock& lock) const
{
    if (lock.mutex() != &lock_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock not owned.").Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs
