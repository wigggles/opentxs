// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                 // IWYU pragma: associated
#include "1_Internal.hpp"               // IWYU pragma: associated
#include "core/crypto/PaymentCode.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstring>
#include <functional>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>

#include "Factory.hpp"
#include "internal/api/Api.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Proto.tpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/crypto/key/Secp256k1.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/Enums.pb.h"
#include "opentxs/protobuf/verify/Credential.hpp"
#include "opentxs/protobuf/verify/PaymentCode.hpp"

template class opentxs::Pimpl<opentxs::PaymentCode>;

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#define OT_METHOD "opentxs::implementation::PaymentCode::"
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

namespace opentxs
{
const VersionNumber PaymentCode::DefaultVersion{1};

using ReturnType = implementation::PaymentCode;

auto Factory::PaymentCode(
    const api::internal::Core& api,
    const std::uint8_t version,
    const bool hasBitmessage,
    const ReadView pubkey,
    const ReadView chaincode,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    ,
    std::unique_ptr<crypto::key::Secp256k1> key
#endif
    ) noexcept -> std::unique_ptr<opentxs::PaymentCode>
{
    return std::make_unique<ReturnType>(
        api,
        version,
        hasBitmessage,
        pubkey,
        chaincode,
        bitmessageVersion,
        bitmessageStream
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        ,
        std::move(key)
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    );
}
}  // namespace opentxs

namespace opentxs::implementation
{
const std::size_t PaymentCode::pubkey_size_{sizeof(SerializedForBase58::key_)};
const std::size_t PaymentCode::chain_code_size_{
    sizeof(SerializedForBase58::code_)};

PaymentCode::PaymentCode(
    const api::internal::Core& api,
    const std::uint8_t version,
    const bool hasBitmessage,
    const ReadView pubkey,
    const ReadView chaincode,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    ,
    std::unique_ptr<crypto::key::Secp256k1> key
#endif
    ) noexcept
    : api_(api)
    , version_(version)
    , hasBitmessage_(hasBitmessage)
    , pubkey_(api.Factory().Data(pubkey))
    , chain_code_(api.Factory().SecretFromBytes(chaincode))
    , bitmessage_version_(bitmessageVersion)
    , bitmessage_stream_(bitmessageStream)
    , id_(calculate_id(api, pubkey, chaincode))
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    , key_(std::move(key))
#else
    , key_(crypto::key::Asymmetric::Factory())
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
{
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    OT_ASSERT(key_);
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
}

PaymentCode::PaymentCode(const PaymentCode& rhs)
    : api_(rhs.api_)
    , version_(rhs.version_)
    , hasBitmessage_(rhs.hasBitmessage_)
    , pubkey_(rhs.pubkey_)
    , chain_code_(rhs.chain_code_)
    , bitmessage_version_(rhs.bitmessage_version_)
    , bitmessage_stream_(rhs.bitmessage_stream_)
    , id_(rhs.id_)
    , key_(rhs.key_)
{
}

PaymentCode::operator const crypto::key::Asymmetric&() const noexcept
{
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    return *key_;
#else
    return key_.get();
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
}

auto PaymentCode::operator==(const proto::PaymentCode& rhs) const noexcept
    -> bool
{
    const auto LHData = api_.Factory().Data(Serialize());
    const auto RHData = api_.Factory().Data(rhs);

    return (LHData == RHData);
}

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
auto PaymentCode::AddPrivateKeys(
    std::string& seed,
    const Bip32Index index,
    const PasswordPrompt& reason) noexcept -> bool
{
    auto pCandidate = api_.Seeds().GetPaymentCode(seed, index, reason);

    if (false == bool(pCandidate)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive private key")
            .Flush();

        return false;
    }

    const auto& candidate = *pCandidate;

    if (0 != pubkey_->Bytes().compare(candidate.PublicKey())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Derived public key does not match this payment code")
            .Flush();

        return false;
    }

    if (0 != chain_code_->Bytes().compare(candidate.Chaincode(reason))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Derived chain code does not match this payment code")
            .Flush();

        return false;
    }

    key_ = std::move(pCandidate);

    OT_ASSERT(key_);

    return true;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32

auto PaymentCode::asBase58() const noexcept -> std::string
{
    const auto key = pubkey_->Bytes();
    const auto code = chain_code_->Bytes();
    auto raw = SerializedForBase58{
        version_, hasBitmessage_, bitmessage_version_, bitmessage_stream_};

    if (nullptr != key.data()) {
        std::memcpy(
            raw.key_.data(), key.data(), std::min(raw.key_.size(), key.size()));
    }

    if (nullptr != code.data()) {
        std::memcpy(
            raw.code_.data(),
            code.data(),
            std::min(raw.code_.size(), code.size()));
    }

    return api_.Crypto().Encode().IdentifierEncode(api_.Factory().Data(
        {reinterpret_cast<const char*>(&raw), sizeof(raw)}));
}

auto PaymentCode::calculate_id(
    const api::internal::Core& api,
    const ReadView key,
    const ReadView code) noexcept -> OTNymID
{
    auto output = api.Factory().NymID();

    if ((nullptr == key.data()) || (nullptr == code.data())) { return output; }

    auto preimage = api.Factory().Data();
    const auto target{pubkey_size_ + chain_code_size_};
    auto raw = preimage->WriteInto()(target);

    OT_ASSERT(raw.valid(target));

    auto* it = raw.as<std::byte>();
    std::memcpy(
        it, key.data(), std::min(key.size(), std::size_t{pubkey_size_}));
    std::advance(it, pubkey_size_);
    std::memcpy(
        it, code.data(), std::min(code.size(), std::size_t{chain_code_size_}));

    output->CalculateDigest(preimage->Bytes());

    return output;
}

auto PaymentCode::Serialize() const noexcept -> Serialized
{
    const auto key = pubkey_->Bytes();
    const auto code = chain_code_->Bytes();
    auto output = Serialized{};
    output.set_version(version_);
    output.set_key(key.data(), key.size());
    output.set_chaincode(code.data(), code.size());
    output.set_bitmessageversion(bitmessage_version_);
    output.set_bitmessagestream(bitmessage_stream_);

    return output;
}

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
auto PaymentCode::Sign(
    const identity::credential::Base& credential,
    proto::Signature& sig,
    const PasswordPrompt& reason) const noexcept -> bool
{
    auto serialized = credential.Serialized(AS_PUBLIC, WITHOUT_SIGNATURES);
    auto& signature = *serialized->add_signature();
    const bool output = key_->Sign(
        [&]() -> std::string { return proto::ToString(*serialized); },
        proto::SIGROLE_NYMIDSOURCE,
        signature,
        ID(),
        reason,
        proto::KEYROLE_SIGN);
    sig.CopyFrom(signature);

    return output;
}

auto PaymentCode::Sign(
    const Data& data,
    Data& output,
    const PasswordPrompt& reason) const noexcept -> bool
{
    const auto& key = *key_;

    return key.engine().Sign(
        api_, data, key, proto::HASHTYPE_SHA256, output, reason);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

auto PaymentCode::Valid() const noexcept -> bool
{
    if (0 == version_) { return false; }

    if (pubkey_size_ != pubkey_->size()) { return false; }

    if (chain_code_size_ != chain_code_->size()) { return false; }

    return proto::Validate<proto::PaymentCode>(Serialize(), SILENT);
}

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
auto PaymentCode::Verify(
    const proto::Credential& master,
    const proto::Signature& sourceSignature) const noexcept -> bool
{
    if (false == proto::Validate<proto::Credential>(
                     master,
                     VERBOSE,
                     proto::KEYMODE_PUBLIC,
                     proto::CREDROLE_MASTERKEY,
                     false)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Invalid master credential syntax.")
            .Flush();

        return false;
    }

    const bool sameSource =
        (*this == master.masterdata().source().paymentcode());

    if (false == sameSource) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Master credential was not derived from this source.")
            .Flush();

        return false;
    }

    auto copy = proto::Credential{};
    copy.CopyFrom(master);
    auto& signature = *copy.add_signature();
    signature.CopyFrom(sourceSignature);
    signature.clear_signature();

    return key_->Verify(api_.Factory().Data(copy), sourceSignature);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
}  // namespace opentxs::implementation
