#pragma once

#ifndef OPENTXS_CORE_STDAFX_HPP
#define OPENTXS_CORE_STDAFX_HPP

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#ifdef _WIN32
#include <direct.h>
#include <shlobj.h>
#endif
#ifdef TARGET_OS_MAC
#include <limits.h>
#include <mach-o/dyld.h>
#endif

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef std::uint8_t BYTE;
typedef uint16_t USHORT;

#ifdef __cplusplus
}
#endif

// NOTE: Turns out moneypunct kind of sucks.
// As a result, for internationalization purposes,
// these values have to be set here before compilation.
//
#define OT_THOUSANDS_SEP ","
#define OT_DECIMAL_POINT "."

#ifdef _WIN32
#ifndef NO_OT_PCH
#else
#undef NO_OT_PCH
#endif
#endif

#ifndef CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_NO_THREADS
#endif

#ifndef OT_USE_CHAI_STDLIB
#define OT_USE_CHAI_STDLIB
#endif

#endif  // OPENTXS_CORE_STDAFX_HPP
