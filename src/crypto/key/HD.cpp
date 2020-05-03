// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"       // IWYU pragma: associated
#include "1_Internal.hpp"     // IWYU pragma: associated
#include "crypto/key/HD.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <iterator>
#include <limits>
#include <string_view>

#include "crypto/key/EllipticCurve.hpp"
#include "internal/api/Api.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"

#define OT_METHOD "opentxs::crypto::key::implementation::HD::"

namespace opentxs::crypto::key
{
auto HD::CalculateFingerprint(
    const api::crypto::Hash& hash,
    const ReadView key) noexcept -> Bip32Fingerprint
{
    auto output = Bip32Fingerprint{0};
    auto digest = Data::Factory();

    if (33 != key.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid public key").Flush();

        return output;
    }

    const auto hashed =
        hash.Digest(proto::HASHTYPE_BITCOIN, key, digest->WriteInto());

    if (false == hashed) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to calculate public key hash")
            .Flush();

        return output;
    }

    if (false == digest->Extract(output)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to set output").Flush();

        return {};
    }

    return output;
}
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key::implementation
{
HD::HD(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey) noexcept(false)
    : EllipticCurve(api, ecdsa, serializedKey)
    , path_(
          serializedKey.has_path()
              ? std::make_shared<proto::HDPath>(serializedKey.path())
              : nullptr)
    , chain_code_(
          serializedKey.has_chaincode()
              ? std::make_unique<proto::Ciphertext>(serializedKey.chaincode())
              : nullptr)
    , plaintext_chain_code_()
    , parent_(serializedKey.bip32_parent())
{
}

HD::HD(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKeyType keyType,
    const proto::KeyRole role,
    const VersionNumber version,
    const PasswordPrompt& reason) noexcept(false)
    : EllipticCurve(api, ecdsa, keyType, role, version, reason)
    , path_(nullptr)
    , chain_code_(nullptr)
    , plaintext_chain_code_()
    , parent_(0)
{
}

#if OT_CRYPTO_WITH_BIP32
HD::HD(
    const api::internal::Core& api,
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
    const PasswordPrompt& reason) noexcept(false)
    : EllipticCurve(
          api,
          ecdsa,
          keyType,
          privateKey,
          publicKey,
          role,
          version,
          sessionKey,
          reason)
    , path_(std::make_shared<proto::HDPath>(path))
    , chain_code_(encrypt_key(sessionKey, reason, false, chainCode.Bytes()))
    , plaintext_chain_code_(chainCode)
    , parent_(parent)
{
    OT_ASSERT(path_);
    OT_ASSERT(chain_code_);
}
#endif  // OT_CRYPTO_WITH_BIP32

HD::HD(const HD& rhs) noexcept
    : EllipticCurve(rhs)
    , path_(bool(rhs.path_) ? new proto::HDPath(*rhs.path_) : nullptr)
    , chain_code_(
          bool(rhs.chain_code_) ? new proto::Ciphertext(*rhs.chain_code_)
                                : nullptr)
    , plaintext_chain_code_(rhs.plaintext_chain_code_)
    , parent_(rhs.parent_)
{
}

auto HD::Chaincode(const PasswordPrompt& reason) const noexcept -> ReadView
{
    auto existing = plaintext_chain_code_.Bytes();

    if (nullptr != existing.data() && 0 < existing.size()) { return existing; }

    if (false == bool(encrypted_key_)) { return {}; }
    if (false == bool(chain_code_)) { return {}; }

    const auto& chaincode = *chain_code_;
    const auto& privateKey = *encrypted_key_;
    // Private key data and chain code are encrypted to the same session key,
    // and this session key is only embedded in the private key ciphertext
    auto sessionKey =
        api_.Symmetric().Key(privateKey.key(), proto::SMODE_CHACHA20POLY1305);

    if (false == sessionKey.get()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to extract session key.")
            .Flush();

        return {};
    }

    auto allocator = plaintext_chain_code_.WriteInto(OTPassword::Mode::Mem);

    if (false == sessionKey->Decrypt(chaincode, reason, allocator)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt chain code")
            .Flush();

        return {};
    }

    return plaintext_chain_code_.Bytes();
}

auto HD::Depth() const noexcept -> int
{
    if (false == bool(path_)) { return -1; }

    return path_->child_size();
}

auto HD::erase_private_data() -> void
{
    EllipticCurve::erase_private_data();
    const_cast<std::shared_ptr<const proto::HDPath>&>(path_).reset();
    const_cast<std::unique_ptr<const proto::Ciphertext>&>(chain_code_).reset();
}

auto HD::Fingerprint() const noexcept -> Bip32Fingerprint
{
    return CalculateFingerprint(api_.Crypto().Hash(), PublicKey());
}

auto HD::get_params() const noexcept -> std::tuple<bool, Bip32Depth, Bip32Index>
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

    depth = static_cast<std::uint8_t>(size);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
    if (0 < depth) {
        const auto& index = *(path_->child().rbegin());
        child = index;
    }
#pragma GCC diagnostic pop

    success = true;

    return output;
}

auto HD::Path() const noexcept -> const std::string
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

auto HD::Path(proto::HDPath& output) const noexcept -> bool
{
    if (path_) {
        output = *path_;

        return true;
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": HDPath not instantiated.").Flush();

    return false;
}

auto HD::Serialize() const noexcept -> std::shared_ptr<proto::AsymmetricKey>
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

auto HD::Xprv(const PasswordPrompt& reason) const noexcept -> std::string
{
    const auto [ready, depth, child] = get_params();

    if (false == ready) { return {}; }

    OTPassword privateKey{OTPassword::Mode::Mem, PrivateKey(reason)};

    // FIXME Bip32::SerializePrivate should accept ReadView

    return api_.Crypto().BIP32().SerializePrivate(
        0x0488ADE4,
        depth,
        parent_,
        child,
        api_.Factory().Data(Chaincode(reason)),
        privateKey);
}

auto HD::Xpub(const PasswordPrompt& reason) const noexcept -> std::string
{
    const auto [ready, depth, child] = get_params();

    if (false == ready) { return {}; }

    // FIXME Bip32::SerializePublic should accept ReadView

    return api_.Crypto().BIP32().SerializePublic(
        0x0488B21E,
        depth,
        parent_,
        child,
        api_.Factory().Data(Chaincode(reason)),
        api_.Factory().Data(PublicKey()));
}
}  // namespace opentxs::crypto::key::implementation
