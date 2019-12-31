// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip39.hpp"

#include "util/Sodium.hpp"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

#include "Bip32.hpp"

#define OT_METHOD "opentxs::crypto::implementation::Bip32::"

namespace opentxs::crypto
{
auto Print(const proto::HDPath& node) -> std::string
{
    std::stringstream output{};
    output << node.root();

    for (const auto& child : node.child()) {
        output << " / ";
        const Bip32Index max = HDIndex{Bip32Child::HARDENED};

        if (max > child) {
            output << std::to_string(child);
        } else {
            output << std::to_string(child - max) << "'";
        }
    }

    return output.str();
}
}  // namespace opentxs::crypto

namespace opentxs::crypto::implementation
{
Bip32::Bip32(const api::Crypto& crypto) noexcept
    : crypto_(crypto)
{
}

auto Bip32::ckd_private_hardened(
    const HDNode& node,
    const be::big_uint32_buf_t i,
    const WritableView& data) const noexcept -> void
{
    static const auto padding = std::byte{0};
    auto out{data.as<std::byte>()};
    std::memcpy(out, &padding, sizeof(padding));
    std::advance(out, 1);
    std::memcpy(out, node.ParentPrivate().data(), 32);
    std::advance(out, 32);
    std::memcpy(out, &i, sizeof(i));
}

auto Bip32::ckd_private_normal(
    const HDNode& node,
    const be::big_uint32_buf_t i,
    const WritableView& data) const noexcept -> void
{
    auto out{data.as<std::byte>()};
    std::memcpy(out, node.ParentPublic().data(), 33);
    std::advance(out, 33);
    std::memcpy(out, &i, sizeof(i));
}

auto Bip32::decode(const std::string& serialized) const noexcept -> OTData
{
    auto input = crypto_.Encode().IdentifierDecode(serialized);

    return Data::Factory(input.c_str(), input.size());
}

#if OT_CRYPTO_WITH_BIP32
auto Bip32::DeriveKey(
    const EcdsaCurve& curve,
    const OTPassword& seed,
    const Path& path) const -> Key
{
    auto output = Key{OTPassword{}, OTPassword{}, Data::Factory(), path, 0};
    auto& [privateKey, chainCode, publicKey, pathOut, parent] = output;
    auto node = HDNode{crypto_};
    auto& hash = node.hash_;
    auto& data = node.data_;

    {
        const auto init = root_node(
            EcdsaCurve::secp256k1,
            seed.Bytes(),
            node.InitPrivate(),
            node.InitCode(),
            node.InitPublic());

        if (false == init) { return output; }

        if (false == data.valid(33 + 4)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to allocate temporary data space")
                .Flush();

            return output;
        }

        if (false == hash.valid(64)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to allocate temporary hash space")
                .Flush();

            return output;
        }
    }

    for (const auto& child : path) {
        parent = node.Fingerprint();
        auto i = be::big_uint32_buf_t{child};

        if (IsHard(child)) {
            ckd_private_hardened(node, i, data);
        } else {
            ckd_private_normal(node, i, data);
        }

        auto success = crypto_.Hash().HMAC(
            proto::HASHTYPE_SHA512,
            node.ParentCode(),
            reader(data),
            [&hash](const auto) {
                return WritableView{hash.data(), 64};
            });

        if (false == success) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate hash")
                .Flush();

            return output;
        }

        try {
            const auto& ecdsa = provider(EcdsaCurve::secp256k1);
            success = ecdsa.ScalarAdd(
                node.ParentPrivate(),
                {hash.as<char>(), 32},
                node.ChildPrivate());

            if (false == success) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid scalar").Flush();

                return output;
            }

            success = ecdsa.ScalarMultiplyBase(
                reader(node.ChildPrivate()(32)), node.ChildPublic());

            if (false == success) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to calculate public key")
                    .Flush();

                return output;
            }
        } catch (const std::exception& e) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

            return output;
        }

        auto code = hash.as<std::byte>();
        std::advance(code, 32);
        std::memcpy(node.ChildCode().data(), code, 32);
        node.Next();
    }

    const auto privateOut = node.ParentPrivate();
    const auto chainOut = node.ParentCode();
    const auto publicOut = node.ParentPublic();

    if (EcdsaCurve::secp256k1 == curve) {
        privateKey.setMemory(privateOut.data(), privateOut.size());
        publicKey->Assign(publicOut);
    } else {
        const auto expanded = sodium::ExpandSeed(
            {reinterpret_cast<const char*>(privateOut.data()),
             privateOut.size()},
            privateKey.WriteInto(OTPassword::Mode::Mem),
            publicKey->WriteInto());

        if (false == expanded) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to expand seed")
                .Flush();
            return output;
        }
    }

    chainCode.setMemory(chainOut.data(), chainOut.size());

    return output;
}
#endif  // OT_CRYPTO_WITH_BIP32

auto Bip32::DeserializePrivate(
    const std::string& serialized,
    Bip32Network& network,
    Bip32Depth& depth,
    Bip32Fingerprint& parent,
    Bip32Index& index,
    Data& chainCode,
    OTPassword& key) const -> bool
{
    const auto input = decode(serialized);
    const auto size = input->size();

    if (78 != size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid input size (")(size)(")")
            .Flush();

        return {};
    }

    bool output = extract(input, network, depth, parent, index, chainCode);

    if (std::byte(0) != input->at(45)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid padding bit").Flush();

        return {};
    }

    key.setMemory(&input->at(46), 32);

    return output;
}

auto Bip32::DeserializePublic(
    const std::string& serialized,
    Bip32Network& network,
    Bip32Depth& depth,
    Bip32Fingerprint& parent,
    Bip32Index& index,
    Data& chainCode,
    Data& key) const -> bool
{
    const auto input = decode(serialized);
    const auto size = input->size();

    if (78 != size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid input size (")(size)(")")
            .Flush();

        return {};
    }

    bool output = extract(input, network, depth, parent, index, chainCode);
    output &= input->Extract(33, key, 45);

    return output;
}

auto Bip32::extract(
    const Data& input,
    Bip32Network& network,
    Bip32Depth& depth,
    Bip32Fingerprint& parent,
    Bip32Index& index,
    Data& chainCode) const noexcept -> bool
{
    bool output{true};
    output &= input.Extract(network);
    output &= input.Extract(depth, 4);
    output &= input.Extract(parent, 5);
    output &= input.Extract(index, 9);
    output &= input.Extract(32, chainCode, 13);

    return output;
}

auto Bip32::IsHard(const Bip32Index index) noexcept -> bool
{
    static const auto hard = Bip32Index{HDIndex{Bip32Child::HARDENED}};

    return index >= hard;
}

auto Bip32::provider(const EcdsaCurve& curve) const noexcept(false)
    -> const crypto::EcdsaProvider&
{
    switch (curve) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        case EcdsaCurve::ed25519: {
            return crypto_.ED25519();
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case EcdsaCurve::secp256k1: {
            return crypto_.SECP256K1();
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        default: {
            throw std::out_of_range("No support for specified curve");
        }
    }
}

#if OT_CRYPTO_WITH_BIP32
auto Bip32::root_node(
    [[maybe_unused]] const EcdsaCurve& curve,
    const ReadView entropy,
    const AllocateOutput key,
    const AllocateOutput code,
    const AllocateOutput pub) const noexcept -> bool
{
    if ((16 > entropy.size()) || (64 < entropy.size())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid entropy size (")(
            entropy.size())(")")
            .Flush();

        return false;
    }

    if (false == bool(key) || false == bool(code)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator")
            .Flush();

        return false;
    }

    auto keyOut = key(32);
    auto codeOut = code(32);

    if (false == keyOut.valid(32) || false == codeOut.valid(32)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": failed to allocate output space")
            .Flush();

        return false;
    }

    static const auto rootKey = std::string{"Bitcoin seed"};
    auto node = Space{};

    if (false == crypto_.Hash().HMAC(
                     proto::HASHTYPE_SHA512, rootKey, entropy, writer(node))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate root node")
            .Flush();

        return false;
    }

    OT_ASSERT(64 == node.size());

    auto start{node.data()};
    std::memcpy(keyOut, start, 32);
    std::advance(start, 32);
    std::memcpy(codeOut, start, 32);
    const auto havePub = provider(curve).ScalarMultiplyBase(
        {reinterpret_cast<const char*>(node.data()), 32}, pub);

    try {
        if (false == havePub) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to calculate root public key")
                .Flush();

            return false;
        }

        return true;
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

        return false;
    }
}
#endif  // OT_CRYPTO_WITH_BIP32

auto Bip32::SeedID(const ReadView entropy) const -> OTIdentifier
{
    auto output = Identifier::Factory();
    output->CalculateDigest(entropy);

    return output;
}

auto Bip32::SerializePrivate(
    const Bip32Network network,
    const Bip32Depth depth,
    const Bip32Fingerprint parent,
    const Bip32Index index,
    const Data& chainCode,
    const OTPassword& key) const -> std::string
{
    const auto size = key.getMemorySize();

    if (32 != size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid key size (")(size)(")")
            .Flush();

        return {};
    }

    auto input = Data::Factory();
    input->DecodeHex("0x00");

    OT_ASSERT(1 == input->size());

    input += Data::Factory(key.getMemory(), key.getMemorySize());

    OT_ASSERT(33 == input->size());

    return SerializePublic(network, depth, parent, index, chainCode, input);
}

auto Bip32::SerializePublic(
    const Bip32Network network,
    const Bip32Depth depth,
    const Bip32Fingerprint parent,
    const Bip32Index index,
    const Data& chainCode,
    const Data& key) const -> std::string
{
    auto size = key.size();

    if (33 != size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid key size (")(size)(")")
            .Flush();

        return {};
    }

    size = chainCode.size();

    if (32 != size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid chain code size (")(size)(
            ")")
            .Flush();

        return {};
    }

    auto output = Data::Factory(network);
    output.get() += depth;
    output.get() += parent;
    output.get() += index;
    output += chainCode;
    output += key;

    OT_ASSERT_MSG(78 == output->size(), std::to_string(output->size()).c_str());

    return crypto_.Encode().IdentifierEncode(output);
}
}  // namespace opentxs::crypto::implementation
