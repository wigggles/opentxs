// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/crypto/library/AsymmetricProvider.hpp"

namespace opentxs::crypto::implementation
{
class AsymmetricProvider : virtual public crypto::AsymmetricProvider
{
public:
    bool SignContract(
        const api::Core& api,
        const String& strContractUnsigned,
        const key::Asymmetric& theKey,
        Signature& theSignature,  // output
        const proto::HashType hashType,
        const PasswordPrompt& reason) const override;
    bool VerifyContractSignature(
        const String& strContractToVerify,
        const key::Asymmetric& theKey,
        const Signature& theSignature,
        const proto::HashType hashType,
        const PasswordPrompt& reason) const override;

    ~AsymmetricProvider() override = default;

protected:
    AsymmetricProvider() = default;

private:
    AsymmetricProvider(const AsymmetricProvider&) = delete;
    AsymmetricProvider(AsymmetricProvider&&) = delete;
    AsymmetricProvider& operator=(const AsymmetricProvider&) = delete;
    AsymmetricProvider& operator=(AsymmetricProvider&&) = delete;
};
}  // namespace opentxs::crypto::implementation
