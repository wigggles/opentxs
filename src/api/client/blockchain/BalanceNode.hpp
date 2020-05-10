// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "internal/api/client/blockchain/Blockchain.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/blockchain/BalanceNode.hpp"
#include "opentxs/api/client/blockchain/BalanceTree.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace crypto
{
namespace key
{
class EllipticCurve;
class HD;
}  // namespace key
}  // namespace crypto

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class AsymmetricKey;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::client::blockchain::implementation
{
class BalanceNode : virtual public internal::BalanceNode
{
public:
    bool AssociateTransaction(
        const std::vector<Activity>& unspent,
        const std::vector<Activity>& outgoing,
        std::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept final;
    const Identifier& ID() const noexcept final { return id_; }
    std::set<std::string> IncomingTransactions(const Key& key) const
        noexcept final;
    const internal::BalanceTree& Parent() const noexcept final
    {
        return parent_;
    }
    bool SetContact(
        const Subchain type,
        const Bip32Index index,
        const Identifier& id) noexcept(false) final;
    bool SetLabel(
        const Subchain type,
        const Bip32Index index,
        const std::string& label) noexcept(false) final;
    BalanceNodeType Type() const noexcept final { return type_; }

    ~BalanceNode() override = default;

protected:
    struct Element final : virtual public internal::BalanceElement {
        std::string Address(const AddressStyle format) const noexcept final;
        OTIdentifier Contact() const noexcept final;
        std::set<OTData> Elements() const noexcept final;
        const Identifier& ID() const noexcept final { return parent_.ID(); }
        std::set<std::string> IncomingTransactions() const noexcept final;
        Bip32Index Index() const noexcept final { return index_; }
        ECKey Key() const noexcept final;
        std::string Label() const noexcept final;
        const identifier::Nym& NymID() const noexcept final
        {
            return parent_.Parent().NymID();
        }
        const blockchain::BalanceNode& Parent() const noexcept final
        {
            return parent_;
        }
        OTData PubkeyHash() const noexcept final;
        SerializedType Serialize() const noexcept final;
        blockchain::Subchain Subchain() const noexcept final
        {
            return subchain_;
        }

        void SetContact(const Identifier& id) noexcept final;
        void SetLabel(const std::string& label) noexcept final;
        void SetMetadata(
            const Identifier& contact,
            const std::string& label) noexcept final;

        Element(
            const internal::BalanceNode& parent,
            const client::internal::Blockchain& api,
            const opentxs::blockchain::Type chain,
            const blockchain::Subchain subchain,
            const Bip32Index index,
            std::unique_ptr<opentxs::crypto::key::HD> key) noexcept(false);
        Element(
            const internal::BalanceNode& parent,
            const client::internal::Blockchain& api,
            const opentxs::blockchain::Type chain,
            const blockchain::Subchain subchain,
            const SerializedType& address) noexcept(false);
        ~Element() final = default;

    private:
        static const VersionNumber DefaultVersion{1};

        const internal::BalanceNode& parent_;
        const client::internal::Blockchain& api_;
        const opentxs::blockchain::Type chain_;
        mutable std::mutex lock_;
        const VersionNumber version_;
        const blockchain::Subchain subchain_;
        const Bip32Index index_;
        std::string label_;
        OTIdentifier contact_;
        std::shared_ptr<opentxs::crypto::key::EllipticCurve> pkey_;
        opentxs::crypto::key::EllipticCurve& key_;

        static std::unique_ptr<opentxs::crypto::key::EllipticCurve> instantiate(
            const api::internal::Core& api,
            const proto::AsymmetricKey& serialized) noexcept(false);

        Element(
            const internal::BalanceNode& parent,
            const client::internal::Blockchain& api,
            const opentxs::blockchain::Type chain,
            const VersionNumber version,
            const blockchain::Subchain subchain,
            const Bip32Index index,
            const std::string label,
            const OTIdentifier contact,
            std::unique_ptr<opentxs::crypto::key::EllipticCurve>
                key) noexcept(false);
        Element() = delete;
    };

    const api::internal::Core& api_;
    const internal::BalanceTree& parent_;
    const opentxs::blockchain::Type chain_;
    const BalanceNodeType type_;
    const OTIdentifier id_;
    mutable std::mutex lock_;
    mutable internal::ActivityMap unspent_;
    mutable internal::ActivityMap spent_;

    static proto::BlockchainActivity convert(Activity&& in) noexcept;
    static Activity convert(const proto::BlockchainActivity& in) noexcept;
    static internal::ActivityMap convert(
        const std::vector<Activity>& in) noexcept;

    void claim_element(
        const Lock& lock,
        const Data& element,
        const blockchain::Key key) const noexcept;
    void process_spent(
        const Lock& lock,
        const Coin& coin,
        const blockchain::Key key,
        const Amount value) const noexcept;
    void process_unspent(
        const Lock& lock,
        const Coin& coin,
        const blockchain::Key key,
        const Amount value) const noexcept;
    virtual bool save(const Lock& lock) const noexcept = 0;

    // NOTE call only from final constructor bodies
    void init() noexcept;
    virtual internal::BalanceElement& mutable_element(
        const Lock& lock,
        const Subchain type,
        const Bip32Index index) noexcept(false) = 0;

    BalanceNode(
        const internal::BalanceTree& parent,
        const BalanceNodeType type,
        const OTIdentifier id,
        std::vector<Activity> unspent,
        std::vector<Activity> spent) noexcept;

private:
    static const VersionNumber ActivityVersion{1};

    virtual bool check_activity(
        const Lock& lock,
        const std::vector<Activity>& unspent,
        std::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept = 0;

    BalanceNode() = delete;
    BalanceNode(const BalanceNode&) = delete;
    BalanceNode(BalanceNode&&) = delete;
    BalanceNode& operator=(const BalanceNode&) = delete;
    BalanceNode& operator=(BalanceNode&&) = delete;
};
}  // namespace opentxs::api::client::blockchain::implementation
