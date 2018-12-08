
// Copyright (c) 2013-2018 niXman (i dotty nixman doggy gmail dotty com)
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

#ifndef _yal__datetime_hpp
#define _yal__datetime_hpp

#include <yal/options.hpp>

#include <cstdint>

/***************************************************************************/

#define YAL_DATE_FORMAT_DMY (0) // day.month.year
#define YAL_DATE_FORMAT_YMD (1) // year.month.day
#define YAL_DATE_FORMAT_MDY (2) // month.day.year

#ifndef YAL_DATE_FORMAT
#	define YAL_DATE_FORMAT YAL_DATE_FORMAT_YMD
#endif // YAL_DATE_FORMAT

#if !(YAL_DATE_FORMAT == YAL_DATE_FORMAT_DMY || \
        YAL_DATE_FORMAT == YAL_DATE_FORMAT_YMD || \
        YAL_DATE_FORMAT == YAL_DATE_FORMAT_MDY)
#error "bad YAL_DATE_FORMAT"
#endif

/***************************************************************************/

namespace yal {

// constants
enum {
	 sec_res_len  = 19
	,usec_res_len = 26
	,nsec_res_len = 29
};

const char* datetime_str(char *buf, const std::size_t buf_size, std::uint32_t opts);
const char* sec_datetime_str(char *buf, const std::size_t buf_size);
const char* usec_datetime_str(char *buf, const std::size_t buf_size);
const char* nsec_datetime_str(char *buf, const std::size_t buf_size);

} // ns yal

/***************************************************************************/

#endif // _yal__datetime_hpp
