// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// A nym contains a list of credential sets.
// The whole purpose of a Nym is to be an identity, which can have
// multiple Authorities.
//
// Each Authority contains list of Credentials. One of the
// Credentials is a MasterCredential, and the rest are ChildCredentials
// signed by the MasterCredential.
//
// A Credential may contain keys, in which case it is a KeyCredential.
//
// Credentials without keys might be an interface to a hardware device
// or other kind of external encryption and authentication system.
//
// Non-key Credentials are not yet implemented.
//
// Each KeyCredential has 3 OTKeypairs: encryption, signing, and authentication.
// Each OTKeypair has 2 crypto::key::Asymmetrics (public and private.)
//
// A MasterCredential must be a KeyCredential, and is only used to sign
// ChildCredentials
//
// ChildCredentials are used for all other actions, and never sign other
// Credentials

#include "stdafx.hpp"

#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/crypto/ChildKeyCredential.hpp"
#include "opentxs/core/crypto/ContactCredential.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/KeyCredential.hpp"
#include "opentxs/core/crypto/MasterCredential.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/VerificationCredential.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/NymIDSource.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Proto.hpp"

#include "internal/identity/Identity.hpp"
#include "Factory.hpp"

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
identity::internal::Authority* Factory::Authority(const api::Core& api)
{
    return new identity::implementation::Authority(api);
}

identity::internal::Authority* Factory::Authority(
    const api::Core& api,
    const proto::KeyMode mode,
    const proto::CredentialSet& serialized)
{
    return new identity::implementation::Authority(api, mode, serialized);
}

identity::internal::Authority* Factory::Authority(
    const api::Core& api,
    const NymParameters& nymParameters,
    const std::uint32_t version,
    const OTPasswordData* pPWData)
{
    return new identity::implementation::Authority(
        api, nymParameters, version, pPWData);
}
}  // namespace opentxs

namespace opentxs::identity::implementation
{
Authority::Authority(const api::Core& api)
    : api_(api)
    , m_MasterCredential{nullptr}
    , m_mapCredentials{}
    , m_mapRevokedCredentials{}
    , m_strNymID{}
    , nym_id_source_{nullptr}
    , m_pImportPassword{nullptr}
    , version_{0}
    , index_{0}
    , mode_{proto::KEYMODE_ERROR}
{
}

Authority::Authority(
    const api::Core& api,
    const proto::KeyMode mode,
    const Serialized& serializedCredentialSet)
    : api_(api)
    , m_MasterCredential{nullptr}
    , m_mapCredentials{}
    , m_mapRevokedCredentials{}
    , m_strNymID(serializedCredentialSet.nymid())
    , nym_id_source_{nullptr}
    , m_pImportPassword{nullptr}
    , version_(serializedCredentialSet.version())
    , index_(serializedCredentialSet.index())
    , mode_(mode)
{
    if (proto::CREDSETMODE_INDEX == serializedCredentialSet.mode()) {
        Load_Master(
            String::Factory(serializedCredentialSet.nymid()),
            String::Factory(serializedCredentialSet.masterid()));

        for (auto& it : serializedCredentialSet.activechildids()) {
            LoadChildKeyCredential(String::Factory(it));
        }
    } else {
        auto master = Credential::Factory(
            api_, *this, serializedCredentialSet.mastercredential(), mode);

        if (master) {
            m_MasterCredential.reset(
                dynamic_cast<MasterCredential*>(master.release()));
        }

        for (auto& it : serializedCredentialSet.activechildren()) {
            LoadChildKeyCredential(it);
        }
    }
}

Authority::Authority(
    const api::Core& api,
    const NymParameters& nymParameters,
    std::uint32_t version,
    const OTPasswordData*)
    : api_(api)
    , m_MasterCredential{nullptr}
    , m_mapCredentials{}
    , m_mapRevokedCredentials{}
    , m_strNymID{}
    , nym_id_source_{nullptr}
    , m_pImportPassword{nullptr}
    , version_(version)
    , index_{0}
    , mode_(proto::KEYMODE_PRIVATE)
{
    CreateMasterCredential(nymParameters);

    OT_ASSERT(m_MasterCredential);

    NymParameters revisedParameters = nymParameters;
    bool haveChildCredential = false;

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    if (!haveChildCredential) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Creating an ed25519 child key credential.")
            .Flush();
        revisedParameters.setNymParameterType(NymParameterType::ED25519);
        haveChildCredential = !AddChildKeyCredential(revisedParameters).empty();
    }
#endif

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    if (!haveChildCredential) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Creating an secp256k1 child key credential.")
            .Flush();
        revisedParameters.setNymParameterType(NymParameterType::SECP256K1);
        haveChildCredential = !AddChildKeyCredential(revisedParameters).empty();
    }
#endif

#if OT_CRYPTO_SUPPORTED_KEY_RSA
    if (!haveChildCredential) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Creating an RSA child key credential.")
            .Flush();
        revisedParameters.setNymParameterType(NymParameterType::RSA);
        haveChildCredential = !AddChildKeyCredential(revisedParameters).empty();
    }
#endif

    OT_ASSERT(haveChildCredential);
}

bool Authority::Path(proto::HDPath& output) const
{
    if (m_MasterCredential) {

        const bool found = m_MasterCredential->Path(output);
        output.mutable_child()->RemoveLast();
        return found;
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Master credential not instantiated.")
        .Flush();

    return false;
}

std::int32_t Authority::GetPublicKeysBySignature(
    crypto::key::Keypair::Keys& listOutput,
    const Signature& theSignature,
    char cKeyType) const  // 'S' (signing key) or 'E' (encryption key)
                          // or 'A' (authentication key)
{
    std::int32_t nCount = 0;
    for (const auto& it : m_mapCredentials) {
        const auto& pSub = it.second;

        OT_ASSERT(pSub);

        const ChildKeyCredential* pKey =
            dynamic_cast<const ChildKeyCredential*>(pSub.get());

        if (nullptr == pKey)
            continue;  // Skip all non-key credentials. We're looking for
                       // keys.

        const std::int32_t nTempCount =
            pKey->GetPublicKeysBySignature(listOutput, theSignature, cKeyType);
        nCount += nTempCount;
    }
    return nCount;
}

bool Authority::VerifyInternally() const
{
    if (!m_MasterCredential) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": This credential set does not have a master credential.")
            .Flush();
        return false;
    }

    // Check for a valid master credential, including whether or not the
    // NymID and MasterID in the Authority match the master
    // credentials's versions.
    if (!(m_MasterCredential->Validate())) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Master Credential failed to verify: ")(GetMasterCredID())(
            " NymID: ")(GetNymID())(".")
            .Flush();
        return false;
    }

    // Check each child credential for validity.
    for (const auto& it : m_mapCredentials) {
        std::string str_sub_id = it.first;
        auto& pSub = it.second;

        OT_ASSERT(pSub);

        if (!pSub->Validate()) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Child credential failed to verify: ")(str_sub_id)(
                " NymID: ")(GetNymID())(".")
                .Flush();

            return false;
        }
    }

    return true;
}

const std::string& Authority::GetNymID() const { return m_strNymID; }

const NymIDSource& Authority::Source() const { return *nym_id_source_; }

/// This sets nym_id_source_.
/// This also sets m_strNymID.
void Authority::SetSource(const std::shared_ptr<NymIDSource>& source)
{
    nym_id_source_ = source;

    m_strNymID = nym_id_source_->NymID()->str();
}

std::string Authority::AddChildKeyCredential(const NymParameters& nymParameters)
{
    std::string output;
    NymParameters revisedParameters = nymParameters;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    revisedParameters.SetCredIndex(index_++);
#endif
    std::unique_ptr<Credential> childCred =
        Credential::Create<ChildKeyCredential>(api_, *this, revisedParameters);

    if (!childCred) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to instantiate child key credential.")
            .Flush();

        return output;
    }

    output = String::Factory(childCred->ID())->Get();
    auto& it = m_mapCredentials[output];
    it.swap(childCred);

    return output;
}

bool Authority::CreateMasterCredential(const NymParameters& nymParameters)
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

    if (m_MasterCredential) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": The master credential already exists.")
            .Flush();

        return false;
    }

    m_MasterCredential =
        Credential::Create<MasterCredential>(api_, *this, nymParameters);

    if (m_MasterCredential) {
        index_++;

        return true;
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Failed to instantiate master credential.")
        .Flush();

    return false;
}

const std::string Authority::GetMasterCredID() const
{
    if (m_MasterCredential) { return m_MasterCredential->ID()->str(); }
    return "";
}

// When exporting a Nym, you don't want his private keys encrypted to the
// cached key for the wallet, so you have to load them up, and then pause
// OTCachedKey, and then save them to string again, re-encrypting them to
// the export passphrase (and not to any "master key" from the wallet.) And
// you have to release all the signatures on the private credentials, since
// the private info is being re-encrypted, and re-sign them all. Joy.
//
bool Authority::ReEncryptPrivateCredentials(
    const OTPassword& theExportPassword,
    bool bImporting)
{
    OT_ASSERT(m_MasterCredential)
    if (m_MasterCredential->Private()) {
        OTPasswordData thePWData(
            bImporting ? "2 Enter passphrase for the Nym being imported."
                       : "2 Enter new passphrase for exported Nym.");

        const OTPassword* passwordToUse =
            bImporting ? nullptr : &theExportPassword;

        // Re-encrypt the private keys in the master credential. (THEN
        // sign.)
        //
        const bool bReEncryptMaster =
            m_MasterCredential->ReEncryptKeys(theExportPassword, bImporting);
        bool bSignedMaster = false;

        if (bReEncryptMaster && bImporting) {
            m_MasterCredential->ReleaseSignatures(true);  // This time we'll
                                                          // sign it in
                                                          // private mode.
            bSignedMaster =
                m_MasterCredential->SelfSign(passwordToUse, &thePWData, true);
        }
        if (!bReEncryptMaster) {
            LogOutput(OT_METHOD)(__FUNCTION__)("In ")(__FILE__)(", line ")(
                __LINE__)(": Failed trying to re-encrypt the private master "
                          "credential.")
                .Flush();
            return false;
        }

        if (bSignedMaster || !bImporting) {
            m_MasterCredential->Save();

            for (auto& it : m_mapCredentials) {
                auto& pSub = it.second;

                OT_ASSERT(pSub);

                ChildKeyCredential* pKey =
                    dynamic_cast<ChildKeyCredential*>(pSub.get());

                if (nullptr == pKey) continue;

                const bool bReEncryptChildKeyCredential =
                    pKey->ReEncryptKeys(theExportPassword, bImporting);
                bool bSignedChildCredential = false;

                if (bReEncryptChildKeyCredential && bImporting) {
                    pKey->ReleaseSignatures(true);
                    bSignedChildCredential =
                        pKey->SelfSign(passwordToUse, &thePWData, true);
                }
                if (!bReEncryptChildKeyCredential) {
                    LogOutput(OT_METHOD)(__FUNCTION__)("In ")(__FILE__)(
                        ", line ")(__LINE__)(
                        ": Failed trying to re-encrypt the private "
                        "child key credential.")
                        .Flush();
                    return false;
                }
                if (bImporting) {
                    if (bSignedChildCredential) {
                        pKey->Save();
                    } else {
                        LogOutput(OT_METHOD)(__FUNCTION__)("In ")(__FILE__)(
                            ", line ")(__LINE__)(
                            ": Failed trying to re-sign the private "
                            "child "
                            "key credential.")
                            .Flush();
                        return false;
                    }
                }
            }

            return true;  // <=== Success.
        } else
            LogOutput(OT_METHOD)(__FUNCTION__)("In ")(__FILE__)(", line ")(
                __LINE__)(": Failed trying to re-sign the master private "
                          "credential.")
                .Flush();
    } else
        LogOutput(OT_METHOD)(__FUNCTION__)("In ")(__FILE__)(", line ")(
            __LINE__)(": Failed: There is no private info on this master "
                      "credential.")
            .Flush();

    return false;
}

bool Authority::Load_Master(
    const String& strNymID,
    const String& strMasterCredID,
    const OTPasswordData*)
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

    auto master = Credential::Factory(
        api_, *this, *serialized, mode_, proto::CREDROLE_MASTERKEY);

    if (master) {
        m_MasterCredential.reset(
            dynamic_cast<MasterCredential*>(master.release()));

        return bool(m_MasterCredential);
    }

    return false;
}

bool Authority::LoadChildKeyCredential(const String& strSubID)
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

    return LoadChildKeyCredential(*child);
}

bool Authority::LoadChildKeyCredential(const proto::Credential& serializedCred)
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

    auto child = Credential::Factory(api_, *this, serializedCred, mode_);

    if (child) {
        m_mapCredentials[serializedCred.id()].swap(child);

        return true;
    }

    return false;
}

const Credential* Authority::GetChildCredential(
    const String& strSubID,
    const String::List* plistRevokedIDs) const
{
    for (const auto& it : m_mapCredentials) {
        const std::string str_cred_id = it.first;
        const auto& pSub = it.second;

        OT_ASSERT(pSub);

        // See if pSub, with ID str_cred_id, is on plistRevokedIDs...
        //
        if (nullptr != plistRevokedIDs) {
            auto iter = std::find(
                plistRevokedIDs->begin(), plistRevokedIDs->end(), str_cred_id);
            if (iter != plistRevokedIDs->end())  // It was on the revoked
                                                 // list.
                continue;  // Skip this revoked credential.
        }
        // At this point we know it's not on the revoked list, if one was
        // passed in.

        if (strSubID.Compare(str_cred_id.c_str())) return pSub.get();
    }
    return nullptr;
}

const crypto::key::Keypair& Authority::GetAuthKeypair(
    proto::AsymmetricKeyType keytype,
    const String::List* plistRevokedIDs) const
{
    for (const auto& it : m_mapCredentials) {
        const std::string str_cred_id = it.first;
        const auto& pSub = it.second;

        OT_ASSERT(pSub);

        const ChildKeyCredential* pKey =
            dynamic_cast<const ChildKeyCredential*>(pSub.get());

        if (nullptr == pKey) continue;

        // See if pKey, with ID str_cred_id, is on plistRevokedIDs...
        //
        if (nullptr != plistRevokedIDs) {
            auto iter = std::find(
                plistRevokedIDs->begin(), plistRevokedIDs->end(), str_cred_id);
            if (iter != plistRevokedIDs->end())  // It was on the revoked
                                                 // list.
                continue;                        // Skip this revoked key.
        }
        // At this point we know it's a key credential, and we know it's not
        // on the revoked list. Therefore, let's return the key! (Any other
        // future smart criteria might go here before taking this final
        // step...)
        //
        return pKey->GetKeypair(keytype, proto::KEYROLE_AUTH);
    }

    // Didn't find any child credentials we can use? For now, we'll return
    // the master key instead. This is purely for backwards compatibility
    // reasons and eventually should be removed. That is, master credentials
    // should only be able to verify child credentials, and only child
    // credentials should be able to do actions. Capiche?

    return m_MasterCredential->authentication_key_;
}

const crypto::key::Keypair& Authority::GetEncrKeypair(
    proto::AsymmetricKeyType keytype,
    const String::List* plistRevokedIDs) const
{
    for (const auto& it : m_mapCredentials) {
        const std::string str_cred_id = it.first;
        const auto& pSub = it.second;

        OT_ASSERT(pSub);

        const ChildKeyCredential* pKey =
            dynamic_cast<const ChildKeyCredential*>(pSub.get());

        if (nullptr == pKey) continue;

        // See if pKey, with ID str_cred_id, is on plistRevokedIDs...
        //
        if (nullptr != plistRevokedIDs) {
            auto iter = std::find(
                plistRevokedIDs->begin(), plistRevokedIDs->end(), str_cred_id);
            if (iter != plistRevokedIDs->end())  // It was on the revoked
                                                 // list.
                continue;                        // Skip this revoked key.
        }
        // At this point we know it's a key credential, and we know it's not
        // on the revoked list. Therefore, let's return the key! (Any other
        // future smart criteria might go here before taking this final
        // step...)

        return pKey->GetKeypair(keytype, proto::KEYROLE_ENCRYPT);
    }

    // Didn't find any child credentials we can use? For now, we'll return
    // the master key instead. This is purely for backwards compatibility
    // reasons and eventually should be removed. That is, master credentials
    // should only be able to verify child credentials, and only child
    // credentials should be able to do actions. Capiche?
    //

    return m_MasterCredential->encryption_key_;
}

const crypto::key::Keypair& Authority::GetSignKeypair(
    proto::AsymmetricKeyType keytype,
    const String::List* plistRevokedIDs) const
{
    for (const auto& it : m_mapCredentials) {
        const std::string str_cred_id = it.first;
        const auto& pSub = it.second;

        OT_ASSERT(pSub);

        const ChildKeyCredential* pKey =
            dynamic_cast<const ChildKeyCredential*>(pSub.get());

        if (nullptr == pKey) continue;

        // See if pKey, with ID str_cred_id, is on plistRevokedIDs...
        //
        if (nullptr != plistRevokedIDs) {
            auto iter = std::find(
                plistRevokedIDs->begin(), plistRevokedIDs->end(), str_cred_id);
            if (iter != plistRevokedIDs->end())  // It was on the revoked
                                                 // list.
                continue;                        // Skip this revoked key.
        }
        // At this point we know it's a key credential, and we know it's not
        // on the revoked list. Therefore, let's return the key! (Any other
        // future smart criteria might go here before taking this final
        // step...)
        //
        return pKey->GetKeypair(keytype, proto::KEYROLE_SIGN);
    }

    // Didn't find any child credentials we can use? For now, we'll return
    // the master key instead. This is purely for backwards compatibility
    // reasons and eventually should be removed. That is, master credentials
    // should only be able to verify child credentials, and only child
    // credentials should be able to do actions. Capiche?
    //
    return m_MasterCredential->signing_key_;
}

// NOTE: Until we figure out the rule by which we decide WHICH
// authentication key is the right auth key, or WHICH signing key is the
// right signing key, we'll just go with the first one we find. We'll also
// weed out any that appear on plistRevokedIDs, if it's passed in.
// (Optional.)

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

void Authority::ClearChildCredentials() { m_mapCredentials.clear(); }

bool Authority::WriteCredentials() const
{
    if (!m_MasterCredential->Save()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to save master credential.")
            .Flush();

        return false;
    };

    for (auto& it : m_mapCredentials) {
        if (!it.second->Save()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to save child credential.")
                .Flush();

            return false;
        }
    }

    return true;
}

std::shared_ptr<Authority::Serialized> Authority::Serialize(
    const CredentialIndexModeFlag mode) const
{
    auto credSet = std::make_shared<Serialized>();
    credSet->set_version(version_);
    credSet->set_nymid(m_strNymID);
    credSet->set_masterid(GetMasterCredID());

    if (CREDENTIAL_INDEX_MODE_ONLY_IDS == mode) {
        if (proto::KEYMODE_PRIVATE == mode_) { credSet->set_index(index_); }
        credSet->set_mode(proto::CREDSETMODE_INDEX);

        for (auto& it : m_mapCredentials) {
            credSet->add_activechildids(it.first);
        }

        for (auto& it : m_mapRevokedCredentials) {
            credSet->add_revokedchildids(it.first);
        }
    } else {
        credSet->set_mode(proto::CREDSETMODE_FULL);
        *(credSet->mutable_mastercredential()) =
            *(m_MasterCredential->Serialized(AS_PUBLIC, WITH_SIGNATURES));
        std::unique_ptr<proto::Credential> pChildCred;

        for (auto& it : m_mapCredentials) {
            pChildCred.reset(credSet->add_activechildren());
            *pChildCred = *(it.second->Serialized(AS_PUBLIC, WITH_SIGNATURES));
            pChildCred.release();
        }

        for (auto& it : m_mapRevokedCredentials) {
            pChildCred.reset(credSet->add_revokedchildren());
            *pChildCred = *(it.second->Serialized(AS_PUBLIC, WITH_SIGNATURES));
            pChildCred.release();
        }
    }

    return credSet;
}

bool Authority::GetContactData(
    std::unique_ptr<proto::ContactData>& contactData) const
{
    bool found = false;

    for (auto& it : m_mapCredentials) {
        if (nullptr != it.second) {
            if (proto::CREDROLE_CONTACT == it.second->Role()) {
                found = it.second->GetContactData(contactData);

                break;
            }
        }
    }

    return found;
}

bool Authority::GetVerificationSet(
    std::unique_ptr<proto::VerificationSet>& verificationSet) const
{
    bool found = false;

    for (auto& it : m_mapCredentials) {
        if (nullptr != it.second) {
            if (proto::CREDROLE_VERIFY == it.second->Role()) {
                found = it.second->GetVerificationSet(verificationSet);
            }
        }
    }

    return found;
}

void Authority::RevokeContactCredentials(
    std::list<std::string>& contactCredentialIDs)
{
    std::list<std::string> credentialsToDelete;

    for (auto& it : m_mapCredentials) {
        if (nullptr != it.second) {
            if (proto::CREDROLE_CONTACT == it.second->Role()) {
                const auto credID = String::Factory(it.second->ID());
                contactCredentialIDs.push_back(credID->Get());
                credentialsToDelete.push_back(credID->Get());
            }
        }
    }

    for (auto& it : credentialsToDelete) { m_mapCredentials.erase(it); }
}

void Authority::RevokeVerificationCredentials(
    std::list<std::string>& verificationCredentialIDs)
{
    std::list<std::string> credentialsToDelete;

    for (auto& it : m_mapCredentials) {
        if (nullptr != it.second) {
            if (proto::CREDROLE_VERIFY == it.second->Role()) {
                const auto credID = String::Factory(it.second->ID());
                verificationCredentialIDs.push_back(credID->Get());
                credentialsToDelete.push_back(credID->Get());
            }
        }
    }

    for (auto& it : credentialsToDelete) { m_mapCredentials.erase(it); }
}

bool Authority::AddContactCredential(const proto::ContactData& contactData)
{
    LogDetail(OT_METHOD)(__FUNCTION__)(": Adding a contact credential.")
        .Flush();

    if (!m_MasterCredential) { return false; }

    NymParameters nymParameters;
    nymParameters.SetContactData(contactData);

    std::unique_ptr<Credential> newChildCredential =
        Credential::Create<ContactCredential>(api_, *this, nymParameters);

    if (!newChildCredential) { return false; }

    auto& it =
        m_mapCredentials[String::Factory(newChildCredential->ID())->Get()];
    it.swap(newChildCredential);

    auto version =
        proto::RequiredCredentialSetVersion(contactData.version(), version_);
    if (version > version_) { version_ = version; }

    return true;
}

bool Authority::AddVerificationCredential(
    const proto::VerificationSet& verificationSet)
{
    LogDetail(OT_METHOD)(__FUNCTION__)(": Adding a verification credential.")
        .Flush();

    if (!m_MasterCredential) { return false; }

    NymParameters nymParameters;
    nymParameters.SetVerificationSet(verificationSet);

    std::unique_ptr<Credential> newChildCredential =
        Credential::Create<VerificationCredential>(api_, *this, nymParameters);

    if (!newChildCredential) { return false; }

    auto& it =
        m_mapCredentials[String::Factory(newChildCredential->ID())->Get()];
    it.swap(newChildCredential);

    return true;
}

bool Authority::Sign(
    const MasterCredential& credential,
    proto::Signature& sig,
    const OTPasswordData* pPWData) const
{
    return nym_id_source_->Sign(credential, sig, pPWData);
}

bool Authority::Verify(
    const Data& plaintext,
    const proto::Signature& sig,
    const proto::KeyRole key) const
{
    std::string signerID(sig.credentialid());

    if (signerID == GetMasterCredID()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Master credentials are only allowed to "
            "sign other credentials.")
            .Flush();

        return false;
    }

    const Credential* credential =
        GetChildCredential(String::Factory(signerID));

    if (nullptr == credential) {
        LogDebug(OT_METHOD)(__FUNCTION__)(
            ": This credential set does not contain the credential which"
            "produced the signature.")
            .Flush();

        return false;
    }

    return credential->Verify(plaintext, sig, key);
}

bool Authority::Verify(const proto::Verification& item) const
{
    auto serialized = VerificationCredential::SigningForm(item);
    auto& signature = *serialized.mutable_sig();
    proto::Signature signatureCopy;
    signatureCopy.CopyFrom(signature);
    signature.clear_signature();

    return Verify(proto::ProtoAsData(serialized), signatureCopy);
}

bool Authority::TransportKey(Data& publicKey, OTPassword& privateKey) const
{
    bool haveKey = false;

    for (auto& it : m_mapCredentials) {
        OT_ASSERT(nullptr != it.second);

        if (nullptr != it.second) {
            const Credential& childCred = *it.second;

            if (childCred.hasCapability(
                    NymCapability::AUTHENTICATE_CONNECTION)) {
                if (childCred.TransportKey(publicKey, privateKey)) {

                    return true;
                }

                OT_FAIL;
            }
        }
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": No child credentials are capable of "
                                       "generating transport keys.")
        .Flush();

    return haveKey;
}

bool Authority::hasCapability(const NymCapability& capability) const
{
    switch (capability) {
        case (NymCapability::SIGN_CHILDCRED): {
            if (m_MasterCredential) {
                return m_MasterCredential->hasCapability(capability);
            }
            break;
        }
        case (NymCapability::SIGN_MESSAGE): {
        }
        case (NymCapability::ENCRYPT_MESSAGE): {
        }
        case (NymCapability::AUTHENTICATE_CONNECTION): {
            for (auto& childCred : m_mapCredentials) {
                if (nullptr != childCred.second) {
                    if (childCred.second->hasCapability(capability)) {
                        return true;
                    }
                }
            }

            break;
        }
        default: {
        }
    }

    return false;
}

bool Authority::Sign(
    const GetPreimage input,
    const proto::SignatureRole role,
    proto::Signature& signature,
    proto::KeyRole key,
    const OTPasswordData* pPWData,
    const proto::HashType hash) const
{
    switch (role) {
        case (proto::SIGROLE_PUBCREDENTIAL): {
            if (m_MasterCredential->hasCapability(
                    NymCapability::SIGN_CHILDCRED)) {
                return m_MasterCredential->Sign(
                    input, role, signature, key, pPWData, hash);
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
            bool haveSignature = false;

            for (auto& it : m_mapCredentials) {
                auto& credential = it.second;

                if (nullptr != credential) {
                    if (credential->hasCapability(
                            NymCapability::SIGN_MESSAGE)) {
                        const auto keyCredential =
                            dynamic_cast<const KeyCredential*>(
                                credential.get());
                        haveSignature = keyCredential->Sign(
                            input, role, signature, key, pPWData, hash);
                    }

                    if (haveSignature) { return true; }
                }
            }
        }
    }

    return false;
}

Authority::~Authority() { ClearChildCredentials(); }
}  // namespace opentxs::identity::implementation
