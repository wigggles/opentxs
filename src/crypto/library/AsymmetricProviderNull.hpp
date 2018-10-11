// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/crypto/library/AsymmetricProvider.hpp"

namespace opentxs::crypto::implementation
{
class AsymmetricProviderNull final : virtual public crypto::AsymmetricProvider
{
public:
    bool Sign(
        const Data&,
        const key::Asymmetric&,
        const proto::HashType,
        Data& signature,
        const OTPasswordData* = nullptr,
        const OTPassword* = nullptr) const override
    {
        return false;
    }
    bool SignContract(
        const String&,
        const key::Asymmetric&,
        Signature&,  // output
        const proto::HashType,
        const OTPasswordData* = nullptr) const override
    {
        return false;
    }
    bool Verify(
        const Data&,
        const key::Asymmetric&,
        const Data&,
        const proto::HashType,
        const OTPasswordData* = nullptr) const override
    {
        return false;
    }
    bool VerifyContractSignature(
        const String&,
        const key::Asymmetric&,
        const Signature&,
        const proto::HashType,
        const OTPasswordData* = nullptr) const override
    {
        return false;
    }

    AsymmetricProviderNull() = default;
    ~AsymmetricProviderNull() = default;

private:
    AsymmetricProviderNull(const AsymmetricProviderNull&) = delete;
    AsymmetricProviderNull(AsymmetricProviderNull&&) = delete;
    AsymmetricProviderNull& operator=(const AsymmetricProviderNull&) = delete;
    AsymmetricProviderNull& operator=(AsymmetricProviderNull&&) = delete;
};
}  // namespace opentxs::crypto::implementation
