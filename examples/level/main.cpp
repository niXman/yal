
// Copyright (c) 2013,2014 niXman (i dotty nixman doggy gmail dotty com)
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

#include <yal/yal.hpp>

/***************************************************************************/

int main() {
	YAL_CREATE(log, "log", 1024*10);

	yal::level lvl = yal::info;
	for ( std::size_t idx = 0; idx < 15; ++idx ) {
		switch (idx) {
			case 0: case 1: case 2:
			lvl = yal::info;
			break;
			case 3: case 4: case 5:
			lvl = yal::debug;
			break;
			case 6: case 7: case 8:
			lvl = yal::warning;
			break;
			case 9: case 10: case 11:
			lvl = yal::error;
			break;
			default:
			lvl = yal::disable;
			break;
		}
		YAL_SESSION_SET_LEVEL(log, lvl);

		YAL_LOG_INFO	(log, "INFO    LEVEL, idx = %1%, lvl = %2%", idx, log->level_str(lvl));
		YAL_LOG_DEBUG	(log, "DEBUG   LEVEL, idx = %1%, lvl = %2%", idx, log->level_str(lvl));
		YAL_LOG_WARNING(log, "WARNING LEVEL, idx = %1%, lvl = %2%", idx, log->level_str(lvl));
		YAL_LOG_ERROR	(log, "ERROR   LEVEL, idx = %1%, lvl = %2%", idx, log->level_str(lvl));
	}
}

/***************************************************************************/
