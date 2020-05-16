// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "api/client/blockchain/Deterministic.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <memory>
#include <utility>

#include "api/client/blockchain/BalanceNode.hpp"
#include "internal/api/Api.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/protobuf/Enums.pb.h"

#if OT_CRYPTO_WITH_BIP32
#define OT_METHOD                                                              \
    "opentxs::api::client::blockchain::implementation::Deterministic::"
#endif  // OT_CRYPTO_WITH_BIP32

namespace opentxs::api::client::blockchain::implementation
{
Deterministic::Deterministic(
    const internal::BalanceTree& parent,
    const BalanceNodeType type,
    const OTIdentifier id,
    std::vector<Activity> unspent,
    std::vector<Activity> spent,
    const proto::HDPath path,
    IndexMap generated,
    IndexMap used) noexcept
    : BalanceNode(parent, type, id, unspent, spent)
    , path_(path)
#if OT_CRYPTO_WITH_BIP32
    , key_(instantiate_key(api_, const_cast<proto::HDPath&>(path_)))
#endif  // OT_CRYPTO_WITH_BIP32
    , generated_(generated)
    , used_(used)
{
}

auto Deterministic::bump(const Lock& lock, const Subchain type, IndexMap map)
    const noexcept -> std::optional<Bip32Index>
{
    try {

        return map.at(type)++;
    } catch (...) {

        return {};
    }
}

#if OT_CRYPTO_WITH_BIP32
void Deterministic::check_lookahead(
    const Lock& lock,
    const Subchain type,
    const PasswordPrompt& reason) const noexcept(false)
{
    while (need_lookahead(lock, type)) { generate_next(lock, type, reason); }
}

auto Deterministic::GenerateNext(
    const Subchain type,
    const PasswordPrompt& reason) const noexcept -> std::optional<Bip32Index>
{
    Lock lock(lock_);

    if (0 == generated_.count(type)) { return {}; }

    try {
        auto output = generate_next(lock, type, reason);

        if (save(lock)) {

            return output;
        } else {

            return {};
        }
    } catch (...) {

        return {};
    }
}
#endif  // OT_CRYPTO_WITH_BIP32

#if OT_CRYPTO_WITH_BIP32
auto Deterministic::instantiate_key(
    const api::internal::Core& api,
    proto::HDPath& path) -> HDKey
{
    std::string fingerprint{path.root()};
    auto reason = api.Factory().PasswordPrompt("Loading account xpriv");
    api::HDSeed::Path children{};

    for (const auto& index : path.child()) { children.emplace_back(index); }

    auto pKey = api.Seeds().GetHDKey(
        fingerprint,
        EcdsaCurve::secp256k1,
        children,
        reason,
        proto::KEYROLE_SIGN,
        opentxs::crypto::key::EllipticCurve::MaxVersion);

    OT_ASSERT(pKey);

    path.set_root(fingerprint);

    return std::move(pKey);
}
#endif  // OT_CRYPTO_WITH_BIP32

auto Deterministic::LastGenerated(const Subchain type) const noexcept
    -> std::optional<Bip32Index>
{
    Lock lock(lock_);

    try {
        auto output = generated_.at(type);

        return (0 == output) ? std::optional<Bip32Index>{} : output - 1;
    } catch (...) {

        return {};
    }
}

auto Deterministic::LastUsed(const Subchain type) const noexcept
    -> std::optional<Bip32Index>
{
    Lock lock(lock_);

    try {
        auto output = used_.at(type);

        return (0 == output) ? std::optional<Bip32Index>{} : output - 1;
    } catch (...) {

        return {};
    }
}

#if OT_CRYPTO_WITH_BIP32
auto Deterministic::need_lookahead(const Lock& lock, const Subchain type) const
    noexcept -> bool
{
    return Lookahead > (generated_.at(type) - used_.at(type));
}

auto Deterministic::RootNode(const PasswordPrompt& reason) const noexcept
    -> HDKey
{
    auto fingerprint(path_.root());
    api::HDSeed::Path path{};

    for (const auto& child : path_.child()) { path.emplace_back(child); }

    return api_.Seeds().GetHDKey(
        fingerprint,
        EcdsaCurve::secp256k1,
        path,
        reason,
        proto::KEYROLE_SIGN,
        opentxs::crypto::key::EllipticCurve::MaxVersion);
}

auto Deterministic::UseNext(
    const Subchain type,
    const PasswordPrompt& reason,
    const Identifier& contact,
    const std::string& label) const noexcept -> std::optional<Bip32Index>
{
    Lock lock(lock_);

    return use_next(lock, type, reason, contact, label);
}

auto Deterministic::use_next(
    const Lock& lock,
    const Subchain type,
    const PasswordPrompt& reason,
    const Identifier& contact,
    const std::string& label) const noexcept -> std::optional<Bip32Index>
{
    try {
        auto& next = used_.at(type);

        if (MaxIndex < next) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Account is full").Flush();

            return {};
        }

        auto output = next++;
        check_lookahead(lock, type, reason);
        set_metadata(lock, type, output, contact, label);

        if (save(lock)) {

            return output;
        } else {

            return {};
        }
    } catch (...) {

        return {};
    }
}
#endif  // OT_CRYPTO_WITH_BIP32
}  // namespace opentxs::api::client::blockchain::implementation
