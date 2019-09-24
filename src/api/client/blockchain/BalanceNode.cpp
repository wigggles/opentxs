// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/crypto/key/HD.hpp"

#include "internal/api/client/Client.hpp"

#include "BalanceNode.hpp"

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
    const Subchain subchain,
    const Bip32Index index,
    const std::string label,
    const OTIdentifier contact,
    std::unique_ptr<opentxs::crypto::key::EllipticCurve> key,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    : parent_(parent)
    , api_(api)
    , chain_(chain)
    , lock_()
    , version_(version)
    , subchain_(subchain)
    , index_(index)
    , label_(label)
    , contact_(contact)
    , pkey_(key->asPublic(reason))
    , key_(*pkey_)
{
    if (false == bool(key_)) { throw std::runtime_error("No key provided"); }
}

BalanceNode::Element::Element(
    const internal::BalanceNode& parent,
    const client::internal::Blockchain& api,
    const opentxs::blockchain::Type chain,
    const Subchain subchain,
    const Bip32Index index,
    std::unique_ptr<opentxs::crypto::key::HD> key,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    : Element(
          parent,
          api,
          chain,
          DefaultVersion,
          subchain,
          index,
          "",
          Identifier::Factory(),
          std::move(key),
          reason)
{
}

BalanceNode::Element::Element(
    const internal::BalanceNode& parent,
    const client::internal::Blockchain& api,
    const opentxs::blockchain::Type chain,
    const Subchain subchain,
    const SerializedType& address,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    : Element(
          parent,
          api,
          chain,
          address.version(),
          subchain,
          address.index(),
          address.label(),
          Identifier::Factory(address.contact()),
          instantiate(api.API(), address.key(), reason),
          reason)
{
}

std::string BalanceNode::Element::Address(
    const AddressStyle format,
    const PasswordPrompt& reason) const noexcept
{
    return api_.CalculateAddress(chain_, format, key_.PublicKey(reason));
}

OTIdentifier BalanceNode::Element::Contact() const noexcept
{
    Lock lock(lock_);

    return contact_;
}

std::set<OTData> BalanceNode::Element::Elements() const noexcept
{
    auto output = std::set<OTData>{};
    Lock lock(lock_);
    auto pubkey = Data::Factory();
    key_.GetKey(pubkey);

    try {
        output.emplace(api_.PubkeyHash(chain_, pubkey));
    } catch (...) {
        OT_FAIL;
    }

    return output;
}

std::set<std::string> BalanceNode::Element::IncomingTransactions() const
    noexcept
{
    return parent_.IncomingTransactions(
        blockchain::Key{parent_.ID().str(), subchain_, index_});
}

std::unique_ptr<opentxs::crypto::key::EllipticCurve> BalanceNode::Element::
    instantiate(
        const api::Core& api,
        const proto::AsymmetricKey& serialized,
        const opentxs::PasswordPrompt& reason) noexcept(false)
{
    auto output = api.Asymmetric().InstantiateECKey(serialized, reason);

    if (false == bool(output)) {
        throw std::runtime_error("Failed to construct key");
    }

    if (false == bool(*output)) { throw std::runtime_error("Wrong key type"); }

    return output->asPublic(reason);
}

ECKey BalanceNode::Element::Key() const noexcept
{
    Lock lock(lock_);

    return pkey_;
}

std::string BalanceNode::Element::Label() const noexcept
{
    Lock lock(lock_);

    return label_;
}

OTData BalanceNode::Element::PubkeyHash(const PasswordPrompt& reason) const
    noexcept
{
    const auto key = key_.PublicKey(reason);

    return api_.PubkeyHash(chain_, key);
}

BalanceNode::Element::SerializedType BalanceNode::Element::Serialize() const
    noexcept
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

bool BalanceNode::AssociateTransaction(
    const std::vector<Activity>& unspent,
    const std::vector<Activity>& spent,
    std::set<OTIdentifier>& contacts,
    const PasswordPrompt& reason) const noexcept
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

proto::BlockchainActivity BalanceNode::convert(Activity&& in) noexcept
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

Activity BalanceNode::convert(const proto::BlockchainActivity& in) noexcept
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

internal::ActivityMap BalanceNode::convert(
    const std::vector<Activity>& in) noexcept
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

std::set<std::string> BalanceNode::IncomingTransactions(
    const Key& element) const noexcept
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

bool BalanceNode::SetContact(
    const Subchain type,
    const Bip32Index index,
    const Identifier& id) noexcept(false)
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

bool BalanceNode::SetLabel(
    const Subchain type,
    const Bip32Index index,
    const std::string& label) noexcept(false)
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
