// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_UTIL_STRINGUTILS_HPP
#define OPENTXS_CORE_UTIL_STRINGUTILS_HPP

#include "opentxs/Forward.hpp"

#include <cinttypes>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>

namespace opentxs
{

// from: http://www.cplusplus.com/faq/sequences/strings/split/
//
struct split {
    enum empties_t { empties_ok, no_empties };
};

template <typename Container>
static Container& split_byChar(
    Container& result,
    const typename Container::value_type& s,
    const typename Container::value_type& delimiters,
    split::empties_t empties)
{
    result.clear();
    std::int64_t next = -1;
    do {
        if (empties == split::no_empties) {
            next = s.find_first_not_of(
                delimiters, static_cast<std::uint32_t>(next) + 1);
            if (static_cast<size_t>(next) == Container::value_type::npos) {
                break;
            }
            next -= 1;
        }
        size_t current = static_cast<size_t>(next + 1);
        next = s.find_first_of(delimiters, current);
        result.push_back(
            s.substr(current, static_cast<std::uint32_t>(next) - current));
    } while (static_cast<size_t>(next) != Container::value_type::npos);
    return result;
}

// If you've already strlen'd the string,
// you can pass the length to str_hsh or str_dup
// and save it the trouble.
//
char* str_dup2(const char* str, std::uint32_t length);

template <typename T>
inline std::string to_string(const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

/*
 * strlcpy and strlcat
 *
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
inline size_t strlcpy(char* dst, const char* src, size_t siz)
{
    char* d = dst;
    const char* s = src;
    size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0') break;
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (siz != 0) *d = '\0'; /* NUL-terminate dst */
        while (*s++)
            ;
    }

    return (s - src - 1); /* count does not include NUL */
}
/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
inline size_t strlcat(char* dst, const char* src, size_t siz)
{
    char* d = dst;
    const char* s = src;
    size_t n = siz;
    size_t dlen;

    /* Find the end of dst and adjust bytes left but don't go past end */
    while (n-- != 0 && *d != '\0') d++;
    dlen = d - dst;
    n = siz - dlen;

    if (n == 0) return (dlen + strlen(s));
    while (*s != '\0') {
        if (n != 1) {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';

    return (dlen + (s - src)); /* count does not include NUL */
}
// (End of the Todd Miller code.)

}  // namespace opentxs

#endif
