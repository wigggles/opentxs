// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::identity::wot::verification::implementation
{
class Item final : public internal::Item
{
public:
    operator SerializedType() const noexcept final;

    Time Begin() const noexcept final { return start_; }
    const Identifier& ClaimID() const noexcept final { return claim_; }
    Time End() const noexcept final { return end_; }
    const Identifier& ID() const noexcept final { return id_; }
    const proto::Signature& Signature() const noexcept final { return sig_; }
    Validity Valid() const noexcept final { return valid_; }
    Type Value() const noexcept final { return value_; }
    VersionNumber Version() const noexcept final { return version_; }

    ~Item() final = default;

private:
    friend opentxs::Factory;

    const VersionNumber version_;
    const OTIdentifier claim_;
    const Type value_;
    const Validity valid_;
    const Time start_;
    const Time end_;
    const OTIdentifier id_;
    const proto::Signature sig_;

    static OTIdentifier calculate_id(
        const api::internal::Core& api,
        const VersionNumber version,
        const Identifier& claim,
        const Type value,
        const Time start,
        const Time end,
        const Validity valid,
        const identifier::Nym& nym) noexcept(false);
    static proto::Signature get_sig(
        const identity::Nym& signer,
        const VersionNumber version,
        const Identifier& id,
        const Identifier& claim,
        const Type value,
        const Time start,
        const Time end,
        const Validity valid,
        const PasswordPrompt& reason) noexcept(false);
    static SerializedType id_form(
        const VersionNumber version,
        const Identifier& claim,
        const Type value,
        const Time start,
        const Time end,
        const Validity valid) noexcept;
    static SerializedType sig_form(
        const VersionNumber version,
        const Identifier& id,
        const Identifier& claim,
        const Type value,
        const Time start,
        const Time end,
        const Validity valid) noexcept;

    Item(
        const internal::Nym& parent,
        const Identifier& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const Type value = Type::Confirm,
        const Time start = {},
        const Time end = {},
        const VersionNumber version = DefaultVersion) noexcept(false);
    Item(
        const internal::Nym& parent,
        const SerializedType& serialized) noexcept(false);
    Item() = delete;
    Item(const Item&) = delete;
    Item(Item&&) = delete;
    Item& operator=(const Item&) = delete;
    Item& operator=(Item&&) = delete;
};
}  // namespace opentxs::identity::wot::verification::implementation
