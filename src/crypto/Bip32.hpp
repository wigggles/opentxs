// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/crypto/Bip32.hpp"

#include <boost/endian/buffers.hpp>

#include "HDNode.hpp"

namespace be = boost::endian;

namespace opentxs::crypto::implementation
{
class Bip32 : virtual public opentxs::crypto::Bip32
{
public:
#if OT_CRYPTO_WITH_BIP32
    Key DeriveKey(
        const EcdsaCurve& curve,
        const OTPassword& seed,
        const Path& path) const final;
#endif  // OT_CRYPTO_WITH_BIP32
    bool DeserializePrivate(
        const std::string& serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        OTPassword& key) const final;
    bool DeserializePublic(
        const std::string& serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Data& key) const final;
    OTIdentifier SeedID(const ReadView entropy) const final;
    std::string SerializePrivate(
        const Bip32Network network,
        const Bip32Depth depth,
        const Bip32Fingerprint parent,
        const Bip32Index index,
        const Data& chainCode,
        const OTPassword& key) const final;
    std::string SerializePublic(
        const Bip32Network network,
        const Bip32Depth depth,
        const Bip32Fingerprint parent,
        const Bip32Index index,
        const Data& chainCode,
        const Data& key) const final;

    Bip32(const api::Crypto& crypto) noexcept;

private:
    const api::Crypto& crypto_;

    static auto IsHard(const Bip32Index) noexcept -> bool;

    auto ckd_private_hardened(
        const HDNode& node,
        const be::big_uint32_buf_t i,
        const WritableView& data) const noexcept -> void;
    auto ckd_private_normal(
        const HDNode& node,
        const be::big_uint32_buf_t i,
        const WritableView& data) const noexcept -> void;
    auto decode(const std::string& serialized) const noexcept -> OTData;
    auto extract(
        const Data& input,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode) const noexcept -> bool;
    auto provider(const EcdsaCurve& curve) const noexcept(false)
        -> const crypto::EcdsaProvider&;
#if OT_CRYPTO_WITH_BIP32
    auto root_node(
        const EcdsaCurve& curve,
        const ReadView entropy,
        const AllocateOutput privateKey,
        const AllocateOutput code,
        const AllocateOutput publicKey) const noexcept -> bool;
#endif  // OT_CRYPTO_WITH_BIP32

    Bip32() = delete;
    Bip32(const Bip32&) = delete;
    Bip32(Bip32&&) = delete;
    Bip32& operator=(const Bip32&) = delete;
    Bip32& operator=(Bip32&&) = delete;
};
}  // namespace opentxs::crypto::implementation
