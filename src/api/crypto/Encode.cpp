// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/library/EncodingProvider.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/Types.hpp"

#include "base64/base64.h"

#include <iostream>
#include <regex>

#include "Encode.hpp"

namespace opentxs
{
api::crypto::Encode* Factory::Encode(const crypto::EncodingProvider& base58)
{
    return new api::crypto::implementation::Encode(base58);
}
}  // namespace opentxs

namespace opentxs::api::crypto::implementation
{
Encode::Encode(const opentxs::crypto::EncodingProvider& base58)
    : base58_(base58)
{
}

std::string Encode::Base64Encode(
    const std::uint8_t* inputStart,
    const std::size_t& size) const
{
    std::string output;
    output.resize(::Base64encode_len(size));
    ::Base64encode(
        const_cast<char*>(output.data()),
        reinterpret_cast<const char*>(inputStart),
        size);

    return BreakLines(output);
}

bool Encode::Base64Decode(const std::string&& input, RawData& output) const
{
    output.resize(::Base64decode_len(input.data()), 0x0);

    const size_t decoded =
        ::Base64decode(reinterpret_cast<char*>(output.data()), input.data());

    if (0 == decoded) { return false; }

    OT_ASSERT(decoded <= output.size());

    output.resize(decoded);

    return true;
}

std::string Encode::BreakLines(const std::string& input) const
{
    std::string output;

    if (0 == input.size()) { return output; }

    std::size_t width = 0;

    for (auto& character : input) {
        output.push_back(character);

        if (++width >= LineWidth) {
            output.push_back('\n');
            width = 0;
        }
    }

    if ('\n' != output.back()) { output.push_back('\n'); }

    return output;
}

std::string Encode::DataEncode(const std::string& input) const
{
    return Base64Encode(
        reinterpret_cast<const std::uint8_t*>(input.data()), input.size());
}

std::string Encode::DataEncode(const Data& input) const
{
    return Base64Encode(
        static_cast<const std::uint8_t*>(input.data()), input.size());
}

std::string Encode::DataDecode(const std::string& input) const
{
    RawData decoded;

    if (Base64Decode(SanatizeBase64(input), decoded)) {

        return std::string(
            reinterpret_cast<const char*>(decoded.data()), decoded.size());
    }

    return "";
}

std::string Encode::IdentifierEncode(const Data& input) const
{
    return base58_.Base58CheckEncode(
        static_cast<const std::uint8_t*>(input.data()), input.size());
}

std::string Encode::IdentifierEncode(const OTPassword& input) const
{
    if (input.isMemory()) {
        return base58_.Base58CheckEncode(
            static_cast<const std::uint8_t*>(input.getMemory()),
            input.getMemorySize());
    } else {
        return base58_.Base58CheckEncode(
            reinterpret_cast<const std::uint8_t*>(input.getPassword()),
            input.getPasswordSize());
    }
}

std::string Encode::IdentifierDecode(const std::string& input) const
{
    RawData decoded;

    if (base58_.Base58CheckDecode(SanatizeBase58(input), decoded)) {

        return std::string(
            reinterpret_cast<const char*>(decoded.data()), decoded.size());
    }

    return "";
}

bool Encode::IsBase62(const std::string& str) const
{
    return str.find_first_not_of("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHI"
                                 "JKLMNOPQRSTUVWXYZ") == std::string::npos;
}

OTString Encode::Nonce(const std::uint32_t size) const
{
    auto unusedOutput = Data::Factory();

    return Nonce(size, unusedOutput);
}

OTString Encode::Nonce(const std::uint32_t size, Data& rawOutput) const
{
    rawOutput.zeroMemory();
    rawOutput.SetSize(size);
    OTPassword source;
    source.randomizeMemory(size);
    auto nonce = String::Factory(IdentifierEncode(source));
    rawOutput.Assign(source.getMemory(), source.getMemorySize());

    return nonce;
}

std::string Encode::RandomFilename() const { return Nonce(16)->Get(); }

std::string Encode::SanatizeBase58(const std::string& input) const
{
    return std::regex_replace(input, std::regex("[^1-9A-HJ-NP-Za-km-z]"), "");
}

std::string Encode::SanatizeBase64(const std::string& input) const
{
    return std::regex_replace(input, std::regex("[^0-9A-Za-z+/=]"), "");
}

std::string Encode::Z85Encode(const Data& input) const
{
    return opentxs::network::zeromq::Context::RawToZ85(
        input.data(), input.size());
}

std::string Encode::Z85Encode(const std::string& input) const
{
    return opentxs::network::zeromq::Context::RawToZ85(
        input.data(), input.size());
}

OTData Encode::Z85Decode(const Data& input) const
{
    return opentxs::network::zeromq::Context::Z85ToRaw(
        input.data(), input.size());
}

std::string Encode::Z85Decode(const std::string& input) const
{
    const auto output =
        opentxs::network::zeromq::Context::Z85ToRaw(input.data(), input.size());

    return {static_cast<const char*>(output->data()), output->size()};
}
}  // namespace opentxs::api::crypto::implementation
