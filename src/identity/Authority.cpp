// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/NymIDSource.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Proto.tpp"

#include "internal/identity/credential/Credential.hpp"
#include "internal/identity/Identity.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "Authority.hpp"

#define OT_METHOD "opentxs::identity::implementation::Authority::"

namespace opentxs
{
template <typename Range, typename Function>
Function for_each(Range& range, Function f)
{
    return std::for_each(std::begin(range), std::end(range), f);
}

identity::internal::Authority* Factory::Authority(
    const api::Core& api,
    const proto::KeyMode mode,
    const proto::CredentialSet& serialized,
    const opentxs::PasswordPrompt& reason)
{
    return new identity::implementation::Authority(
        api, mode, serialized, reason);
}

identity::internal::Authority* Factory::Authority(
    const api::Core& api,
    const NymParameters& nymParameters,
    const VersionNumber nymVersion,
    const opentxs::PasswordPrompt& reason)
{
    return new identity::implementation::Authority(
        api, nymParameters, nymVersion, reason);
}
}  // namespace opentxs

namespace opentxs::identity::implementation
{
const VersionConversionMap Authority::authority_to_contact_{
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 4},
    {5, 5},
    {6, 6},
};
const VersionConversionMap Authority::authority_to_primary_{
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 4},
    {5, 5},
    {6, 6},
};
const VersionConversionMap Authority::authority_to_secondary_{
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 4},
    {5, 5},
    {6, 6},
};
const VersionConversionMap Authority::authority_to_verification_{
    {1, 1},
    {2, 1},
    {3, 1},
    {4, 1},
    {5, 1},
    {6, 1},
};
const VersionConversionMap Authority::nym_to_authority_{
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 4},
    {5, 5},
    {6, 6},
};

Authority::Authority(
    const api::Core& api,
    const VersionNumber version,
    const Bip32Index index,
    const proto::KeyMode mode,
    const std::string& nymID) noexcept
    : api_(api)
    , master_(nullptr)
    , key_credentials_()
    , contact_credentials_()
    , verification_credentials_()
    , m_mapRevokedCredentials()
    , m_strNymID(nymID)
    , nym_id_source_(nullptr)
    , m_pImportPassword(nullptr)
    , version_(version)
    , index_(index)
    , mode_(mode)
{
    OT_ASSERT(0 != version);
}

Authority::Authority(
    const api::Core& api,
    const proto::KeyMode mode,
    const Serialized& serialized,
    const opentxs::PasswordPrompt& reason) noexcept
    : Authority(
          api,
          serialized.version(),
          serialized.index(),
          mode,
          serialized.nymid())
{
    if (proto::CREDSETMODE_INDEX == serialized.mode()) {
        Load_Master(
            String::Factory(serialized.nymid()),
            String::Factory(serialized.masterid()),
            reason);

        for (auto& it : serialized.activechildids()) {
            LoadChildKeyCredential(String::Factory(it), reason);
        }
    } else {
        std::unique_ptr<credential::internal::Primary> master{
            opentxs::Factory::Credential<credential::internal::Primary>(
                api_,
                *this,
                serialized.mastercredential(),
                mode,
                proto::CREDROLE_MASTERKEY,
                reason)};

        if (master) { master_.reset(master.release()); }

        for (auto& it : serialized.activechildren()) {
            LoadChildKeyCredential(it, reason);
        }
    }
}

Authority::Authority(
    const api::Core& api,
    const NymParameters& nymParameters,
    VersionNumber nymVersion,
    const opentxs::PasswordPrompt& reason) noexcept
    : Authority(api, nym_to_authority_.at(nymVersion))
{
    CreateMasterCredential(nymParameters, reason);

    OT_ASSERT(master_);

    NymParameters revisedParameters{nymParameters};
    bool haveChildCredential{false};

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    if (!haveChildCredential) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Creating an ed25519 child key credential.")
            .Flush();
        revisedParameters.setNymParameterType(NymParameterType::ed25519);
        haveChildCredential =
            !AddChildKeyCredential(revisedParameters, reason).empty();
    }
#endif

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    if (!haveChildCredential) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Creating an secp256k1 child key credential.")
            .Flush();
        revisedParameters.setNymParameterType(NymParameterType::secp256k1);
        haveChildCredential =
            !AddChildKeyCredential(revisedParameters, reason).empty();
    }
#endif

#if OT_CRYPTO_SUPPORTED_KEY_RSA
    if (!haveChildCredential) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Creating an RSA child key credential.")
            .Flush();
        revisedParameters.setNymParameterType(NymParameterType::rsa);
        haveChildCredential =
            !AddChildKeyCredential(revisedParameters, reason).empty();
    }
#endif

    OT_ASSERT(haveChildCredential);
}

std::string Authority::AddChildKeyCredential(
    const NymParameters& nymParameters,
    const opentxs::PasswordPrompt& reason)
{
    auto output = api_.Factory().Identifier();
    NymParameters revisedParameters{nymParameters};
#if OT_CRYPTO_SUPPORTED_KEY_HD
    revisedParameters.SetCredIndex(index_++);
#endif
    std::unique_ptr<credential::internal::Secondary> child{
        opentxs::Factory::Credential<credential::internal::Secondary>(
            api_,
            *this,
            authority_to_secondary_.at(version_),
            revisedParameters,
            proto::CREDROLE_CHILDKEY,
            reason)};

    if (!child) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to instantiate child key credential.")
            .Flush();

        return output->str();
    }

    output->Assign(child->ID());
    key_credentials_.emplace(output, child.release());

    return output->str();
}

bool Authority::AddContactCredential(
    const proto::ContactData& contactData,
    const opentxs::PasswordPrompt& reason)
{
    LogDetail(OT_METHOD)(__FUNCTION__)(": Adding a contact credential.")
        .Flush();

    if (!master_) { return false; }

    NymParameters nymParameters;
    nymParameters.SetContactData(contactData);
    std::unique_ptr<credential::internal::Contact> credential{
        opentxs::Factory::Credential<credential::internal::Contact>(
            api_,
            *this,
            authority_to_contact_.at(version_),
            nymParameters,
            proto::CREDROLE_CONTACT,
            reason)};

    if (false == bool(credential)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct credential")
            .Flush();

        return false;
    }

    auto id{credential->ID()};
    contact_credentials_.emplace(std::move(id), credential.release());
    const auto version =
        proto::RequiredCredentialSetVersion(contactData.version(), version_);

    if (version > version_) { version_ = version; }

    return true;
}

bool Authority::AddVerificationCredential(
    const proto::VerificationSet& verificationSet,
    const opentxs::PasswordPrompt& reason)
{
    LogDetail(OT_METHOD)(__FUNCTION__)(": Adding a verification credential.")
        .Flush();

    if (!master_) { return false; }

    NymParameters nymParameters;
    nymParameters.SetVerificationSet(verificationSet);
    std::unique_ptr<credential::internal::Verification> credential{
        opentxs::Factory::Credential<credential::internal::Verification>(
            api_,
            *this,
            authority_to_verification_.at(version_),
            nymParameters,
            proto::CREDROLE_VERIFY,
            reason)};

    if (false == bool(credential)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct credential")
            .Flush();

        return false;
    }

    auto id{credential->ID()};
    verification_credentials_.emplace(std::move(id), credential.release());

    return true;
}

bool Authority::CreateMasterCredential(
    const NymParameters& nymParameters,
    const opentxs::PasswordPrompt& reason)
{
#if OT_CRYPTO_SUPPORTED_KEY_HD
    if (0 != index_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": The master credential must be the first "
            "credential created.")
            .Flush();

        return false;
    }

    if (0 != nymParameters.CredIndex()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Invalid CredIndex in nymParameters.")
            .Flush();

        return false;
    }
#endif

    if (master_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": The master credential already exists.")
            .Flush();

        return false;
    }

    master_.reset(opentxs::Factory::Credential<credential::internal::Primary>(
        api_,
        *this,
        authority_to_primary_.at(version_),
        nymParameters,
        proto::CREDROLE_MASTERKEY,
        reason));

    if (master_) {
        index_++;

        return true;
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Failed to instantiate master credential.")
        .Flush();

    return false;
}

const crypto::key::Keypair& Authority::get_keypair(
    const proto::AsymmetricKeyType type,
    const proto::KeyRole role,
    const String::List* plistRevokedIDs) const
{
    for (const auto& [id, pCredential] : key_credentials_) {
        OT_ASSERT(pCredential);

        const auto& credential = *pCredential;

        if (is_revoked(id->str(), plistRevokedIDs)) { continue; }

        try {
            return credential.GetKeypair(type, role);
        } catch (...) {
            continue;
        }
    }

    throw std::out_of_range("no matching keypair");
}

const credential::Base* Authority::get_secondary_credential(
    const std::string& strSubID,
    const String::List* plistRevokedIDs) const
{
    if (is_revoked(strSubID, plistRevokedIDs)) { return nullptr; }

    const auto it = key_credentials_.find(api_.Factory().Identifier(strSubID));

    if (key_credentials_.end() == it) { return nullptr; }

    return it->second.get();
}

bool Authority::GetContactData(
    std::unique_ptr<proto::ContactData>& contactData) const
{
    if (contact_credentials_.empty()) { return false; }

    // TODO handle more than one contact credential

    contact_credentials_.begin()->second->GetContactData(contactData);

    return true;
}

const std::string Authority::GetMasterCredID() const
{
    if (master_) { return master_->ID()->str(); }

    return "";
}

const crypto::key::Keypair& Authority::GetAuthKeypair(
    proto::AsymmetricKeyType keytype,
    const String::List* plistRevokedIDs) const
{
    return get_keypair(keytype, proto::KEYROLE_AUTH, plistRevokedIDs);
}

const crypto::key::Keypair& Authority::GetEncrKeypair(
    proto::AsymmetricKeyType keytype,
    const String::List* plistRevokedIDs) const
{
    return get_keypair(keytype, proto::KEYROLE_ENCRYPT, plistRevokedIDs);
}

const crypto::key::Asymmetric& Authority::GetPublicAuthKey(
    proto::AsymmetricKeyType keytype,
    const String::List* plistRevokedIDs) const
{
    return GetAuthKeypair(keytype, plistRevokedIDs).GetPublicKey();
}

const crypto::key::Asymmetric& Authority::GetPublicEncrKey(
    proto::AsymmetricKeyType keytype,
    const String::List* plistRevokedIDs) const
{
    return GetEncrKeypair(keytype, plistRevokedIDs).GetPublicKey();
}

std::int32_t Authority::GetPublicKeysBySignature(
    crypto::key::Keypair::Keys& listOutput,
    const Signature& theSignature,
    char cKeyType) const
{
    std::int32_t output{0};
    const auto getKeys = [&](const auto& item) -> void {
        output += item.second->GetPublicKeysBySignature(
            listOutput, theSignature, cKeyType);
    };

    for_each(key_credentials_, getKeys);

    return output;
}

const crypto::key::Asymmetric& Authority::GetPublicSignKey(
    proto::AsymmetricKeyType keytype,
    const String::List* plistRevokedIDs) const
{
    return GetSignKeypair(keytype, plistRevokedIDs).GetPublicKey();
}

const crypto::key::Asymmetric& Authority::GetPrivateAuthKey(
    proto::AsymmetricKeyType keytype,
    const String::List* plistRevokedIDs) const
{
    return GetAuthKeypair(keytype, plistRevokedIDs).GetPrivateKey();
}

const crypto::key::Asymmetric& Authority::GetPrivateEncrKey(
    proto::AsymmetricKeyType keytype,
    const String::List* plistRevokedIDs) const
{
    return GetEncrKeypair(keytype, plistRevokedIDs).GetPrivateKey();
}

const crypto::key::Asymmetric& Authority::GetPrivateSignKey(
    proto::AsymmetricKeyType keytype,
    const String::List* plistRevokedIDs) const
{
    return GetSignKeypair(keytype, plistRevokedIDs).GetPrivateKey();
}

const crypto::key::Keypair& Authority::GetSignKeypair(
    proto::AsymmetricKeyType keytype,
    const String::List* plistRevokedIDs) const
{
    return get_keypair(keytype, proto::KEYROLE_SIGN, plistRevokedIDs);
}

bool Authority::GetVerificationSet(
    std::unique_ptr<proto::VerificationSet>& output) const
{
    if (verification_credentials_.empty()) { return false; }

    // TODO handle more than one verification credential

    verification_credentials_.begin()->second->GetVerificationSet(output);

    return true;
}

bool Authority::hasCapability(const NymCapability& capability) const
{
    switch (capability) {
        case (NymCapability::SIGN_CHILDCRED): {
            if (master_) { return master_->hasCapability(capability); }
        } break;
        case (NymCapability::SIGN_MESSAGE):
        case (NymCapability::ENCRYPT_MESSAGE):
        case (NymCapability::AUTHENTICATE_CONNECTION):
        default: {
            for (const auto& [id, pCredential] : key_credentials_) {
                OT_ASSERT(pCredential);

                const auto& credential = *pCredential;

                if (credential.hasCapability(capability)) { return true; }
            }
        }
    }

    return false;
}

bool Authority::is_revoked(
    const std::string& id,
    const String::List* plistRevokedIDs)
{
    if (nullptr == plistRevokedIDs) { return false; }

    return std::find(plistRevokedIDs->begin(), plistRevokedIDs->end(), id) !=
           plistRevokedIDs->end();
}

bool Authority::LoadChildKeyCredential(
    const String& strSubID,
    const opentxs::PasswordPrompt& reason)
{

    OT_ASSERT(!GetNymID().empty());

    std::shared_ptr<proto::Credential> child;
    bool loaded = api_.Wallet().LoadCredential(strSubID.Get(), child);

    if (!loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: Key Credential ")(
            strSubID)(" doesn't exist for Nym ")(GetNymID())(".")
            .Flush();
        return false;
    }

    return LoadChildKeyCredential(*child, reason);
}

bool Authority::LoadChildKeyCredential(
    const proto::Credential& serializedCred,
    const opentxs::PasswordPrompt& reason)
{
    bool validProto = proto::Validate<proto::Credential>(
        serializedCred, VERBOSE, mode_, proto::CREDROLE_ERROR, true);

    if (!validProto) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Invalid serialized child key credential.")
            .Flush();
        return false;
    }

    if (proto::CREDROLE_MASTERKEY == serializedCred.role()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unexpected master credential.")
            .Flush();

        return false;
    }

    switch (serializedCred.role()) {
        case proto::CREDROLE_CHILDKEY: {
            std::unique_ptr<credential::internal::Secondary> child{
                opentxs::Factory::Credential<credential::internal::Secondary>(
                    api_,
                    *this,
                    serializedCred,
                    mode_,
                    proto::CREDROLE_CHILDKEY,
                    reason)};

            if (false == bool(child)) { return false; }

            auto id{child->ID()};

            key_credentials_.emplace(std::move(id), child.release());
        } break;
        case proto::CREDROLE_CONTACT: {
            std::unique_ptr<credential::internal::Contact> child{
                opentxs::Factory::Credential<credential::internal::Contact>(
                    api_,
                    *this,
                    serializedCred,
                    mode_,
                    proto::CREDROLE_CONTACT,
                    reason)};

            if (false == bool(child)) { return false; }

            auto id{child->ID()};

            contact_credentials_.emplace(std::move(id), child.release());
        } break;
        case proto::CREDROLE_VERIFY: {
            std::unique_ptr<credential::internal::Verification> child{
                opentxs::Factory::Credential<
                    credential::internal::Verification>(
                    api_,
                    *this,
                    serializedCred,
                    mode_,
                    proto::CREDROLE_VERIFY,
                    reason)};

            if (false == bool(child)) { return false; }

            auto id{child->ID()};

            verification_credentials_.emplace(std::move(id), child.release());
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid credential type")
                .Flush();

            return false;
        }
    }

    return true;
}

bool Authority::Load_Master(
    const String& strNymID,
    const String& strMasterCredID,
    const opentxs::PasswordPrompt& reason)
{
    std::shared_ptr<proto::Credential> serialized;
    bool loaded =
        api_.Wallet().LoadCredential(strMasterCredID.Get(), serialized);

    if (!loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: Master Credential ")(
            strMasterCredID)(" doesn't exist for Nym ")(strNymID)(".")
            .Flush();

        return false;
    }

    std::unique_ptr<credential::internal::Primary> master{
        opentxs::Factory::Credential<credential::internal::Primary>(
            api_,
            *this,
            *serialized,
            mode_,
            proto::CREDROLE_MASTERKEY,
            reason)};

    if (master) {
        master_.reset(master.release());

        return bool(master_);
    }

    return false;
}

bool Authority::Path(proto::HDPath& output) const
{
    if (master_) {
        const bool found = master_->Path(output);
        output.mutable_child()->RemoveLast();

        return found;
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Master credential not instantiated.")
        .Flush();

    return false;
}

void Authority::RevokeContactCredentials(std::list<std::string>& output)
{
    const auto revoke = [&](const auto& item) -> void {
        output.push_back(item.first->str());
    };

    for_each(contact_credentials_, revoke);
    contact_credentials_.clear();
}

void Authority::RevokeVerificationCredentials(std::list<std::string>& output)
{
    const auto revoke = [&](const auto& item) -> void {
        output.push_back(item.first->str());
    };

    for_each(verification_credentials_, revoke);
    verification_credentials_.clear();
}

std::shared_ptr<Authority::Serialized> Authority::Serialize(
    const CredentialIndexModeFlag mode) const
{
    auto credSet = std::make_shared<Serialized>();
    credSet->set_version(version_);
    credSet->set_nymid(m_strNymID);
    credSet->set_masterid(GetMasterCredID());
    const auto add_active_id = [&](const auto& item) -> void {
        credSet->add_activechildids(item.first->str());
    };
    const auto add_revoked_id = [&](const auto& item) -> void {
        credSet->add_revokedchildids(item.first);
    };
    const auto add_active_child = [&](const auto& item) -> void {
        *(credSet->add_activechildren()) =
            *item.second->Serialized(AS_PUBLIC, WITH_SIGNATURES);
    };
    const auto add_revoked_child = [&](const auto& item) -> void {
        *(credSet->add_revokedchildren()) =
            *item.second->Serialized(AS_PUBLIC, WITH_SIGNATURES);
    };

    if (CREDENTIAL_INDEX_MODE_ONLY_IDS == mode) {
        if (proto::KEYMODE_PRIVATE == mode_) { credSet->set_index(index_); }

        credSet->set_mode(proto::CREDSETMODE_INDEX);
        for_each(key_credentials_, add_active_id);
        for_each(contact_credentials_, add_active_id);
        for_each(verification_credentials_, add_active_id);
        for_each(m_mapRevokedCredentials, add_revoked_id);
    } else {
        credSet->set_mode(proto::CREDSETMODE_FULL);
        *(credSet->mutable_mastercredential()) =
            *(master_->Serialized(AS_PUBLIC, WITH_SIGNATURES));

        for_each(key_credentials_, add_active_child);
        for_each(contact_credentials_, add_active_child);
        for_each(verification_credentials_, add_active_child);
        for_each(m_mapRevokedCredentials, add_revoked_child);
    }

    return credSet;
}

void Authority::SetSource(const std::shared_ptr<NymIDSource>& source)
{
    nym_id_source_ = source;
    m_strNymID = nym_id_source_->NymID()->str();
}

bool Authority::Sign(
    const credential::Primary& credential,
    proto::Signature& sig,
    const opentxs::PasswordPrompt& reason) const
{
    return nym_id_source_->Sign(credential, sig, reason);
}

bool Authority::Sign(
    const GetPreimage input,
    const proto::SignatureRole role,
    proto::Signature& signature,
    const opentxs::PasswordPrompt& reason,
    proto::KeyRole key,
    const proto::HashType hash) const
{
    switch (role) {
        case (proto::SIGROLE_PUBCREDENTIAL): {
            if (master_->hasCapability(NymCapability::SIGN_CHILDCRED)) {
                return master_->Sign(input, role, signature, reason, key, hash);
            }

            break;
        }
        case (proto::SIGROLE_NYMIDSOURCE): {
            LogOutput(": Credentials to be signed with a nym source can not "
                      "use this method.")
                .Flush();

            return false;
        }
        case (proto::SIGROLE_PRIVCREDENTIAL): {
            LogOutput(": Private credential can not use this method.").Flush();

            return false;
        }
        default: {
            for (const auto& [id, pCredential] : key_credentials_) {
                OT_ASSERT(pCredential);

                const auto& credential = *pCredential;

                if (false ==
                    credential.hasCapability(NymCapability::SIGN_MESSAGE)) {
                    continue;
                }

                if (credential.Sign(
                        input, role, signature, reason, key, hash)) {

                    return true;
                }
            }
        }
    }

    return false;
}

bool Authority::TransportKey(
    Data& publicKey,
    OTPassword& privateKey,
    const opentxs::PasswordPrompt& reason) const
{
    for (const auto& [id, pCredential] : key_credentials_) {
        OT_ASSERT(pCredential);

        const auto& credential = *pCredential;

        if (false ==
            credential.hasCapability(NymCapability::AUTHENTICATE_CONNECTION)) {
            continue;
        }

        if (credential.TransportKey(publicKey, privateKey, reason)) {
            return true;
        }
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": No child credentials are capable of "
                                       "generating transport keys.")
        .Flush();

    return false;
}

template <typename Item>
bool Authority::validate_credential(
    const Item& item,
    const opentxs::PasswordPrompt& reason) const
{
    const auto& [id, pCredential] = item;

    if (nullptr == pCredential) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null credential ")(id)(" in map")
            .Flush();

        return false;
    }

    const auto& credential = *pCredential;

    if (credential.Validate(reason)) { return true; }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid credential ")(id).Flush();

    return false;
}

bool Authority::Verify(
    const Data& plaintext,
    const proto::Signature& sig,
    const opentxs::PasswordPrompt& reason,
    const proto::KeyRole key) const
{
    std::string signerID(sig.credentialid());

    if (signerID == GetMasterCredID()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Master credentials are only allowed to sign other credentials.")
            .Flush();

        return false;
    }

    const auto* credential = get_secondary_credential(signerID);

    if (nullptr == credential) {
        LogDebug(OT_METHOD)(__FUNCTION__)(
            ": This Authority does not contain the credential which"
            "produced the signature.")
            .Flush();

        return false;
    }

    return credential->Verify(plaintext, sig, reason, key);
}

bool Authority::Verify(
    const proto::Verification& item,
    const opentxs::PasswordPrompt& reason) const
{
    auto serialized = credential::Verification::SigningForm(item);
    auto& signature = *serialized.mutable_sig();
    proto::Signature signatureCopy;
    signatureCopy.CopyFrom(signature);
    signature.clear_signature();

    return Verify(api_.Factory().Data(serialized), signatureCopy, reason);
}

bool Authority::VerifyInternally(const opentxs::PasswordPrompt& reason) const
{
    if (false == bool(master_)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Missing master credential.")
            .Flush();
        return false;
    }

    if (false == master_->Validate(reason)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Master Credential failed to verify: ")(GetMasterCredID())(
            " NymID: ")(GetNymID())(".")
            .Flush();

        return false;
    }

    bool output{true};
    const auto validate = [&](const auto& item) -> void {
        output &= validate_credential(item, reason);
    };

    for_each(key_credentials_, validate);
    for_each(contact_credentials_, validate);
    for_each(verification_credentials_, validate);

    return output;
}

bool Authority::WriteCredentials() const
{
    if (!master_->Save()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to save master credential.")
            .Flush();

        return false;
    };

    bool output{true};
    const auto save = [&](const auto& item) -> void {
        output &= item.second->Save();
    };

    for_each(key_credentials_, save);
    for_each(contact_credentials_, save);
    for_each(verification_credentials_, save);

    if (false == output) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save child credential.")
            .Flush();
    }

    return output;
}
}  // namespace opentxs::identity::implementation
