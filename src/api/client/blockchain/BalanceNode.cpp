// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "api/client/blockchain/BalanceNode.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/HD.hpp"  // IWYU pragma: keep

// #define OT_METHOD
// "opentxs::api::client::blockchain::implementation::BalanceNode::"

namespace opentxs::api::client::blockchain::implementation
{
BalanceNode::BalanceNode(
    const internal::BalanceTree& parent,
    const BalanceNodeType type,
    const OTIdentifier id,
    std::vector<Activity> unspent,
    std::vector<Activity> spent) noexcept
    : api_(parent.API())
    , parent_(parent)
    , chain_(parent.Chain())
    , type_(type)
    , id_(id)
    , lock_()
    , unspent_(convert(unspent))
    , spent_(convert(spent))
{
}

BalanceNode::Element::Element(
    const internal::BalanceNode& parent,
    const client::internal::Blockchain& api,
    const opentxs::blockchain::Type chain,
    const VersionNumber version,
    const blockchain::Subchain subchain,
    const Bip32Index index,
    const std::string label,
    const OTIdentifier contact,
    std::unique_ptr<opentxs::crypto::key::EllipticCurve> key) noexcept(false)
    : parent_(parent)
    , api_(api)
    , chain_(chain)
    , lock_()
    , version_(version)
    , subchain_(subchain)
    , index_(index)
    , label_(label)
    , contact_(contact)
    , pkey_(key->asPublicEC())
    , key_(*pkey_)
{
    if (false == bool(key_)) { throw std::runtime_error("No key provided"); }
}

BalanceNode::Element::Element(
    const internal::BalanceNode& parent,
    const client::internal::Blockchain& api,
    const opentxs::blockchain::Type chain,
    const blockchain::Subchain subchain,
    const Bip32Index index,
    std::unique_ptr<opentxs::crypto::key::HD> key) noexcept(false)
    : Element(
          parent,
          api,
          chain,
          DefaultVersion,
          subchain,
          index,
          "",
          Identifier::Factory(),
          std::move(key))
{
}

BalanceNode::Element::Element(
    const internal::BalanceNode& parent,
    const client::internal::Blockchain& api,
    const opentxs::blockchain::Type chain,
    const blockchain::Subchain subchain,
    const SerializedType& address) noexcept(false)
    : Element(
          parent,
          api,
          chain,
          address.version(),
          subchain,
          address.index(),
          address.label(),
          Identifier::Factory(address.contact()),
          instantiate(api.API(), address.key()))
{
}

auto BalanceNode::Element::Address(const AddressStyle format) const noexcept
    -> std::string
{
    return api_.CalculateAddress(
        chain_, format, api_.API().Factory().Data(key_.PublicKey()));
}

auto BalanceNode::Element::Contact() const noexcept -> OTIdentifier
{
    Lock lock(lock_);

    return contact_;
}

auto BalanceNode::Element::Elements() const noexcept -> std::set<OTData>
{
    auto output = std::set<OTData>{};
    Lock lock(lock_);
    auto pubkey = api_.API().Factory().Data(key_.PublicKey());

    try {
        output.emplace(api_.PubkeyHash(chain_, pubkey));
    } catch (...) {
        OT_FAIL;
    }

    return output;
}

auto BalanceNode::Element::IncomingTransactions() const noexcept
    -> std::set<std::string>
{
    return parent_.IncomingTransactions(
        blockchain::Key{parent_.ID().str(), subchain_, index_});
}

auto BalanceNode::Element::instantiate(
    const api::internal::Core& api,
    const proto::AsymmetricKey& serialized) noexcept(false)
    -> std::unique_ptr<opentxs::crypto::key::EllipticCurve>
{
    auto output = api.Asymmetric().InstantiateECKey(serialized);

    if (false == bool(output)) {
        throw std::runtime_error("Failed to construct key");
    }

    if (false == bool(*output)) { throw std::runtime_error("Wrong key type"); }

    return output->asPublicEC();
}

auto BalanceNode::Element::Key() const noexcept -> ECKey
{
    Lock lock(lock_);

    return pkey_;
}

auto BalanceNode::Element::Label() const noexcept -> std::string
{
    Lock lock(lock_);

    return label_;
}

auto BalanceNode::Element::PubkeyHash() const noexcept -> OTData
{
    const auto key = api_.API().Factory().Data(key_.PublicKey());

    return api_.PubkeyHash(chain_, key);
}

auto BalanceNode::Element::Serialize() const noexcept
    -> BalanceNode::Element::SerializedType
{
    auto serialized = key_.Serialize();

    OT_ASSERT(serialized);

    SerializedType output{};
    output.set_version((DefaultVersion > version_) ? DefaultVersion : version_);
    output.set_index(index_);
    output.set_label(label_);
    output.set_contact(contact_->str());
    *output.mutable_key() = *serialized;

    return output;
}

void BalanceNode::Element::SetContact(const Identifier& contact) noexcept
{
    Lock lock(lock_);
    contact_ = contact;
}

void BalanceNode::Element::SetLabel(const std::string& label) noexcept
{
    Lock lock(lock_);
    label_ = label;
}

void BalanceNode::Element::SetMetadata(
    const Identifier& contact,
    const std::string& label) noexcept
{
    Lock lock(lock_);
    contact_ = contact;
    label_ = label;
}

auto BalanceNode::AssociateTransaction(
    const std::vector<Activity>& unspent,
    const std::vector<Activity>& spent,
    std::set<OTIdentifier>& contacts,
    const PasswordPrompt& reason) const noexcept -> bool
{
    Lock lock(lock_);

    if (false == check_activity(lock, unspent, contacts, reason)) {

        return false;
    }

    for (const auto& [coin, key, value] : spent) {
        process_spent(lock, coin, key, value);
    }

    for (const auto& [coin, key, value] : unspent) {
        process_unspent(lock, coin, key, value);
    }

    return save(lock);
}

void BalanceNode::claim_element(
    const Lock& lock,
    const Data& element,
    const blockchain::Key key) const noexcept
{
    const auto& db = parent_.Parent().Parent().DB();
    const auto& nymID = parent_.NymID();
    const auto status = db.Lookup(nymID, element);

    for (const auto& txo : status) {
        const auto [coin, spent] = txo;

        if (spent) {
            process_spent(lock, coin, key, 0);
        } else {
            process_unspent(lock, coin, key, 0);
        }

        db.Claim(nymID, coin);
    }
}

auto BalanceNode::convert(Activity&& in) noexcept -> proto::BlockchainActivity
{
    const auto& [coin, key, value] = in;
    const auto& [txid, out] = coin;
    const auto& [account, chain, index] = key;
    proto::BlockchainActivity output{};
    output.set_version(ActivityVersion);
    output.set_txid(txid);
    output.set_output(out);
    output.set_amount(value);
    output.set_account(account);
    output.set_subchain(static_cast<std::uint32_t>(chain));
    output.set_index(index);

    return output;
}

auto BalanceNode::convert(const proto::BlockchainActivity& in) noexcept
    -> Activity
{
    Activity output{};
    auto& [coin, key, value] = output;
    auto& [txid, out] = coin;
    auto& [account, chain, index] = key;
    txid = in.txid();
    out = in.output();
    value = in.amount();
    account = in.account();
    chain = static_cast<Subchain>(in.subchain());
    index = in.index();

    return output;
}

auto BalanceNode::convert(const std::vector<Activity>& in) noexcept
    -> internal::ActivityMap
{
    auto output = internal::ActivityMap{};

    for (const auto& [coin, key, value] : in) {
        output.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(coin),
            std::forward_as_tuple(key, value));
    }

    return output;
}

auto BalanceNode::IncomingTransactions(const Key& element) const noexcept
    -> std::set<std::string>
{
    Lock lock(lock_);
    auto output = std::set<std::string>{};

    for (const auto& [coin, data] : unspent_) {
        const auto& [key, amount] = data;

        if (key == element) { output.emplace(coin.first); }
    }

    for (const auto& [coin, data] : spent_) {
        const auto& [key, amount] = data;

        if (key == element) { output.emplace(coin.first); }
    }

    return output;
}

void BalanceNode::init() noexcept { parent_.ClaimAccountID(id_->str(), this); }

// Due to asynchronous blockchain scanning, spends may be discovered out of
// order compared to receipts.
void BalanceNode::process_spent(
    const Lock& lock,
    const Coin& coin,
    const blockchain::Key key,
    const Amount value) const noexcept
{
    auto targetValue{value};

    if (0 < unspent_.count(coin)) {
        // Normal case
        targetValue = std::max(targetValue, unspent_.at(coin).second);
        unspent_.erase(coin);
    }

    // If the spend was found before the receipt, the value is not known
    spent_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(coin),
        std::forward_as_tuple(key, targetValue));
}

void BalanceNode::process_unspent(
    const Lock& lock,
    const Coin& coin,
    const blockchain::Key key,
    const Amount value) const noexcept
{
    if (0 == spent_.count(coin)) {
        // Normal case
        unspent_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(coin),
            std::forward_as_tuple(key, value));
    } else {
        // Spend was discovered out of order, so correct the value now
        auto& storedValue = spent_.at(coin).second;
        storedValue = std::max(storedValue, value);
    }
}

auto BalanceNode::SetContact(
    const Subchain type,
    const Bip32Index index,
    const Identifier& id) noexcept(false) -> bool
{
    Lock lock(lock_);

    try {
        auto& element = mutable_element(lock, type, index);
        element.SetContact(id);

        return true;
    } catch (...) {
        return false;
    }
}

auto BalanceNode::SetLabel(
    const Subchain type,
    const Bip32Index index,
    const std::string& label) noexcept(false) -> bool
{
    Lock lock(lock_);

    try {
        auto& element = mutable_element(lock, type, index);
        element.SetLabel(label);

        return true;
    } catch (...) {
        return false;
    }
}
}  // namespace opentxs::api::client::blockchain::implementation
