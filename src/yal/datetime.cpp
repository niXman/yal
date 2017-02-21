
// Copyright (c) 2013-2016 niXman (i dotty nixman doggy gmail dotty com)
// All rights reserved.
//
// This file is part of YAL(https://github.com/niXman/yal) project.
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

#include <yal/throw.hpp>
#include <yal/datetime.hpp>

#include <string>
#include <ctime>
#include <cmath>

namespace yal {
namespace detail {

std::size_t itoa(char *buf, long long v) {
	if ( v < 0 ) { *buf++ = '-'; }
	long long t = v = std::abs(v);

	std::size_t len = 1;
	if ( t >= 10000000000000000ll ) { len += 16; t /= 10000000000000000ll; }
	if ( t >= 100000000 ) { len += 8; t /= 100000000; }
	if ( t >= 10000 ) { len += 4; t /= 10000; }
	if ( t >= 100 ) { len += 2; t /= 100; }
	if ( t >= 10 ) { len += 1; }

	char *p = buf+len-1;
	switch ( len ) {
		case 19: *p-- = '0' + (v % 10); v /= 10;
		case 18: *p-- = '0' + (v % 10); v /= 10;
		case 17: *p-- = '0' + (v % 10); v /= 10;
		case 16: *p-- = '0' + (v % 10); v /= 10;
		case 15: *p-- = '0' + (v % 10); v /= 10;
		case 14: *p-- = '0' + (v % 10); v /= 10;
		case 13: *p-- = '0' + (v % 10); v /= 10;
		case 12: *p-- = '0' + (v % 10); v /= 10;
		case 11: *p-- = '0' + (v % 10); v /= 10;
		case 10: *p-- = '0' + (v % 10); v /= 10;
		case 9 : *p-- = '0' + (v % 10); v /= 10;
		case 8 : *p-- = '0' + (v % 10); v /= 10;
		case 7 : *p-- = '0' + (v % 10); v /= 10;
		case 6 : *p-- = '0' + (v % 10); v /= 10;
		case 5 : *p-- = '0' + (v % 10); v /= 10;
		case 4 : *p-- = '0' + (v % 10); v /= 10;
		case 3 : *p-- = '0' + (v % 10); v /= 10;
		case 2 : *p-- = '0' + (v % 10); v /= 10;
		case 1 : *p-- = '0' + (v % 10); v /= 10;
	}

	return len;
}

/***************************************************************************/

static const short spm[13] = {
	 0
	,(31)
	,(31+28)
	,(31+28+31)
	,(31+28+31+30)
	,(31+28+31+30+31)
	,(31+28+31+30+31+30)
	,(31+28+31+30+31+30+31)
	,(31+28+31+30+31+30+31+31)
	,(31+28+31+30+31+30+31+31+30)
	,(31+28+31+30+31+30+31+31+30+31)
	,(31+28+31+30+31+30+31+31+30+31+30)
	,(31+28+31+30+31+30+31+31+30+31+30+31)
};

#define IS_LEAP_YEAR(year) \
	(!(year%4) && ((year%100) || !(year%400)))

std::tm* time_t_to_tm(const std::time_t t, std::tm *r) {
	static const int SPD = 24*60*60;
	time_t i;
	time_t work = t % SPD;
	r->tm_sec = work%60;
	work/=60;
	r->tm_min = work%60;
	r->tm_hour = work/60;
	work = t / SPD;
	r->tm_wday = (4+work) % 7;
	for ( i = 1970; ; ++i ) {
		const time_t k = IS_LEAP_YEAR(i) ? 366 : 365;
		if ( work >= k )
			work -= k;
		else
			break;
	}
	r->tm_year = i-1900;
	r->tm_yday = work;

	r->tm_mday = 1;
	if ( IS_LEAP_YEAR(i) && (work > 58) ) {
	  if ( work == 59 )
		  r->tm_mday = 2;
	  work -= 1;
	}

	for (i = 11; i && (spm[i] > work); --i )
		;

	r->tm_mon = i;
	r->tm_mday += work - spm[i];

	return r;
}

/***************************************************************************/

std::size_t datetime_str(char *buf, std::uint32_t res) {
	struct timespec ts;
	char *p = buf;

	clock_gettime(CLOCK_REALTIME, &ts);
	std::tm tm;
	time_t_to_tm(ts.tv_sec, &tm);
	tm.tm_hour += std::abs(timezone/(60*60));

#define YAL_YEAR(p, tm) \
	*p++ = ((tm.tm_year+1900)/1000)%10+'0'; \
	*p++ = ((tm.tm_year+1900)/100)%10+'0'; \
	*p++ = ((tm.tm_year+1900)/10)%10+'0'; \
	*p++ = (tm.tm_year+1900)%10+'0';
#define YAL_MONTH(p, tm) \
	*p++ = ((tm.tm_mon+1)/10)%10+'0'; \
	*p++ = (tm.tm_mon+1)%10+'0';
#define YAL_DAY(p, tm) \
	*p++ = (tm.tm_mday/10)%10+'0'; \
	*p++ = tm.tm_mday%10+'0';

#if YAL_DATE_FORMAT == YAL_DATE_FORMAT_DMY // day.month.year
	// day
	YAL_DAY(p, tm)
	/* sep */
	*p++ = '.';
	// month
	YAL_MONTH(p, tm)
	/* sep */
	*p++ = '.';
	// year
	YAL_YEAR(p, tm)
#elif YAL_DATE_FORMAT == YAL_DATE_FORMAT_YMD // year.month.day
	// year
	YAL_YEAR(p, tm)
	/* sep */
	*p++ = '.';
	// month
	YAL_MONTH(p, tm)
	/* sep */
	*p++ = '.';
	// day
	YAL_DAY(p, tm)
#else                                       // month.day.year
	// month
	YAL_MONTH(p, tm)
	/* sep */
	*p++ = '.';
	// day
	YAL_DAY(p, tm)
	/* sep */
	*p++ = '.';
	// year
	YAL_YEAR(p, tm)
#endif

#undef YAL_YEAR
#undef YAL_MONTH
#undef YAL_DAY

	/* sep */
	*p++ = '-';
	// hours
	*p++ = (tm.tm_hour/10)%10+'0';
	*p++ = tm.tm_hour%10+'0';
	/* sep */
	*p++ = '.';
	// minutes
	*p++ = (tm.tm_min/10)%10+'0';
	*p++ = tm.tm_min%10+'0';
	/* sep */
	*p++ = '.';
	// seconds
	*p++ = (tm.tm_sec/10)%10+'0';
	*p++ = tm.tm_sec%10+'0';
	if ( res & usec_res ) {
		/* sep */
		*p++ = '-';
		p += itoa(p, ts.tv_nsec/1000);
	} else if ( res & nsec_res ) {
		/* sep */
		*p++ = '-';
		p += itoa(p, ts.tv_nsec);
	}

    return static_cast<std::size_t>(p-buf);
}

} // ns detail

const char *datetime_str(char *buf, const std::size_t buf_size, std::uint32_t opts) {
	const std::size_t expbufsize = (
		(opts & sec_res)
			? sec_res_len
			: (opts & usec_res)
				? usec_res_len
				: nsec_res_len
	);
	YAL_THROW_IF(buf_size < expbufsize, "buffer size is less than " + std::to_string(expbufsize));

	detail::datetime_str(buf, opts);

	return buf;
}

const char* sec_datetime_str(char *buf, const std::size_t buf_size) {
	YAL_THROW_IF(buf_size < sec_res_len, "buffer size is less than " + std::to_string(sec_res_len));
	detail::datetime_str(buf, sec_res);
	return buf;
}
const char* usec_datetime_str(char *buf, const std::size_t buf_size) {
	YAL_THROW_IF(buf_size < usec_res_len, "buffer size is less than " + std::to_string(usec_res_len));
	detail::datetime_str(buf, usec_res);
	return buf;
}
const char* nsec_datetime_str(char *buf, const std::size_t buf_size) {
	YAL_THROW_IF(buf_size < nsec_res_len, "buffer size is less than " + std::to_string(nsec_res_len));
	detail::datetime_str(buf, nsec_res);
	return buf;
}

} // ns yal
