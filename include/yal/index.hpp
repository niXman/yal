
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

#ifndef _yal__index_hpp
#define _yal__index_hpp

#include <cstdint>
#include <cstdio>

#include <string>

namespace yal {

/**************************************************************************/

#pragma pack(push, 1)
struct index_record {
	std::uint32_t start_pos;
	std::uint8_t dt_off;
	std::uint8_t dt_len;
	std::uint8_t lvl_off;
	std::uint8_t lvl_len;
	std::uint8_t fl_off;
	std::uint8_t fl_len;
	std::uint8_t func_off;
	std::uint8_t func_len;
	std::uint8_t data_off;
	std::uint32_t data_len;
};
#pragma pack(pop)

/**************************************************************************/

struct index_data {
	std::string datetime;
	char errlvl;
	std::string fileline;
	std::string func;
	std::string data;
};

/**************************************************************************/

std::size_t index_count(std::FILE *idxfile);
bool index_read(index_record *idx, std::size_t n, std::FILE *idxfile);
bool index_read_data(index_data *data, const index_record &idx, std::FILE *logfile);
bool index_read_data(index_data *data, std::size_t n, std::FILE *logfile, std::FILE *idxfile);

/**************************************************************************/

} // ns yal

#endif // _yal__index_hpp
