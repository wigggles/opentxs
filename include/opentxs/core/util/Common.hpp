// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_UTIL_COMMON_HPP
#define OPENTXS_CORE_UTIL_COMMON_HPP

#include "opentxs/Forward.hpp"

#include <cinttypes>
#include <ctime>
#include <memory>

#include <string>
#include <cstdlib>
#include <ctime>

// forward decleration.  (need to match what is in the irr source code). Cam.
namespace irr
{
namespace io
{
template <class char_type, class super_class>
class IIrrXMLReader;
class IFileReadCallBack;
class IXMLBase;

typedef IIrrXMLReader<char, IXMLBase> IrrXMLReader;
}  // namespace io
}  // namespace irr

#define OT_ERROR_AMOUNT INT64_MIN

#define OT_TIME_YEAR_IN_SECONDS                                                \
    OTTimeGetTimeFromSeconds(31536000)  // 60 * 60 * 24 * 365
#define OT_TIME_SIX_MONTHS_IN_SECONDS                                          \
    OTTimeGetTimeFromSeconds(15552000)  // 60 * 60 * 24 * 180
#define OT_TIME_THREE_MONTHS_IN_SECONDS                                        \
    OTTimeGetTimeFromSeconds(7776000)  // 60 * 60 * 24 * 90
#define OT_TIME_MONTH_IN_SECONDS                                               \
    OTTimeGetTimeFromSeconds(2592000)  // 60 * 60 * 24 * 30
#define OT_TIME_DAY_IN_SECONDS OTTimeGetTimeFromSeconds(86400)  // 60 * 60 * 24
#define OT_TIME_HOUR_IN_SECONDS OTTimeGetTimeFromSeconds(3600)  // 60 * 60
#define OT_TIME_MINUTE_IN_SECONDS OTTimeGetTimeFromSeconds(60)  // 60

#define OT_TIME_ZERO OTTimeGetTimeFromSeconds(static_cast<std::int64_t>(0))

//#define FORCE_COMPILE_ERRORS_TO_FIND_USAGE  // uncomment this line to find
// non-localized time64_t usage
#ifdef FORCE_COMPILE_ERRORS_TO_FIND_USAGE
class time64_t
{
public:
    int operator<(const time64_t& rhs) const;
    int operator>(const time64_t& rhs) const;
    int operator<=(const time64_t& rhs) const;
    int operator>=(const time64_t& rhs) const;
    int operator==(const time64_t& rhs) const;
    int operator!=(const time64_t& rhs) const;
};
std::stringstream& operator<<(const std::stringstream& str, const time64_t& t);

EXPORT time64_t OTTimeGetCurrentTime();  // { return std::time(nullptr); }
EXPORT time64_t OTTimeGetTimeFromSeconds(std::int64_t seconds);  // { return
                                                                 // seconds;
                                                                 // }
EXPORT time64_t OTTimeGetTimeFromSeconds(const char* pSeconds);  // { return
// std::stol(pSeconds);
// }
EXPORT std::int64_t OTTimeGetSecondsFromTime(time64_t time);  // { return time;
                                                              // }
EXPORT std::int64_t OTTimeGetTimeInterval(
    time64_t lhs,
    time64_t rhs);  // { return
                    // lhs - rhs;
                    // }
EXPORT time64_t
OTTimeAddTimeInterval(time64_t lhs, std::int64_t rhs);  // { return
                                                        // lhs + rhs;
                                                        // }
#else
typedef std::int64_t time64_t;

inline time64_t OTTimeGetCurrentTime() { return std::time(nullptr); }
inline time64_t OTTimeGetTimeFromSeconds(std::int64_t seconds)
{
    return seconds;
}
#if !defined(ANDROID)
inline time64_t OTTimeGetTimeFromSeconds(const char* pSeconds)
{
    return std::stol(pSeconds);
}
#else
inline time64_t OTTimeGetTimeFromSeconds(const char* pSeconds)
{
    return std::atol(pSeconds);
}
#endif
inline std::int64_t OTTimeGetSecondsFromTime(time64_t time) { return time; }
inline std::int64_t OTTimeGetTimeInterval(time64_t lhs, time64_t rhs)
{
    return lhs - rhs;
}
inline time64_t OTTimeAddTimeInterval(time64_t lhs, std::int64_t rhs)
{
    return lhs + rhs;
}

EXPORT std::string getTimestamp();
EXPORT std::string formatTimestamp(time64_t tt);
EXPORT time64_t parseTimestamp(std::string extendedTimeString);

EXPORT std::string formatLong(std::int64_t tt);
EXPORT std::string formatUlong(std::uint64_t tt);

EXPORT std::string formatBool(bool tt);
EXPORT std::string formatChar(char tt);

EXPORT std::string formatInt(std::int32_t tt);
EXPORT std::string formatUint(std::uint32_t tt);

#endif

#endif
