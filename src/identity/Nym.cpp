// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/api/HDSeed.hpp"
#endif
#include "opentxs/api/Wallet.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTSignedFile.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/identity/Authority.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Source.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/Proto.tpp"

#include "internal/api/Api.hpp"
#include "internal/identity/Identity.hpp"

#include <irrxml/irrXML.hpp>
#include <sodium/crypto_box.h>
#include <sys/types.h>

#include <array>
#include <atomic>
#include <deque>
#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <mutex>

#include "Nym.hpp"

#define OT_METHOD "opentxs::identity::implementation::Nym::"

namespace opentxs
{
identity::internal::Nym* Factory::Nym(
    const api::internal::Core& api,
    const NymParameters& params,
    const proto::ContactItemType type,
    const std::string name,
    const opentxs::PasswordPrompt& reason)
{
    using ReturnType = identity::implementation::Nym;

    if ((proto::CREDTYPE_LEGACY == params.credentialType()) &&
        (proto::SOURCETYPE_BIP47 == params.SourceType())) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Invalid parameters")
            .Flush();

        return nullptr;
    }

    try {
        auto revised = ReturnType::normalize(api, params, reason);
        auto pSource = std::unique_ptr<identity::Source>{
            NymIDSource(api, revised, reason)};

        if (false == bool(pSource)) {
            LogOutput("opentxs::Factory::")(__FUNCTION__)(
                ": Failed to generate nym id source")
                .Flush();

            return nullptr;
        }

        if (proto::CITEMTYPE_ERROR != type && !name.empty()) {
            const auto version =
                ReturnType::contact_credential_to_contact_data_version_.at(
                    identity::internal::Authority::NymToContactCredential(
                        identity::Nym::DefaultVersion));
            const auto blank = ContactData{api,
                                           pSource->NymID()->str(),
                                           version,
                                           version,
                                           ContactData::SectionMap{}};
            const auto scope = blank.SetScope(type, name);
            revised.SetContactData(scope.Serialize());
        }

        return new ReturnType(api, revised, std::move(pSource), reason);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Failed to create nym: ")(e.what())
            .Flush();

        return nullptr;
    }
}

identity::internal::Nym* Factory::Nym(
    const api::internal::Core& api,
    const proto::Nym& serialized,
    const std::string& alias)
{
    try {
        return new identity::implementation::Nym(api, serialized, alias);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Failed to instantiate nym: ")(e.what())
            .Flush();

        return nullptr;
    }
}
}  // namespace opentxs

namespace opentxs::identity
{
const VersionNumber Nym::DefaultVersion{6};
const VersionNumber Nym::MaxVersion{6};
}  // namespace opentxs::identity

namespace opentxs::identity::implementation
{
bool session_key_from_iv(
    const api::internal::Core& api,
    const crypto::key::Asymmetric& signingKey,
    const Data& iv,
    const proto::HashType hashType,
    opentxs::PasswordPrompt& reason);

const VersionConversionMap Nym::akey_to_session_key_version_{
    {1, 1},
    {2, 1},
};
const VersionConversionMap Nym::contact_credential_to_contact_data_version_{
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 4},
    {5, 5},
    {6, 6},
};

Nym::Nym(
    const api::internal::Core& api,
    NymParameters& params,
    std::unique_ptr<const identity::Source> source,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    : api_(api)
    , source_p_(std::move(source))
    , source_(*source_p_)
    , id_(source_.NymID())
    , mode_(proto::NYM_PRIVATE)
    , version_(DefaultVersion)
    , index_(1)
    , alias_()
    , revision_(0)
    , contact_data_(nullptr)
    , active_(create_authority(api_, *this, source_, version_, params, reason))
    , m_mapRevokedSets()
    , m_listRevokedIDs()
{
    if (false == bool(source_p_)) {
        throw std::runtime_error("Invalid nym id source");
    }
}

Nym::Nym(
    const api::internal::Core& api,
    const proto::Nym& serialized,
    const std::string& alias) noexcept(false)
    : api_(api)
    , source_p_(opentxs::Factory::NymIDSource(api, serialized.source()))
    , source_(*source_p_)
    , id_(source_.NymID())
    , mode_(serialized.mode())
    , version_(serialized.version())
    , index_(serialized.index())
    , alias_(alias)
    , revision_(serialized.revision())
    , contact_data_(nullptr)
    , active_(load_authorities(api_, *this, source_, serialized))
    , m_mapRevokedSets()
    , m_listRevokedIDs(
          load_revoked(api_, *this, source_, serialized, m_mapRevokedSets))
{
    if (false == bool(source_p_)) {
        throw std::runtime_error("Invalid nym id source");
    }
}

bool Nym::add_contact_credential(
    const eLock& lock,
    const proto::ContactData& data,
    const opentxs::PasswordPrompt& reason)
{
    OT_ASSERT(verify_lock(lock));

    bool added = false;

    for (auto& it : active_) {
        if (nullptr != it.second) {
            if (it.second->hasCapability(NymCapability::SIGN_CHILDCRED)) {
                added = it.second->AddContactCredential(data, reason);

                break;
            }
        }
    }

    return added;
}

bool Nym::add_verification_credential(
    const eLock& lock,
    const proto::VerificationSet& data,
    const opentxs::PasswordPrompt& reason)
{
    OT_ASSERT(verify_lock(lock));

    bool added = false;

    for (auto& it : active_) {
        if (nullptr != it.second) {
            if (it.second->hasCapability(NymCapability::SIGN_CHILDCRED)) {
                added = it.second->AddVerificationCredential(data, reason);

                break;
            }
        }
    }

    return added;
}

std::string Nym::AddChildKeyCredential(
    const Identifier& masterID,
    const NymParameters& nymParameters,
    const opentxs::PasswordPrompt& reason)
{
    eLock lock(shared_lock_);

    std::string output;
    auto it = active_.find(masterID);
    const bool noMaster = (it == active_.end());

    if (noMaster) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Master ID not found.").Flush();

        return output;
    }

    if (it->second) {
        output = it->second->AddChildKeyCredential(nymParameters, reason);
    }

    return output;
}

bool Nym::AddClaim(const Claim& claim, const opentxs::PasswordPrompt& reason)
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(contact_data_->AddItem(claim)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize(), reason);
}

bool Nym::AddContract(
    const identifier::UnitDefinition& instrumentDefinitionID,
    const proto::ContactItemType currency,
    const opentxs::PasswordPrompt& reason,
    const bool primary,
    const bool active)
{
    const std::string id(instrumentDefinitionID.str());

    if (id.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(
        contact_data_->AddContract(id, currency, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize(), reason);
}

bool Nym::AddEmail(
    const std::string& value,
    const opentxs::PasswordPrompt& reason,
    const bool primary,
    const bool active)
{
    if (value.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(
        new ContactData(contact_data_->AddEmail(value, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize(), reason);
}

bool Nym::AddPaymentCode(
    const opentxs::PaymentCode& code,
    const proto::ContactItemType currency,
    const opentxs::PasswordPrompt& reason,
    const bool primary,
    const bool active)
{
    const auto paymentCode = code.asBase58();

    if (paymentCode.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(
        contact_data_->AddPaymentCode(paymentCode, currency, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize(), reason);
}

bool Nym::AddPhoneNumber(
    const std::string& value,
    const opentxs::PasswordPrompt& reason,
    const bool primary,
    const bool active)
{
    if (value.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(
        new ContactData(contact_data_->AddPhoneNumber(value, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize(), reason);
}

bool Nym::AddPreferredOTServer(
    const Identifier& id,
    const opentxs::PasswordPrompt& reason,
    const bool primary)
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_)

    contact_data_.reset(
        new ContactData(contact_data_->AddPreferredOTServer(id, primary)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize(), reason);
}

bool Nym::AddSocialMediaProfile(
    const std::string& value,
    const proto::ContactItemType type,
    const opentxs::PasswordPrompt& reason,
    const bool primary,
    const bool active)
{
    if (value.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(
        contact_data_->AddSocialMediaProfile(value, type, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize(), reason);
}

std::string Nym::Alias() const { return alias_; }

const Nym::Serialized Nym::asPublicNym() const
{
    return SerializeCredentialIndex(Mode::Full);
}

const Nym::value_type& Nym::at(const std::size_t& index) const noexcept(false)
{
    for (auto i{active_.cbegin()}; i != active_.cend(); ++i) {
        if (static_cast<std::size_t>(std::distance(active_.cbegin(), i)) ==
            index) {
            return *i->second;
        }
    }

    throw std::out_of_range("Invalid authority index");
}

std::string Nym::BestEmail() const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->BestEmail();
}

std::string Nym::BestPhoneNumber() const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->BestPhoneNumber();
}

std::string Nym::BestSocialMediaProfile(const proto::ContactItemType type) const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->BestSocialMediaProfile(type);
}

const opentxs::ContactData& Nym::Claims() const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return *contact_data_;
}

bool Nym::CompareID(const identity::Nym& rhs) const
{
    sLock lock(shared_lock_);

    return rhs.CompareID(id_);
}

bool Nym::CompareID(const identifier::Nym& rhs) const
{
    sLock lock(shared_lock_);

    return id_ == rhs;
}

VersionNumber Nym::ContactCredentialVersion() const
{
    // TODO support multiple authorities
    OT_ASSERT(0 < active_.size())

    return active_.cbegin()->second->ContactCredentialVersion();
}

std::set<OTIdentifier> Nym::Contracts(
    const proto::ContactItemType currency,
    const bool onlyActive) const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->Contracts(currency, onlyActive);
}

auto Nym::create_authority(
    const api::internal::Core& api,
    const identity::Nym& parent,
    const identity::Source& source,
    const VersionNumber version,
    const NymParameters& params,
    const PasswordPrompt& reason) noexcept(false) -> CredentialMap
{
    auto output = CredentialMap{};
    auto pAuthority = std::unique_ptr<identity::internal::Authority>(
        opentxs::Factory::Authority(
            api, parent, source, params, version, reason));

    if (false == bool(pAuthority)) {
        throw std::runtime_error("Failed to create nym authority");
    }

    auto& authority = *pAuthority;
    auto id{authority.GetMasterCredID()};
    output.emplace(std::move(id), std::move(pAuthority));

    return output;
}

bool Nym::DeleteClaim(
    const Identifier& id,
    const opentxs::PasswordPrompt& reason)
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(contact_data_->Delete(id)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize(), reason);
}

std::string Nym::EmailAddresses(bool active) const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->EmailAddresses(active);
}

auto Nym::EncryptionTargets() const noexcept -> NymKeys
{
    sLock lock(shared_lock_);
    auto output = NymKeys{id_, {}};

    for (const auto& [id, pAuthority] : active_) {
        const auto& authority = *pAuthority;

        if (authority.hasCapability(NymCapability::ENCRYPT_MESSAGE)) {
            output.second.emplace_back(authority.EncryptionTargets());
        }
    }

    return output;
}

void Nym::GetIdentifier(identifier::Nym& theIdentifier) const
{
    sLock lock(shared_lock_);

    theIdentifier.Assign(id_);
}

// sets argument based on internal member
void Nym::GetIdentifier(String& theIdentifier) const
{
    sLock lock(shared_lock_);

    id_->GetString(theIdentifier);
}

template <typename T>
const crypto::key::Asymmetric& Nym::get_private_auth_key(
    const T& lock,
    proto::AsymmetricKeyType keytype) const
{
    OT_ASSERT(!active_.empty());

    OT_ASSERT(verify_lock(lock));
    const identity::Authority* pCredential{nullptr};

    for (const auto& it : active_) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second.get();
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPrivateAuthKey(
        keytype, &m_listRevokedIDs);  // success
}

const crypto::key::Asymmetric& Nym::GetPrivateAuthKey(
    proto::AsymmetricKeyType keytype) const
{
    sLock lock(shared_lock_);

    return get_private_auth_key(lock, keytype);
}

const crypto::key::Asymmetric& Nym::GetPrivateEncrKey(
    proto::AsymmetricKeyType keytype) const
{
    sLock lock(shared_lock_);

    OT_ASSERT(!active_.empty());

    const identity::Authority* pCredential{nullptr};

    for (const auto& it : active_) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second.get();
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPrivateEncrKey(
        keytype,
        &m_listRevokedIDs);  // success
}

const crypto::key::Asymmetric& Nym::GetPrivateSignKey(
    proto::AsymmetricKeyType keytype) const
{
    sLock lock(shared_lock_);

    return get_private_sign_key(lock, keytype);
}

template <typename T>
const crypto::key::Asymmetric& Nym::get_private_sign_key(
    const T& lock,
    proto::AsymmetricKeyType keytype) const
{
    OT_ASSERT(!active_.empty());

    OT_ASSERT(verify_lock(lock));

    const identity::Authority* pCredential{nullptr};

    for (const auto& it : active_) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second.get();
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPrivateSignKey(
        keytype,
        &m_listRevokedIDs);  // success
}

template <typename T>
const crypto::key::Asymmetric& Nym::get_public_sign_key(
    const T& lock,
    proto::AsymmetricKeyType keytype) const
{
    OT_ASSERT(!active_.empty());

    OT_ASSERT(verify_lock(lock));

    const identity::Authority* pCredential{nullptr};

    for (const auto& it : active_) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second.get();
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPublicSignKey(
        keytype,
        &m_listRevokedIDs);  // success
}

const crypto::key::Asymmetric& Nym::GetPublicAuthKey(
    proto::AsymmetricKeyType keytype) const
{
    sLock lock(shared_lock_);

    OT_ASSERT(!active_.empty());

    const identity::Authority* pCredential{nullptr};

    for (const auto& it : active_) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second.get();
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPublicAuthKey(
        keytype,
        &m_listRevokedIDs);  // success
}

const crypto::key::Asymmetric& Nym::GetPublicEncrKey(
    proto::AsymmetricKeyType keytype) const
{
    sLock lock(shared_lock_);

    OT_ASSERT(!active_.empty());

    const identity::Authority* pCredential{nullptr};
    for (const auto& it : active_) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second.get();
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPublicEncrKey(
        keytype,
        &m_listRevokedIDs);  // success
}

// This is being called by:
// Contract::VerifySignature(const Nym& theNym, const Signature&
// theSignature, PasswordPrompt * pPWData=nullptr)
//
// Note: Need to change Contract::VerifySignature so that it checks all of
// these keys when verifying.
//
// OT uses the signature's metadata to narrow down its search for the correct
// public key.
// Return value is the count of public keys found that matched the metadata on
// the signature.
std::int32_t Nym::GetPublicKeysBySignature(
    crypto::key::Keypair::Keys& listOutput,
    const Signature& theSignature,
    char cKeyType) const
{
    // Unfortunately, theSignature can only narrow the search down (there may be
    // multiple results.)
    std::int32_t nCount = 0;

    sLock lock(shared_lock_);

    for (const auto& it : active_) {
        const identity::Authority* pCredential = it.second.get();
        OT_ASSERT(nullptr != pCredential);

        const std::int32_t nTempCount = pCredential->GetPublicKeysBySignature(
            listOutput, theSignature, cKeyType);
        nCount += nTempCount;
    }

    return nCount;
}

const crypto::key::Asymmetric& Nym::GetPublicSignKey(
    proto::AsymmetricKeyType keytype) const
{
    sLock lock(shared_lock_);

    return get_public_sign_key(lock, keytype);
}

bool Nym::has_capability(const eLock& lock, const NymCapability& capability)
    const
{
    OT_ASSERT(verify_lock(lock));

    for (auto& it : active_) {
        OT_ASSERT(nullptr != it.second);

        if (nullptr != it.second) {
            const identity::Authority& credSet = *it.second;

            if (credSet.hasCapability(capability)) { return true; }
        }
    }

    return false;
}

bool Nym::HasCapability(const NymCapability& capability) const
{
    eLock lock(shared_lock_);

    return has_capability(lock, capability);
}

void Nym::init_claims(const eLock& lock) const
{
    OT_ASSERT(verify_lock(lock));

    const auto nymID{id_->str()};
    const auto dataVersion = ContactDataVersion();
    contact_data_.reset(new opentxs::ContactData(
        api_, nymID, dataVersion, dataVersion, ContactData::SectionMap()));

    OT_ASSERT(contact_data_);

    std::unique_ptr<proto::ContactData> serialized{nullptr};

    for (auto& it : active_) {
        OT_ASSERT(nullptr != it.second);

        const auto& credSet = *it.second;
        credSet.GetContactData(serialized);

        if (serialized) {
            OT_ASSERT(proto::Validate(
                *serialized, VERBOSE, proto::ClaimType::Normal));

            opentxs::ContactData claimCred(
                api_, nymID, dataVersion, *serialized);
            contact_data_.reset(
                new opentxs::ContactData(*contact_data_ + claimCred));
            serialized.reset();
        }
    }

    OT_ASSERT(contact_data_)
}

auto Nym::load_authorities(
    const api::internal::Core& api,
    const identity::Nym& parent,
    const identity::Source& source,
    const Serialized& serialized) noexcept(false) -> CredentialMap
{
    auto output = CredentialMap{};

    if (false == proto::Validate<proto::Nym>(serialized, VERBOSE)) {
        throw std::runtime_error("Invalid serialized nym");
    }

    const auto mode = (proto::NYM_PRIVATE == serialized.mode())
                          ? proto::KEYMODE_PRIVATE
                          : proto::KEYMODE_PUBLIC;

    for (auto& it : serialized.activecredentials()) {
        auto pCandidate = std::unique_ptr<identity::internal::Authority>{
            opentxs::Factory::Authority(api, parent, source, mode, it)};

        if (false == bool(pCandidate)) {
            throw std::runtime_error("Failed to instantiate authority");
        }

        const auto& candidate = *pCandidate;
        auto id{candidate.GetMasterCredID()};
        output.emplace(std::move(id), std::move(pCandidate));
    }

    return output;
}

String::List Nym::load_revoked(
    const api::internal::Core& api,
    const identity::Nym& parent,
    const identity::Source& source,
    const Serialized& serialized,
    CredentialMap& revoked) noexcept(false)
{
    auto output = String::List{};

    if (!opentxs::operator==(Serialized::default_instance(), serialized)) {
        const auto mode = (proto::NYM_PRIVATE == serialized.mode())
                              ? proto::KEYMODE_PRIVATE
                              : proto::KEYMODE_PUBLIC;

        for (auto& it : serialized.revokedcredentials()) {
            auto pCandidate = std::unique_ptr<identity::internal::Authority>{
                opentxs::Factory::Authority(api, parent, source, mode, it)};

            if (false == bool(pCandidate)) {
                throw std::runtime_error("Failed to instantiate authority");
            }

            const auto& candidate = *pCandidate;
            auto id{candidate.GetMasterCredID()};
            output.push_back(id->str());
            revoked.emplace(std::move(id), std::move(pCandidate));
        }
    }

    return output;
}

std::string Nym::Name() const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    std::string output = contact_data_->Name();

    if (false == output.empty()) { return output; }

    return alias_;
}

NymParameters Nym::normalize(
    const api::internal::Core& api,
    const NymParameters& in,
    const PasswordPrompt& reason) noexcept(false)
{
    auto output{in};

    if (proto::CREDTYPE_HD == in.credentialType()) {
#if OT_CRYPTO_WITH_BIP32
        output.SetCredset(0);
        Bip32Index nymIndex = 0;
        std::string fingerprint = in.Seed();
        auto seed = api.Seeds().Seed(fingerprint, nymIndex, reason);

        OT_ASSERT(seed);

        const bool defaultIndex = in.UseAutoIndex();

        if (false == defaultIndex) {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Re-creating nym at specified path.")
                .Flush();

            nymIndex = in.Nym();
        }

        const std::int32_t newIndex = nymIndex + 1;
        api.Seeds().UpdateIndex(fingerprint, newIndex, reason);
        output.SetEntropy(*seed);
        output.SetSeed(fingerprint);
        output.SetNym(nymIndex);
#else
        throw std::runtime_error(
            "opentxs compiled without hd credential support");
#endif
    }

    return output;
}

bool Nym::Path(proto::HDPath& output) const
{
    sLock lock(shared_lock_);

    for (const auto& it : active_) {
        OT_ASSERT(nullptr != it.second);
        const auto& set = *it.second;

        if (set.Path(output)) {
            output.mutable_child()->RemoveLast();
            return true;
        }
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": No credential set contains a path.")
        .Flush();

    return false;
}

std::string Nym::PaymentCode() const
{
    if (proto::SOURCETYPE_BIP47 != source_.Type()) { return ""; }

    auto serialized = source_.Serialize();

    if (!serialized) { return ""; }

    auto paymentCode = api_.Factory().PaymentCode(serialized->paymentcode());

    return paymentCode->asBase58();
}

std::string Nym::PhoneNumbers(bool active) const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->PhoneNumbers(active);
}

std::uint64_t Nym::Revision() const { return revision_.load(); }

void Nym::revoke_contact_credentials(const eLock& lock)
{
    OT_ASSERT(verify_lock(lock));

    std::list<std::string> revokedIDs;

    for (auto& it : active_) {
        if (nullptr != it.second) {
            it.second->RevokeContactCredentials(revokedIDs);
        }
    }

    for (auto& it : revokedIDs) { m_listRevokedIDs.push_back(it); }
}

void Nym::revoke_verification_credentials(const eLock& lock)
{
    OT_ASSERT(verify_lock(lock));

    std::list<std::string> revokedIDs;

    for (auto& it : active_) {
        if (nullptr != it.second) {
            it.second->RevokeVerificationCredentials(revokedIDs);
        }
    }

    for (auto& it : revokedIDs) { m_listRevokedIDs.push_back(it); }
}

Nym::Serialized Nym::SerializeCredentialIndex(const Mode mode) const
{
    sLock lock(shared_lock_);
    Serialized index;
    index.set_version(version_);
    auto nymID = String::Factory(id_);
    index.set_nymid(nymID->Get());

    if (Mode::Abbreviated == mode) {
        index.set_mode(mode_);

        if (proto::NYM_PRIVATE == mode_) { index.set_index(index_); }
    } else {
        index.set_mode(proto::NYM_PUBLIC);
    }

    index.set_revision(revision_.load());
    *(index.mutable_source()) = *(source_.Serialize());

    for (auto& it : active_) {
        if (nullptr != it.second) {
            auto credset = it.second->Serialize(static_cast<bool>(mode));
            auto pCredSet = index.add_activecredentials();
            *pCredSet = *credset;
            pCredSet = nullptr;
        }
    }

    for (auto& it : m_mapRevokedSets) {
        if (nullptr != it.second) {
            auto credset = it.second->Serialize(static_cast<bool>(mode));
            auto pCredSet = index.add_revokedcredentials();
            *pCredSet = *credset;
            pCredSet = nullptr;
        }
    }

    return index;
}

void Nym::SerializeNymIDSource(Tag& parent) const
{
    // We encode these before storing.
    TagPtr pTag(new Tag("nymIDSource", source_.asString()->Get()));
    const auto description = source_.Description();

    if (description->Exists()) {
        auto ascDescription = Armored::Factory();
        ascDescription->SetString(
            description,
            false);  // bLineBreaks=true by default.

        pTag->add_attribute("Description", ascDescription->Get());
    }

    parent.add_tag(pTag);
}

bool Nym::set_contact_data(
    const eLock& lock,
    const proto::ContactData& data,
    const opentxs::PasswordPrompt& reason)
{
    OT_ASSERT(verify_lock(lock));

    auto version = proto::NymRequiredVersion(data.version(), version_);

    if ((0 == version) || version > MaxVersion) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Contact data version not supported by this nym.")
            .Flush();

        return false;
    }

    if (false == has_capability(lock, NymCapability::SIGN_CHILDCRED)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": This nym can not be modified.")
            .Flush();

        return false;
    }

    if (false == proto::Validate(data, VERBOSE, proto::ClaimType::Normal)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid contact data.").Flush();

        return false;
    }

    revoke_contact_credentials(lock);

    if (add_contact_credential(lock, data, reason)) {

        return update_nym(lock, version, reason);
    }

    return false;
}

void Nym::SetAlias(const std::string& alias)
{
    eLock lock(shared_lock_);

    alias_ = alias;
    revision_++;
}

bool Nym::SetCommonName(
    const std::string& name,
    const opentxs::PasswordPrompt& reason)
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(contact_data_->SetCommonName(name)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize(), reason);
}

bool Nym::SetContactData(
    const proto::ContactData& data,
    const opentxs::PasswordPrompt& reason)
{
    eLock lock(shared_lock_);
    contact_data_.reset(
        new ContactData(api_, id_->str(), ContactDataVersion(), data));

    return set_contact_data(lock, data, reason);
}

bool Nym::SetScope(
    const proto::ContactItemType type,
    const std::string& name,
    const opentxs::PasswordPrompt& reason,
    const bool primary)
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    if (proto::CITEMTYPE_UNKNOWN != contact_data_->Type()) {
        contact_data_.reset(
            new ContactData(contact_data_->SetName(name, primary)));
    } else {
        contact_data_.reset(
            new ContactData(contact_data_->SetScope(type, name)));
    }

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize(), reason);
}

bool Nym::Sign(
    const ProtobufType& input,
    const proto::SignatureRole role,
    proto::Signature& signature,
    const opentxs::PasswordPrompt& reason,
    const proto::HashType hash) const
{
    sLock lock(shared_lock_);

    bool haveSig = false;

    auto preimage = [&input]() -> std::string {
        return proto::ToString(input);
    };

    for (auto& it : active_) {
        if (nullptr != it.second) {
            bool success = it.second->Sign(
                preimage, role, signature, reason, proto::KEYROLE_SIGN, hash);

            if (success) {
                haveSig = true;
                break;
            } else {
                LogOutput(": Credential set ")(it.second->GetMasterCredID())(
                    " could not sign protobuf.")
                    .Flush();
            }
        }

        LogOutput(": Did not find any credential sets capable of signing on "
                  "this nym.")
            .Flush();
    }

    return haveSig;
}

std::string Nym::SocialMediaProfiles(
    const proto::ContactItemType type,
    bool active) const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->SocialMediaProfiles(type, active);
}

const std::set<proto::ContactItemType> Nym::SocialMediaProfileTypes() const
{
    sLock lock(shared_lock_);

    return contact_data_->SocialMediaProfileTypes();
}

std::unique_ptr<OTPassword> Nym::TransportKey(
    Data& pubkey,
    const opentxs::PasswordPrompt& reason) const
{
    bool found{false};
    auto privateKey = api_.Factory().BinarySecret();

    OT_ASSERT(privateKey);

    sLock lock(shared_lock_);

    for (auto& it : active_) {
        OT_ASSERT(nullptr != it.second);

        if (nullptr != it.second) {
            const identity::Authority& credSet = *it.second;
            found = credSet.TransportKey(pubkey, *privateKey, reason);

            if (found) { break; }
        }
    }

    OT_ASSERT(found);

    return privateKey;
}

auto Nym::Unlock(
    const crypto::key::Asymmetric& dhKey,
    const std::uint32_t tag,
    const proto::AsymmetricKeyType type,
    const crypto::key::Symmetric& key,
    PasswordPrompt& reason) const noexcept -> bool
{
    for (const auto& [id, authority] : active_) {
        if (authority->Unlock(dhKey, tag, type, key, reason)) { return true; }
    }

    for (const auto& [id, authority] : m_mapRevokedSets) {
        if (authority->Unlock(dhKey, tag, type, key, reason)) { return true; }
    }

    return false;
}

bool Nym::update_nym(
    const eLock& lock,
    const std::int32_t version,
    const opentxs::PasswordPrompt& reason)
{
    OT_ASSERT(verify_lock(lock));

    if (verify_pseudonym(lock)) {
        // Upgrade version
        if (version > version_) { version_ = version; }

        ++revision_;

        return true;
    }

    return false;
}

bool Nym::Verify(const ProtobufType& input, proto::Signature& signature) const
{
    const auto copy{signature};
    signature.clear_signature();
    const auto plaintext = api_.Factory().Data(input);

    for (auto& it : active_) {
        if (nullptr != it.second) {
            if (it.second->Verify(plaintext, copy)) { return true; }
        }
    }

    return false;
}

bool Nym::verify_pseudonym(const eLock& lock) const
{
    // If there are credentials, then we verify the Nym via his credentials.
    if (!active_.empty()) {
        // Verify Nym by his own credentials.
        for (const auto& it : active_) {
            const identity::Authority* pCredential = it.second.get();
            OT_ASSERT(nullptr != pCredential);

            // Verify all Credentials in the Authority, including source
            // verification for the master credential.
            if (!pCredential->VerifyInternally()) {
                LogNormal(OT_METHOD)(__FUNCTION__)(": Credential (")(
                    pCredential->GetMasterCredID())(
                    ") failed its own internal verification.")
                    .Flush();
                return false;
            }
        }
        return true;
    }
    LogOutput(OT_METHOD)(__FUNCTION__)(": No credentials.").Flush();
    return false;
}

bool Nym::VerifyPseudonym() const
{
    eLock lock(shared_lock_);

    return verify_pseudonym(lock);
}

bool Nym::WriteCredentials() const
{
    sLock lock(shared_lock_);

    for (auto& it : active_) {
        if (!it.second->WriteCredentials()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save credentials.")
                .Flush();

            return false;
        }
    }

    return true;
}
}  // namespace opentxs::identity::implementation
