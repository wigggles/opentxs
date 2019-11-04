// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_LIBRARY_ASYMMETRICPROVIDER_HPP
#define OPENTXS_CRYPTO_LIBRARY_ASYMMETRICPROVIDER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace crypto
{
class AsymmetricProvider
{
public:
    OPENTXS_EXPORT static proto::AsymmetricKeyType CurveToKeyType(
        const EcdsaCurve& curve);
    OPENTXS_EXPORT static EcdsaCurve KeyTypeToCurve(
        const proto::AsymmetricKeyType& type);

    OPENTXS_EXPORT virtual bool Sign(
        const api::internal::Core& api,
        const Data& plaintext,
        const key::Asymmetric& theKey,
        const proto::HashType hashType,
        Data& signature,  // output
        const PasswordPrompt& reason,
        const OTPassword* exportPassword = nullptr) const = 0;
    OPENTXS_EXPORT virtual bool SignContract(
        const api::internal::Core& api,
        const String& strContractUnsigned,
        const key::Asymmetric& theKey,
        Signature& theSignature,  // output
        const proto::HashType hashType,
        const PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual bool Verify(
        const Data& plaintext,
        const key::Asymmetric& theKey,
        const Data& signature,
        const proto::HashType hashType,
        const PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual bool VerifyContractSignature(
        const String& strContractToVerify,
        const key::Asymmetric& theKey,
        const Signature& theSignature,
        const proto::HashType hashType,
        const PasswordPrompt& reason) const = 0;

    OPENTXS_EXPORT virtual ~AsymmetricProvider() = default;

protected:
    AsymmetricProvider() = default;

private:
    AsymmetricProvider(const AsymmetricProvider&) = delete;
    AsymmetricProvider(AsymmetricProvider&&) = delete;
    AsymmetricProvider& operator=(const AsymmetricProvider&) = delete;
    AsymmetricProvider& operator=(AsymmetricProvider&&) = delete;
};
}  // namespace crypto
}  // namespace opentxs
#endif
