// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/crypto/library/EcdsaProvider.hpp"

namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
class EcdsaProvider : virtual public crypto::EcdsaProvider
{
public:
    ~EcdsaProvider() override = default;

protected:
    const api::Crypto& crypto_;

    EcdsaProvider(const api::Crypto& crypto);

private:
    EcdsaProvider() = delete;
    EcdsaProvider(const EcdsaProvider&) = delete;
    EcdsaProvider(EcdsaProvider&&) = delete;
    EcdsaProvider& operator=(const EcdsaProvider&) = delete;
    EcdsaProvider& operator=(EcdsaProvider&&) = delete;
};
}  // namespace opentxs::crypto::implementation
