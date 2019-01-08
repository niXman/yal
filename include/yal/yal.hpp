
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

#ifndef _yal__yal_hpp
#define _yal__yal_hpp

#include <yal/options.hpp>

#define DTF_HEADER_ONLY
#include <yal/dtf.hpp>

#include <boost/format.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/tuple/size.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION >= 106000
#   include <boost/vmd/is_empty.hpp>
#endif // ! BOOST_VERSION >= 106000

#include <cstdint>
#include <cstring>
#include <climits>
#include <memory>
#include <functional>

/***************************************************************************/

#ifndef YAL_MAX_VOLUME_NUMBER
#   define YAL_MAX_VOLUME_NUMBER 99999
#endif // YAL_MAX_VOLUME_NUMBER

#ifndef YAL_COMPRESSION_LEVEL
#   define YAL_COMPRESSION_LEVEL 1
#endif // YAL_COMPRESSION_LEVEL

/***************************************************************************/

namespace yal {

enum level {
     info    = 4
    ,debug   = 3
    ,warning = 2
    ,error   = 1
    ,disable = 0
};

char level_chr(const level lvl);

/***************************************************************************/

namespace detail {

using process_buffer = std::function<
    std::pair<const char*, std::size_t>(const char*, std::size_t)
>;

struct session {
    session(const session &) = delete;
    session& operator=(const session &) = delete;

    session(
         const std::string &path
        ,const std::string &name
        ,std::size_t volume_size
        ,uint32_t opts
        ,process_buffer broc
    );
    virtual ~session();

    const std::string& name() const;

    void to_term(const bool ok, const std::string &pref);

    void set_level(const level lvl);
    yal::level get_level() const;

    void write(
         const char *fileline
        ,const std::size_t fileline_len
        ,const char *sfunc
        ,const std::size_t sfunc_len
        ,const char *func
        ,const std::size_t func_len
        ,const std::string &data
        ,const level lvl
    );
    void flush();

private:
    struct impl;
    impl *pimpl;
}; // struct session

/***************************************************************************/

struct session_manager {
    session_manager(const session_manager &) = delete;
    session_manager& operator=(const session_manager &) = delete;

    session_manager();
    virtual ~session_manager();

    const std::string& root_path() const;
    void root_path(const std::string& path);

    std::shared_ptr<session>
    create(const std::string &name, std::size_t volume_size, uint32_t opts, process_buffer proc);

    void write(
         const char *fileline
        ,const std::size_t fileline_len
        ,const char *sfunc
        ,const std::size_t sfunc_len
        ,const char *func
        ,const std::size_t func_len
        ,const std::string &data
        ,const level lvl
    );

    std::shared_ptr<session>
    get(const std::string &name) const;

    void flush();

private:
    struct impl;
    impl *pimpl;
}; // struct session_manager

/***************************************************************************/

} // ns detail

/***************************************************************************/

using session = std::shared_ptr<detail::session>;

struct logger {
    logger(const logger &) = delete;
    logger& operator=(const logger &) = delete;

    static void root_path(const std::string &path);
    static const std::string& root_path();

    static yal::session create(
         const std::string &name
        ,std::size_t volume_size = UINT_MAX
        ,std::uint32_t opts = options::sec_res
        ,detail::process_buffer proc = detail::process_buffer()
    );

    static yal::session get(const std::string &name);

    static void write(
         const char *fileline
        ,const std::size_t fileline_len
        ,const char *sfunc
        ,const std::size_t sfunc_len
        ,const char *func
        ,const std::size_t func_len
        ,const std::string &data
        ,const level lvl
    );

    static void flush();

private:
    static detail::session_manager* instance();
}; // struct logger

/***************************************************************************/

} // ns yal

/***************************************************************************/

#if BOOST_VERSION >= 106000
#   define __YAL_PP_TUPLE_IS_EMPTY(...) BOOST_VMD_IS_EMPTY(__VA_ARGS__)
#else // ! BOOST_VERSION >= 106000
// based on the: http://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments
#   define __YAL_PP_ARG50( \
          _0 , _1 , _2 , _3 , _4 , _5 , _6 , _7 , _8 , _9 \
        , _10, _11, _12, _13, _14, _15, _16, _17, _18, _19 \
        , _20, _21, _22, _23, _24, _25, _26, _27, _28, _29 \
        , _30, _31, _32, _33, _34, _35, _36, _37, _38, _39 \
        , _40, _41, _42, _43, _44, _45, _46, _47, _48, _49 \
        , ...) _49
#   define __YAL_PP_HAS_COMMA(...) \
        __YAL_PP_ARG50( \
            __VA_ARGS__, \
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
            1, 1, 1, 1, 1, 1, 1, 1, 0 \
        )
#   define __YAL_PP_TRIGGER_PARENTHESIS_(...) ,
#   define __YAL_PP_PASTE5(_0, _1, _2, _3, _4) _0 ## _1 ## _2 ## _3 ## _4
#   define __YAL_PP_IS_EMPTY_CASE_0001 ,
#   define __YAL_PP_IS_EMPTY_IMPL(_0, _1, _2, _3) __YAL_PP_HAS_COMMA(__YAL_PP_PASTE5(__YAL_PP_IS_EMPTY_CASE_, _0, _1, _2, _3))
#   define __YAL_PP_TUPLE_IS_EMPTY(...) \
        __YAL_PP_IS_EMPTY_IMPL( \
            __YAL_PP_HAS_COMMA(__VA_ARGS__), \
            __YAL_PP_HAS_COMMA(__YAL_PP_TRIGGER_PARENTHESIS_ __VA_ARGS__),                 \
            __YAL_PP_HAS_COMMA(__VA_ARGS__ (/*empty*/)), \
            __YAL_PP_HAS_COMMA(__YAL_PP_TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/)) \
        )
#endif // BOOST_VERSION >= 106000

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
         __YAL_PP_TUPLE_IS_EMPTY(__VA_ARGS__) \
        ,__YAL_FORMAT_MESSAGE_WITHOUT_ARGS \
        ,__YAL_FORMAT_MESSAGE_WITH_ARGS \
    )(__VA_ARGS__)

#define YAL_FORMAT_MESSAGE_AS_STRING(fmt, ...) \
    (YAL_FORMAT_MESSAGE(fmt, __VA_ARGS__)).str()

/***************************************************************************/

#ifndef YAL_DISABLE_LOGGING
#   define YAL_EXPAND_EXPR(...) \
        __VA_ARGS__
#   define YAL_SET_ROOT_PATH(path) \
        ::yal::logger::root_path(path)

#   define YAL_GET_ROOT_PATH() \
        ::yal::logger::root_path()
#   define YAL_GET_ROOT_PATH2(var) \
        const std::string &var = YAL_GET_ROOT_PATH()
#   define YAL_GET_ROOT_PATH3(var) \
        var = YAL_GET_ROOT_PATH()

#   define YAL_FLUSH() \
        ::yal::logger::flush()

#   define YAL_SESSION_DECLARE_VAR(var) \
        ::yal::session var

#   define YAL_SESSION_INIT_VAR(l, r) \
        :l(r)
#   define YAL_SESSION_INIT_VAR2(l, r) \
        ,l(r)

#   define YAL_SESSION_CREATE(var, ...) \
        YAL_SESSION_DECLARE_VAR(var) = ::yal::logger::create(__VA_ARGS__)
#   define YAL_SESSION_CREATE2(var, ...) \
        var = ::yal::logger::create(__VA_ARGS__)
#   define YAL_SESSION_CREATE3(var, ...) \
        :var(::yal::logger::create(__VA_ARGS__))
#   define YAL_SESSION_CREATE4(var, ...) \
        ,var(::yal::logger::create(__VA_ARGS__))

#   define YAL_SESSION_GET(name) \
        ::yal::logger::get(name)
#   define YAL_SESSION_GET2(var, name) \
        YAL_SESSION_DECLARE_VAR(var) = YAL_SESSION_GET(name)

#   define YAL_SESSION_EXISTS(name) \
        (::yal::logger::get(name).get() != 0)

#   define YAL_SESSION_FLUSH(log) \
        log->flush()

#   define YAL_SESSION_SET_LEVEL(log, lvl) \
        log->set_level((lvl))
#   define YAL_SESSION_SET_BUFFER(log, size) \
        log->set_buffer((size))
#   define YAL_SESSION_SET_UNBUFFERED(log) \
        log->set_buffer(0)
#   define YAL_SESSION_TO_TERM(log, flag, pref) \
        log->to_term((flag), (pref))

#   define __YAL_LOG_IMPL(log, errlvl, ...) \
        do { \
            if ( log->get_level() >= ::yal::level::errlvl ) { \
                static const char flbuf[] = __FILE__ ":" BOOST_PP_STRINGIZE(__LINE__); \
                log->write( \
                     flbuf \
                    ,sizeof(flbuf)-1 \
                    ,__FUNCTION__ \
                    ,sizeof(__FUNCTION__)-1 \
                    ,__PRETTY_FUNCTION__ \
                    ,sizeof(__PRETTY_FUNCTION__)-1 \
                    ,YAL_FORMAT_MESSAGE_AS_STRING(__VA_ARGS__) \
                    ,::yal::level::errlvl \
                ); \
            } \
        } while(false)
#   define __YAL_GLOBAL_LOG_IMPL(errlvl, ...) \
        do { \
            static const char flbuf[] = __FILE__ ":" BOOST_PP_STRINGIZE(__LINE__); \
            ::yal::logger::write( \
                 flbuf \
                ,sizeof(flbuf)-1 \
                ,__FUNCTION__ \
                ,sizeof(__FUNCTION__)-1 \
                ,__PRETTY_FUNCTION__ \
                ,sizeof(__PRETTY_FUNCTION__)-1 \
                ,YAL_FORMAT_MESSAGE_AS_STRING(__VA_ARGS__) \
                ,::yal::level::errlvl \
            ); \
        } while(false)

#   ifndef YAL_DISABLE_LOG_ERROR
#       define YAL_LOG_ERROR(log, ...) __YAL_LOG_IMPL(log, error, __VA_ARGS__)
#       define YAL_LOG_ERROR_IF(log, cond, ...) if ( (cond) ) YAL_LOG_ERROR(log, __VA_ARGS__)
#       define YAL_GLOBAL_LOG_ERROR(...) __YAL_GLOBAL_LOG_IMPL(error, __VA_ARGS__)
#       define YAL_GLOBAL_LOG_ERROR_IF(cond, ...) if ( (cond) ) YAL_GLOBAL_LOG_ERROR(__VA_ARGS__)
#   else // YAL_DISABLE_LOG_ERROR
#       define YAL_LOG_ERROR(log, ...) do {} while(false)
#       define YAL_LOG_ERROR_IF(log, cond, ...) do {} while(false)
#       define YAL_GLOBAL_LOG_ERROR(...) do {} while(false)
#       define YAL_GLOBAL_LOG_ERROR_IF(cond, ...) do {} while(false)
#   endif // YAL_DISABLE_LOG_ERROR

#   ifndef YAL_DISABLE_LOG_WARNING
#       define YAL_LOG_WARNING(log, ...) __YAL_LOG_IMPL(log, warning, __VA_ARGS__)
#       define YAL_LOG_WARNING_IF(log, cond, ...) if ( (cond) ) YAL_LOG_WARNING(log, __VA_ARGS__)
#       define YAL_GLOBAL_LOG_WARNING(...) __YAL_GLOBAL_LOG_IMPL(warning, __VA_ARGS__)
#       define YAL_GLOBAL_LOG_WARNING_IF(cond, ...) if ( (cond) ) YAL_GLOBAL_LOG_WARNING(__VA_ARGS__)
#   else // YAL_DISABLE_LOG_WARNING
#       define YAL_LOG_WARNING(log, ...) do {} while(false)
#       define YAL_LOG_WARNING_IF(log, cond, ...) do {} while(false)
#       define YAL_GLOBAL_LOG_WARNING(...) do {} while(false)
#       define YAL_GLOBAL_LOG_WARNING_IF(cond, ...) do {} while(false)
#   endif // YAL_DISABLE_LOG_WARNING

#   ifndef YAL_DISABLE_LOG_DEBUG
#       define YAL_LOG_DEBUG(log, ...) __YAL_LOG_IMPL(log, debug, __VA_ARGS__)
#       define YAL_LOG_DEBUG_IF(log, cond, ...) if ( (cond) ) YAL_LOG_DEBUG(log, __VA_ARGS__)
#       define YAL_GLOBAL_LOG_DEBUG(...) __YAL_GLOBAL_LOG_IMPL(debug, __VA_ARGS__)
#       define YAL_GLOBAL_LOG_DEBUG_IF(cond, ...) if ( (cond) ) YAL_GLOBAL_LOG_DEBUG(__VA_ARGS__)
#   else // YAL_DISABLE_LOG_DEBUG
#       define YAL_LOG_DEBUG(log, ...) do {} while(false)
#       define YAL_LOG_DEBUG_IF(log, cond, ...) do {} while(false)
#       define YAL_GLOBAL_LOG_DEBUG(...) do {} while(false)
#       define YAL_GLOBAL_LOG_DEBUG_IF(cond, ...) do {} while(false)
#   endif // YAL_DISABLE_LOG_DEBUG

#   ifndef YAL_DISABLE_LOG_INFO
#       define YAL_LOG_INFO(log, ...) __YAL_LOG_IMPL(log, info, __VA_ARGS__)
#       define YAL_LOG_INFO_IF(log, cond, ...) if ( (cond) ) YAL_LOG_INFO(log, __VA_ARGS__)
#       define YAL_GLOBAL_LOG_INFO(...) __YAL_GLOBAL_LOG_IMPL(info, __VA_ARGS__)
#       define YAL_GLOBAL_LOG_INFO_IF(cond, ...) if ( (cond) ) YAL_GLOBAL_LOG_INFO(__VA_ARGS__)
#   else // YAL_DISABLE_LOG_INFO
#       define YAL_LOG_INFO(log, ...) do {} while(false)
#       define YAL_LOG_INFO_IF(log, cond, ...) do {} while(false)
#       define YAL_GLOBAL_LOG_INFO(...) do {} while(false)
#       define YAL_GLOBAL_LOG_INFO_IF(cond, ...) do {} while(false)
#   endif // YAL_DISABLE_LOG_INFO
#else // YAL_DISABLE_LOGGING == true
#   define YAL_EXPAND_EXPR(...)
#   define YAL_SET_ROOT_PATH(path)
#   define YAL_GET_ROOT_PATH(var)
#   define YAL_GET_ROOT_PATH2(var)
#   define YAL_GET_ROOT_PATH3(var)

#   define YAL_FLUSH()

#   define YAL_SESSION_DECLARE_VAR(var)

#   define YAL_SESSION_INIT_VAR(l, r)
#   define YAL_SESSION_INIT_VAR2(l, r)

#   define YAL_SESSION_CREATE(var, ...)
#   define YAL_SESSION_CREATE2(var, ...)
#   define YAL_SESSION_CREATE3(var, ...)
#   define YAL_SESSION_CREATE4(var, ...)

#   define YAL_SESSION_GET(name)
#   define YAL_SESSION_GET2(var, name)

#   define YAL_SESSION_EXISTS(name)

#   define YAL_SESSION_FLUSH(log)

#   define YAL_SESSION_SET_LEVEL(log, lvl)
#   define YAL_SESSION_SET_BUFFER(log, size)
#   define YAL_SESSION_SET_UNBUFFERED(log)
#   define YAL_SESSION_TO_TERM(log, flag, pref)

#   define YAL_LOG_ERROR(log, ...) do {} while(false)
#   define YAL_LOG_ERROR_IF(log, cond, ...) do {} while(false)
#   define YAL_GLOBAL_LOG_ERROR(...) do {} while(false)
#   define YAL_GLOBAL_LOG_ERROR_IF(cond, ...) do {} while(false)
#   define YAL_LOG_WARNING(log, ...) do {} while(false)
#   define YAL_LOG_WARNING_IF(log, cond, ...) do {} while(false)
#   define YAL_GLOBAL_LOG_WARNING(...) do {} while(false)
#   define YAL_GLOBAL_LOG_WARNING_IF(cond, ...) do {} while(false)
#   define YAL_LOG_DEBUG(log, ...) do {} while(false)
#   define YAL_LOG_DEBUG_IF(log, cond, ...) do {} while(false)
#   define YAL_GLOBAL_LOG_DEBUG(...) do {} while(false)
#   define YAL_GLOBAL_LOG_DEBUG_IF(cond, ...) do {} while(false)
#   define YAL_LOG_INFO(log, ...) do {} while(false)
#   define YAL_LOG_INFO_IF(log, cond, ...) do {} while(false)
#   define YAL_GLOBAL_LOG_INFO(...) do {} while(false)
#   define YAL_GLOBAL_LOG_INFO_IF(cond, ...) do {} while(false)
#endif // YAL_DISABLE_LOGGING

/***************************************************************************/

#ifndef YAL_DISABLE_TESTS
#   define YAL_TEST_COND(log, l, cmp, r) \
        do { if ( !((l) cmp (r)) ) YAL_LOG_ERROR(log, "test_condition \"" #l " " #cmp " " #r "\" is false"); } while(false)
#   define YAL_TEST_LESS(log, l, r) YAL_TEST_COND(log, l, <, r)
#   define YAL_TEST_LESSEQ(log, l, r) YAL_TEST_COND(log, l, <=, r)
#   define YAL_TEST_EQ(log, l, r) YAL_TEST_COND(log, l, ==, r)
#   define YAL_TEST_NEQ(log, l, r) YAL_TEST_COND(log, l, !=, r)
#   define YAL_TEST_GR(log, l, r) YAL_TEST_COND(log, l, >, r)
#   define YAL_TEST_GREQ(log, l, r) YAL_TEST_COND(log, l, >=, r)
#   define YAL_TEST_ZERO(log, v) YAL_TEST_COND(log, v, ==, 0)
#   define YAL_TEST_NOTZERO(log, v) YAL_TEST_COND(log, v, !=, 0)
#   define YAL_TEST_NULL(log, v) YAL_TEST_COND(log, v, ==, nullptr)
#   define YAL_TEST_NOTNULL(log, v) YAL_TEST_COND(log, v, !=, nullptr)
#else // !YAL_DISABLE_TESTS
#   define YAL_TEST_LESS(log, l, r) do {} while(false)
#   define YAL_TEST_LESSEQ(log, l, r) do {} while(false)
#   define YAL_TEST_EQ(log, l, r) do {} while(false)
#   define YAL_TEST_NEQ(log, l, r) do {} while(false)
#   define YAL_TEST_GR(log, l, r) do {} while(false)
#   define YAL_TEST_GREQ(log, l, r) do {} while(false)
#   define YAL_TEST_ZERO(log, v) do {} while(false)
#   define YAL_TEST_NOTZERO(log, v) do {} while(false)
#   define YAL_TEST_NULL(log, v) do {} while(false)
#   define YAL_TEST_NOTNULL(log, v) do {} while(false)
#endif // YAL_DISABLE_TESTS

/***************************************************************************/

#ifndef YAL_DISABLE_ASSERT
#   define YAL_ASSERT_LOG(log, ...) \
        if ( !(__VA_ARGS__) ) { \
            static const char flbuf[] = __FILE__ ":" BOOST_PP_STRINGIZE(__LINE__); \
            log->write( \
                 flbuf \
                ,sizeof(flbuf)-1 \
                ,__FUNCTION__ \
                ,sizeof(__FUNCTION__)-1 \
                ,__PRETTY_FUNCTION__ \
                ,sizeof(__PRETTY_FUNCTION__)-1 \
                ,"assert \"" #__VA_ARGS__ "\" is false" \
                ,::yal::level::error \
            ); \
            ::yal::logger::flush(); \
            std::abort(); \
        }

#   define YAL_ASSERT_TERM(stream, ...) \
        if ( !(__VA_ARGS__) ) { \
            char dtbuf[::dtf::bufsize]; \
            constexpr auto flags = ::dtf::flags::yyyy_mm_dd|::dtf::flags::sep3|::dtf::flags::msecs; \
            auto n = ::dtf::timestamp_to_chars(dtbuf, ::dtf::timestamp(), flags); \
            dtbuf[n] = 0; \
            stream \
                << "[" << dtbuf << "][assert ][" __FILE__ ":" BOOST_PP_STRINGIZE(__LINE__) "][" \
                << __PRETTY_FUNCTION__ << "]: expression \"" #__VA_ARGS__ "\" is false" \
            << std::endl; \
            std::abort(); \
        }

#else // !YAL_DISABLE_ASSERT
#   define YAL_ASSERT_LOG(log, ...) do {} while(false)
#   define YAL_ASSERT_TERM(stream, ...) do {} while(false)
#endif // YAL_DISABLE_ASSERT

/***************************************************************************/

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

#   define YAL_MAKE_TIMEPOINT(name, descr) \
        const ::yal::detail::timepoint _yal_timepoint_##name{__LINE__, descr, std::chrono::high_resolution_clock::now()}
#   define YAL_PRINT_TIMEPOINT(log, name) \
        do { \
            static const char flbuf[] = __FILE__ ":" BOOST_PP_STRINGIZE(__LINE__); \
            const auto d = std::chrono::high_resolution_clock::now() - _yal_timepoint_##name.time; \
            log->write( \
                 flbuf \
                ,sizeof(flbuf)-1 \
                ,__FUNCTION__ \
                ,sizeof(__FUNCTION__)-1 \
                ,__PRETTY_FUNCTION__ \
                ,sizeof(__PRETTY_FUNCTION__)-1 \
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
        } while(false)
#   define YAL_PRINT_TIMEPOINT_IF(log, expr, name) \
        if ( (expr) ) YAL_PRINT_TIMEPOINT(log, name) \

#else // !YAL_DISABLE_TIMEPOINT
#   define YAL_MAKE_TIMEPOINT(name, descr) do {} while(false)
#   define YAL_PRINT_TIMEPOINT(log, name) do {} while(false)
#   define YAL_PRINT_TIMEPOINT_IF(log, expr, name) do {} while(false)
#endif // YAL_DISABLE_TIMEPOINT

/***************************************************************************/

#ifndef YAL_DISABLE_TRY_CATCH
#   define YAL_TRY(flag) \
        bool flag = false; \
        ((void)flag); \
        static const auto _yal_try_##flag##_line = __LINE__; \
        try
#   define YAL_TYPED_CATCH(log, extype, flag, msg) \
        catch (const extype &ex) { \
            flag = true; \
            YAL_LOG_ERROR(log, "[" #extype "](in_lines:%1%-%2%): \"%3%\", msg: \"%4%\"", _yal_try_##flag##_line, __LINE__, ex.what(), msg); \
        }
#   define YAL_CATCH(log, flag, msg) \
        YAL_TYPED_CATCH(log, std::exception, flag, msg) \
        catch (...) { \
            flag = true; \
            YAL_LOG_ERROR(log, "[unknown_exception](in_lines:%1%-%2%): \"%3%\"", _yal_try_##flag##_line, __LINE__, msg); \
        }
#else
#   define YAL_TRY(flag) \
        try
#   define YAL_TYPED_CATCH(log, extype, flag, msg) \
    catch (const extype &ex) { \
        throw ex; \
    }
#   define YAL_CATCH(log, flag, msg) \
    catch (...) { \
        throw; \
    }
#endif // YAL_DISABLE_TRY_CATCH

/***************************************************************************/

#ifndef YAL_DISABLE_THROW
#   define __YAL_MAKE_FILELINE() __FILE__ "(" BOOST_PP_STRINGIZE(__LINE__) "): "
#   define YAL_TYPED_THROW(type, msg) throw type(__YAL_MAKE_FILELINE() msg);
#   define YAL_THROW(msg) YAL_TYPED_THROW(std::runtime_error, msg);
#   define YAL_THROW_IF(expr) if ( (expr) ) YAL_THROW(#expr)
#   define YAL_TYPED_THROW_IF(type, expr) if ( (expr) ) YAL_TYPED_THROW(type, #expr)
#   define YAL_TEST_THROW(expr) if ( !(expr) ) YAL_THROW(#expr)
#   define YAL_TEST_TYPED_THROW(type, expr) if ( !(expr) ) YAL_TYPED_THROW(type, #expr)
#else
#   define YAL_TYPED_THROW(type, msg)
#   define YAL_THROW(msg)
#   define YAL_THROW_IF(expr)
#   define YAL_TYPED_THROW_IF(type, expr)
#   define YAL_TEST_THROW(expr)
#   define YAL_TEST_TYPED_THROW(type, expr)
#endif // YAL_DISABLE_THROW

/***************************************************************************/

#endif // _yal__yal_hpp
