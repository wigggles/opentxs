// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Bytes.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/protobuf/Enums.pb.h"

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
namespace key
{
class Asymmetric;
}  // namespace key
}  // namespace crypto

class PasswordPrompt;
class Signature;
class String;
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
class AsymmetricProvider : virtual public crypto::AsymmetricProvider
{
public:
    bool SeedToCurveKey(
        const ReadView seed,
        const AllocateOutput privateKey,
        const AllocateOutput publicKey) const noexcept final;
    bool SignContract(
        const api::internal::Core& api,
        const String& strContractUnsigned,
        const key::Asymmetric& theKey,
        Signature& theSignature,  // output
        const proto::HashType hashType,
        const PasswordPrompt& reason) const override;
    bool VerifyContractSignature(
        const String& strContractToVerify,
        const key::Asymmetric& theKey,
        const Signature& theSignature,
        const proto::HashType hashType) const override;

    ~AsymmetricProvider() override = default;

protected:
    AsymmetricProvider() noexcept;

private:
    AsymmetricProvider(const AsymmetricProvider&) = delete;
    AsymmetricProvider(AsymmetricProvider&&) = delete;
    AsymmetricProvider& operator=(const AsymmetricProvider&) = delete;
    AsymmetricProvider& operator=(AsymmetricProvider&&) = delete;
};
}  // namespace opentxs::crypto::implementation
