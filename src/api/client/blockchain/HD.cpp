// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "api/client/blockchain/HD.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "api/client/blockchain/Deterministic.hpp"
#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/api/client/blockchain/Factory.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/protobuf/BlockchainActivity.pb.h"
#include "opentxs/protobuf/BlockchainAddress.pb.h"
#include "opentxs/protobuf/HDPath.pb.h"

#define OT_METHOD "opentxs::api::client::blockchain::implementation::HD::"

namespace opentxs::factory
{
using ReturnType = api::client::blockchain::implementation::HD;

auto BlockchainHDBalanceNode(
    const api::internal::Core& api,
    const api::client::blockchain::internal::BalanceTree& parent,
    const proto::HDPath& path,
    Identifier& id) noexcept
    -> std::unique_ptr<api::client::blockchain::internal::HD>
{
    auto reason =
        api.Factory().PasswordPrompt("Creating a new blockchain account");

    try {
        return std::make_unique<ReturnType>(api, parent, path, reason, id);
    } catch (const std::exception& e) {
        LogVerbose("opentxs::Factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto BlockchainHDBalanceNode(
    const api::internal::Core& api,
    const api::client::blockchain::internal::BalanceTree& parent,
    const proto::HDAccount& serialized,
    Identifier& id) noexcept
    -> std::unique_ptr<api::client::blockchain::internal::HD>
{
    using ReturnType = api::client::blockchain::implementation::HD;
    auto reason = api.Factory().PasswordPrompt("Loading a blockchain account");

    try {
        return std::make_unique<ReturnType>(
            api, parent, serialized, reason, id);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return nullptr;
    }
}
}  // namespace opentxs::factory

namespace opentxs::api::client::blockchain::implementation
{
HD::HD(
    const api::internal::Core& api,
    const internal::BalanceTree& parent,
    const proto::HDPath& path,
    const PasswordPrompt& reason,
    Identifier& id) noexcept(false)
    : Deterministic(
          api,
          parent,
          BalanceNodeType::HD,
          Identifier::Factory(Translate(parent.Chain()), path),
          {},
          {},
          path,
          {{Subchain::Internal, 0}, {Subchain::External, 0}},
          {{Subchain::Internal, 0}, {Subchain::External, 0}})
    , version_(DefaultVersion)
    , revision_(0)
    , internal_addresses_()
    , external_addresses_()
{
    id.Assign(id_);
    Lock lock(lock_);
    const auto existing = api_.Storage().BlockchainAccountList(
        parent_.NymID().str(), Translate(chain_));

    if (0 < existing.count(id_->str())) {
        throw std::runtime_error("Account already exists");
    }

#if OT_CRYPTO_WITH_BIP32
    check_lookahead(lock, Subchain::Internal, reason);
    check_lookahead(lock, Subchain::External, reason);
#endif  // OT_CRYPTO_WITH_BIP32

    if (false == save(lock)) {
        throw std::runtime_error("Failed to save new account");
    }

    init();
}

HD::HD(
    const api::internal::Core& api,
    const internal::BalanceTree& parent,
    const SerializedType& serialized,
    const PasswordPrompt& reason,
    Identifier& id) noexcept(false)
    : Deterministic(
          api,
          parent,
          BalanceNodeType::HD,
          Identifier::Factory(serialized.id()),
          extract_incoming(serialized),
          extract_outgoing(serialized),
          serialized.path(),
          {{Subchain::Internal, serialized.internaladdress().size()},
           {Subchain::External, serialized.externaladdress().size()}},
          {{Subchain::Internal, serialized.internalindex()},
           {Subchain::External, serialized.externalindex()}})
    , version_(serialized.version())
    , revision_(serialized.revision())
    , internal_addresses_(extract_internal(
          api_,
          parent.Parent().Parent(),
          *this,
          chain_,
          serialized))
    , external_addresses_(extract_external(
          api_,
          parent.Parent().Parent(),
          *this,
          chain_,
          serialized))
{
    id.Assign(id_);

    if (Translate(serialized.type()) != chain_) {
        throw std::runtime_error("Wrong account type");
    }
}

auto HD::BalanceElement(const Subchain type, const Bip32Index index) const
    noexcept(false) -> const HD::Element&
{
    switch (type) {
        case Subchain::Internal: {
            return internal_addresses_.at(index);
        }
        case Subchain::External: {
            return external_addresses_.at(index);
        }
        default: {
            throw std::out_of_range("Invalid subchain");
        }
    }
}

auto HD::check_activity(
    const Lock& lock,
    const std::vector<Activity>& unspent,
    std::set<OTIdentifier>& contacts,
    const PasswordPrompt& reason) const noexcept -> bool
{
    try {
        const auto currentExternal = used_.at(Subchain::External) - 1;
        const auto currentInternal = used_.at(Subchain::Internal) - 1;
        auto targetExternal{currentExternal};
        auto targetInternal{currentInternal};

        for (const auto& [coin, key, value] : unspent) {
            const auto& [account, subchain, index] = key;

            if (Subchain::External == subchain) {
                targetExternal = std::max(targetExternal, index);

                try {
                    auto contact = external_addresses_.at(index).Contact();

                    if (false == contact->empty()) {
                        contacts.emplace(std::move(contact));
                    }
                } catch (...) {
                }
            } else if (Subchain::Internal == subchain) {
                targetInternal = std::max(targetInternal, index);
            } else {

                return false;
            }
        }

        const auto blank = api_.Factory().Identifier();
        const auto empty = std::string{};

#if OT_CRYPTO_WITH_BIP32
        for (auto i{currentExternal}; i < targetExternal; ++i) {
            use_next(lock, Subchain::External, reason, blank, empty);
        }

        for (auto i{currentInternal}; i < targetInternal; ++i) {
            use_next(lock, Subchain::Internal, reason, blank, empty);
        }
#endif  // OT_CRYPTO_WITH_BIP32

        OT_ASSERT(targetExternal < used_.at(Subchain::External));
        OT_ASSERT(targetInternal < used_.at(Subchain::Internal));

        return true;
    } catch (...) {

        return false;
    }
}

auto HD::extract_external(
    const api::internal::Core& api,
    const client::internal::Blockchain& blockchain,
    const internal::BalanceNode& parent,
    const opentxs::blockchain::Type chain,
    const SerializedType& in) noexcept(false) -> HD::AddressMap
{
    AddressMap output{};

    for (const auto& address : in.externaladdress()) {
        output.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(address.index()),
            std::forward_as_tuple(
                api, blockchain, parent, chain, Subchain::External, address));
    }

    return output;
}

auto HD::extract_incoming(const SerializedType& in) -> std::vector<Activity>
{
    std::vector<Activity> output{};

    for (const auto& activity : in.incoming()) {
        output.emplace_back(convert(activity));
    }

    return output;
}

auto HD::extract_internal(
    const api::internal::Core& api,
    const client::internal::Blockchain& blockchain,
    const internal::BalanceNode& parent,
    const opentxs::blockchain::Type chain,
    const SerializedType& in) noexcept(false) -> HD::AddressMap
{
    AddressMap output{};

    for (const auto& address : in.internaladdress()) {
        output.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(address.index()),
            std::forward_as_tuple(
                api, blockchain, parent, chain, Subchain::Internal, address));
    }

    return output;
}

auto HD::extract_outgoing(const SerializedType& in) -> std::vector<Activity>
{
    std::vector<Activity> output{};

    for (const auto& activity : in.outgoing()) {
        output.emplace_back(convert(activity));
    }

    return output;
}

#if OT_CRYPTO_WITH_BIP32
auto HD::generate_next(
    const Lock& lock,
    const Subchain type,
    const PasswordPrompt& reason) const noexcept(false) -> Bip32Index
{
    auto& index = generated_.at(type);

    if (MaxIndex <= index) { throw std::runtime_error("Account is full"); }

    auto pKey = api_.Seeds().AccountChildKey(
        path_,
        (Subchain::Internal == type) ? INTERNAL_CHAIN : EXTERNAL_CHAIN,
        index,
        reason);

    if (false == bool(pKey)) {
        throw std::runtime_error("Failed to generate key");
    }

    auto& addressMap = (Subchain::Internal == type) ? internal_addresses_
                                                    : external_addresses_;
    const auto [it, added] = addressMap.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(index),
        std::forward_as_tuple(
            api_,
            parent_.Parent().Parent(),
            *this,
            chain_,
            type,
            index,
            std::move(pKey)));

    if (false == added) { throw std::runtime_error("Failed to add key"); }

#if OT_BLOCKCHAIN
    parent_.Parent().Parent().KeyGenerated(chain_);
#endif  // OT_BLOCKCHAIN

    return index++;
}
#endif  // OT_CRYPTO_WITH_BIP32

auto HD::Key(const Subchain type, const Bip32Index index) const noexcept
    -> ECKey
{
    try {
        switch (type) {
            case Subchain::Internal: {

                return internal_addresses_.at(index).Key();
            }
            case Subchain::External: {

                return external_addresses_.at(index).Key();
            }
            default: {
                return nullptr;
            }
        }
    } catch (...) {

        return nullptr;
    }
}

auto HD::mutable_element(
    const Lock& lock,
    const Subchain type,
    const Bip32Index index) noexcept(false) -> internal::BalanceElement&
{
    switch (type) {
        case Subchain::Internal: {
            return internal_addresses_.at(index);
        }
        case Subchain::External: {
            return external_addresses_.at(index);
        }
        default: {
            throw std::out_of_range("Invalid subchain");
        }
    }
}

#if OT_CRYPTO_WITH_BIP32
auto HD::PrivateKey(
    const Subchain type,
    const Bip32Index index,
    const PasswordPrompt& reason) const noexcept -> ECKey
{
    return api_.Seeds().AccountChildKey(
        path_,
        (Subchain::Internal == type) ? INTERNAL_CHAIN : EXTERNAL_CHAIN,
        index,
        reason);
}
#endif  // OT_CRYPTO_WITH_BIP32

auto HD::save(const Lock& lock) const noexcept -> bool
{
    const auto type = Translate(chain_);
    SerializedType serialized{};
    serialized.set_version(version_);
    serialized.set_id(id_->str());
    serialized.set_type(type);
    serialized.set_revision(++revision_);
    *serialized.mutable_path() = path_;
    serialized.set_internalindex(used_.at(Subchain::Internal));
    serialized.set_externalindex(used_.at(Subchain::External));

    for (const auto& [index, address] : internal_addresses_) {
        *serialized.add_internaladdress() = address.Serialize();
    }

    for (const auto& [index, address] : external_addresses_) {
        *serialized.add_externaladdress() = address.Serialize();
    }

    for (const auto& [coin, data] : unspent_) {
        auto converted = Activity{coin, data.first, data.second};
        *serialized.add_incoming() = convert(std::move(converted));
    }

    for (const auto& [coin, data] : spent_) {
        auto converted = Activity{coin, data.first, data.second};
        *serialized.add_outgoing() = convert(std::move(converted));
    }

    const bool saved =
        api_.Storage().Store(parent_.NymID().str(), type, serialized);

    if (false == saved) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save HD account.")
            .Flush();

        return false;
    }

    return saved;
}

void HD::set_metadata(
    const Lock& lock,
    const Subchain subchain,
    const Bip32Index index,
    const Identifier& contact,
    const std::string& label) const noexcept
{
    const auto blank = api_.Factory().Identifier();

    try {
        switch (subchain) {
            case Subchain::Internal: {
                // Change addresses do not get assigned to contacts
                internal_addresses_.at(index).SetMetadata(blank, label);
            } break;
            case Subchain::External: {
                external_addresses_.at(index).SetMetadata(contact, label);
            } break;
            default: {
            }
        }
    } catch (...) {
    }
}
}  // namespace opentxs::api::client::blockchain::implementation
