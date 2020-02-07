// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::identity::wot::verification::implementation
{
class Nym final : public internal::Nym
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
        return *items_.at(position);
    }
    const_iterator begin() const noexcept final { return cbegin(); }
    const_iterator cbegin() const noexcept final
    {
        return const_iterator(this, 0);
    }
    const_iterator cend() const noexcept final
    {
        return const_iterator(this, items_.size());
    }
    const_iterator end() const noexcept final { return cend(); }
    const identifier::Nym& ID() const noexcept final { return id_; }
    const identifier::Nym& NymID() const noexcept
    {
        return parent_.External() ? id_.get() : parent_.NymID();
    }
    std::size_t size() const noexcept final { return items_.size(); }
    VersionNumber Version() const noexcept final { return version_; }

    bool AddItem(
        const Identifier& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const Item::Type value,
        const Time start,
        const Time end,
        const VersionNumber version) noexcept final;
    bool AddItem(const Item::SerializedType item) noexcept final;
    bool DeleteItem(const Identifier& item) noexcept final;
    bool UpgradeItemVersion(
        const VersionNumber itemVersion,
        VersionNumber& nymVersion) noexcept final;

    ~Nym() final = default;

private:
    friend opentxs::Factory;

    using Child = std::unique_ptr<internal::Item>;
    using Vector = std::vector<Child>;

    enum class Match { Accept, Reject, Replace };

    internal::Group& parent_;
    const VersionNumber version_;
    const OTNymID id_;
    Vector items_;

    static Vector instantiate(
        internal::Nym& parent,
        const SerializedType& serialized) noexcept;
    static Match match(
        const internal::Item& lhs,
        const internal::Item& rhs) noexcept;

    bool add_item(Child pCandidate) noexcept;

    Nym(internal::Group& parent,
        const identifier::Nym& nym,
        const VersionNumber version = DefaultVersion) noexcept;
    Nym(internal::Group& parent, const SerializedType& serialized) noexcept;
    Nym() = delete;
    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    Nym& operator=(const Nym&) = delete;
    Nym& operator=(Nym&&) = delete;
};
}  // namespace opentxs::identity::wot::verification::implementation
