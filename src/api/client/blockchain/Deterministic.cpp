// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/PasswordPrompt.hpp"

#include "internal/api/client/blockchain/Blockchain.hpp"

#include "Deterministic.hpp"

#define OT_METHOD                                                              \
    "opentxs::api::client::blockchain::implementation::Deterministic::"

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
    , key_(instantiate_key(api_, const_cast<proto::HDPath&>(path_)))
    , generated_(generated)
    , used_(used)
{
}

std::optional<Bip32Index> Deterministic::bump(
    const Lock& lock,
    const Subchain type,
    IndexMap map) const noexcept
{
    try {

        return map.at(type)++;
    } catch (...) {

        return {};
    }
}

void Deterministic::check_lookahead(
    const Lock& lock,
    const Subchain type,
    const PasswordPrompt& reason) const noexcept(false)
{
    while (need_lookahead(lock, type)) { generate_next(lock, type, reason); }
}

std::optional<Bip32Index> Deterministic::GenerateNext(
    const Subchain type,
    const PasswordPrompt& reason) const noexcept
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

HDKey Deterministic::instantiate_key(const api::Core& api, proto::HDPath& path)
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

std::optional<Bip32Index> Deterministic::LastGenerated(
    const Subchain type) const noexcept
{
    Lock lock(lock_);

    try {
        auto output = generated_.at(type);

        return (0 == output) ? std::optional<Bip32Index>{} : output - 1;
    } catch (...) {

        return {};
    }
}

std::optional<Bip32Index> Deterministic::LastUsed(const Subchain type) const
    noexcept
{
    Lock lock(lock_);

    try {
        auto output = used_.at(type);

        return (0 == output) ? std::optional<Bip32Index>{} : output - 1;
    } catch (...) {

        return {};
    }
}

bool Deterministic::need_lookahead(const Lock& lock, const Subchain type) const
    noexcept
{
    return Lookahead > (generated_.at(type) - used_.at(type));
}

HDKey Deterministic::RootNode(const PasswordPrompt& reason) const noexcept
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

std::optional<Bip32Index> Deterministic::UseNext(
    const Subchain type,
    const PasswordPrompt& reason,
    const Identifier& contact,
    const std::string& label) const noexcept
{
    Lock lock(lock_);

    return use_next(lock, type, reason, contact, label);
}

std::optional<Bip32Index> Deterministic::use_next(
    const Lock& lock,
    const Subchain type,
    const PasswordPrompt& reason,
    const Identifier& contact,
    const std::string& label) const noexcept
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
}  // namespace opentxs::api::client::blockchain::implementation
