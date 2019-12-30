// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_USING_TREZOR
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#endif  // OT_CRYPTO_WITH_BIP32
#include "opentxs/crypto/library/Trezor.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include "crypto/Bip32.hpp"
#include "util/Sodium.hpp"

extern "C" {
#include <bip39.h>
#include <bignum.h>
#include <bip32.h>
#include <base58.h>
#include <curves.h>
#include <ecdsa.h>
#include <ripemd160.h>
}

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#endif  // OT_CRYPTO_WITH_BIP32

#include "Trezor.hpp"

#if OT_CRYPTO_WITH_BIP32
#define OT_METHOD "opentxs::crypto::implementation::Trezor::"
#endif  // OT_CRYPTO_WITH_BIP32

namespace opentxs
{
crypto::Trezor* Factory::Trezor(const api::Crypto& crypto)
{
    return new crypto::implementation::Trezor(crypto);
}
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
Trezor::Trezor(const api::Crypto& crypto)
#if OT_CRYPTO_WITH_BIP32
    : Bip32(crypto)
#endif  // OT_CRYPTO_WITH_BIP32
{
}

#if OT_CRYPTO_WITH_BIP32
std::unique_ptr<HDNode> Trezor::derive_child(
    const HDNode& parent,
    const Bip32Index index)
{
    auto output = std::make_unique<HDNode>(parent);
    int result{0};

    OT_ASSERT(output);

    result = hdnode_private_ckd(output.get(), index);
    hdnode_fill_public_key(output.get());

    if (1 != result) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive child").Flush();

        return {};
    }

    return output;
}

std::unique_ptr<HDNode> Trezor::derive_child(
    const api::crypto::Hash& hash,
    const EcdsaCurve& curve,
    const OTPassword& seed,
    const Path& path,
    Bip32Fingerprint& parentID) const
{
    std::unique_ptr<HDNode> output{nullptr};
    Bip32Index depth = path.size();

    if (0 == depth) {
        parentID = 0;
        output = instantiate_node(curve, seed);
    } else {
        auto parentPath{path};
        parentPath.pop_back();
        auto parentNode = derive_child(hash, curve, seed, parentPath, parentID);

        if (parentNode) {
            const auto& childIndex = *path.crbegin();
            output = derive_child(*parentNode, childIndex);
            const auto pubkey = Data::Factory(
                parentNode->public_key, sizeof(parentNode->public_key));
            parentID = key::HD::CalculateFingerprint(hash, pubkey);
        }
    }

    return output;
}

Trezor::Key Trezor::DeriveKey(
    const api::crypto::Hash& hash,
    const EcdsaCurve& curve,
    const OTPassword& seed,
    const Path& path) const
{
    Key output{OTPassword{}, OTPassword{}, Data::Factory(), {}, 0};
    auto& [privateKey, chainCode, publicKey, pathOut, parent] = output;
    const auto pNode = derive_child(hash, curve, seed, path, parent);

    if (false == bool(pNode)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive child.").Flush();
        privateKey = OTPassword{};
        chainCode = OTPassword{};
        publicKey = Data::Factory();
        pathOut = {};
        parent = 0;

        return output;
    }

    const auto& node = *pNode;

    if (EcdsaCurve::secp256k1 == curve) {
        privateKey.setMemory(node.private_key, sizeof(node.private_key));
        publicKey->Assign(node.public_key, sizeof(node.public_key));
    } else {
        const auto expanded = sodium::ExpandSeed(
            {reinterpret_cast<const char*>(node.private_key),
             sizeof(node.private_key)},
            privateKey.WriteInto(OTPassword::Mode::Mem),
            publicKey->WriteInto());

        if (false == expanded) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to expand seed")
                .Flush();
            return output;
        }
    }

    chainCode.setMemory(node.chain_code, sizeof(node.chain_code));
    pathOut = path;

    return output;
}

std::unique_ptr<HDNode> Trezor::instantiate_node(
    const EcdsaCurve& curve,
    const OTPassword& seed)
{
    std::unique_ptr<HDNode> output;
    output.reset(new HDNode);

    OT_ASSERT_MSG(output, "Instantiation of HD node failed.");

    int result = ::hdnode_from_seed(
        static_cast<const std::uint8_t*>(seed.getMemory()),
        seed.getMemorySize(),
        ::SECP256K1_NAME,
        output.get());

    OT_ASSERT_MSG((1 == result), "Setup of HD node failed.");

    ::hdnode_fill_public_key(output.get());

    return output;
}

std::string Trezor::SeedToFingerprint(
    const EcdsaCurve& curve,
    const OTPassword& seed) const
{
    auto node = instantiate_node(curve, seed);

    if (node) {
        auto pubkey = Data::Factory(
            static_cast<void*>(node->public_key), sizeof(node->public_key));
        auto output = Identifier::Factory();
        output->CalculateDigest(pubkey);

        return output->str();
    }

    return "";
}
#endif  // OT_CRYPTO_WITH_BIP32
}  // namespace opentxs::crypto::implementation
#endif  // OT_CRYPTO_USING_TREZOR
