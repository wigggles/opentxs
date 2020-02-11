// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::crypto::implementation
{
class Encode final : virtual public api::crypto::Encode
{
public:
    std::string DataEncode(const std::string& input) const final;
    std::string DataEncode(const Data& input) const final;
    std::string DataDecode(const std::string& input) const final;
    std::string IdentifierEncode(const Data& input) const final;
    std::string IdentifierDecode(const std::string& input) const final;
    bool IsBase62(const std::string& str) const final;
    OTString Nonce(const std::uint32_t size) const final;
    OTString Nonce(const std::uint32_t size, Data& rawOutput) const final;
    std::string RandomFilename() const final;
    std::string SanatizeBase58(const std::string& input) const final;
    std::string SanatizeBase64(const std::string& input) const final;
    std::string Z85Encode(const Data& input) const final;
    std::string Z85Encode(const std::string& input) const final;
    OTData Z85Decode(const Data& input) const final;
    std::string Z85Decode(const std::string& input) const final;

    ~Encode() final = default;

private:
    friend opentxs::Factory;

    static const std::uint8_t LineWidth{72};

    const api::Crypto& crypto_;

    std::string Base64Encode(
        const std::uint8_t* inputStart,
        const std::size_t& inputSize) const;
    bool Base64Decode(const std::string&& input, RawData& output) const;
    std::string BreakLines(const std::string& input) const;
    std::string IdentifierEncode(const OTPassword& input) const;

    Encode(const api::Crypto& crypto);
    Encode() = delete;
    Encode(const Encode&) = delete;
    Encode& operator=(const Encode&) = delete;
};
}  // namespace opentxs::api::crypto::implementation
