// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#define OPENTXS_TREZOR_PROVIDES_ECDSA OT_CRYPTO_SUPPORTED_KEY_SECP256K1

namespace opentxs::crypto::implementation
{
class Trezor final : virtual public crypto::Trezor,
                     virtual public EncodingProvider
#if OT_CRYPTO_WITH_BIP32
    ,
                     public Bip32
#endif
{
public:
    std::string Base58CheckEncode(
        const std::uint8_t* inputStart,
        const std::size_t& inputSize) const final;
    bool Base58CheckDecode(const std::string&& input, RawData& output)
        const final;
    bool RIPEMD160(
        const std::uint8_t* input,
        const size_t inputSize,
        std::uint8_t* output) const final;

#if OT_CRYPTO_WITH_BIP32
    Key DeriveKey(
        const api::crypto::Hash& hash,
        const EcdsaCurve& curve,
        const OTPassword& seed,
        const Path& path) const final;
    std::string SeedToFingerprint(
        const EcdsaCurve& curve,
        const OTPassword& seed) const final;
#endif

    ~Trezor() final = default;

private:
    friend opentxs::Factory;

    typedef bool DerivationMode;
    const DerivationMode DERIVE_PRIVATE = true;
    const DerivationMode DERIVE_PUBLIC = false;

    const std::uint8_t KeyMax[32]{
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFC, 0x2F};

    static std::string curve_name(const EcdsaCurve& curve);

#if OT_CRYPTO_WITH_BIP32
    static std::unique_ptr<HDNode> derive_child(
        const HDNode& parent,
        const Bip32Index index,
        const DerivationMode privateVersion);
    static std::unique_ptr<HDNode> instantiate_node(
        const EcdsaCurve& curve,
        const OTPassword& seed);

    std::unique_ptr<HDNode> derive_child(
        const api::crypto::Hash& hash,
        const EcdsaCurve& curve,
        const OTPassword& seed,
        const Path& path,
        Bip32Fingerprint& parentID) const;
#endif

    Trezor(const api::Crypto& crypto);
    Trezor() = delete;
    Trezor(const Trezor&) = delete;
    Trezor(Trezor&&) = delete;
    Trezor& operator=(const Trezor&) = delete;
    Trezor& operator=(Trezor&&) = delete;
};
}  // namespace opentxs::crypto::implementation
