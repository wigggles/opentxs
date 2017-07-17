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

#ifdef ANDROID
#include "opentxs/core/util/android_string.hpp"
#endif  // ANDROID

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t BYTE;
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

#ifndef OPENTXS_CHAISCRIPT_5
#define OPENTXS_CHAISCRIPT_5
#endif

#if defined(ANDROID) || defined(OT_KEYRING_IOS)
// DON'T use ChaiScript on mobile devices
#undef OT_USE_SCRIPT_CHAI

#ifdef OPENTXS_CHAISCRIPT_5
#undef OPENTXS_CHAISCRIPT_5
#endif

#endif

#ifdef OT_USE_CHAI_STDLIB
#undef OT_USE_CHAI_STDLIB
#endif

#ifdef OPENTXS_CHAISCRIPT_5
#define OT_USE_CHAI_STDLIB
#endif

#ifdef _WIN32
#ifndef NO_OT_PCH
#else
#undef NO_OT_PCH
#endif
#endif

#endif  // OPENTXS_CORE_STDAFX_HPP
