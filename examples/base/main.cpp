
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

#include <iostream>
#include <thread>

#include <yal/yal.hpp>

/***************************************************************************/

int main() {
	static const char *s1name = "test1/test1/test1.log";
	static const char *s2name = "test2.log";
	static const char *s3name = "test3/test3";
	static const char *s4name = "test4";
	static const char *s5name = "test5";

	try {
		auto lmbd = [](const char *ptr, std::size_t size) { return std::make_pair(ptr, size); };
		YAL_SESSION_CREATE(test1, s1name, 1024*1024, yal::sec_res |yal::flush_each_record, std::move(lmbd));
		YAL_SESSION_CREATE(test2, s2name, 1024*1024, yal::usec_res|yal::compress);
		YAL_SESSION_CREATE(test3, s3name, 1024*1024, yal::nsec_res|yal::unbuffered);
		YAL_SESSION_CREATE(test4, s4name, 1024*1024, yal::nsec_res|yal::compress);
//		YAL_SESSION_TO_TERM(test1, true, "term1");

		for ( auto idx = 0ul, idx2 = 0ul; idx < 1024ul*10ul; idx+=2, idx2+=3 ) {
			YAL_LOG_INFO	(test1, "%1% -> %2% -> %1%", idx, idx+1);
			YAL_LOG_DEBUG	(test1, "%1% -> %2% -> %1%", idx, idx+1);
			YAL_LOG_WARNING(test1, "%1% -> %2% -> %1%", idx, idx+1);
			YAL_LOG_ERROR	(test1, "%1% -> %2% -> %1%", idx, idx+1);

			YAL_LOG_INFO	(test2, "%016d -> %016d", idx, idx);
			YAL_LOG_DEBUG	(test2, "%016d -> %016d", idx, idx);
			YAL_LOG_WARNING(test2, "%016d -> %016d", idx, idx);
			YAL_LOG_ERROR	(test2, "%016d -> %016d", idx, idx);

			YAL_LOG_INFO	(test3, "%016d -> %016d", idx, idx);
			YAL_LOG_DEBUG	(test3, "%016d -> %016d", idx, idx);
			YAL_LOG_WARNING(test3, "%016d -> %016d", idx, idx);
			YAL_LOG_ERROR	(test3, "%016d -> %016d", idx, idx);

			YAL_LOG_INFO	(test4, "%016d -> %016d", idx, idx);
			YAL_LOG_DEBUG	(test4, "%016d -> %016d", idx, idx);
			YAL_LOG_WARNING(test4, "%016d -> %016d", idx, idx);
			YAL_LOG_ERROR	(test4, "%016d -> %016d", idx, idx);

			YAL_GLOBAL_LOG_INFO		("%016d -> %016d", idx2, idx2);
			YAL_GLOBAL_LOG_DEBUG		("%016d -> %016d", idx2, idx2);
			YAL_GLOBAL_LOG_WARNING	("%016d -> %016d", idx2, idx2);
			YAL_GLOBAL_LOG_ERROR		("%016d -> %016d", idx2, idx2);
		}
		YAL_FLUSH();

		YAL_SESSION_GET2(ts1, s1name);
		YAL_ASSERT_TERM(std::cerr, ts1);
		YAL_SESSION_GET2(ts2, s2name);
		YAL_ASSERT_TERM(std::cerr, ts2);
		YAL_SESSION_GET2(ts3, s3name);
		YAL_ASSERT_TERM(std::cerr, ts3);
		YAL_SESSION_GET2(ts4, s4name);
		YAL_ASSERT_TERM(std::cerr, ts4);
		YAL_SESSION_GET2(ts5, s5name);
		YAL_ASSERT_TERM(std::cerr, !ts5);

		YAL_ASSERT_TERM(std::cerr,  YAL_SESSION_EXISTS(s1name));
		YAL_ASSERT_TERM(std::cerr,  YAL_SESSION_EXISTS(s2name));
		YAL_ASSERT_TERM(std::cerr,  YAL_SESSION_EXISTS(s3name));
		YAL_ASSERT_TERM(std::cerr,  YAL_SESSION_EXISTS(s4name));
		YAL_ASSERT_TERM(std::cerr, !YAL_SESSION_EXISTS(s5name));

		YAL_SESSION_CREATE(test5, s5name, 1024*10, yal::usec_res|yal::flush_each_record);
		YAL_SESSION_TO_TERM(test5, true, "test5 term");
		YAL_TEST_LESS		(test5, 0, 1);
		YAL_TEST_LESS		(test5, 1, 1); // test fail
		YAL_TEST_LESSEQ	(test5, 1, 1);
		YAL_TEST_LESSEQ	(test5, 2, 1); // test fail
		YAL_TEST_EQ			(test5, 1, 1);
		YAL_TEST_EQ			(test5, 2, 1); // test fail
		YAL_TEST_NEQ		(test5, 0, 1);
		YAL_TEST_NEQ		(test5, 1, 1); // test fail
		YAL_TEST_GR			(test5, 1, 0);
		YAL_TEST_GR			(test5, 1, 1); // test fail
		YAL_TEST_GREQ		(test5, 1, 0);
		YAL_TEST_GREQ		(test5, 1, 2); // test fail
		YAL_TEST_ZERO		(test5, 0);
		YAL_TEST_ZERO		(test5, 1); // test fail
		YAL_TEST_NOTZERO	(test5, 1);
		YAL_TEST_NOTZERO	(test5, 0); // test fail
		YAL_TEST_NULL		(test5, nullptr);
		YAL_TEST_NULL		(test5, "cstring"); // test fail
		YAL_TEST_NOTNULL	(test5, "cstring");
		YAL_TEST_NOTNULL	(test5, nullptr); // test fail

#if 0
		YAL_ASSERT_LOG(test5, true);
		YAL_ASSERT_LOG(test5, false); // test fail
		YAL_ASSERT_LOG(test5, true);
		YAL_ASSERT_LOG(test5, false); // test fail

		YAL_ASSERT_TERM(std::cerr, true);
		YAL_ASSERT_TERM(std::cerr, false); // test fail
		YAL_ASSERT_TERM(std::cerr, true);
		YAL_ASSERT_TERM(std::cerr, false); // test fail
#endif

		YAL_MAKE_TIMEPOINT(tp1, "tp1 description");
		std::this_thread::sleep_for(std::chrono::nanoseconds(3333333333));
		YAL_PRINT_TIMEPOINT(test5, tp1);

		YAL_TRY(scope_flag0) {
			throw std::runtime_error("std::exception message");
		}
		YAL_CATCH(test5, scope_flag0, "catch0 message")
		YAL_ASSERT_LOG(test5, scope_flag0);

		YAL_TRY(scope_flag1) {
			throw 1;
		}
		YAL_CATCH(test5, scope_flag1, "catch1 message")
		YAL_ASSERT_LOG(test5, scope_flag1);

		YAL_TRY(scope_flag2) {
			throw std::invalid_argument("std::invalid_argument message");
		}
		YAL_TYPED_CATCH(test5, std::invalid_argument, scope_flag2, "catch2 message")
		YAL_CATCH(test5, scope_flag2, "catch3 message")
		YAL_ASSERT_LOG(test5, scope_flag2);

	} catch(const std::exception &ex) {
		std::cout << "[std::exception]: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	} catch(...) {
		std::cout << "[unexpected exception]" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "EXIT_SUCCESS" << std::endl;
	return EXIT_SUCCESS;
}

/***************************************************************************/
