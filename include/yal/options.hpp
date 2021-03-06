
// Copyright (c) 2013-2020 niXman (i dotty nixman doggy gmail dotty com)
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

#ifndef _yal__options_hpp
#define _yal__options_hpp

#include <cstdint>

namespace yal {

enum options: std::size_t {
     sec_res             = 1u<<0u  // seconds resolution (by default)
    ,msec_res            = 1u<<1u  // milliseconds resolution
    ,usec_res            = 1u<<2u  // microseconds resolution
    ,nsec_res            = 1u<<3u  // nanoseconds resolution
    ,remove_empty_logs   = 1u<<4u  // remove empty volumes if exists
    ,fsync_each_record   = 1u<<5u  // fdatasync's each record
    ,compress            = 1u<<6u  // compress volumes
    ,full_source_name    = 1u<<7u  // don't show full file path
    ,full_func_name      = 1u<<8u  // i.e. 'void func(int)'
    ,create_index_file   = 1u<<9u  // create index-file for each log file
};

} // ns yal

#endif // _yal__options_hpp
