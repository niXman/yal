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

#include <yal/yal.hpp>

#include <boost/filesystem.hpp>

#include <cstdio>
#include <cctype>
#include <cerrno>

#include <unordered_map>
#include <algorithm>
#include <mutex>

#define _STRINGIZE(x) \
	#x

#define STRINGIZE(x) \
	_STRINGIZE(x)

#define YAL_THROW(msg) \
	throw std::runtime_error(std::string("YAL: " __FILE__ "(" STRINGIZE(__LINE__) "): ") + msg)

namespace yal {

/***************************************************************************/

namespace detail {

using mutex_t = std::mutex;
using guard_t = std::lock_guard<mutex_t>;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

struct session::impl {
	impl(const std::string &path, const std::string &name, std::size_t volume_size, std::size_t shift_after)
		:path(path)
		,name(name)
		,volume_size(volume_size)
		,shift_after(shift_after)
		,file(0)
		,toterm(false)
		,prefix()
		,level(yal::info)
		,writen_bytes(0)
		,volume_number(0)
	{
		if ( name != "disable" ) {
			volume_number = get_last_volume_number(name, path);
			create_volume();
		} else {
			level = yal::disable;
		}
	}

	~impl() {
		std::fclose(file);
	}

	static std::size_t get_last_volume_number(const std::string &name, const std::string &path) {
		std::size_t volume_number = 0;
		std::string logpath, logfname;

		std::size_t pos = name.find_last_of('/');
		if ( pos != std::string::npos ) {
			logpath = path + "/" + name.substr(0, pos);
			logfname= name.substr(pos+1);
		} else {
			logpath = path;
			logfname= name;
		}

		if ( (pos=logfname.find('.')) != std::string::npos ) {
			logfname = logfname.substr(0, pos);
		}

		boost::filesystem::directory_iterator fs_beg(logpath), fs_end;
		for ( ; fs_beg != fs_end; ++fs_beg ) {
			const auto filepath = fs_beg->path().string();
			const auto filename = fs_beg->path().filename().string();
			if ( filename == "." || filename == ".." || boost::filesystem::is_directory(filepath) )
				continue;

			if ( filename.find(logfname+"-") == std::string::npos )
				continue;

			boost::system::error_code ec;
			const auto filesize = boost::filesystem::file_size(filepath, ec);
			if ( ec ) YAL_THROW("can't get filesize(" +filepath+ "): " + ec.message());
			if ( filesize == 0 ) {
				boost::system::error_code ec;
				boost::filesystem::remove(filepath, ec);
				if ( ec ) YAL_THROW("can't remove empty volume(" +filepath+ "): " + ec.message());

				continue;
			}

			auto beg = std::find(filename.begin(), filename.end(), '-');
			if ( beg == filename.end() || beg+1 == filename.end() )
				continue;

			++beg;

			if ( !std::isdigit(*beg) )
				continue;

			auto end = beg;
			for ( ; std::isdigit(*end); ++end)
				;

			const auto num = std::stoul(std::string(beg, end)) + 1;
			if ( num > volume_number )
				volume_number = num;
		}

		return volume_number;
	}

	enum resolution { sec_res, usec_res };

	static const char* datetime(const resolution res, char *buf, const std::size_t size) {
		if ( size < 32 )
			YAL_THROW("buffer's size for date-time format should be greater or equal to 32 bytes");

		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		const std::tm *tm = std::localtime(&ts.tv_sec);
		if ( res == usec_res ) {
			std::snprintf(
				 buf, size
				,"%02d.%02d.%04d-%02d.%02d.%02d-%6ld"
				,tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900
				,tm->tm_hour, tm->tm_min, tm->tm_sec
				,ts.tv_nsec/1000
			);
		} else {
			std::snprintf(
				 buf, size
				,"%02d.%02d.%04d-%02d.%02d.%02d"
				,tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900
				,tm->tm_hour, tm->tm_min, tm->tm_sec
			);
		}

		return buf;
	}

	void create_volume() {
		char fmt[64] = "\0";
		char datebuf[64] = "\0";
		char pathbuf[1024*4] = "\0";

		if ( volume_number > shift_after ) {
			shift_after = shift_after * 10 + 9;
		}

		const int digits = std::log10(shift_after)+1;
		std::snprintf(fmt, sizeof(fmt), "%s%d%s", "%s/%s-%0", digits, "d-%s");

		const char *pos = std::strchr(name.c_str(), '.');
		if ( !pos ) {
			std::snprintf(pathbuf, sizeof(pathbuf), fmt, path.c_str(), name.c_str(), volume_number, datetime(sec_res, datebuf, sizeof(datebuf)));
		} else {
			std::strcat(fmt, "%s");
			std::snprintf(pathbuf, sizeof(pathbuf), fmt, path.c_str(), name.c_str(), volume_number, datetime(sec_res, datebuf, sizeof(datebuf)), pos);
		}

		file = std::fopen(pathbuf, "wb");
		if ( !file ) {
			int error = errno;
			YAL_THROW("cannot create logfile \"" + std::string(pathbuf) + "\" with errno="+std::to_string(error));
		}
	}

	void set_buffer(std::size_t size) {
		std::setvbuf(file, 0, (size == 0 ? _IONBF : _IOFBF), size);
	}

	void to_term(bool ok, const std::string &pref) {
		toterm = ok;
		prefix = pref;
	}

	void set_level(const yal::level lvl) {
		level = lvl;
	}

	void write(const char *fileline, const char *func, const std::string &data, const yal::level lvl) {
		const char *levelstr = (
			lvl == yal::info ? "info   "
				: lvl == yal::debug ? "debug  "
					: lvl == yal::warning ? "warning"
						: lvl == yal::error ? "error  "
							: "disabled"
		);

		static const char *fmt0 = "[%s][%s][%s][%s]: %s\n";
		static const char *fmt1 = "<%s>[%s][%s][%s][%s]: %s\n";
		char datebuf[64] = "\0";
		const char *datestr = datetime(usec_res, datebuf, sizeof(datebuf));

		if ( toterm ) {
			auto termlog = ((lvl == yal::info || lvl == yal::debug) ? stdout : stderr);
			if ( prefix.empty() ) {
				std::fprintf(termlog, fmt0, datestr, levelstr, fileline, func, data.c_str());
			} else {
				std::fprintf(termlog, fmt1, prefix.c_str(), datestr, levelstr, fileline, func, data.c_str());
			}
			std::fflush(termlog);
		}

		const int writen = std::fprintf(file, fmt0, datestr, levelstr, fileline, func, data.c_str());

		if ( writen < 0 ) {
			int error = errno;
			YAL_THROW("cannot write to logfile with errno="+std::to_string(error));
		}

		writen_bytes += writen;
		if ( writen_bytes >= volume_size ) {
			writen_bytes = 0;

			if ( std::fclose(file) != 0 ) {
				int error = errno;
				YAL_THROW("cannot close logfile with errno="+std::to_string(error));
			}

			volume_number += 1;
			create_volume();
		}
	}

	void flush() {
		if ( file )
			std::fflush(file);
	}

	const std::string path;
	const std::string name;
	const std::size_t volume_size;
	std::size_t			shift_after;
	FILE					*file;
	bool					toterm;
	std::string			prefix;
	yal::level			level;
	std::size_t			writen_bytes;
	std::size_t			volume_number;
};

/***************************************************************************/

session::session(const std::string &path, const std::string &name, std::size_t volume_size, std::size_t shift_after)
	:pimpl(new impl(path, name, volume_size, shift_after))
{}

session::~session()
{ delete pimpl; }

const std::string &session::name() const { return pimpl->name; }

const char* session::sec_date_str(char *buf, const std::size_t size) { return impl::datetime(impl::sec_res, buf, size); }
const char* session::usec_date_str(char *buf, const std::size_t size) { return impl::datetime(impl::usec_res, buf, size); }

const char* session::level_str(level lvl) {
	return (
		lvl == yal::info ? "info"
			: lvl == yal::debug ? "debug"
				: lvl == yal::warning ? "warning"
					: lvl == yal::error ? "error"
						: "disabled"
	);
}

/***************************************************************************/

void session::set_buffer(const std::size_t size) {
	pimpl->set_buffer(size);
}

void session::to_term(const bool ok, const std::string &pref) {
	pimpl->to_term(ok, pref);
}

void session::set_level(const level lvl) {
	pimpl->set_level(lvl);
}

level session::get_level() const { return pimpl->level; }

void session::write(const char *fileline, const char *func, const std::string &data, const level lvl) {
	if ( data.empty() ) return;
	pimpl->write(fileline, func, data, lvl);
}

void session::flush() {
	pimpl->flush();
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

struct session_manager::impl {
	using session_weak_ptr = std::weak_ptr<session>;
	using sessions_map = std::unordered_map<std::string, session_weak_ptr>;

	impl()
		:mutex()
		,root_path(".")
		,sessions()
	{}
	~impl() {
		flush();
	}

	template<typename F>
	void iterate(F func) {
		for ( auto it = sessions.begin(), end = sessions.end(); it != end; ) {
			if ( auto session = it->second.lock() ) {
				func(session);
				++it;
			} else {
				it = sessions.erase(it);
			}
		}
	}

	void flush() {
		iterate([](yal::session s) { s->flush(); });
	}

	mutex_t mutex;
	std::string root_path;
	sessions_map sessions;
}; // struct impl

/***************************************************************************/

session_manager::session_manager()
	:pimpl(new impl)
{}

session_manager::~session_manager() {
	delete pimpl;
}

/***************************************************************************/

const std::string &session_manager::root_path() const {
	guard_t lock(pimpl->mutex);

	return pimpl->root_path;
}

void session_manager::root_path(const std::string &path) {
	guard_t lock(pimpl->mutex);

	if ( !boost::filesystem::exists(path) ) {
		boost::system::error_code ec;
		boost::filesystem::create_directories(path, ec);
		if ( ec )
			YAL_THROW("can't create directory(" +path+ "): " + ec.message());
	}

	pimpl->root_path = path;
}

std::shared_ptr<session>
session_manager::create(const std::string &name, std::size_t volume_size, std::size_t shift_after) {
	guard_t lock(pimpl->mutex);

	if ( !name.empty() && name[0] == '/' )
		YAL_THROW("session name can't be a full path");
	if ( !shift_after )
		YAL_THROW("shift_after can be 1 or greater");

	pimpl->iterate(
		[&name](yal::session s) {
			if ( s->name() == name )
				YAL_THROW("session \""+name+"\" already exists");
		}
	);

	const auto pos = name.find_last_of('/');
	if ( pos != std::string::npos ) {
		const std::string path = pimpl->root_path+"/"+name.substr(0, pos);
		//std::cout << "path:" << path << std::endl;
		if ( !boost::filesystem::exists(path) ) {
			boost::system::error_code ec;
			boost::filesystem::create_directories(path, ec);
			if ( ec )
				YAL_THROW("can't create directory(" +path+ "): " + ec.message());
		}
	}

	yal::session session = std::make_shared<detail::session>(pimpl->root_path, name, volume_size, shift_after);
	pimpl->sessions.insert({name, session});

	return session;
}

/***************************************************************************/

void session_manager::write(const char *fileline, const char *func, const std::string &data, level lvl) {
	guard_t lock(pimpl->mutex);

	pimpl->iterate(
		[fileline, func, &data, lvl](yal::session s) { s->write(fileline, func, data, lvl); }
	);
}

std::shared_ptr<session>
session_manager::get(const std::string &name) const {
	guard_t lock(pimpl->mutex);

	auto it = pimpl->sessions.find(name);
	if ( it == pimpl->sessions.end() )
		return std::shared_ptr<session>();

	if ( auto s = it->second.lock() ) {
		return s;
	} else {
		pimpl->sessions.erase(it);
		return std::shared_ptr<session>();
	}
}

/***************************************************************************/

void session_manager::flush() {
	guard_t lock(pimpl->mutex);

	pimpl->flush();
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

} // ns detail

detail::session_manager* logger::instance() {
	static std::unique_ptr<detail::session_manager> object(new detail::session_manager);
	return object.get();
}

const std::string &logger::root_path() { return instance()->root_path(); }

session logger::create(const std::string &name, std::size_t volume_size, std::size_t shift_after) {
	return instance()->create(name, volume_size, shift_after);
}

yal::session logger::get(const std::string &name) {
	return instance()->get(name);
}

void logger::write(const char *fileline, const char *func, const std::string &data, level lvl) {
	instance()->write(fileline, func, data, lvl);
}

void logger::flush() { instance()->flush(); }

void logger::root_path(const std::string &path) { instance()->root_path(path); }

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

} // ns yal
