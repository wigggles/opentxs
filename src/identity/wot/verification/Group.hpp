// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::identity::wot::verification::implementation
{
class Group final : public internal::Group
{
public:
    operator SerializedType() const noexcept final;

    const api::internal::Core& API() const noexcept final
    {
        return parent_.API();
    }
    /// Throws std::out_of_range for invalid position
    const value_type& at(const std::size_t position) const noexcept(false) final
    {
        return *nyms_.at(position);
    }
    const_iterator begin() const noexcept final { return cbegin(); }
    const_iterator cbegin() const noexcept final
    {
        return const_iterator(this, 0);
    }
    const_iterator cend() const noexcept final
    {
        return const_iterator(this, nyms_.size());
    }
    const_iterator end() const noexcept final { return cend(); }
    bool External() const noexcept { return external_; }
    const identifier::Nym& NymID() const noexcept { return parent_.NymID(); }
    std::size_t size() const noexcept final { return nyms_.size(); }
    bool UpgradeNymVersion(const VersionNumber version) noexcept final;
    VersionNumber Version() const noexcept final { return version_; }

    bool AddItem(
        const identifier::Nym& claimOwner,
        const Identifier& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const Item::Type value,
        const Time start,
        const Time end,
        const VersionNumber version) noexcept final;
    bool AddItem(
        const identifier::Nym& verifier,
        const Item::SerializedType verification) noexcept final;
    /// Throws std::out_of_range for invalid position
    value_type& at(const std::size_t position) noexcept(false) final
    {
        return *nyms_.at(position);
    }
    iterator begin() noexcept final { return iterator(this, 0); }
    bool DeleteItem(const Identifier& item) noexcept final;
    iterator end() noexcept final { return iterator(this, nyms_.size()); }
    void Register(
        const Identifier& id,
        const identifier::Nym& nym) noexcept final;
    void Unregister(const Identifier& id) noexcept final;

    ~Group() final = default;

private:
    friend opentxs::Factory;

    using Vector = std::vector<std::unique_ptr<internal::Nym>>;

    internal::Set& parent_;
    const VersionNumber version_;
    const bool external_;
    Vector nyms_;
    std::map<OTIdentifier, OTNymID> map_;

    static Vector instantiate(
        internal::Group& parent,
        const SerializedType& serialized) noexcept;

    internal::Nym& get_nym(const identifier::Nym& nym) noexcept;

    Group(
        internal::Set& parent,
        bool external,
        const VersionNumber version = DefaultVersion) noexcept;
    Group(
        internal::Set& parent,
        const SerializedType& serialized,
        bool external) noexcept;
    Group() = delete;
    Group(const Group&) = delete;
    Group(Group&&) = delete;
    Group& operator=(const Group&) = delete;
    Group& operator=(Group&&) = delete;
};
}  // namespace opentxs::identity::wot::verification::implementation
