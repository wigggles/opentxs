// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::crypto::implementation
{
class Config final : public api::crypto::Config
{
public:
    std::uint32_t IterationCount() const override;
    std::uint32_t SymmetricSaltSize() const override;
    std::uint32_t SymmetricKeySize() const override;
    std::uint32_t SymmetricKeySizeMax() const override;
    std::uint32_t SymmetricIvSize() const override;
    std::uint32_t SymmetricBufferSize() const override;
    std::uint32_t PublicKeysize() const override;
    std::uint32_t PublicKeysizeMax() const override;

private:
    friend opentxs::Factory;

    const api::Settings& config_;
    mutable std::int32_t sp_nIterationCount{0};
    mutable std::int32_t sp_nSymmetricSaltSize{0};
    mutable std::int32_t sp_nSymmetricKeySize{0};
    mutable std::int32_t sp_nSymmetricKeySizeMax{0};
    mutable std::int32_t sp_nSymmetricIvSize{0};
    mutable std::int32_t sp_nSymmetricBufferSize{0};
    mutable std::int32_t sp_nPublicKeysize{0};
    mutable std::int32_t sp_nPublicKeysizeMax{0};

    bool GetSetAll() const;
    bool GetSetValue(
        const std::string& strKeyName,
        const std::int32_t nDefaultValue,
        std::int32_t& out_nValue) const;

    Config(const api::Settings& settings);
    Config() = delete;
    Config(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(const Config&) = delete;
    Config& operator=(Config&&) = delete;
};
}  // namespace opentxs::api::crypto::implementation
