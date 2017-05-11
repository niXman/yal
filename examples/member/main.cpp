
// Copyright (c) 2013-2017 niXman (i dotty nixman doggy gmail dotty com)
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

#include <iostream>

//#define YAL_DISABLE_LOGGING

#include <yal/yal.hpp>

/***************************************************************************/

struct type {
	type()
		YAL_SESSION_CREATE3(log1, "log1") // :log1(...)
		YAL_SESSION_CREATE4(log2, "log2") // ,log2(...)
	{
		YAL_LOG_ERROR(log1, "fmt string %1%", 33);
		YAL_LOG_DEBUG(log2, "fmt string %1%", 44);
	}

	type(const type &r)
		YAL_SESSION_INIT_VAR(log1, r.log1) // :log1(r.log1)
		YAL_SESSION_INIT_VAR2(log2, r.log2)// ,log2(r.log2)
	{ ((void)r); }

	type(type &&r)
		YAL_SESSION_INIT_VAR(log1, std::move(r.log1)) // :log1(std::move(r.log1))
		YAL_SESSION_INIT_VAR2(log2, std::move(r.log2))// ,log2(std::move(r.log2))
	{}

	void func() {
		YAL_LOG_ERROR_IF(log1, 0, "some error %", "message 1");
		YAL_LOG_ERROR_IF(log2, 0, "some error %", "message 2");
	}
private:
	YAL_SESSION_DECLARE_VAR(log1); // ::yal::session log1;
	YAL_SESSION_DECLARE_VAR(log2); // ::yal::session log2;
};

/***************************************************************************/

int main() {

	type t;
	t.func();

	std::cout << "EXIT_SUCCESS" << std::endl;
	return EXIT_SUCCESS;
}

/***************************************************************************/
