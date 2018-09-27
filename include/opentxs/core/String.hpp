// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_STRING_HPP
#define OPENTXS_CORE_STRING_HPP

#include "opentxs/Forward.hpp"

#ifdef _WIN32
// support for changing between std::string and std::wstring
#include "util/win32_utf8conv.hpp"
#endif

#include <stddef.h>
#include <cstdarg>
#include <cstdint>
#include <iosfwd>
#include <list>
#include <utility>
#include <string>
#include <map>

#define MAX_STRING_LENGTH 0x800000  // this is about 8 megs.

#ifdef __GNUC__
#define ATTR_PRINTF(a, b) __attribute__((format(printf, a, b)))
#else
#define ATTR_PRINTF(a, b)
#endif

#ifdef _MSC_VER
#define PRI_SIZE "Iu"
#else
#define PRI_SIZE "zu"
#endif

namespace opentxs
{
class String
{
public:
    using List = std::list<std::string>;
    using Map = std::map<std::string, std::string>;

    EXPORT static opentxs::Pimpl<opentxs::String> Factory();
    EXPORT static opentxs::Pimpl<opentxs::String> Factory(const Armored& value);
    EXPORT static opentxs::Pimpl<opentxs::String> Factory(
        const OTSignature& value);
    EXPORT static opentxs::Pimpl<opentxs::String> Factory(
        const Contract& value);
    EXPORT static opentxs::Pimpl<opentxs::String> Factory(
        const Identifier& value);
    EXPORT static opentxs::Pimpl<opentxs::String> Factory(const NymFile& value);
    EXPORT static opentxs::Pimpl<opentxs::String> Factory(const char* value);
    EXPORT static opentxs::Pimpl<opentxs::String> Factory(
        const std::string& value);
    EXPORT static opentxs::Pimpl<opentxs::String> Factory(
        const char* value,
        std::size_t size);

    EXPORT static std::string LongToString(const std::int64_t& lNumber);
    EXPORT static std::string replace_chars(
        const std::string& str,
        const std::string& charsFrom,
        const char& charTo);
#ifdef _WIN32
    EXPORT static std::wstring s2ws(const std::string& s);
#endif
    EXPORT static std::size_t safe_strlen(const char* s, std::size_t max);
    EXPORT static std::int32_t StringToInt(const std::string& number);
    EXPORT static std::int64_t StringToLong(const std::string& number);
    EXPORT static std::uint32_t StringToUint(const std::string& number);
    EXPORT static std::uint64_t StringToUlong(const std::string& number);
    EXPORT static std::string& trim(std::string& str);
    EXPORT static std::string UlongToString(const std::uint64_t& uNumber);
    EXPORT static bool vformat(
        const char* fmt,
        std::va_list* pvl,
        std::string& s) ATTR_PRINTF(1, 0);
#ifdef _WIN32
    EXPORT static std::string ws2s(const std::wstring& s);
#endif

    EXPORT virtual bool operator>(const String& rhs) const;
    EXPORT virtual bool operator<(const String& rhs) const;
    EXPORT virtual bool operator<=(const String& rhs) const;
    EXPORT virtual bool operator>=(const String& rhs) const;
    EXPORT virtual bool operator==(const String& rhs) const;

    EXPORT virtual bool At(std::uint32_t index, char& c) const;
    EXPORT virtual bool Compare(const char* compare) const;
    EXPORT virtual bool Compare(const String& compare) const;
    EXPORT virtual bool Contains(const char* compare) const;
    EXPORT virtual bool Contains(const String& compare) const;
    EXPORT virtual bool empty() const;
    EXPORT virtual bool Exists() const;
    EXPORT virtual const char* Get() const;
    EXPORT virtual std::uint32_t GetLength() const;
    EXPORT virtual std::int32_t ToInt() const;
    EXPORT virtual bool TokenizeIntoKeyValuePairs(Map& map) const;
    EXPORT virtual std::int64_t ToLong() const;
    EXPORT virtual std::uint32_t ToUint() const;
    EXPORT virtual std::uint64_t ToUlong() const;
    EXPORT virtual void WriteToFile(std::ostream& ofs) const;

    EXPORT virtual void Concatenate(const char* arg, ...) ATTR_PRINTF(2, 3);
    EXPORT virtual void Concatenate(const String& data);
    EXPORT virtual void ConvertToUpperCase();
    EXPORT virtual bool DecodeIfArmored(bool escapedIsAllowed = true);
    EXPORT virtual void Format(const char* fmt, ...) ATTR_PRINTF(2, 3);
    EXPORT virtual void Initialize();
    /** For a straight-across, exact-size copy of bytes. Source not expected to
     * be null-terminated. */
    EXPORT virtual bool MemSet(const char* mem, std::uint32_t size);
    EXPORT virtual void OTfgets(std::istream& ofs);
    EXPORT virtual void Release();
    /** new_string MUST be at least nEnforcedMaxLength in size if
    nEnforcedMaxLength is passed in at all.
    That's because this function forces the null terminator at that length,
    minus 1. For example, if the max is set to 10, then the valid range is 0..9.
    Therefore 9 (10 minus 1) is where the nullptr terminator goes. */
    EXPORT void Set(const char* data, std::uint32_t enforcedMaxLength = 0);
    EXPORT void Set(const String& data);
    /** true  == there are more lines to read.
    false == this is the last line. Like EOF. */
    EXPORT virtual bool sgets(char* buffer, std::uint32_t size);
    EXPORT virtual char sgetc();
    EXPORT virtual void sungetc();
    EXPORT virtual void reset();
    EXPORT virtual void swap(String& rhs);
    EXPORT virtual void Truncate(std::uint32_t index);
    EXPORT virtual void zeroMemory();

    EXPORT virtual ~String();

protected:
    std::uint32_t length_{0};
    std::uint32_t position_{0};
    char* data_{nullptr};

    virtual void Release_String();

private:
    friend OTString;
    friend std::ostream& operator<<(std::ostream& os, const String& obj);

    String* clone() const;
    /** Only call this right after calling Initialize() or Release(). Also, this
     * function ASSUMES the new_string pointer is good. */
    void LowLevelSet(const char* data, std::uint32_t enforcedMaxLength);
    /** You better have called Initialize() or Release() before you dare call
     * this. */
    void LowLevelSetStr(const String& buffer);

protected:
//public:
    EXPORT explicit String(const Armored& value);
    EXPORT explicit String(const OTSignature& value);
    EXPORT explicit String(const Contract& value);
    EXPORT explicit String(const Identifier& value);
    EXPORT explicit String(const NymFile& value);
    EXPORT String(const char* value);
    EXPORT explicit String(const std::string& value);
    EXPORT String(const char* value, std::size_t size);
    EXPORT String();
    EXPORT String(const String& rhs);
    EXPORT String& operator=(const String& rhs);
};
}  // namespace opentxs
#endif
