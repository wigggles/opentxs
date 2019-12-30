// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::client::blockchain::implementation
{
class HD final : public internal::HD, public Deterministic
{
public:
    using Element = implementation::BalanceNode::Element;

    const Element& BalanceElement(const Subchain type, const Bip32Index index)
        const noexcept(false) final;
    ECKey Key(const Subchain type, const Bip32Index index) const noexcept final;

    ~HD() final = default;

private:
    friend opentxs::Factory;

    using AddressMap = std::map<Bip32Index, Element>;
    using Revision = std::uint64_t;
    using SerializedType = proto::HDAccount;

    static const VersionNumber DefaultVersion{1};

    VersionNumber version_;
    mutable std::atomic<Revision> revision_;
    mutable AddressMap internal_addresses_;
    mutable AddressMap external_addresses_;

    static AddressMap extract_external(
        const internal::BalanceNode& parent,
        const client::internal::Blockchain& api,
        const opentxs::blockchain::Type chain,
        const SerializedType& in) noexcept(false);
    static std::vector<Activity> extract_incoming(const SerializedType& in);
    static AddressMap extract_internal(
        const internal::BalanceNode& parent,
        const client::internal::Blockchain& api,
        const opentxs::blockchain::Type chain,
        const SerializedType& in) noexcept(false);
    static std::vector<Activity> extract_outgoing(const SerializedType& in);

    bool check_activity(
        const Lock& lock,
        const std::vector<Activity>& unspent,
        std::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept final;
    Bip32Index generate_next(
        const Lock& lock,
        const Subchain type,
        const PasswordPrompt& reason) const noexcept(false) final;
    internal::BalanceElement& mutable_element(
        const Lock& lock,
        const Subchain type,
        const Bip32Index index) noexcept(false) final;
    bool save(const Lock& lock) const noexcept final;
    void set_metadata(
        const Lock& lock,
        const Subchain subchain,
        const Bip32Index index,
        const Identifier& contact,
        const std::string& label) const noexcept final;

    HD(const internal::BalanceTree& parent,
       const proto::HDPath& path,
       const PasswordPrompt& reason,
       Identifier& id)
    noexcept(false);
    HD(const internal::BalanceTree& parent,
       const SerializedType& serialized,
       const PasswordPrompt& reason,
       Identifier& id)
    noexcept(false);
    HD(const HD&) = delete;
    HD(HD&&) = delete;
    HD& operator=(const HD&) = delete;
    HD& operator=(HD&&) = delete;
};
}  // namespace opentxs::api::client::blockchain::implementation
