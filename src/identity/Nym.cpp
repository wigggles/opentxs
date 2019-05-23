// Copyright (c) 2018 The Open-Transactions developers
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
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/HDSeed.hpp"
#endif
#include "opentxs/api/Wallet.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSignedFile.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NymIDSource.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/LegacySymmetric.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/identity/Authority.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/ext/OTPayment.hpp"

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

#define NYMFILE_VERSION "1.1"

#define OT_METHOD "opentxs::identity::implementation::Nym::"

namespace opentxs
{
identity::internal::Nym* Factory::Nym(
    const api::Core& api,
    const NymParameters& nymParameters)
{
    return new identity::implementation::Nym(api, nymParameters);
}

identity::internal::Nym* Factory::Nym(
    const api::Core& api,
    const identifier::Nym& nymID,
    const proto::CredentialIndexMode mode)
{
    // NOTE version may be incorrect until LoadCredentialIndex is called

    return new identity::implementation::Nym(
        api, nymID, mode, identity::Nym::DefaultVersion);
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
    const api::Core& api,
    const crypto::key::Asymmetric& signingKey,
    const Data& iv,
    const proto::HashType hashType,
    OTPasswordData& output);

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
    const api::Core& api,
    const identifier::Nym& nymID,
    const proto::CredentialIndexMode mode,
    const VersionNumber version)
    : api_(api)
    , version_(version)
    , index_(0)
    , alias_()
    , revision_(1)
    , mode_(mode)
    , m_strVersion(String::Factory(NYMFILE_VERSION))
    , m_strDescription(String::Factory())
    , m_nymID(nymID)
    , source_(nullptr)
    , contact_data_(nullptr)
    , m_mapCredentialSets()
    , m_mapRevokedSets()
    , m_listRevokedIDs()
{
    OT_ASSERT(0 != version_);
}

Nym::Nym(const api::Core& api, const NymParameters& nymParameters)
    : Nym(api, api.Factory().NymID(), proto::CREDINDEX_PRIVATE, DefaultVersion)
{
    NymParameters revisedParameters = nymParameters;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    revisedParameters.SetCredset(index_++);
    Bip32Index nymIndex = 0;
    std::string fingerprint = nymParameters.Seed();
    auto seed = api_.Seeds().Seed(fingerprint, nymIndex);

    OT_ASSERT(seed);

    const bool defaultIndex = nymParameters.UseAutoIndex();

    if (!defaultIndex) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Re-creating nym at specified path.")
            .Flush();

        nymIndex = nymParameters.Nym();
    }

    const std::int32_t newIndex = nymIndex + 1;
    api_.Seeds().UpdateIndex(fingerprint, newIndex);
    revisedParameters.SetEntropy(*seed);
    revisedParameters.SetSeed(fingerprint);
    revisedParameters.SetNym(nymIndex);
#endif
    auto* pNewCredentialSet =
        opentxs::Factory::Authority(api_, revisedParameters, version_);

    OT_ASSERT(nullptr != pNewCredentialSet);

    source_ = std::make_shared<NymIDSource>(pNewCredentialSet->Source());
    const_cast<OTNymID&>(m_nymID) = source_->NymID();
    SetDescription(source_->Description());
    m_mapCredentialSets.emplace(
        pNewCredentialSet->GetMasterCredID(), pNewCredentialSet);
}

bool Nym::add_contact_credential(
    const eLock& lock,
    const proto::ContactData& data)
{
    OT_ASSERT(verify_lock(lock));

    bool added = false;

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            if (it.second->hasCapability(NymCapability::SIGN_CHILDCRED)) {
                added = it.second->AddContactCredential(data);

                break;
            }
        }
    }

    return added;
}

bool Nym::add_verification_credential(
    const eLock& lock,
    const proto::VerificationSet& data)
{
    OT_ASSERT(verify_lock(lock));

    bool added = false;

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            if (it.second->hasCapability(NymCapability::SIGN_CHILDCRED)) {
                added = it.second->AddVerificationCredential(data);

                break;
            }
        }
    }

    return added;
}

std::string Nym::AddChildKeyCredential(
    const Identifier& masterID,
    const NymParameters& nymParameters)
{
    eLock lock(shared_lock_);

    std::string output;
    std::string master = masterID.str();
    auto it = m_mapCredentialSets.find(master);
    const bool noMaster = (it == m_mapCredentialSets.end());

    if (noMaster) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Master ID not found.").Flush();

        return output;
    }

    if (it->second) {
        output = it->second->AddChildKeyCredential(nymParameters);
    }

    return output;
}

bool Nym::AddClaim(const Claim& claim)
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(contact_data_->AddItem(claim)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

bool Nym::AddContract(
    const identifier::UnitDefinition& instrumentDefinitionID,
    const proto::ContactItemType currency,
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

    return set_contact_data(lock, contact_data_->Serialize());
}

bool Nym::AddEmail(
    const std::string& value,
    const bool primary,
    const bool active)
{
    if (value.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(
        new ContactData(contact_data_->AddEmail(value, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
bool Nym::AddPaymentCode(
    const opentxs::PaymentCode& code,
    const proto::ContactItemType currency,
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

    return set_contact_data(lock, contact_data_->Serialize());
}
#endif

bool Nym::AddPhoneNumber(
    const std::string& value,
    const bool primary,
    const bool active)
{
    if (value.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(
        new ContactData(contact_data_->AddPhoneNumber(value, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

bool Nym::AddPreferredOTServer(const Identifier& id, const bool primary)
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_)

    contact_data_.reset(
        new ContactData(contact_data_->AddPreferredOTServer(id, primary)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

bool Nym::AddSocialMediaProfile(
    const std::string& value,
    const proto::ContactItemType type,
    const bool primary,
    const bool active)
{
    if (value.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(
        contact_data_->AddSocialMediaProfile(value, type, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

std::string Nym::Alias() const { return alias_; }

const Nym::Serialized Nym::asPublicNym() const
{
    return SerializeCredentialIndex(Mode::Full);
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

void Nym::clear_credentials(const eLock& lock)
{
    OT_ASSERT(verify_lock(lock));

    m_listRevokedIDs.clear();

    while (!m_mapCredentialSets.empty()) {
        identity::Authority* pCredential = m_mapCredentialSets.begin()->second;
        m_mapCredentialSets.erase(m_mapCredentialSets.begin());
        delete pCredential;
        pCredential = nullptr;
    }

    while (!m_mapRevokedSets.empty()) {
        identity::Authority* pCredential = m_mapRevokedSets.begin()->second;
        m_mapRevokedSets.erase(m_mapRevokedSets.begin());
        delete pCredential;
        pCredential = nullptr;
    }
}

void Nym::ClearCredentials()
{
    eLock lock(shared_lock_);

    clear_credentials(lock);
}

bool Nym::CompareID(const identity::Nym& rhs) const
{
    sLock lock(shared_lock_);

    return rhs.CompareID(m_nymID);
}

bool Nym::CompareID(const identifier::Nym& rhs) const
{
    sLock lock(shared_lock_);

    return m_nymID == rhs;
}

VersionNumber Nym::ContactCredentialVersion() const
{
    // TODO support multiple authorities
    OT_ASSERT(0 < m_mapCredentialSets.size())

    return m_mapCredentialSets.cbegin()->second->ContactCredentialVersion();
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

bool Nym::DeleteClaim(const Identifier& id)
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(contact_data_->Delete(id)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

std::string Nym::EmailAddresses(bool active) const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->EmailAddresses(active);
}

void Nym::GetIdentifier(identifier::Nym& theIdentifier) const
{
    sLock lock(shared_lock_);

    theIdentifier.Assign(m_nymID);
}

// sets argument based on internal member
void Nym::GetIdentifier(String& theIdentifier) const
{
    sLock lock(shared_lock_);

    m_nymID->GetString(theIdentifier);
}

template <typename T>
const crypto::key::Asymmetric& Nym::get_private_auth_key(
    const T& lock,
    proto::AsymmetricKeyType keytype) const
{
    OT_ASSERT(!m_mapCredentialSets.empty());

    OT_ASSERT(verify_lock(lock));
    const identity::Authority* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
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

    OT_ASSERT(!m_mapCredentialSets.empty());

    const identity::Authority* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
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
    OT_ASSERT(!m_mapCredentialSets.empty());

    OT_ASSERT(verify_lock(lock));

    const identity::Authority* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
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
    OT_ASSERT(!m_mapCredentialSets.empty());

    OT_ASSERT(verify_lock(lock));

    const identity::Authority* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
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

    OT_ASSERT(!m_mapCredentialSets.empty());

    const identity::Authority* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
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

    OT_ASSERT(!m_mapCredentialSets.empty());

    const identity::Authority* pCredential = nullptr;
    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPublicEncrKey(
        keytype,
        &m_listRevokedIDs);  // success
}

// This is being called by:
// Contract::VerifySignature(const Nym& theNym, const Signature&
// theSignature, OTPasswordData * pPWData=nullptr)
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

    for (const auto& it : m_mapCredentialSets) {
        const identity::Authority* pCredential = it.second;
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

    for (auto& it : m_mapCredentialSets) {
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

    const auto nymID{m_nymID->str()};
    const auto dataVersion = ContactDataVersion();
    contact_data_.reset(new opentxs::ContactData(
        nymID, dataVersion, dataVersion, ContactData::SectionMap()));

    OT_ASSERT(contact_data_);

    std::unique_ptr<proto::ContactData> serialized{nullptr};

    for (auto& it : m_mapCredentialSets) {
        OT_ASSERT(nullptr != it.second);

        const auto& credSet = *it.second;
        credSet.GetContactData(serialized);

        if (serialized) {
            OT_ASSERT(
                proto::Validate(*serialized, VERBOSE, proto::CLAIMS_NORMAL));

            opentxs::ContactData claimCred(nymID, dataVersion, *serialized);
            contact_data_.reset(
                new opentxs::ContactData(*contact_data_ + claimCred));
            serialized.reset();
        }
    }

    OT_ASSERT(contact_data_)
}

bool Nym::load_credential_index(const eLock& lock, const Serialized& index)
{
    if (!proto::Validate<proto::CredentialIndex>(index, VERBOSE)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to load invalid serialized"
                                           " credential index.")
            .Flush();

        return false;
    }

    OT_ASSERT(verify_lock(lock));

    const auto nymID = api_.Factory().NymID(index.nymid());

    if (m_nymID != nymID) { return false; }

    version_ = index.version();
    index_ = index.index();
    revision_.store(index.revision());
    mode_ = index.mode();
    source_ = std::make_shared<NymIDSource>(api_.Factory(), index.source());
    proto::KeyMode mode = (proto::CREDINDEX_PRIVATE == mode_)
                              ? proto::KEYMODE_PRIVATE
                              : proto::KEYMODE_PUBLIC;
    contact_data_.reset();
    m_mapCredentialSets.clear();

    for (auto& it : index.activecredentials()) {
        auto* newSet = opentxs::Factory::Authority(api_, mode, it);

        if (nullptr != newSet) {
            m_mapCredentialSets.emplace(
                std::make_pair(newSet->GetMasterCredID(), newSet));
        }
    }

    m_mapRevokedSets.clear();

    for (auto& it : index.revokedcredentials()) {
        auto* newSet = opentxs::Factory::Authority(api_, mode, it);

        if (nullptr != newSet) {
            m_mapRevokedSets.emplace(
                std::make_pair(newSet->GetMasterCredID(), newSet));
        }
    }

    return true;
}

bool Nym::LoadCredentialIndex(const Serialized& index)
{
    eLock lock(shared_lock_);

    return load_credential_index(lock, index);
}

bool Nym::Lock(
    const OTPassword& password,
    crypto::key::Symmetric& key,
    proto::Ciphertext& output) const
{
    static const proto::HashType hashType{proto::HASHTYPE_BLAKE2B256};
    static const proto::SymmetricMode mode{proto::SMODE_CHACHA20POLY1305};

    if (false == HasCapability(NymCapability::ENCRYPT_MESSAGE)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No private key available")
            .Flush();

        return false;
    }

    OTPasswordData keyPassword{""};
    keyPassword.SetOverride(password);

    if (false == key.Unlock(keyPassword)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Supplied password does not unlock the supplied key")
            .Flush();

        return false;
    }

    OTPassword iv;
    const auto ivSize = api_.Crypto().Symmetric().IvSize(mode);

    if (0 == ivSize) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid mode").Flush();

        return false;
    }

    if (static_cast<std::int32_t>(ivSize) != iv.randomizeMemory(ivSize)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to allocate iv").Flush();

        return false;
    }

    const auto ivData = Data::Factory(iv.getMemory(), iv.getMemorySize());
    OTPasswordData sessionkeyPassword{""};
    const auto haveSessionPassword = session_key_from_iv(
        api_, GetPrivateEncrKey(), ivData, hashType, sessionkeyPassword);

    if (false == haveSessionPassword) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to calculate session key password")
            .Flush();

        return false;
    }

    auto sessionKey = api_.Crypto().Symmetric().Key(sessionkeyPassword, mode);

    if (false == bool(sessionKey.get())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to generate session key")
            .Flush();

        return false;
    }

    return sessionKey->Encrypt(
        password, ivData, sessionkeyPassword, output, true, mode);
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

bool Nym::Open(
    const proto::SessionKey& input,
    crypto::key::Symmetric& key,
    OTPassword& password) const
{
    static const proto::SymmetricMode mode{proto::SMODE_CHACHA20POLY1305};
    const auto& serializedDHPublic = input.dh();
    const auto& ciphertext = input.key();

    if (false == HasCapability(NymCapability::ENCRYPT_MESSAGE)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No private key available")
            .Flush();

        return false;
    }

    auto sessionKey = api_.Crypto().Symmetric().Key(ciphertext.key(), mode);

    if (false == bool(sessionKey.get())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to instantiate session key")
            .Flush();

        return false;
    }

    OTPasswordData sessionkeyPassword{""};
    auto dhPublic = api_.Factory().AsymmetricKey(serializedDHPublic);
    const auto& encryptKey = GetPrivateEncrKey();
    const auto opened =
        encryptKey.Open(dhPublic, sessionKey, sessionkeyPassword);

    if (false == opened) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Failed to decrypt session key")
            .Flush();

        return false;
    }
    const auto output =
        sessionKey->Decrypt(ciphertext, sessionkeyPassword, password);

    if (false == output) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt key password")
            .Flush();

        return false;
    }

    OTPasswordData keyPassword{""};
    keyPassword.SetOverride(password);

    if (false == key.Unlock(keyPassword)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Decrypted password does not unlock the supplied key")
            .Flush();

        return false;
    }

    return true;
}

bool Nym::Path(proto::HDPath& output) const
{
    sLock lock(shared_lock_);

    for (const auto& it : m_mapCredentialSets) {
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
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    if (!source_) { return ""; }

    if (proto::SOURCETYPE_BIP47 != source_->Type()) { return ""; }

    auto serialized = source_->Serialize();

    if (!serialized) { return ""; }

    auto paymentCode = api_.Factory().PaymentCode(serialized->paymentcode());

    return paymentCode->asBase58();

#else
    return "";
#endif
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

    for (auto& it : m_mapCredentialSets) {
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

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            it.second->RevokeVerificationCredentials(revokedIDs);
        }
    }

    for (auto& it : revokedIDs) { m_listRevokedIDs.push_back(it); }
}

bool Nym::Seal(
    const OTPassword& password,
    crypto::key::Symmetric& key,
    proto::SessionKey& output) const
{
    static const proto::SymmetricMode mode{proto::SMODE_CHACHA20POLY1305};
    OTPasswordData keyPassword{""};
    keyPassword.SetOverride(password);

    if (false == key.Unlock(keyPassword)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Supplied password does not unlock the supplied key")
            .Flush();

        return false;
    }

    OTPassword randomPassword{};

    if (32 != randomPassword.randomizeMemory(32)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed generate random password")
            .Flush();

        return false;
    }

    OTPasswordData sessionKeyPassword{""};
    sessionKeyPassword.SetOverride(randomPassword);
    auto sessionKey = api_.Crypto().Symmetric().Key(sessionKeyPassword, mode);

    if (false == bool(sessionKey.get())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to generate session key")
            .Flush();

        return false;
    }

    auto dhPublic = crypto::key::Asymmetric::Factory();
    const auto& encryptKey = GetPublicEncrKey();
    const auto sealed =
        encryptKey.Seal(api_, dhPublic, sessionKey, sessionKeyPassword);

    if (false == sealed) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to set session key password")
            .Flush();

        return false;
    }

    const auto serializedDH = dhPublic->Serialize();

    if (false == bool(serializedDH)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to serialized DH public key")
            .Flush();

        return false;
    }

    auto& dh = *output.mutable_dh();
    dh = *serializedDH;
    output.set_version(akey_to_session_key_version_.at(dh.version()));

    return sessionKey->Encrypt(
        password,
        Data::Factory(),
        sessionKeyPassword,
        *output.mutable_key(),
        true,
        mode);
}

Nym::Serialized Nym::SerializeCredentialIndex(const Mode mode) const
{
    sLock lock(shared_lock_);
    Serialized index;
    index.set_version(version_);
    auto nymID = String::Factory(m_nymID);
    index.set_nymid(nymID->Get());

    if (Mode::Abbreviated == mode) {
        index.set_mode(mode_);

        if (proto::CREDINDEX_PRIVATE == mode_) { index.set_index(index_); }
    } else {
        index.set_mode(proto::CREDINDEX_PUBLIC);
    }

    index.set_revision(revision_.load());
    *(index.mutable_source()) = *(source_->Serialize());

    for (auto& it : m_mapCredentialSets) {
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
    if (source_) {

        TagPtr pTag(new Tag("nymIDSource", source_->asString()->Get()));

        if (m_strDescription->Exists()) {
            auto ascDescription = Armored::Factory();
            ascDescription->SetString(
                m_strDescription,
                false);  // bLineBreaks=true by default.

            pTag->add_attribute("Description", ascDescription->Get());
        }
        parent.add_tag(pTag);
    }
}

bool session_key_from_iv(
    const api::Core& api,
    const crypto::key::Asymmetric& signingKey,
    const Data& iv,
    const proto::HashType hashType,
    OTPasswordData& output)
{
    const auto& engine = signingKey.engine();
    const auto hash = signingKey.CalculateHash(hashType, "");

    if (hash->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to hash private key")
            .Flush();

        return false;
    }

    OTPassword salt{hash->data(), hash->size()};
    OTPassword hmac{};
    const auto salted = api.Crypto().Hash().HMAC(hashType, salt, iv, hmac);

    if (false == salted) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate hmac")
            .Flush();

        return false;
    }

    auto signature = Data::Factory();
    const auto haveSig = engine.Sign(
        Data::Factory(hmac.getMemory(), hmac.getMemorySize()),
        signingKey,
        hashType,
        signature);

    if (false == haveSig) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to sign IV").Flush();

        return false;
    }

    OTPassword sessionPassword{};
    const auto hashed =
        api.Crypto().Hash().HMAC(hashType, salt, signature, sessionPassword);

    if (false == hashed) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to hash signature")
            .Flush();

        return false;
    }

    if (false == output.SetOverride(sessionPassword)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed set session password")
            .Flush();

        return false;
    }

    return true;
}

bool Nym::set_contact_data(const eLock& lock, const proto::ContactData& data)
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

    if (false == proto::Validate(data, VERBOSE, false)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid contact data.").Flush();

        return false;
    }

    revoke_contact_credentials(lock);

    if (add_contact_credential(lock, data)) {

        return update_nym(lock, version);
    }

    return false;
}

void Nym::SetAlias(const std::string& alias)
{
    eLock lock(shared_lock_);

    alias_ = alias;
    revision_++;
}

bool Nym::SetCommonName(const std::string& name)
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(contact_data_->SetCommonName(name)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

bool Nym::SetContactData(const proto::ContactData& data)
{
    eLock lock(shared_lock_);
    contact_data_.reset(
        new ContactData(m_nymID->str(), ContactDataVersion(), data));

    return set_contact_data(lock, data);
}

bool Nym::SetScope(
    const proto::ContactItemType type,
    const std::string& name,
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

    return set_contact_data(lock, contact_data_->Serialize());
}

bool Nym::SetVerificationSet(const proto::VerificationSet& data)
{
    eLock lock(shared_lock_);

    if (false == has_capability(lock, NymCapability::SIGN_CHILDCRED)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": This nym can not be modified.")
            .Flush();

        return false;
    }

    revoke_verification_credentials(lock);

    if (add_verification_credential(lock, data)) {

        return update_nym(lock, version_);
    }

    return false;
}

bool Nym::Sign(
    const GetPreimage input,
    const proto::SignatureRole role,
    const proto::HashType hash,
    proto::Signature& signature,
    const OTPasswordData* pPWData) const
{
    sLock lock(shared_lock_);

    bool haveSig = false;

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            bool success = it.second->Sign(
                input, role, signature, proto::KEYROLE_SIGN, pPWData, hash);

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

std::unique_ptr<OTPassword> Nym::TransportKey(Data& pubkey) const
{
    bool found{false};
    auto privateKey = std::make_unique<OTPassword>();

    OT_ASSERT(privateKey);

    sLock lock(shared_lock_);

    for (auto& it : m_mapCredentialSets) {
        OT_ASSERT(nullptr != it.second);

        if (nullptr != it.second) {
            const identity::Authority& credSet = *it.second;
            found = credSet.TransportKey(pubkey, *privateKey);

            if (found) { break; }
        }
    }

    OT_ASSERT(found);

    return privateKey;
}

bool Nym::Unlock(
    const proto::Ciphertext& input,
    crypto::key::Symmetric& key,
    OTPassword& password) const
{
    static const proto::HashType hashType{proto::HASHTYPE_BLAKE2B256};
    static const proto::SymmetricMode mode{proto::SMODE_CHACHA20POLY1305};

    if (false == HasCapability(NymCapability::ENCRYPT_MESSAGE)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No private key available")
            .Flush();

        return false;
    }

    const auto iv = Data::Factory(input.iv().data(), input.iv().size());
    OTPasswordData sessionkeyPassword{""};
    const auto haveSessionPassword = session_key_from_iv(
        api_, GetPrivateEncrKey(), iv, hashType, sessionkeyPassword);

    if (false == haveSessionPassword) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to calculate session key password")
            .Flush();

        return false;
    }

    auto sessionKey = api_.Crypto().Symmetric().Key(input.key(), mode);

    if (false == bool(sessionKey.get())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to instantiate session key")
            .Flush();

        return false;
    }

    const auto output =
        sessionKey->Decrypt(input, sessionkeyPassword, password);

    if (false == output) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt key password")
            .Flush();

        return false;
    }

    OTPasswordData keyPassword{""};
    keyPassword.SetOverride(password);

    if (false == key.Unlock(keyPassword)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Decrypted password does not unlock the supplied key")
            .Flush();

        return false;
    }

    return true;
}

bool Nym::update_nym(const eLock& lock, const std::int32_t version)
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

VersionNumber Nym::VerificationCredentialVersion() const
{
    // TODO support multiple authorities
    OT_ASSERT(0 < m_mapCredentialSets.size())

    return m_mapCredentialSets.cbegin()
        ->second->VerificationCredentialVersion();
}

std::unique_ptr<proto::VerificationSet> Nym::VerificationSet() const
{
    std::unique_ptr<proto::VerificationSet> verificationSet;

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            it.second->GetVerificationSet(verificationSet);
        }
    }

    return verificationSet;
}

bool Nym::Verify(const Data& plaintext, const proto::Signature& sig) const
{
    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            if (it.second->Verify(plaintext, sig)) { return true; }
        }
    }

    return false;
}

bool Nym::verify_pseudonym(const eLock& lock) const
{
    // If there are credentials, then we verify the Nym via his credentials.
    if (!m_mapCredentialSets.empty()) {
        // Verify Nym by his own credentials.
        for (const auto& it : m_mapCredentialSets) {
            const identity::Authority* pCredential = it.second;
            OT_ASSERT(nullptr != pCredential);

            const auto theCredentialNymID =
                api_.Factory().NymID(pCredential->GetNymID());

            if (m_nymID != theCredentialNymID) {
                LogNormal(OT_METHOD)(__FUNCTION__)(": Credential NymID (")(
                    pCredential->GetNymID())(") doesn't match actual NymID: ")(
                    m_nymID->str())(".")
                    .Flush();
                return false;
            }

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

    for (auto& it : m_mapCredentialSets) {
        if (!it.second->WriteCredentials()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save credentials.")
                .Flush();

            return false;
        }
    }

    return true;
}

Nym::~Nym() { ClearCredentials(); }
}  // namespace opentxs::identity::implementation
