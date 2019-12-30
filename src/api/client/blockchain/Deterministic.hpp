// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/Proto.hpp"

#include "BalanceNode.hpp"

#include <atomic>

namespace opentxs::api::client::blockchain::implementation
{
class Deterministic : virtual public internal::Deterministic, public BalanceNode
{
public:
#if OT_CRYPTO_WITH_BIP32
    std::optional<Bip32Index> GenerateNext(
        const Subchain type,
        const PasswordPrompt& reason) const noexcept final;
#endif  // OT_CRYPTO_WITH_BIP32
    std::optional<Bip32Index> LastGenerated(const Subchain type) const
        noexcept final;
    std::optional<Bip32Index> LastUsed(const Subchain type) const
        noexcept final;
    proto::HDPath Path() const noexcept final { return path_; }
#if OT_CRYPTO_WITH_BIP32
    HDKey RootNode(const PasswordPrompt& reason) const noexcept final;
    std::optional<Bip32Index> UseNext(
        const Subchain type,
        const PasswordPrompt& reason,
        const Identifier& contact,
        const std::string& label) const noexcept final;
#endif  // OT_CRYPTO_WITH_BIP32

    ~Deterministic() override = default;

protected:
    using IndexMap = std::map<Subchain, Bip32Index>;

    static const Bip32Index Lookahead{5};
    static const Bip32Index MaxIndex{2147483648};

    const proto::HDPath path_;
#if OT_CRYPTO_WITH_BIP32
    HDKey key_;
#endif  // OT_CRYPTO_WITH_BIP32
    mutable IndexMap generated_;
    mutable IndexMap used_;

#if OT_CRYPTO_WITH_BIP32
    static HDKey instantiate_key(
        const api::internal::Core& api,
        proto::HDPath& path);
#endif  // OT_CRYPTO_WITH_BIP32

    std::optional<Bip32Index> bump_generated(
        const Lock& lock,
        const Subchain type) const noexcept
    {
        return bump(lock, type, generated_);
    }
    std::optional<Bip32Index> bump_used(const Lock& lock, const Subchain type)
        const noexcept
    {
        return bump(lock, type, used_);
    }
#if OT_CRYPTO_WITH_BIP32
    void check_lookahead(
        const Lock& lock,
        const Subchain type,
        const PasswordPrompt& reason) const noexcept(false);
    bool need_lookahead(const Lock& lock, const Subchain type) const noexcept;
    std::optional<Bip32Index> use_next(
        const Lock& lock,
        const Subchain type,
        const PasswordPrompt& reason,
        const Identifier& contact,
        const std::string& label) const noexcept;
#endif  // OT_CRYPTO_WITH_BIP32

    Deterministic(
        const internal::BalanceTree& parent,
        const BalanceNodeType type,
        const OTIdentifier id,
        std::vector<Activity> unspent,
        std::vector<Activity> spent,
        const proto::HDPath path,
        IndexMap generated,
        IndexMap used) noexcept;

private:
    std::optional<Bip32Index> bump(
        const Lock& lock,
        const Subchain type,
        IndexMap map) const noexcept;
#if OT_CRYPTO_WITH_BIP32
    virtual Bip32Index generate_next(
        const Lock& lock,
        const Subchain type,
        const PasswordPrompt& reason) const noexcept(false) = 0;
#endif  // OT_CRYPTO_WITH_BIP32
    virtual void set_metadata(
        const Lock& lock,
        const Subchain subchain,
        const Bip32Index index,
        const Identifier& contact,
        const std::string& label) const noexcept = 0;

    Deterministic() = delete;
    Deterministic(const Deterministic&) = delete;
    Deterministic(Deterministic&&) = delete;
    Deterministic& operator=(const Deterministic&) = delete;
    Deterministic& operator=(Deterministic&&) = delete;
};
}  // namespace opentxs::api::client::blockchain::implementation
