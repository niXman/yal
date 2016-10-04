
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

#include <yal/index.hpp>

#include <cstdint>
#include <cstdio>

namespace yal {

/**************************************************************************/

std::size_t index_count(std::FILE *idxfile) {
	std::fseek(idxfile, 0, SEEK_END);
	auto fsize = std::ftell(idxfile);

	return fsize/sizeof(index_record);
}

/**************************************************************************/

bool index_read(index_record *idx, std::size_t n, std::FILE *idxfile) {
	if ( std::fseek(idxfile, n*sizeof(index_record), SEEK_SET) != 0 )
		return false;

	auto rd = std::fread(idx, 1, sizeof(index_record), idxfile);

	return rd == sizeof(index_record);
}

/**************************************************************************/

bool index_read_data(index_data *data, const index_record &idx, std::FILE *logfile) {
	if ( std::fseek(logfile, idx.start_pos, SEEK_SET) != 0 )
		return false;

	if ( std::fseek(logfile, idx.dt_off, SEEK_CUR) != 0 )
		return false;

	data->datetime.resize(idx.dt_len, 0);
	if ( std::fread(const_cast<char*>(data->datetime.data()), 1, idx.dt_len, logfile) != idx.dt_len)
		return false;

	if ( std::fseek(logfile, idx.lvl_off, SEEK_CUR) != 0 )
		return false;

	if ( std::fread(&(data->errlvl), 1, sizeof(data->errlvl), logfile) != idx.lvl_len )
		return false;

	if ( std::fseek(logfile, idx.fl_off, SEEK_CUR) != 0 )
		return false;

	data->fileline.resize(idx.fl_len, 0);
	if ( std::fread(const_cast<char*>(data->fileline.data()), 1, idx.fl_len, logfile) != idx.fl_len)
		return false;

	if ( std::fseek(logfile, idx.func_off, SEEK_CUR) != 0 )
		return false;

	data->func.resize(idx.func_len, 0);
	if ( std::fread(const_cast<char*>(data->func.data()), 1, idx.func_len, logfile) != idx.func_len)
		return false;

	if ( std::fseek(logfile, idx.data_off, SEEK_CUR) != 0 )
		return false;

	data->data.resize(idx.data_len-1, 0);
	if ( std::fread(const_cast<char*>(data->data.data()), 1, idx.data_len-1, logfile) != idx.data_len-1)
		return false;

	return true;
}

/**************************************************************************/

bool index_read_data(index_data *data, std::size_t n, std::FILE *logfile, std::FILE *idxfile) {
	index_record rec{};

	if ( !index_read(&rec, n, idxfile) )
		return false;

	return index_read_data(data, rec, logfile);
}

/**************************************************************************/

} // ns yal
