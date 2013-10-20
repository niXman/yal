
#ifndef _yal__yal_hpp
#define _yal__yal_hpp

#include <memory>
#include <mutex>

#include <boost/format.hpp>
#include <boost/noncopyable.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>

/***************************************************************************/

namespace yal {

enum level {
	 info    = 0x01
	,debug   = 0x02
	,warning = 0x04
	,error   = 0x08
};

namespace detail {

/***************************************************************************/

struct session: private boost::noncopyable {
	session(const std::string &path, const std::string &name, std::size_t volume_size, std::size_t shift_after);
	~session();

	const std::string& name() const;
	void set_buffer(const std::size_t size);
	void to_term(const bool ok, const std::string &pref);
	void set_level(const level lvl);
	void write(const std::string &data, const level lvl);
	void flush();

private:
	struct impl;
	impl *pimpl;
}; // struct session

/***************************************************************************/

} // ns detail

/***************************************************************************/

using session = std::shared_ptr<detail::session>;

/***************************************************************************/

namespace detail {

/***************************************************************************/

struct session_manager: private boost::noncopyable {
	const std::string& root_path() const;
	void root_path(const std::string& path);

	yal::session create(const std::string &name, std::size_t volume_size, std::size_t shift_after);

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

struct logger: private boost::noncopyable {
	using logger_session_manager_ptr = std::shared_ptr<detail::session_manager>;

	static void root_path(const std::string &path);
	static const std::string& root_path();

	static yal::session create(const std::string &name, std::size_t volume_size = UINT_MAX, std::size_t shift_after = 9999);

	static void flush();

private:
	static logger_session_manager_ptr instance();
	static void init();

	static logger_session_manager_ptr object;
	static std::once_flag flag;

}; // struct logger

/***************************************************************************/

} // ns yal

/***************************************************************************/

#define YAL_DEBUG_LOG_ARG_LIST(...) \
	BOOST_PP_CAT(YAL_DEBUG_LOG_ARG_LIST, YAL_VA_MORE1_ARGS(__VA_ARGS__))(__VA_ARGS__)

#define YAL_DEBUG_LOG_ARG_LIST0(...)

#define YAL_SEPARATOR %

#define YAL_DEBUG_LOG_ARG_LIST1(F_, ...) \
	YAL_SEPARATOR YAL_GET(__VA_ARGS__)

#define YAL_DEBUG_LOG_FORMAT(F_, ...) \
	F_

#define YAL_RSEQ_N() \
	63,62,61,60,59,58,57,56,55,54, \
	53,52,51,50,49,48,47,46,45,44, \
	43,42,41,40,39,38,37,36,35,34, \
	33,32,31,30,29,28,27,26,25,24, \
	23,22,21,20,19,18,17,16,15,14, \
	13,12,11,10,9,8,7,6,5,4,3,2,1,0

#define YAL_NARG(...) \
	YAL_NARG_(__VA_ARGS__, YAL_RSEQ_N())

#define YAL_NARG_(...) \
	YAL_VA_ARG_N(__VA_ARGS__)

#define YAL_VA_ARG_N( \
	_1, _2 ,_3, _4, _5, _6, _7, _8, _9, _10,_11,_12,_13,_14,_15,_16, \
	_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32, \
	_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48, \
	_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,  N, ...) N

#define YAL_VA_MORE1_ARGS(...) \
	YAL_VA_ARG_N(__VA_ARGS__ \
		,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 \
		,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 \
	)

#define YAL_GET_1(item1, ...) item1
#define YAL_GET_2(item1, ...) item1 YAL_SEPARATOR YAL_GET_1(__VA_ARGS__)
#define YAL_GET_3(item1, ...) item1 YAL_SEPARATOR YAL_GET_2(__VA_ARGS__)
#define YAL_GET_4(item1, ...) item1 YAL_SEPARATOR YAL_GET_3(__VA_ARGS__)
#define YAL_GET_5(item1, ...) item1 YAL_SEPARATOR YAL_GET_4(__VA_ARGS__)
#define YAL_GET_6(item1, ...) item1 YAL_SEPARATOR YAL_GET_5(__VA_ARGS__)
#define YAL_GET_7(item1, ...) item1 YAL_SEPARATOR YAL_GET_6(__VA_ARGS__)
#define YAL_GET_8(item1, ...) item1 YAL_SEPARATOR YAL_GET_7(__VA_ARGS__)
#define YAL_GET_9(item1, ...) item1 YAL_SEPARATOR YAL_GET_8(__VA_ARGS__)
#define YAL_GET_10(item1, ...) item1 YAL_SEPARATOR YAL_GET_9(__VA_ARGS__)
#define YAL_GET_11(item1, ...) item1 YAL_SEPARATOR YAL_GET_10(__VA_ARGS__)
#define YAL_GET_12(item1, ...) item1 YAL_SEPARATOR YAL_GET_11(__VA_ARGS__)
#define YAL_GET_13(item1, ...) item1 YAL_SEPARATOR YAL_GET_12(__VA_ARGS__)
#define YAL_GET_14(item1, ...) item1 YAL_SEPARATOR YAL_GET_13(__VA_ARGS__)
#define YAL_GET_15(item1, ...) item1 YAL_SEPARATOR YAL_GET_14(__VA_ARGS__)
#define YAL_GET_16(item1, ...) item1 YAL_SEPARATOR YAL_GET_15(__VA_ARGS__)
#define YAL_GET_17(item1, ...) item1 YAL_SEPARATOR YAL_GET_16(__VA_ARGS__)
#define YAL_GET_18(item1, ...) item1 YAL_SEPARATOR YAL_GET_17(__VA_ARGS__)
#define YAL_GET_19(item1, ...) item1 YAL_SEPARATOR YAL_GET_18(__VA_ARGS__)
#define YAL_GET_20(item1, ...) item1 YAL_SEPARATOR YAL_GET_19(__VA_ARGS__)
#define YAL_GET_21(item1, ...) item1 YAL_SEPARATOR YAL_GET_20(__VA_ARGS__)
#define YAL_GET_22(item1, ...) item1 YAL_SEPARATOR YAL_GET_21(__VA_ARGS__)
#define YAL_GET_23(item1, ...) item1 YAL_SEPARATOR YAL_GET_22(__VA_ARGS__)
#define YAL_GET_24(item1, ...) item1 YAL_SEPARATOR YAL_GET_23(__VA_ARGS__)
#define YAL_GET_25(item1, ...) item1 YAL_SEPARATOR YAL_GET_24(__VA_ARGS__)
#define YAL_GET_26(item1, ...) item1 YAL_SEPARATOR YAL_GET_25(__VA_ARGS__)
#define YAL_GET_27(item1, ...) item1 YAL_SEPARATOR YAL_GET_26(__VA_ARGS__)
#define YAL_GET_28(item1, ...) item1 YAL_SEPARATOR YAL_GET_27(__VA_ARGS__)
#define YAL_GET_29(item1, ...) item1 YAL_SEPARATOR YAL_GET_28(__VA_ARGS__)
#define YAL_GET_30(item1, ...) item1 YAL_SEPARATOR YAL_GET_29(__VA_ARGS__)
#define YAL_GET_31(item1, ...) item1 YAL_SEPARATOR YAL_GET_30(__VA_ARGS__)
#define YAL_GET_32(item1, ...) item1 YAL_SEPARATOR YAL_GET_31(__VA_ARGS__)
#define YAL_GET_33(item1, ...) item1 YAL_SEPARATOR YAL_GET_32(__VA_ARGS__)
#define YAL_GET_34(item1, ...) item1 YAL_SEPARATOR YAL_GET_33(__VA_ARGS__)
#define YAL_GET_35(item1, ...) item1 YAL_SEPARATOR YAL_GET_34(__VA_ARGS__)
#define YAL_GET_36(item1, ...) item1 YAL_SEPARATOR YAL_GET_35(__VA_ARGS__)
#define YAL_GET_37(item1, ...) item1 YAL_SEPARATOR YAL_GET_36(__VA_ARGS__)
#define YAL_GET_38(item1, ...) item1 YAL_SEPARATOR YAL_GET_37(__VA_ARGS__)
#define YAL_GET_39(item1, ...) item1 YAL_SEPARATOR YAL_GET_38(__VA_ARGS__)
#define YAL_GET_40(item1, ...) item1 YAL_SEPARATOR YAL_GET_39(__VA_ARGS__)
#define YAL_GET_41(item1, ...) item1 YAL_SEPARATOR YAL_GET_40(__VA_ARGS__)
#define YAL_GET_42(item1, ...) item1 YAL_SEPARATOR YAL_GET_41(__VA_ARGS__)
#define YAL_GET_43(item1, ...) item1 YAL_SEPARATOR YAL_GET_42(__VA_ARGS__)
#define YAL_GET_44(item1, ...) item1 YAL_SEPARATOR YAL_GET_43(__VA_ARGS__)
#define YAL_GET_45(item1, ...) item1 YAL_SEPARATOR YAL_GET_44(__VA_ARGS__)
#define YAL_GET_46(item1, ...) item1 YAL_SEPARATOR YAL_GET_45(__VA_ARGS__)
#define YAL_GET_47(item1, ...) item1 YAL_SEPARATOR YAL_GET_46(__VA_ARGS__)
#define YAL_GET_48(item1, ...) item1 YAL_SEPARATOR YAL_GET_47(__VA_ARGS__)
#define YAL_GET_49(item1, ...) item1 YAL_SEPARATOR YAL_GET_48(__VA_ARGS__)
#define YAL_GET_50(item1, ...) item1 YAL_SEPARATOR YAL_GET_49(__VA_ARGS__)
#define YAL_GET_51(item1, ...) item1 YAL_SEPARATOR YAL_GET_50(__VA_ARGS__)
#define YAL_GET_52(item1, ...) item1 YAL_SEPARATOR YAL_GET_51(__VA_ARGS__)
#define YAL_GET_53(item1, ...) item1 YAL_SEPARATOR YAL_GET_52(__VA_ARGS__)
#define YAL_GET_54(item1, ...) item1 YAL_SEPARATOR YAL_GET_53(__VA_ARGS__)
#define YAL_GET_55(item1, ...) item1 YAL_SEPARATOR YAL_GET_54(__VA_ARGS__)
#define YAL_GET_56(item1, ...) item1 YAL_SEPARATOR YAL_GET_55(__VA_ARGS__)
#define YAL_GET_57(item1, ...) item1 YAL_SEPARATOR YAL_GET_56(__VA_ARGS__)
#define YAL_GET_58(item1, ...) item1 YAL_SEPARATOR YAL_GET_57(__VA_ARGS__)
#define YAL_GET_59(item1, ...) item1 YAL_SEPARATOR YAL_GET_58(__VA_ARGS__)
#define YAL_GET_60(item1, ...) item1 YAL_SEPARATOR YAL_GET_59(__VA_ARGS__)
#define YAL_GET_61(item1, ...) item1 YAL_SEPARATOR YAL_GET_60(__VA_ARGS__)
#define YAL_GET_62(item1, ...) item1 YAL_SEPARATOR YAL_GET_61(__VA_ARGS__)
#define YAL_GET_63(item1, ...) item1 YAL_SEPARATOR YAL_GET_62(__VA_ARGS__)
#define YAL_GET(...) BOOST_PP_CAT(YAL_GET_, YAL_NARG(__VA_ARGS__))(__VA_ARGS__)

/***************************************************************************/

/* usage:
 * std::cout << YAL_MESSAGE_AS_STRING("message: %s, %d, %s", "string1", 33, "string2") << std::endl;
 * std::string str = YAL_MESSAGE_AS_STRING("message: %s, %d, %s", "string1", 33, "string2");
 */

#define YAL_MESSAGE_AS_STRING(...) \
	(boost::format( \
		"[" __FILE__ ":" BOOST_PP_STRINGIZE(__LINE__) "][%s]: " \
		YAL_DEBUG_LOG_FORMAT(__VA_ARGS__,) \
	) % __PRETTY_FUNCTION__ YAL_DEBUG_LOG_ARG_LIST(__VA_ARGS__)).str()

/***************************************************************************/

#ifndef YAL_DISABLE_LOGGING
#	define YAL_FLUSH(log) \
		log->flush()
#  define YAL_SET_LEVEL(log, lvl) \
		log->set_level((lvl))
#	define YAL_SET_BUFFER(log, size) \
		log->set_buffer((size))
#	define YAL_SET_UNBUFFERED(log) \
		log->set_buffer(0)
#  define YAL_TO_TERM(log, flag, pref) \
		log->to_term((flag), (pref))

#  ifndef YAL_DISABLE_ERROR
#		define YAL_ERROR(log, ...) \
			log->write(YAL_MESSAGE_AS_STRING(__VA_ARGS__), ::yal::level::error)
#	else
#     define YAL_ERROR(log, ...)
#  endif

#  ifndef YAL_DISABLE_WARNING
#     define YAL_WARNING(log, ...) \
			log->write(YAL_MESSAGE_AS_STRING(__VA_ARGS__), ::yal::level::warning)
#  else
#     define YAL_WARNING(log, ...)
#  endif

#  ifndef YAL_DISABLE_DEBUG
#     define YAL_DEBUG(log, ...) \
			log->write(YAL_MESSAGE_AS_STRING(__VA_ARGS__), ::yal::level::debug)
#  else
#     define YAL_DEBUG(log, ...)
#  endif

#  ifndef YAL_DISABLE_INFO
#     define YAL_INFO(log, ...) \
			log->write(YAL_MESSAGE_AS_STRING(__VA_ARGS__), ::yal::level::info)
#	else
#		define YAL_INFO(log, ...)
#	endif
#else // YAL_DISABLE_LOGGING == true
#	define YAL_FLUSH(log)
#	define YAL_SET_LEVEL(log, lvl)
#	define YAL_SET_BUFFER(log, size)
#	define YAL_SET_UNBUFFERED(log)
#	define YAL_TO_TERM(log, flag, pref)

#	define YAL_ERROR(log, ...)
#	define YAL_WARNING(log, ...)
#	define YAL_DEBUG(log, ...)
#	define YAL_INFO(log, ...)
#endif // YAL_DISABLE_LOGGING

/***************************************************************************/

#endif // _yal__yal_hpp
