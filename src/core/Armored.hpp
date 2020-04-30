// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>

#include "String.hpp"
#include "opentxs/core/Armored.hpp"

namespace opentxs
{
namespace OTDB
{
class OTPacker;
}  // namespace OTDB

namespace crypto
{
class Envelope;
}  // namespace crypto

class Data;
class Factory;
class String;
}  // namespace opentxs

namespace opentxs::implementation
{
class Armored : virtual public opentxs::Armored, public String
{
public:
    bool GetData(Data& theData, bool bLineBreaks = true) const override;
    bool GetString(opentxs::String& theData, bool bLineBreaks = true)
        const override;
    bool WriteArmoredString(
        opentxs::String& strOutput,
        const std::string str_type,
        bool bEscaped = false) const override;
    bool LoadFrom_ifstream(std::ifstream& fin) override;
    bool LoadFromExactPath(const std::string& filename) override;
    bool LoadFromString(
        opentxs::String& theStr,
        bool bEscaped = false,
        const std::string str_override = "-----BEGIN") override;
    bool SaveTo_ofstream(std::ofstream& fout) override;
    bool SaveToExactPath(const std::string& filename) override;
    bool SetData(const Data& theData, bool bLineBreaks = true) override;
    bool SetString(const opentxs::String& theData, bool bLineBreaks = true)
        override;

    ~Armored() override = default;

protected:
    Armored();

private:
    friend OTArmored;
    friend opentxs::Armored;
    friend opentxs::Factory;

    static std::unique_ptr<OTDB::OTPacker> s_pPacker;

    Armored* clone() const override;
    std::string compress_string(
        const std::string& str,
        std::int32_t compressionlevel) const;
    std::string decompress_string(const std::string& str) const;

    explicit Armored(const Data& theValue);
    explicit Armored(const opentxs::String& strValue);
    explicit Armored(const crypto::Envelope& theEnvelope);
    Armored(const Armored& strValue);

    Armored& operator=(const char* szValue);
    Armored& operator=(const Data& theValue);
    Armored& operator=(const opentxs::String& strValue);
    Armored& operator=(const Armored& strValue);
};
}  // namespace opentxs::implementation
