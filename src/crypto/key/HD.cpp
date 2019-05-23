// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/Proto.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "crypto/key/EllipticCurve.hpp"

#include "HD.hpp"

#define OT_METHOD "opentxs::crypto::key::implementation::HD::"

namespace opentxs::crypto::key
{
Bip32Fingerprint HD::CalculateFingerprint(
    const api::crypto::Hash& hash,
    const Data& key)
{
    Bip32Fingerprint output{0};
    auto sha = Data::Factory();
    auto ripe = Data::Factory();

    if (33 != key.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid public key").Flush();

        return output;
    }

    const bool haveSha = hash.Digest(proto::HASHTYPE_SHA256, key, sha);

    if (false == haveSha) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to calculate public key hash (sha256)")
            .Flush();

        return output;
    }

    const bool haveRipe = hash.Digest(proto::HASHTYPE_RIMEMD160, sha, ripe);

    if (false == haveRipe) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to calculate public key hash (ripemd160)")
            .Flush();

        return output;
    }

    if (false == ripe->Extract(output)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to set output").Flush();

        OT_FAIL;
    }

    return output;
}
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key::implementation
{
HD::HD(
    const api::crypto::Asymmetric& crypto,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey) noexcept
    : EllipticCurve(crypto, ecdsa, serializedKey)
    , path_(nullptr)
    , chain_code_(nullptr)
    , parent_(serializedKey.bip32_parent())
{
    if (serializedKey.has_path()) {
        path_ = std::make_shared<proto::HDPath>(serializedKey.path());
    }

    if (serializedKey.has_chaincode()) {
        chain_code_.reset(new proto::Ciphertext(serializedKey.chaincode()));
    }
}

HD::HD(
    const api::crypto::Asymmetric& crypto,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKeyType keyType,
    const proto::KeyRole role,
    const VersionNumber version) noexcept
    : EllipticCurve(crypto, ecdsa, keyType, role, version)
    , path_(nullptr)
    , chain_code_(nullptr)
    , parent_(0)
{
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
HD::HD(
    const api::crypto::Asymmetric& crypto,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKeyType keyType,
    const OTPassword& privateKey,
    const OTPassword& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const proto::KeyRole role,
    const VersionNumber version,
    key::Symmetric& sessionKey,
    const OTPasswordData& reason) noexcept
    : EllipticCurve(
          crypto,
          ecdsa,
          keyType,
          privateKey,
          publicKey,
          role,
          version,
          sessionKey,
          reason)
    , path_(std::make_shared<proto::HDPath>(path))
    , chain_code_(encrypt_key(sessionKey, reason, false, chainCode))
    , parent_(parent)
{
    OT_ASSERT(path_);
    OT_ASSERT(chain_code_);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

HD::HD(const HD& rhs) noexcept
    : key::HD()
    , EllipticCurve(rhs)
    , path_(bool(rhs.path_) ? new proto::HDPath(*rhs.path_) : nullptr)
    , chain_code_(
          bool(rhs.chain_code_) ? new proto::Ciphertext(*rhs.chain_code_)
                                : nullptr)
    , parent_(rhs.parent_)
{
}

OTData HD::Chaincode() const
{
    auto output = Data::Factory();

    if (false == bool(encrypted_key_)) { return output; }
    if (false == bool(chain_code_)) { return output; }

    const auto& chaincode = *chain_code_;
    const auto& privateKey = *encrypted_key_;
    // Private key data and chain code are encrypted to the same session key,
    // and this session key is only embedded in the private key ciphertext
    auto sessionKey = crypto_.Symmetric().Key(
        privateKey.key(), proto::SMODE_CHACHA20POLY1305);

    if (false == sessionKey.get()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to extract session key.")
            .Flush();

        return output;
    }

    if (false == sessionKey->Decrypt(chaincode, __FUNCTION__, output)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt chain code")
            .Flush();

        return Data::Factory();
    }

    return output;
}

int HD::Depth() const
{
    if (false == bool(path_)) { return -1; }

    return path_->child_size();
}

void HD::erase_private_data()
{
    EllipticCurve::erase_private_data();
    path_.reset();
    chain_code_.reset();
}

Bip32Fingerprint HD::Fingerprint() const
{
    return CalculateFingerprint(crypto_.Hash(), PublicKey());
}

std::tuple<bool, Bip32Depth, Bip32Index> HD::get_params() const
{
    std::tuple<bool, Bip32Depth, Bip32Index> output{false, 0x0, 0x0};
    auto& [success, depth, child] = output;

    if (false == bool(path_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": missing path").Flush();

        return output;
    }

    const auto& path = *path_;
    auto size = path.child_size();

    if (0 > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid depth (")(size)(")")
            .Flush();

        return output;
    }

    if (std::numeric_limits<Bip32Depth>::max() < size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid depth (")(size)(")")
            .Flush();

        return output;
    }

    depth = size;

    if (0 < depth) {
        const auto& index = *(path_->child().rbegin());
        child = index;
    }

    success = true;

    return output;
}

const std::string HD::Path() const
{
    auto path = String::Factory();

    if (path_) {
        if (path_->has_root()) {
            auto root = Identifier::Factory();
            root->SetString(path_->root());
            path->Concatenate(String::Factory(root));

            for (auto& it : path_->child()) {
                path->Concatenate(" / ");
                if (it < HDIndex{Bip32Child::HARDENED}) {
                    path->Concatenate(String::Factory(std::to_string(it)));
                } else {
                    path->Concatenate(String::Factory(
                        std::to_string(it - HDIndex{Bip32Child::HARDENED})));
                    path->Concatenate("'");
                }
            }
        }
    }

    return path->Get();
}

bool HD::Path(proto::HDPath& output) const
{
    if (path_) {
        output = *path_;

        return true;
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": HDPath not instantiated.").Flush();

    return false;
}

std::shared_ptr<proto::AsymmetricKey> HD::Serialize() const
{
    auto output = EllipticCurve::Serialize();

    OT_ASSERT(output)

    if (HasPrivate()) {
        if (path_) { *(output->mutable_path()) = *path_; }

        if (chain_code_) { *output->mutable_chaincode() = *chain_code_; }
    }

    if (1 < version_) { output->set_bip32_parent(parent_); }

    return output;
}

std::string HD::Xprv() const
{
    const auto [ready, depth, child] = get_params();

    if (false == ready) { return {}; }

    const auto priv = PrivateKey();
    OTPassword privateKey{};
    privateKey.setMemory(priv->data(), priv->size());

    return crypto_.BIP32().SerializePrivate(
        0x0488ADE4, depth, parent_, child, Chaincode(), privateKey);
}

std::string HD::Xpub() const
{
    const auto [ready, depth, child] = get_params();

    if (false == ready) { return {}; }

    return crypto_.BIP32().SerializePublic(
        0x0488B21E, depth, parent_, child, Chaincode(), PublicKey());
}
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
