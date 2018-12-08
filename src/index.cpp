
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

#include <yal/index.hpp>

#include <cstdint>

#include <unistd.h>

namespace yal {

/**************************************************************************/

std::size_t index_count(int idxfd) {
	auto fsize = ::lseek(idxfd, 0, SEEK_END);

	return fsize/sizeof(index_record);
}

/**************************************************************************/

bool index_read(index_record *idx, std::size_t n, int idxfd) {
	const std::size_t off = ::lseek(idxfd, n*sizeof(index_record), SEEK_SET);
	if ( off != n*sizeof(index_record) )
		return false;

	auto rd = ::read(idxfd, idx, sizeof(index_record));

	return rd == sizeof(index_record);
}

/**************************************************************************/

bool index_read_data(index_data *data, const index_record &idx, int logfd) {
	if ( ::lseek(logfd, idx.start_pos, SEEK_SET) == -1 )
		return false;

	if ( ::lseek(logfd, idx.dt_off, SEEK_CUR) == -1 )
		return false;

	data->datetime.resize(idx.dt_len, 0);
	if ( ::read(logfd, const_cast<char*>(data->datetime.data()), idx.dt_len) != idx.dt_len )
		return false;

	if ( ::lseek(logfd, idx.lvl_off, SEEK_CUR) == -1 )
		return false;

	if ( ::read(logfd, &(data->errlvl), sizeof(data->errlvl)) != idx.lvl_len )
		return false;

	if ( ::lseek(logfd, idx.fl_off, SEEK_CUR) == -1 )
		return false;

	data->fileline.resize(idx.fl_len, 0);
	if ( ::read(logfd, const_cast<char*>(data->fileline.data()), idx.fl_len) != idx.fl_len )
		return false;

	if ( ::lseek(logfd, idx.func_off, SEEK_CUR) == -1 )
		return false;

	data->func.resize(idx.func_len, 0);
	if ( ::read(logfd, const_cast<char*>(data->func.data()), idx.func_len) != idx.func_len )
		return false;

	if ( ::lseek(logfd, idx.data_off, SEEK_CUR) == -1 )
		return false;

	data->data.resize(idx.data_len-1, 0);
	if ( ::read(logfd, const_cast<char*>(data->data.data()), idx.data_len-1) != idx.data_len-1)
		return false;

	return true;
}

/**************************************************************************/

bool index_read_data(index_data *data, std::size_t n, int idxfd, int logfd) {
	index_record rec;

	if ( !index_read(&rec, n, idxfd) )
		return false;

	return index_read_data(data, rec, logfd);
}

/**************************************************************************/

bool index_read_all(std::vector<index_data> *data, int idxfd, int logfd) {
	const auto size = index_count(idxfd);
	data->resize(size);

	for ( auto it = data->begin(); it != data->end(); ++it ) {
		if ( !index_read_data(&(*it), std::distance(data->begin(), it), idxfd, logfd) )
			return false;
	}

	return true;
}

/**************************************************************************/

} // ns yal
