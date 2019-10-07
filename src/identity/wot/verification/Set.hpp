// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::identity::wot::verification::implementation
{
class Set final : public internal::Set
{
public:
    operator SerializedType() const noexcept final;

    const api::Core& API() const noexcept final { return api_; }
    const Group& External() const noexcept final { return *external_; }
    const Group& Internal() const noexcept final { return *internal_; }
    const identifier::Nym& NymID() const noexcept { return nym_id_; }
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
    bool DeleteItem(const Identifier& item) noexcept final;
    Group& External() noexcept final { return *external_; }
    Group& Internal() noexcept final { return *internal_; }
    void Register(const Identifier& id, const bool external) noexcept final;
    void Unregister(const Identifier& id) noexcept final;
    bool UpgradeGroupVersion(const VersionNumber groupVersion) noexcept final;

    ~Set() final = default;

private:
    friend opentxs::Factory;

    using GroupPointer = std::unique_ptr<internal::Group>;
    using ChildType = verification::Group::SerializedType;

    const api::Core& api_;
    const VersionNumber version_;
    const OTNymID nym_id_;
    GroupPointer internal_;
    GroupPointer external_;
    std::map<OTIdentifier, bool> map_;

    static GroupPointer instantiate(
        internal::Set& parent,
        const ChildType& serialized,
        bool external) noexcept;

    Set(const api::Core& api,
        const identifier::Nym& nym,
        const VersionNumber version = DefaultVersion) noexcept(false);
    Set(const api::Core& api,
        const identifier::Nym& nym,
        const SerializedType& serialized) noexcept(false);
    Set() = delete;
    Set(const Set&) = delete;
    Set(Set&&) = delete;
    Set& operator=(const Set&) = delete;
    Set& operator=(Set&&) = delete;
};
}  // namespace opentxs::identity::wot::verification::implementation
