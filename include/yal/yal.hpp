
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

#ifndef _yal__yal_hpp
#define _yal__yal_hpp

#include <climits>
#include <memory>

#include <boost/format.hpp>
#include <boost/noncopyable.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/tuple/size.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/stringize.hpp>

/***************************************************************************/

namespace yal {

enum level {
	 info    = 4
	,debug   = 3
	,warning = 2
	,error   = 1
	,disable = 0
};

namespace detail {

/***************************************************************************/

struct session: private boost::noncopyable {
	static const char* sec_date_str(char *buf, const std::size_t size);
	static const char* usec_date_str(char *buf, const std::size_t size);
	static const char* level_str(level lvl);

	session(const std::string &path, const std::string &name, std::size_t volume_size, std::size_t shift_after);
	~session();

	const std::string& name() const;

	void set_buffer(const std::size_t size);
	void to_term(const bool ok, const std::string &pref);

	void set_level(const level lvl);
	yal::level get_level() const;

	void write(const char *fileline, const char *func, const std::string &data, level lvl);
	void flush();

private:
	struct impl;
	impl *pimpl;
}; // struct session

/***************************************************************************/

struct session_manager: private boost::noncopyable {
	const std::string& root_path() const;
	void root_path(const std::string& path);

	std::shared_ptr<session>
	create(const std::string &name, std::size_t volume_size, std::size_t shift_after);

	void write(const char *fileline, const char *func, const std::string &data, level lvl);

	std::shared_ptr<session>
	get(const std::string &name) const;

	void flush();

	session_manager();
	~session_manager();

private:
	struct impl;
	impl *pimpl;
}; // struct session_manager

/***************************************************************************/

} // ns detail

/***************************************************************************/

using session = std::shared_ptr<detail::session>;

struct logger: private boost::noncopyable {
	static void root_path(const std::string &path);
	static const std::string& root_path();

	static yal::session create(const std::string &name, std::size_t volume_size = UINT_MAX, std::size_t shift_after = 9999);

	static yal::session get(const std::string &name);

	static void write(const char *fileline, const char *func, const std::string &data, level lvl);

	static void flush();

private:
	static detail::session_manager* instance();
}; // struct logger

/***************************************************************************/

} // ns yal

/***************************************************************************/

// based on the: http://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments
#define __YAL_ARG16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define __YAL_HAS_COMMA(...) __YAL_ARG16(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
#define __YAL__TRIGGER_PARENTHESIS_(...) ,
#define __YAL_PASTE5(_0, _1, _2, _3, _4) _0 ## _1 ## _2 ## _3 ## _4
#define __YAL__IS_EMPTY_CASE_0001 ,
#define __YAL__IS_EMPTY(_0, _1, _2, _3) __YAL_HAS_COMMA(__YAL_PASTE5(__YAL__IS_EMPTY_CASE_, _0, _1, _2, _3))

#define __YAL_TUPLE_IS_EMPTY(...) \
	__YAL__IS_EMPTY( \
		/* test if there is just one argument, eventually an empty one */ \
		__YAL_HAS_COMMA(__VA_ARGS__), \
		/* test if _TRIGGER_PARENTHESIS_ together with the argument adds a comma */ \
		__YAL_HAS_COMMA(__YAL__TRIGGER_PARENTHESIS_ __VA_ARGS__),                 \
		/* test if the argument together with a parenthesis adds a comma */ \
		__YAL_HAS_COMMA(__VA_ARGS__ (/*empty*/)), \
		/* test if placing it between _TRIGGER_PARENTHESIS_ and the parenthesis adds a comma */ \
		__YAL_HAS_COMMA(__YAL__TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/)) \
	)

/***************************************************************************/

#define __YAL_PASTE_ELEM(unused, idx, args) \
	% (BOOST_PP_TUPLE_ELEM(idx, args))

#define __YAL_FORMAT_MESSAGE_WITHOUT_ARGS(...)

#define __YAL_FORMAT_MESSAGE_WITH_ARGS(...) \
	BOOST_PP_REPEAT( \
		 BOOST_PP_TUPLE_SIZE((__VA_ARGS__)) \
		,__YAL_PASTE_ELEM \
		,(__VA_ARGS__) \
	)

/***************************************************************************/

/* usage:
 * std::cout << YAL_FORMAT_MESSAGE("message: %1%, %2%, %3%", "string1", 33, "string2") << std::endl;
 * std::string str = YAL_FORMAT_MESSAGE_AS_STRING("message: %1%, %2%, %3%", "string1", 33, "string2");
 */

#define YAL_FORMAT_MESSAGE(fmt, ...) \
	boost::format(fmt) \
	BOOST_PP_IF( \
		 __YAL_TUPLE_IS_EMPTY(__VA_ARGS__) \
		,__YAL_FORMAT_MESSAGE_WITHOUT_ARGS \
		,__YAL_FORMAT_MESSAGE_WITH_ARGS \
	)(__VA_ARGS__)

#define YAL_FORMAT_MESSAGE_AS_STRING(fmt, ...) \
	(YAL_FORMAT_MESSAGE(fmt, __VA_ARGS__)).str()

/***************************************************************************/

#ifndef YAL_DISABLE_LOGGING
#	define YAL_SET_ROOT_PATH(path) \
		::yal::logger::root_path(path)

#	define YAL_GET_ROOT_PATH() \
		::yal::logger::root_path()
#	define YAL_GET_ROOT_PATH2(var) \
		const std::string &var = YAL_GET_ROOT_PATH()
#	define YAL_GET_ROOT_PATH3(var) \
		var = YAL_GET_ROOT_PATH()

#	define YAL_FLUSH() \
		::yal::logger::flush()

#	define YAL_SESSION_DECLARE_VAR(var) \
		::yal::session var

#	define YAL_SESSION_CREATE(var, ...) \
		YAL_SESSION_DECLARE_VAR(var) = ::yal::logger::create(__VA_ARGS__)
#	define YAL_SESSION_CREATE2(var, ...) \
		var = ::yal::logger::create(__VA_ARGS__)
#	define YAL_SESSION_CREATE3(...) \
		::yal::logger::create(__VA_ARGS__)

#	define YAL_SESSION_GET(var, name) \
		::yal::session var = ::yal::logger::get(name)
#	define YAL_SESSION_GET2(name) \
		::yal::logger::get(name)

#	define YAL_SESSION_EXISTS(name) \
		(::yal::logger::get(name).get() != 0)

#	define YAL_SESSION_FLUSH(log) \
		log->flush()

#	define YAL_SESSION_SET_LEVEL(log, lvl) \
		log->set_level((lvl))
#	define YAL_SESSION_SET_BUFFER(log, size) \
		log->set_buffer((size))
#	define YAL_SESSION_SET_UNBUFFERED(log) \
		log->set_buffer(0)
#	define YAL_SESSION_TO_TERM(log, flag, pref) \
		log->to_term((flag), (pref))

#	ifndef YAL_DISABLE_LOG_ERROR
#		define YAL_LOG_ERROR(log, ...) \
			do { \
				if ( log->get_level() >= ::yal::level::error ) { \
					log->write( \
						 __FILE__ ":" BOOST_PP_STRINGIZE(__LINE__) \
						,__PRETTY_FUNCTION__ \
						,YAL_FORMAT_MESSAGE_AS_STRING(__VA_ARGS__) \
						,::yal::level::error \
					); \
				} \
			} while(0)
#		define YAL_LOG_ERROR_IF(cond, log, ...) \
			do { \
				if ( (cond) ) YAL_LOG_ERROR(log, __VA_ARGS__); \
			} while(0)
#		define YAL_GLOBAL_LOG_ERROR(...) \
			do { \
				::yal::logger::write( \
					__FILE__ ":" BOOST_PP_STRINGIZE(__LINE__) \
				  ,__PRETTY_FUNCTION__ \
				  ,YAL_FORMAT_MESSAGE_AS_STRING(__VA_ARGS__) \
				  ,::yal::level::error \
				); \
			} while(0)
#		define YAL_GLOBAL_LOG_ERROR_IF(cond, ...) \
			do { \
				if ( (cond) ) YAL_GLOBAL_LOG_ERROR(__VA_ARGS__); \
			} while(0)
#	else // YAL_DISABLE_LOG_ERROR
#		define YAL_LOG_ERROR(log, ...) \
			do {} while(0)
#		define YAL_LOG_ERROR_IF(cond, log, ...) \
			do {} while(0)
#		define YAL_GLOBAL_LOG_ERROR(...) \
			do {} while(0)
#		define YAL_GLOBAL_LOG_ERROR_IF(cond, ...) \
			do {} while(0)
#	endif // YAL_DISABLE_LOG_ERROR

#	ifndef YAL_DISABLE_LOG_WARNING
#		define YAL_LOG_WARNING(log, ...) \
			do { \
				if ( log->get_level() >= ::yal::level::warning ) { \
					log->write( \
						 __FILE__ ":" BOOST_PP_STRINGIZE(__LINE__) \
						,__PRETTY_FUNCTION__ \
						,YAL_FORMAT_MESSAGE_AS_STRING(__VA_ARGS__) \
						,::yal::level::warning \
					); \
				} \
			} while(0)
#		define YAL_LOG_WARNING_IF(cond, log, ...) \
			do { \
				if ( (cond) ) YAL_LOG_WARNING(log, __VA_ARGS__); \
			} while(0)
#		define YAL_GLOBAL_LOG_WARNING(...) \
			do { \
				::yal::logger::write( \
					__FILE__ ":" BOOST_PP_STRINGIZE(__LINE__) \
				  ,__PRETTY_FUNCTION__ \
				  ,YAL_FORMAT_MESSAGE_AS_STRING(__VA_ARGS__) \
				  ,::yal::level::warning \
				); \
			} while(0)
#		define YAL_GLOBAL_LOG_WARNING_IF(cond, ...) \
			do { \
				if ( (cond) ) YAL_GLOBAL_LOG_WARNING(__VA_ARGS__); \
			} while(0)
#	else // YAL_DISABLE_LOG_WARNING
#		define YAL_LOG_WARNING(log, ...) \
			do {} while(0)
#		define YAL_LOG_WARNING_IF(cond, log, ...) \
			do {} while(0)
#		define YAL_GLOBAL_LOG_WARNING(...) \
			do {} while(0)
#		define YAL_GLOBAL_LOG_WARNING_IF(cond, ...) \
			do {} while(0)
#	endif // YAL_DISABLE_LOG_WARNING

#	ifndef YAL_DISABLE_LOG_DEBUG
#		define YAL_LOG_DEBUG(log, ...) \
			do { \
				if ( log->get_level() >= ::yal::level::debug ) { \
					log->write( \
						 __FILE__ ":" BOOST_PP_STRINGIZE(__LINE__) \
						,__PRETTY_FUNCTION__ \
						,YAL_FORMAT_MESSAGE_AS_STRING(__VA_ARGS__) \
						,::yal::level::debug \
					); \
				} \
			} while(0)
#		define YAL_LOG_DEBUG_IF(cond, log, ...) \
			do { \
				if ( (cond) ) YAL_LOG_DEBUG(log, __VA_ARGS__); \
			} while(0)
#		define YAL_GLOBAL_LOG_DEBUG(...) \
			do { \
				::yal::logger::write( \
					__FILE__ ":" BOOST_PP_STRINGIZE(__LINE__) \
				  ,__PRETTY_FUNCTION__ \
				  ,YAL_FORMAT_MESSAGE_AS_STRING(__VA_ARGS__) \
				  ,::yal::level::debug \
				); \
			} while(0)
#		define YAL_GLOBAL_LOG_DEBUG_IF(cond, ...) \
			do { \
				if ( (cond) ) YAL_GLOBAL_LOG_DEBUG(__VA_ARGS__); \
			} while(0)
#	else // YAL_DISABLE_LOG_DEBUG
#		define YAL_LOG_DEBUG(log, ...) \
			do {} while(0)
#		define YAL_LOG_DEBUG_IF(cond, log, ...) \
			do {} while(0)
#		define YAL_GLOBAL_LOG_DEBUG(...) \
			do {} while(0)
#		define YAL_GLOBAL_LOG_DEBUG_IF(cond, ...) \
			do {} while(0)
#	endif // YAL_DISABLE_LOG_DEBUG

#	ifndef YAL_DISABLE_LOG_INFO
#		define YAL_LOG_INFO(log, ...) \
			do { \
				if ( log->get_level() == ::yal::level::info ) { \
					log->write( \
						 __FILE__ ":" BOOST_PP_STRINGIZE(__LINE__) \
						,__PRETTY_FUNCTION__ \
						,YAL_FORMAT_MESSAGE_AS_STRING(__VA_ARGS__) \
						,::yal::level::info \
					); \
				} \
			} while(0)
#		define YAL_LOG_INFO_IF(cond, log, ...) \
			do { \
				if ( (cond) ) YAL_LOG_INFO(log, __VA_ARGS__); \
			} while(0)
#		define YAL_GLOBAL_LOG_INFO(...) \
			do { \
				::yal::logger::write( \
					__FILE__ ":" BOOST_PP_STRINGIZE(__LINE__) \
				  ,__PRETTY_FUNCTION__ \
				  ,YAL_FORMAT_MESSAGE_AS_STRING(__VA_ARGS__) \
				  ,::yal::level::info \
				); \
			} while(0)
#		define YAL_GLOBAL_LOG_INFO_IF(cond, ...) \
			do { \
				if ( (cond) ) YAL_GLOBAL_LOG_INFO(__VA_ARGS__); \
			} while(0)
#	else // YAL_DISABLE_LOG_INFO
#		define YAL_LOG_INFO(log, ...) \
			do {} while(0)
#		define YAL_LOG_INFO_IF(cond, log, ...) \
			do {} while(0)
#		define YAL_GLOBAL_LOG_INFO(...) \
			do {} while(0)
#		define YAL_GLOBAL_LOG_INFO_IF(cond, ...) \
			do {} while(0)
#	endif // YAL_DISABLE_LOG_INFO
#else // YAL_DISABLE_LOGGING == true
#	define YAL_SET_ROOT_PATH(path)
#	define YAL_GET_ROOT_PATH(var)
#	define YAL_GET_ROOT_PATH2(var)
#	define YAL_GET_ROOT_PATH3(var)

#	define YAL_FLUSH()

#	define YAL_SESSION_DECLARE_VAR(var)

#	define YAL_SESSION_CREATE(var, ...)
#	define YAL_SESSION_CREATE2(var, ...)
#	define YAL_SESSION_CREATE3(...)

#	define YAL_SESSION_GET(var, name)
#	define YAL_SESSION_GET2(name)

#	define YAL_SESSION_EXISTS(name)

#	define YAL_SESSION_FLUSH(log)

#	define YAL_SESSION_SET_LEVEL(log, lvl)
#	define YAL_SESSION_SET_BUFFER(log, size)
#	define YAL_SESSION_SET_UNBUFFERED(log)
#	define YAL_SESSION_TO_TERM(log, flag, pref)

#	define YAL_LOG_ERROR(log, ...) \
		do {} while(0)
#	define YAL_LOG_ERROR_IF(cond, log, ...) \
		do {} while(0)
#	define YAL_GLOBAL_LOG_ERROR(...) \
		do {} while(0)
#	define YAL_GLOBAL_LOG_ERROR_IF(cond, ...) \
		do {} while(0)
#	define YAL_LOG_WARNING(log, ...) \
		do {} while(0)
#	define YAL_LOG_WARNING_IF(cond, log, ...) \
		do {} while(0)
#	define YAL_GLOBAL_LOG_WARNING(...) \
		do {} while(0)
#	define YAL_GLOBAL_LOG_WARNING_IF(cond, ...) \
		do {} while(0)
#	define YAL_LOG_DEBUG(log, ...) \
		do {} while(0)
#	define YAL_LOG_DEBUG_IF(cond, log, ...) \
		do {} while(0)
#	define YAL_GLOBAL_LOG_DEBUG(...) \
		do {} while(0)
#	define YAL_GLOBAL_LOG_DEBUG_IF(cond, ...) \
		do {} while(0)
#	define YAL_LOG_INFO(log, ...) \
		do {} while(0)
#	define YAL_LOG_INFO_IF(cond, log, ...) \
		do {} while(0)
#	define YAL_GLOBAL_LOG_INFO(...) \
		do {} while(0)
#	define YAL_GLOBAL_LOG_INFO_IF(cond, ...) \
		do {} while(0)
#endif // YAL_DISABLE_LOGGING

/***************************************************************************/

#ifndef YAL_DISABLE_TESTS
#	define YAL_TEST_COND(log, l, cmp, r) \
		do { if ( !((l) cmp (r)) ) YAL_LOG_ERROR(log, "test_condition \"" #l " " #cmp " " #r "\" is false"); } while(0)
#	define YAL_TEST_LESS(log, l, r) \
		YAL_TEST_COND(log, l, <, r)
#	define YAL_TEST_LESSEQ(log, l, r) \
		YAL_TEST_COND(log, l, <=, r)
#	define YAL_TEST_EQ(log, l, r) \
		YAL_TEST_COND(log, l, ==, r)
#	define YAL_TEST_NEQ(log, l, r) \
		YAL_TEST_COND(log, l, !=, r)
#	define YAL_TEST_GR(log, l, r) \
		YAL_TEST_COND(log, l, >, r)
#	define YAL_TEST_GREQ(log, l, r) \
		YAL_TEST_COND(log, l, >=, r)
#	define YAL_TEST_ZERO(log, v) \
		YAL_TEST_COND(log, v, ==, 0)
#	define YAL_TEST_NOTZERO(log, v) \
		YAL_TEST_COND(log, v, !=, 0)
#	define YAL_TEST_NULL(log, v) \
		YAL_TEST_COND(log, v, ==, nullptr)
#	define YAL_TEST_NOTNULL(log, v) \
		YAL_TEST_COND(log, v, !=, nullptr)
#else // !YAL_DISABLE_TESTS
#	define YAL_TEST_LESS(log, l, r) \
		do {} while(0)
#	define YAL_TEST_LESSEQ(log, l, r) \
		do {} while(0)
#	define YAL_TEST_EQ(log, l, r) \
		do {} while(0)
#	define YAL_TEST_NEQ(log, l, r) \
		do {} while(0)
#	define YAL_TEST_GR(log, l, r) \
		do {} while(0)
#	define YAL_TEST_GREQ(log, l, r) \
		do {} while(0)
#	define YAL_TEST_ZERO(log, v) \
		do {} while(0)
#	define YAL_TEST_NOTZERO(log, v) \
		do {} while(0)
#	define YAL_TEST_NULL(log, v) \
		do {} while(0)
#	define YAL_TEST_NOTNULL(log, v) \
		do {} while(0)
#endif // YAL_DISABLE_TESTS

#ifndef YAL_DISABLE_ASSERT
#	define YAL_ASSERT_IMPL_TO_LOG(expr, log) \
		do { \
			if ( !(expr) ) { \
				log->write( \
					 __FILE__ ":" BOOST_PP_STRINGIZE(__LINE__) \
					,__PRETTY_FUNCTION__ \
					,"assert \"" #expr "\" is false" \
					,::yal::level::error \
				); \
				::yal::logger::flush(); \
				std::abort(); \
			} \
		} while(0)
#	define YAL_ASSERT_IMPL_TO_CERR(expr) \
		do { \
			if ( !(expr) ) { \
				char datebuf[32] = "\0"; \
				fprintf( \
					 stderr \
					,"[%1%][assert ][" __FILE__ ":" BOOST_PP_STRINGIZE(__LINE__) "][%2%]: expression \"" #expr "\" is false\n" \
					,::yal::detail::session::usec_date_str(datebuf, sizeof(datebuf)) \
					,__PRETTY_FUNCTION__ \
				); \
				std::fflush(stderr); \
				std::abort(); \
			} \
		} while(0)
#	define YAL_ASSERT(...) \
		BOOST_PP_IF( \
			 BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), 1) \
			,YAL_ASSERT_IMPL_TO_CERR \
			,YAL_ASSERT_IMPL_TO_LOG \
		)(__VA_ARGS__)
#else // !YAL_DISABLE_ASSERT
#	define YAL_ASSERT(...) \
		do {} while(0)
#endif // YAL_DISABLE_ASSERT

#ifndef YAL_DISABLE_TIMEPOINT
#include <chrono>

namespace yal {
namespace detail {

struct timepoint {
	const std::size_t sline;
	const char *descr;
	const std::chrono::high_resolution_clock::time_point time;
};

} // ns detail
} // ns yal

#	define YAL_MAKE_TIMEPOINT(name, descr) \
		const ::yal::detail::timepoint _yal_timepoint_##name{__LINE__, descr, std::chrono::high_resolution_clock::now()}
#	define YAL_PRINT_TIMEPOINT(log, name) \
		do { \
			const auto d  = std::chrono::high_resolution_clock::now() - _yal_timepoint_##name.time; \
			log->write( \
				 __FILE__ ":" BOOST_PP_STRINGIZE(__LINE__) \
				,__PRETTY_FUNCTION__ \
				,YAL_FORMAT_MESSAGE_AS_STRING( \
					 "execution time of scope(\"%s\") in lines %d-%d is %ds-%dms-%dus-%dns" \
					,_yal_timepoint_##name.descr \
					,_yal_timepoint_##name.sline \
					,__LINE__ \
					,std::chrono::duration_cast<std::chrono::seconds     >(d % std::chrono::minutes(1)     ).count() \
					,std::chrono::duration_cast<std::chrono::milliseconds>(d % std::chrono::seconds(1)     ).count() \
					,std::chrono::duration_cast<std::chrono::microseconds>(d % std::chrono::milliseconds(1)).count() \
					,std::chrono::duration_cast<std::chrono::nanoseconds >(d % std::chrono::microseconds(1)).count() \
				) \
				,::yal::level::info \
			); \
		} while(0)
#	define YAL_PRINT_TIMEPOINT_IF(expr, log, name) \
		if ( (expr) ) YAL_PRINT_TIMEPOINT(log, name) \

#else // !YAL_DISABLE_TIMEPOINT
#	define YAL_MAKE_TIMEPOINT(name, descr) \
		do {} while(0)
#	define YAL_PRINT_TIMEPOINT(log, name) \
		do {} while(0)
#	define YAL_PRINT_TIMEPOINT_IF(expr, log, name) \
		do {} while(0)
#endif // YAL_DISABLE_TIMEPOINT

/***************************************************************************/

#endif // _yal__yal_hpp
