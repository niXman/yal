
// Copyright (c) 2019 niXman (i dotty nixman doggy gmail dotty com)
// All rights reserved.
//
// This file is part of DTF(https://github.com/niXman/dtf) project.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
//   Redistributions in binary form must reproduce the above copyright notice, this
//   list of conditions and the following disclaimer in the documentation and/or
//   other materials provided with the distribution.
//
//   Neither the name of the {organization} nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dtf.hpp"

#include <chrono>
#include <utility>
#include <cmath>
#include <cassert>
#include <cstring>

#ifdef DTF_HEADER_ONLY
#   define __DTF_INLINE inline
#else
#   define __DTF_INLINE
#endif // DTF_HEADER_ONLY

namespace dtf {

/*************************************************************************************************/

__DTF_INLINE std::uint64_t timestamp(int offset) {
    std::uint64_t ts = std::chrono::duration_cast<
        std::chrono::nanoseconds
    >(std::chrono::system_clock::now().time_since_epoch()).count();

    std::uint64_t val = 60 * 60 * 1000000000ull * std::abs(offset);
    ts = (offset < 0) ? ts - val : ts + val;

    return ts;
}

/*************************************************************************************************/

__DTF_INLINE std::size_t num_chars(std::size_t v) {
    std::size_t n = 1;
    v = (v >= 100000000000000000ull) ? ((n += 17),v / 100000000000000000ull) : v;
    v = (v >= 100000000ull) ? ((n += 8),v / 100000000ull) : v;
    v = (v >= 10000ull) ? ((n += 4),v / 10000ull) : v;
    v = (v >= 100ull) ? ((n += 2),v / 100ull) : v;
    n = (v >= 10ull) ? n + 1 : n;

    return n;
}

__DTF_INLINE void utoa(char *ptr, std::size_t n, std::uint64_t v) {
    char *p = ptr + n - 1;
    switch ( n ) {
        case 20: *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 19: *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 18: *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 17: *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 16: *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 15: *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 14: *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 13: *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 12: *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 11: *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 10: *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 9 : *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 8 : *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 7 : *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 6 : *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 5 : *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 4 : *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 3 : *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 2 : *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        case 1 : *p-- = static_cast<char>('0'+(v % 10)); v /= 10; // fallthrough
        default: break;
    }
}

/*************************************************************************************************/

#define __DTF_YEAR(p, v) \
    *p++ = (v / 1000) % 10 + '0'; \
    *p++ = (v / 100) % 10 + '0'; \
    *p++ = (v / 10) % 10 + '0'; \
    *p++ = (v) % 10 + '0';

#define __DTF_MONTH(p, v) \
    *p++ = ((v + 1) / 10) % 10 + '0'; \
    *p++ = ((v + 1)) % 10 + '0';

#define __DTF_DAY(p, v) \
    *p++ = (v / 10) % 10 + '0'; \
    *p++ = (v % 10) + '0';

#define __DTF_HMS(p, v) \
    *p++ = (v / 10) % 10 + '0'; \
    *p++ = v % 10 + '0';

__DTF_INLINE std::size_t timestamp_to_chars(char *ptr, std::uint64_t ts, std::size_t f) {
    const auto datesep = (f & flags::sep1) ? '-' : '.';
    const auto timesep = (f & flags::sep1)||(f & flags::sep3) ? ':' : '.';
    const auto sepsep  = (f & flags::sep1) ? ' ' : '-';
    const auto ss = ts / 1000000000ull;
    const auto ps = ts % 1000000000ull;

    static const std::size_t SPD = 24 * 60 * 60;
    static const std::uint16_t spm[13] = {
         0
        ,(31)
        ,(31 + 28)
        ,(31 + 28 + 31)
        ,(31 + 28 + 31 + 30)
        ,(31 + 28 + 31 + 30 + 31)
        ,(31 + 28 + 31 + 30 + 31 + 30)
        ,(31 + 28 + 31 + 30 + 31 + 30 + 31)
        ,(31 + 28 + 31 + 30 + 31 + 30 + 31 + 31)
        ,(31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30)
        ,(31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31)
        ,(31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)
        ,(31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31)
    };
    static const auto is_leap_year = [](std::size_t t) {
        return (!(t % 4) && ((t % 100) || !(t % 400)));
    };

    std::size_t secs{}, mins{}, hours{}, years{}, mons{}, days{};
    std::size_t work = ss % SPD;

    secs = work % 60;
    work /= 60;
    mins = work % 60;
    hours = work / 60;
    work = ss / SPD;

    std::size_t i = 1970;
    for ( ; ; ++i ) {
        std::size_t k = is_leap_year(i) ? 366 : 365;
        if ( work >= k ) {
            work -= k;
        } else {
            break;
        }
    }
    years = i;

    days = 1;
    if ( is_leap_year(i) && (work > 58) ) {
        if (work == 59) {
            days = 2;
        }
        work -= 1;
    }

    for ( i = 11; i && (spm[i] > work); --i )
        ;
    mons = i;
    days += work - spm[i];

    char *p = ptr;
    std::memset(p, '0', bufsize);

    if ( f & flags::yyyy_mm_dd ) {
        __DTF_YEAR(p, years);
        *p++ = datesep;
        __DTF_MONTH(p, mons);
        *p++ = datesep;
        __DTF_DAY(p, days);

    } else if ( f & flags::dd_mm_yyyy ) {
        __DTF_DAY(p, days);
        *p++ = datesep;
        __DTF_MONTH(p, mons);
        *p++ = datesep;
        __DTF_YEAR(p, years);
    } else {
        assert(!"unreachable");
    }

    *p++ = sepsep;

    __DTF_HMS(p, hours);
    *p++ = timesep;
    __DTF_HMS(p, mins);
    *p++ = timesep;
    __DTF_HMS(p, secs);


    const auto pi = (f & flags::secs) ? std::make_pair(ps / 1000000000ull, 0u)
        : (f & flags::msecs) ? std::make_pair(ps / 1000000ull, 3u)
            : (f & flags::usecs) ? std::make_pair(ps / 1000ull, 6u)
                : std::make_pair(ps, 9u)
    ;
    if ( pi.first ) {
        *p++ = '.';

        const auto n = num_chars(pi.first);
        p += pi.second - n;

        utoa(p, n, pi.first);
        p += n;
    }

    return p - ptr;
}

/*************************************************************************************************/

} // ns dtf
