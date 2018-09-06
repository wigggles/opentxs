// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::crypto::implementation
{
class Encode : virtual public api::crypto::Encode
{
public:
    std::string DataEncode(const std::string& input) const override;
    std::string DataEncode(const Data& input) const override;
    std::string DataDecode(const std::string& input) const override;
    std::string IdentifierEncode(const Data& input) const override;
    std::string IdentifierDecode(const std::string& input) const override;
    bool IsBase62(const std::string& str) const override;
    OTString Nonce(const std::uint32_t size) const override;
    OTString Nonce(const std::uint32_t size, Data& rawOutput) const override;
    std::string RandomFilename() const override;
    std::string SanatizeBase58(const std::string& input) const override;
    std::string SanatizeBase64(const std::string& input) const override;
    std::string Z85Encode(const Data& input) const override;
    std::string Z85Encode(const std::string& input) const override;
    OTData Z85Decode(const Data& input) const override;
    std::string Z85Decode(const std::string& input) const override;

    ~Encode() = default;

private:
    friend opentxs::Factory;

    static const std::uint8_t LineWidth{72};

    const opentxs::crypto::EncodingProvider& base58_;

    std::string Base64Encode(
        const std::uint8_t* inputStart,
        const std::size_t& inputSize) const;
    bool Base64Decode(const std::string&& input, RawData& output) const;
    std::string BreakLines(const std::string& input) const;
    std::string IdentifierEncode(const OTPassword& input) const;

    Encode(const opentxs::crypto::EncodingProvider& base58);
    Encode() = delete;
    Encode(const Encode&) = delete;
    Encode& operator=(const Encode&) = delete;
};
}  // namespace opentxs::api::crypto::implementation
