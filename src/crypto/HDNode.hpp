// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/Bytes.hpp"

namespace opentxs::crypto::implementation
{
class HDNode
{
    OTPassword data_space_;
    OTPassword hash_space_;

public:
    WritableView data_;
    WritableView hash_;

    auto Fingerprint() const noexcept -> Bip32Fingerprint;
    auto ParentCode() const noexcept -> ReadView;
    auto ParentPrivate() const noexcept -> ReadView;
    auto ParentPublic() const noexcept -> ReadView;

    auto ChildCode() noexcept -> WritableView;
    auto ChildPrivate() noexcept -> AllocateOutput;
    auto ChildPublic() noexcept -> AllocateOutput;

    auto InitCode() noexcept -> AllocateOutput;
    auto InitPrivate() noexcept -> AllocateOutput;
    auto InitPublic() noexcept -> AllocateOutput;

    auto Next() noexcept -> void;

    HDNode(const api::Crypto& crypto) noexcept;

private:
    const api::Crypto& crypto_;
    int switch_;
    OTPassword a_;
    OTPassword b_;

    auto parent() const noexcept -> const OTPassword&;

    auto child() noexcept -> OTPassword&;
    auto parent() noexcept -> OTPassword&;
};
}  // namespace opentxs::crypto::implementation
