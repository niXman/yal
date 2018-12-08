
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

#ifndef _yal__throw_hpp
#define _yal__throw_hpp

#include <string>
#include <cerrno>

#include <string.h>

/***************************************************************************/

namespace yal {

struct exception: std::exception {
	exception(std::string msg) noexcept
		:std::exception()
		,msg(std::move(msg))
	{}
	virtual ~exception() {}

	const char* what() const noexcept { return msg.c_str(); }

	const std::string msg;
};

} // ns yal

#define __YAL_STRINGIZE_AUX(x) #x
#define __YAL_STRINGIZE(x) __YAL_STRINGIZE_AUX(x)

#ifdef _WIN32
#define __YAL_THROW_IF(expr, msg) \
	if ( (expr) ) { \
		int __ec = errno; \
		throw ::yal::exception(std::string("YAL: " __FILE__ "(" __YAL_STRINGIZE(__LINE__) "): \"") + msg + "\", errno=" + std::to_string(__ec) + "(" + strerror(__ec) + ")"); \
	}
#else
#define __YAL_THROW_IF(expr, msg) \
	if ( (expr) ) { \
		const int __ec = errno; \
		char __buf[1024] = "\0"; \
		const char *__bufp = strerror_r(__ec, __buf, sizeof(__buf)); \
		throw ::yal::exception(std::string("YAL: " __FILE__ "(" __YAL_STRINGIZE(__LINE__) "): \"") + msg + "\", errno=" + std::to_string(__ec) + "(" + __bufp + ")"); \
	}
#endif

/***************************************************************************/

#endif // _yal__throw_hpp
