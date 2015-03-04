
// Copyright (c) 2013-2015 niXman (i dotty nixman doggy gmail dotty com)
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

#include <cstdio>
#include <cctype>

#include <unordered_map>
#include <algorithm>
#include <mutex>

#include <boost/filesystem.hpp>

#include <fcntl.h>

/***************************************************************************/

#ifndef YAL_SUPPORT_COMPRESSION
#	define YAL_SUPPORT_COMPRESSION (1)
#endif // YAL_SUPPORT_COMPRESSION

#if YAL_SUPPORT_COMPRESSION
#	include <zlib.h>
#endif // YAL_SUPPORT_COMPRESSION

/***************************************************************************/

#ifndef YAL_THREAD_SAFE
#	define YAL_THREAD_SAFE (0)
#endif // YAL_THREAD_SAFE

#if YAL_THREAD_SAFE
using mutex_t = std::mutex;
#else
struct mutex_t {
	void lock() {}
	bool try_lock() noexcept { return true; }
	void unlock() {}
};
#endif // YAL_THREAD_SAFE

using guard_t = std::lock_guard<mutex_t>;

/***************************************************************************/

namespace yal {

/***************************************************************************/

const char* level_str(const level lvl) {
	static const char info_str[9]     = "info    ";
	static const char debug_str[9]    = "debug   ";
	static const char warning_str[9]  = "warning ";
	static const char error_str[9]    = "error   ";
	static const char disabled_str[9] = "disabled";
	return (
		lvl == yal::info ? info_str
			: lvl == yal::debug ? debug_str
				: lvl == yal::warning ? warning_str
					: lvl == yal::error ? error_str
						: disabled_str
	);
}

/***************************************************************************/

namespace detail {

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

static const char *active_ext = ".active";

struct io_base {
	virtual ~io_base() {}

	virtual void create(const std::string &fname) = 0;
	virtual void write(const void *ptr, const std::size_t size) = 0;
	virtual void close() = 0;
	virtual void set_buffer(const std::size_t size) = 0;
	virtual void flush() = 0;
	virtual void fsync() = 0;

	static std::string normalize_fname(const std::string &fname) {
		return fname.substr(0, fname.length()-std::strlen(active_ext));
	}
};

struct file_io: io_base {
	file_io()
		:file(0)
		,fname()
	{}
	virtual ~file_io() { close(); }

	void create(const std::string &fn) {
		close();
		fname = fn+active_ext;
		file = std::fopen(fname.c_str(), "wb");
		YAL_THROW_IF(file == 0, "can't create file \"" +fname+ "\"");
	}
	void write(const void *ptr, const std::size_t size) {
		YAL_THROW_IF(file == 0, "file \"" +fname+ "\" is not open");
		YAL_THROW_IF(size != std::fwrite(ptr, 1, size, file), "write error");
	}
	void close() {
		if ( file ) {
			std::fclose(file);
			file = 0;

			boost::system::error_code ec;
			boost::filesystem::rename(fname, normalize_fname(fname), ec);
		}
	}
	void set_buffer(const std::size_t size) {
		YAL_THROW_IF(file == 0, "file \"" +fname+ "\" is not open");
		std::setvbuf(file, 0, (size ? _IOFBF : _IONBF), size);
	}
	void flush() {
		YAL_THROW_IF(file == 0, "file \"" +fname+ "\" is not open");
		std::fflush(file);
	}
	void fsync() {
		YAL_THROW_IF(file == 0, "file \"" +fname+ "\" is not open");
		fdatasync(fileno(file));
	}

private:
	FILE *file;
	std::string fname;
};

#if YAL_SUPPORT_COMPRESSION
struct gz_file_io: io_base {
	gz_file_io()
		:fd(-1)
		,gzfile(0)
		,fname()
	{}
	virtual ~gz_file_io() { close(); }

	void create(const std::string &fn) {
		close();
		fname = fn+".gz"+active_ext;
		fd = ::open(fname.c_str(), O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
		YAL_THROW_IF(fd == -1, "can't create file \"" +fname+ "\"");

		static const char mode[4] = {'w','b','0'+YAL_COMPRESSION_LEVEL,0};
		gzfile = gzdopen(fd, mode);
		YAL_THROW_IF(gzfile == 0, "can't create file \"" +fname+ "\"");
	}
	void write(const void *ptr, const std::size_t size) {
		YAL_THROW_IF(gzfile == 0 || fd == -1, "file \"" +fname+ "\" is not open");
		YAL_THROW_IF(size != (std::size_t)gzwrite(gzfile, ptr, size), "write error");
	}
	void close() {
		if ( gzfile ) {
			gzclose(gzfile);
			gzfile = 0;

			::close(fd);
			fd = -1;

			boost::system::error_code ec;
			boost::filesystem::rename(fname, normalize_fname(fname), ec);
		}
	}
	void set_buffer(const std::size_t size) {
		gzbuffer(gzfile, size);
	}
	void flush() {
		YAL_THROW_IF(gzfile == 0 || fd == -1, "file \"" +fname+ "\" is not open");
		gzflush(gzfile, Z_FINISH);
	}
	void fsync() {
		YAL_THROW_IF(gzfile == 0 || fd == -1, "file \"" +fname+ "\" is not open");
		fdatasync(fd);
	}

private:
	int fd;
	gzFile gzfile;
	std::string fname;
};
#else
struct gz_file_io: file_io {};
#endif // YAL_SUPPORT_COMPRESSION

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

struct session::impl {
	impl(const std::string &path, const std::string &name, std::size_t volume_size, std::uint32_t opts)
		:path(path)
		,name(name)
		,volume_size(volume_size)
		,options(opts)
		,shift_after(YAL_MAX_VOLUME_NUMBER)
		,file(opts & yal::compress ? (io_base*)new gz_file_io : (io_base*)new file_io)
		,toterm(false)
		,prefix()
		,level(yal::info)
		,recbuf()
		,writen_bytes(0)
		,volume_number(0)
	{
		if ( name != "disable" ) {
			volume_number = get_last_volume_number(path, name, options & yal::remove_empty_logs);
			create_volume();
		} else {
			level = yal::disable;
		}
	}
	~impl() {}

	static std::string final_log_fname(const std::string &fname) {
		return fname.substr(0, fname.length()-std::strlen(active_ext));
	}
	static std::size_t get_last_volume_number(const std::string &path, const std::string &name, bool remove_empty) {
		std::size_t volnum = 0;
		std::string logpath, logfname;

		std::size_t pos = name.find_last_of('/');
		if ( pos != std::string::npos ) {
			logpath = path + "/" + name.substr(0, pos);
			logfname= name.substr(pos+1);
		} else {
			logpath = path;
			logfname= name;
		}

//		if ( (pos=logfname.find('.')) != std::string::npos ) {
//			logfname = logfname.substr(0, pos);
//		}

		boost::system::error_code ec;
		std::vector<std::string> empty_logs;
		std::vector<std::string> for_rename;
		boost::filesystem::directory_iterator fs_beg(logpath), fs_end;
		for ( ; fs_beg != fs_end; ++fs_beg ) {
			const auto path     = fs_beg->path().string();
			const auto filename = fs_beg->path().filename().string();

			if ( boost::filesystem::is_directory(*fs_beg) )
				continue;

			if ( path.find(logfname+"-") == std::string::npos )
				continue;

			if ( remove_empty ) {
				const auto filesize = boost::filesystem::file_size(*fs_beg, ec);
				YAL_THROW_IF(ec, "can't get filesize(" +path+ "): " + ec.message());
				if ( filesize == 0 ) {
					empty_logs.push_back(path);
					continue;
				}
			}

			if ( filename.find(active_ext) != std::string::npos )
				for_rename.push_back(path);

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
			if ( num > volnum )
				volnum = num;
		}

		ec.clear();
		for ( const auto &it: empty_logs ) {
			boost::filesystem::remove(it, ec);
			YAL_THROW_IF(ec, "can't remove empty volume(" +it+ "): " + ec.message());
		}

		ec.clear();
		for ( const auto &it: for_rename ) {
			boost::filesystem::rename(it, final_log_fname(it), ec);
			YAL_THROW_IF(ec, "can't rename unfinished volume(" +it+ "): " + ec.message());
		}

		return volnum;
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

		sec_datetime_str(datebuf, sizeof(datebuf));
		datebuf[sec_res_len] = 0;
		std::snprintf(pathbuf, sizeof(pathbuf), fmt, path.c_str(), name.c_str(), volume_number, datebuf);

		const char *pos = std::strchr(name.c_str(), '.');
		if ( pos )
			std::strcat(pathbuf, pos);

		file->create(pathbuf);

		if ( options & unbuffered )
			set_buffer(0);
	}

	void set_buffer(std::size_t size) { file->set_buffer(size); }
	void flush() { file->flush(); }
	void to_term(bool ok, const std::string &pref) { toterm = ok; prefix = pref; }

	void write(
			const char *fileline
		  ,const std::size_t fileline_len
		  ,const char *func
		  ,const std::size_t func_len
		  ,const std::string &data
		  ,const level lvl)
	{

		const std::size_t dtlen = (
			(options & usec_res)
				? usec_res_len
				: (options & nsec_res)
					? nsec_res_len
					: sec_res_len
		); // date-time string length

		const std::size_t reclen =
			 1 // '['
			+dtlen
			+2 // ']['
			+level_str_len // log-level string length
			+2 // ']['
			+fileline_len
			+2 // ']['
			+func_len
			+3 // ']: '
			+data.length()
			+1 // '\n'
		;

		if ( reclen > recbuf.size() )
			recbuf.resize(reclen);

		char dtbuf[32] = "\0";
		std::memset(dtbuf, ' ', sizeof(dtbuf));
		datetime_str(dtbuf, sizeof(dtbuf), options);
		const char *lvlstr = level_str(lvl);

		/*********************************************/
		char *p = (char*)recbuf.c_str();
		*p++ = '[';
		std::memcpy(p, dtbuf, dtlen);
		p += dtlen;
		*p++ = ']';
		*p++ = '[';
		std::memcpy(p, lvlstr, level_str_len);
		p += level_str_len;
		*p++ = ']';
		*p++ = '[';
		std::memcpy(p, fileline, fileline_len);
		p += fileline_len;
		*p++ = ']';
		*p++ = '[';
		std::memcpy(p, func, func_len);
		p += func_len;
		*p++ = ']';
		*p++ = ':';
		*p++ = ' ';
		std::memcpy(p, data.c_str(), data.length());
		p += data.length();
		*p++ = '\n';
		*p = 0;
		/*********************************************/

		if ( toterm ) {
			FILE *term = ((lvl == yal::info || lvl == yal::debug) ? stdout : stderr);
			if ( !prefix.empty() ) {
				std::fprintf(term, "<%s>%s", prefix.c_str(), recbuf.c_str());
			} else {
				std::fprintf(term, "%s", recbuf.c_str());
			}

			if ( options & flush_each_record )
				std::fflush(term);
		}

		file->write(recbuf.c_str(), reclen);

		if ( options & flush_each_record )
			file->flush();
		if ( options & fsync_each_record )
			file->fsync();

		writen_bytes += reclen;
		if ( writen_bytes >= volume_size ) {
//			std::cout << "writen_bytes=" << writen_bytes << std::endl;

			writen_bytes = 0;

			volume_number += 1;
			create_volume();
		}
	}

	const std::string path;
	const std::string name;
	const std::size_t volume_size;
	const std::uint32_t options;
	std::size_t       shift_after;
	std::unique_ptr<io_base> file;
	bool              toterm;
	std::string       prefix;
	yal::level        level;
	std::string       recbuf;
	std::size_t       writen_bytes;
	std::size_t       volume_number;
};

/***************************************************************************/

session::session(const std::string &path, const std::string &name, std::size_t volume_size, std::uint32_t opts)
	:pimpl(new impl(path, name, volume_size, opts))
{}

session::~session()
{ delete pimpl; }

/***************************************************************************/

const std::string& session::name() const { return pimpl->name; }
void session::set_buffer(const std::size_t size) { pimpl->set_buffer(size); }
void session::to_term(const bool ok, const std::string &pref) { pimpl->to_term(ok, pref); }
void session::set_level(const level lvl) { pimpl->level = lvl; }
level session::get_level() const { return pimpl->level; }

void session::write(
		const char *fileline
	  ,const std::size_t fileline_len
	  ,const char *func
	  ,const std::size_t func_len
	  ,const std::string &data
	  ,const level lvl)
{
	pimpl->write(fileline, fileline_len, func, func_len, data, lvl);
}

void session::flush() { pimpl->flush(); }

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
{
	// for setting the 'timezone' extern var
	std::time_t t = time(0);
	std::tm *lt = localtime(&t);
	(void)lt;
}

session_manager::~session_manager() {
	delete pimpl;
}

/***************************************************************************/

const std::string& session_manager::root_path() const {
	guard_t lock(pimpl->mutex);

	return pimpl->root_path;
}

void session_manager::root_path(const std::string &path) {
	guard_t lock(pimpl->mutex);

	if ( !boost::filesystem::exists(path) ) {
		boost::system::error_code ec;
		boost::filesystem::create_directories(path, ec);
		YAL_THROW_IF(ec, "can't create directory(" +path+ "): " + ec.message());
	}

	pimpl->root_path = path;
}

std::shared_ptr<session>
session_manager::create(const std::string &name, std::size_t volume_size, std::uint32_t opts) {
	guard_t lock(pimpl->mutex);

	YAL_THROW_IF(!name.empty() && name[0] == '/', "session name can't be a full path");

	pimpl->iterate(
		[&name](yal::session s) {
			YAL_THROW_IF(s->name() == name, "session \""+name+"\" already exists");
		}
	);

	const auto pos = name.find_last_of('/');
	if ( pos != std::string::npos ) {
		const std::string path = pimpl->root_path+"/"+name.substr(0, pos);
		//std::cout << "path:" << path << std::endl;
		if ( !boost::filesystem::exists(path) ) {
			boost::system::error_code ec;
			boost::filesystem::create_directories(path, ec);
			YAL_THROW_IF(ec, "can't create directory(" +path+ "): " + ec.message());
		}
	}

	yal::session session = std::make_shared<detail::session>(
		 pimpl->root_path
		,name
		,volume_size
		,opts
	);
	pimpl->sessions.insert({name, session});

	return session;
}

/***************************************************************************/

void session_manager::write(
	 const char *fileline
	,const std::size_t fileline_len
	,const char *func
	,const std::size_t func_len
	,const std::string &data
	,const level lvl)
{
	guard_t lock(pimpl->mutex);

	pimpl->iterate(
		[fileline, fileline_len, func, func_len, &data, lvl](yal::session s) {
			if ( s->get_level() >= lvl )
				s->write(fileline, fileline_len, func, func_len, data, lvl);
		}
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

session logger::create(const std::string &name, std::size_t volume_size, std::uint32_t opts) {
	return instance()->create(name, volume_size, opts);
}

yal::session logger::get(const std::string &name) {
	return instance()->get(name);
}

void logger::write(
	 const char *fileline
	,const std::size_t fileline_len
	,const char *func
	,const std::size_t func_len
	,const std::string &data
	,const level lvl)
{
	instance()->write(fileline, fileline_len, func, func_len, data, lvl);
}

void logger::flush() { instance()->flush(); }

void logger::root_path(const std::string &path) { instance()->root_path(path); }

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

} // ns yal
