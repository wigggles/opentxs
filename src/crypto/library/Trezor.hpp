// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::crypto::implementation
{
class Trezor final : virtual public crypto::Trezor
#if OT_CRYPTO_WITH_BIP32
    ,
                     public Bip32
#endif
{
public:
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

#if OT_CRYPTO_WITH_BIP32
    static std::unique_ptr<HDNode> derive_child(
        const HDNode& parent,
        const Bip32Index index);
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
