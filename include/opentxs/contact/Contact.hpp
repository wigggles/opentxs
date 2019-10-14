// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CONTACT_CONTACT_HPP
#define OPENTXS_CONTACT_CONTACT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/identity/Nym.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <atomic>
#include <cstdint>
#include <ctime>
#include <mutex>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace client
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

class Contact
{
public:
#if OT_CRYPTO_SUPPORTED_KEY_HD
    using AddressStyle = api::client::blockchain::AddressStyle;
    using BlockchainType = blockchain::Type;
    using BlockchainAddress = std::tuple<OTData, AddressStyle, BlockchainType>;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

    static std::shared_ptr<ContactItem> Best(const ContactGroup& group);
    static std::string ExtractLabel(const identity::Nym& nym);
    static proto::ContactItemType ExtractType(const identity::Nym& nym);
    static std::string PaymentCode(
        const ContactData& data,
        const proto::ContactItemType currency);

    Contact(
        const PasswordPrompt& reason,
        const api::client::internal::Manager& api,
        const proto::Contact& serialized);
    Contact(
        const api::client::internal::Manager& api,
        const std::string& label);

    operator proto::Contact() const;
    Contact& operator+=(Contact& rhs);

    std::string BestEmail() const;
    std::string BestPhoneNumber() const;
    std::string BestSocialMediaProfile(const proto::ContactItemType type) const;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    std::vector<BlockchainAddress> BlockchainAddresses() const;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    std::shared_ptr<ContactData> Data() const;
    std::string EmailAddresses(bool active = true) const;
    const Identifier& ID() const;
    const std::string& Label() const;
    std::time_t LastUpdated() const;
    std::vector<OTNymID> Nyms(const bool includeInactive = false) const;
    std::string PaymentCode(
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC) const;
    std::vector<std::string> PaymentCodes(
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC) const;
    std::string PhoneNumbers(bool active = true) const;
    std::string Print() const;
    std::string SocialMediaProfiles(
        const proto::ContactItemType type,
        bool active = true) const;
    const std::set<proto::ContactItemType> SocialMediaProfileTypes() const;
    proto::ContactItemType Type() const;

#if OT_CRYPTO_SUPPORTED_KEY_HD
    bool AddBlockchainAddress(
        const std::string& address,
        const proto::ContactItemType currency = proto::CITEMTYPE_UNKNOWN);
    bool AddBlockchainAddress(
        const api::client::blockchain::AddressStyle& style,
        const blockchain::Type chain,
        const opentxs::Data& bytes);
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    bool AddEmail(
        const std::string& value,
        const bool primary,
        const bool active);
    bool AddNym(const Nym_p& nym, const bool primary);
    bool AddNym(const identifier::Nym& nymID, const bool primary);
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    bool AddPaymentCode(
        const class PaymentCode& code,
        const bool primary,
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC,
        const bool active = true);
#endif
    bool AddPhoneNumber(
        const std::string& value,
        const bool primary,
        const bool active);
    bool AddSocialMediaProfile(
        const std::string& value,
        const proto::ContactItemType type,
        const bool primary,
        const bool active);
    bool RemoveNym(const identifier::Nym& nymID);
    void SetLabel(const std::string& label);
    void Update(
        const identity::Nym::Serialized& nym,
        const PasswordPrompt& reason);

    ~Contact() = default;

private:
    const api::client::internal::Manager& api_;
    VersionNumber version_{0};
    std::string label_{""};
    mutable std::mutex lock_{};
    const OTIdentifier id_;
    OTIdentifier parent_;
    OTNymID primary_nym_;
    std::map<OTNymID, Nym_p> nyms_;
    std::set<OTIdentifier> merged_children_;
    std::unique_ptr<ContactData> contact_data_{};
    mutable std::shared_ptr<ContactData> cached_contact_data_{};
    std::atomic<std::uint64_t> revision_{0};

    static VersionNumber check_version(
        const VersionNumber in,
        const VersionNumber targetVersion);
    static OTIdentifier generate_id(const api::internal::Core& api);
#if OT_CRYPTO_SUPPORTED_KEY_HD
    static BlockchainAddress translate(
        const api::client::internal::Manager& api,
        const proto::ContactItemType chain,
        const std::string& value,
        const std::string& subtype) noexcept(false);
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

    std::shared_ptr<ContactGroup> payment_codes(
        const Lock& lock,
        const proto::ContactItemType currency) const;
    std::shared_ptr<ContactData> merged_data(const Lock& lock) const;
    proto::ContactItemType type(const Lock& lock) const;
    bool verify_write_lock(const Lock& lock) const;

    bool add_nym(const Lock& lock, const Nym_p& nym, const bool primary);
    bool add_claim(const std::shared_ptr<ContactItem>& item);
    bool add_claim(const Lock& lock, const std::shared_ptr<ContactItem>& item);
    void add_nym_claim(
        const Lock& lock,
        const identifier::Nym& nymID,
        const bool primary);
    void add_verified_claim(
        const Lock& lock,
        const std::shared_ptr<ContactItem>& item);
    void init_nyms(const PasswordPrompt& reason);
    void update_label(const Lock& lock, const identity::Nym& nym);

    Contact() = delete;
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    Contact& operator=(const Contact&) = delete;
    Contact& operator=(Contact&&) = delete;
};
}  // namespace opentxs
#endif
