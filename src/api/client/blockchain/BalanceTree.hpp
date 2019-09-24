// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::client::blockchain::implementation
{
class BalanceTree final : public internal::BalanceTree
{
public:
    const api::Core& API() const noexcept { return api_; }
    bool AssociateTransaction(
        const std::vector<Activity>& unspent,
        const std::vector<Activity>& spent,
        std::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept final;
    opentxs::blockchain::Type Chain() const noexcept final { return chain_; }
    void ClaimAccountID(const std::string& id, internal::BalanceNode* node)
        const noexcept final;
    const HDAccounts& GetHD() const noexcept final { return hd_; }
    const ImportedAccounts& GetImported() const noexcept final
    {
        return imported_;
    }
    const PaymentCodeAccounts& GetPaymentCode() const noexcept final
    {
        return payment_code_;
    }
    const blockchain::internal::HD& HDChain(const Identifier& account) const
        noexcept(false) final
    {
        return hd_.at(account);
    }
    std::optional<std::pair<Key, Amount>> LookupUTXO(const Coin& coin) const
        noexcept final;
    internal::BalanceNode& Node(const Identifier& id) const
        noexcept(false) final;
    const identifier::Nym& NymID() const noexcept final { return nym_id_; }
    const internal::BalanceList& Parent() const noexcept final
    {
        return parent_;
    }

    bool AddHDNode(const proto::HDPath& path, Identifier& id) noexcept final
    {
        return hd_.Construct(path, id);
    }

    ~BalanceTree() final = default;

private:
    friend opentxs::Factory;

    template <typename InterfaceType, typename PayloadType>
    class NodeGroup final : virtual public InterfaceType
    {
    public:
        using const_iterator = typename InterfaceType::const_iterator;
        using value_type = typename InterfaceType::value_type;

        const value_type& at(const std::size_t position) const final
        {
            Lock lock(lock_);

            return *nodes_.at(position);
        }
        const PayloadType& at(const Identifier& id) const final
        {
            Lock lock(lock_);

            return *nodes_.at(index_.at(id));
        }
        const_iterator begin() const noexcept final
        {
            return const_iterator(this, 0);
        }
        const_iterator cbegin() const noexcept final
        {
            return const_iterator(this, 0);
        }
        const_iterator cend() const noexcept final
        {
            return const_iterator(this, nodes_.size());
        }
        const_iterator end() const noexcept final
        {
            return const_iterator(this, nodes_.size());
        }
        std::size_t size() const noexcept final { return nodes_.size(); }

        value_type& at(const std::size_t position)
        {
            Lock lock(lock_);

            return *nodes_.at(position);
        }
        PayloadType& at(const Identifier& id)
        {
            Lock lock(lock_);

            return *nodes_.at(index_.at(id));
        }
        template <typename ArgumentType>
        bool Construct(const ArgumentType& data, Identifier& id) noexcept
        {
            Lock lock(lock_);

            return construct(lock, data, id);
        }

        NodeGroup(BalanceTree& parent) noexcept
            : parent_(parent)
            , lock_()
            , nodes_()
            , index_()
        {
        }

    private:
        BalanceTree& parent_;
        mutable std::mutex lock_;
        std::vector<std::unique_ptr<PayloadType>> nodes_;
        std::map<OTIdentifier, std::size_t> index_;

        bool add(
            const Lock& lock,
            const Identifier& id,
            std::unique_ptr<PayloadType> node) noexcept;
        template <typename ArgumentType>
        bool construct(
            const Lock& lock,
            const ArgumentType& data,
            Identifier& id) noexcept
        {
            std::unique_ptr<PayloadType> node{
                Factory<PayloadType, ArgumentType>::get(parent_, data, id)};

            if (false == bool(node)) { return false; }

            if (0 < index_.count(id)) {
                LogOutput("Blockchain account ")(id)(" already exists").Flush();

                return false;
            }

            return add(lock, id, std::move(node));
        }
    };

    template <typename ReturnType, typename ArgumentType>
    struct Factory {
        static ReturnType* get(
            const BalanceTree& parent,
            const ArgumentType& data,
            Identifier& id) noexcept;
    };

    struct NodeIndex {
        internal::BalanceNode* Find(const std::string& id) const noexcept;

        void Add(const std::string& id, internal::BalanceNode* node) noexcept;

        NodeIndex() noexcept
            : lock_()
            , index_()
        {
        }

    private:
        mutable std::mutex lock_;
        std::map<std::string, internal::BalanceNode*> index_;
    };

    using HDNodes = NodeGroup<HDAccounts, internal::HD>;
    using ImportedNodes = NodeGroup<ImportedAccounts, internal::Imported>;
    using PaymentCodeNodes =
        NodeGroup<PaymentCodeAccounts, internal::PaymentCode>;

    const api::Core& api_;
    const internal::BalanceList& parent_;
    const opentxs::blockchain::Type chain_;
    const OTNymID nym_id_;
    HDNodes hd_;
    ImportedNodes imported_;
    PaymentCodeNodes payment_code_;
    mutable NodeIndex node_index_;
    mutable std::mutex lock_;
    mutable internal::ActivityMap unspent_;
    mutable internal::ActivityMap spent_;

    void init(const std::set<OTIdentifier>& HDAccounts) noexcept;

    BalanceTree(
        const internal::BalanceList& parent,
        const identifier::Nym& nym,
        const std::set<OTIdentifier>& accounts) noexcept;
    BalanceTree() = delete;
    BalanceTree(const BalanceTree&) = delete;
    BalanceTree(BalanceTree&&) = delete;
    BalanceTree& operator=(const BalanceTree&) = delete;
    BalanceTree& operator=(BalanceTree&&) = delete;
};
}  // namespace opentxs::api::client::blockchain::implementation
