// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/crypto/library/AsymmetricProvider.hpp"

namespace opentxs::crypto::implementation
{
class AsymmetricProviderNull final : virtual public crypto::AsymmetricProvider
{
public:
    bool RandomKeypair(
        const AllocateOutput,
        const AllocateOutput,
        const proto::KeyRole,
        const NymParameters&,
        const AllocateOutput) const noexcept final
    {
        return false;
    }
    bool SeedToCurveKey(
        const ReadView,
        const AllocateOutput,
        const AllocateOutput) const noexcept final
    {
        return false;
    }
    bool SharedSecret(
        const key::Asymmetric&,
        const key::Asymmetric&,
        const PasswordPrompt&,
        OTPassword&) const noexcept final
    {
        return false;
    }
    bool Sign(
        const api::internal::Core&,
        const Data&,
        const key::Asymmetric&,
        const proto::HashType,
        Data&,
        const PasswordPrompt&,
        const OTPassword* = nullptr) const final
    {
        return false;
    }
    bool SignContract(
        const api::internal::Core&,
        const String&,
        const key::Asymmetric&,
        Signature&,  // output
        const proto::HashType,
        const PasswordPrompt&) const final
    {
        return false;
    }
    bool Verify(
        const Data&,
        const key::Asymmetric&,
        const Data&,
        const proto::HashType) const final
    {
        return false;
    }
    bool VerifyContractSignature(
        const String&,
        const key::Asymmetric&,
        const Signature&,
        const proto::HashType) const final
    {
        return false;
    }

    AsymmetricProviderNull() = default;
    ~AsymmetricProviderNull() final = default;

private:
    AsymmetricProviderNull(const AsymmetricProviderNull&) = delete;
    AsymmetricProviderNull(AsymmetricProviderNull&&) = delete;
    AsymmetricProviderNull& operator=(const AsymmetricProviderNull&) = delete;
    AsymmetricProviderNull& operator=(AsymmetricProviderNull&&) = delete;
};
}  // namespace opentxs::crypto::implementation
