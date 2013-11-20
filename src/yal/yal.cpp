#include <yal/yal.hpp>

#include <boost/filesystem.hpp>

#include <cstdio>
#include <cctype>
#include <cerrno>

#include <unordered_map>
#include <algorithm>

namespace yal {

/***************************************************************************/

logger::logger_session_manager_ptr logger::object;
std::once_flag logger::flag;

/***************************************************************************/

namespace detail {

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
		boost::filesystem::directory_iterator fs_beg(path), fs_end;
		for ( ; fs_beg != fs_end; ++fs_beg ) {
			auto filename = fs_beg->path().filename().string();
			if ( filename == "." || filename == ".." || boost::filesystem::is_directory(*fs_beg) )
				continue;

			if ( filename.find(name+"-") == std::string::npos )
				continue;

			auto beg = std::find(filename.begin(), filename.end(), '-');
			if ( beg == filename.end() || beg+1 == filename.end() )
				continue;

			std::advance(beg, 1);

			if ( !std::isdigit(*beg) )
				continue;

			auto end = beg;
			for ( ; std::isdigit(*end); ++end)
				;

			const std::size_t num = std::stoul(std::string(beg, end)) + 1;
			if ( num > volume_number )
				volume_number = num;
		}

		create_volume();
	}

	~impl() {
		std::fclose(file);
	}

	static const char* date_str() {
		static const char *fmtstring = "%d.%m.%Y-%H.%M.%S";

		static char buf[32];
		std::time_t raw_time;
		std::tm* time;

		std::time(&raw_time);
		time = localtime(&raw_time);
		strftime(buf, sizeof(buf), fmtstring, time);

		return &buf[0];
	}

	std::string volume_fname() {
		std::string result;

		if ( volume_number > shift_after ) {
			shift_after = shift_after * 10 + 9;
		}

		char fmt[64] = {0};
		const int digits = std::log10(shift_after)+1;
		std::snprintf(fmt, sizeof(fmt), "%s%d%s", "%s/%s-%0", digits, "d-%s");

		std::string::const_iterator it = std::find(name.begin(), name.end(), '.');
		if ( it == name.end() ) {
			result = (
				boost::format(fmt)
					% path
					% name
					% volume_number
					% date_str()
			).str();
		} else {
			std::strcat(fmt, "%s");
			result = (
				boost::format(fmt)
					% path
					% std::string(name.begin(), it)
					% volume_number
					% date_str()
					% std::string(it, name.end())
			).str();
		}

		//std::cout << "shift_after:" << shift_after << ", digits:" << digits << ", fmt:" << fmt << ", name:" << result << std::endl;

		return result;
	}

	void create_volume() {
		const std::string &fname = volume_fname();
		file = std::fopen(fname.c_str(), "wb");
		if ( !file ) {
			int error = errno;
			throw std::runtime_error("yal: cannot create logfile \"" + fname + "\" with errno="+std::to_string(error));
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
			lvl == yal::debug ? "debug  "
				:lvl == yal::error ? "error  "
					:lvl == yal::info ? "info   "
						:lvl == yal::warning ? "warning"
							: 0
		);
		if ( toterm ) {
			auto termlog = ((lvl == yal::info || lvl == yal::debug) ? stdout : stderr);
			if ( prefix.empty() ) {
				std::fprintf(termlog, "[%s][%s][%s][%s]: %s\n", date_str(), levelstr, fileline, func, data.c_str());
			} else {
				std::fprintf(termlog, "<%s>[%s][%s][%s][%s]: %s\n", prefix.c_str(), date_str(), levelstr, fileline, func, data.c_str());
			}
			std::fflush(termlog);
		}

		const int writen = std::fprintf(file, "[%s][%s][%s][%s]: %s\n", date_str(), levelstr, fileline, func, data.c_str());

		if ( writen < 0 ) {
			int error = errno;
			throw std::runtime_error("yal: cannot write to logfile with errno="+std::to_string(error));
		}

		writen_bytes += writen;
		if ( writen_bytes >= volume_size ) {
			writen_bytes = 0;

			std::fclose(file);

			volume_number += 1;
			create_volume();
		}
	}

	void flush() {
		if ( file )
			std::fflush(file);
	}

	const std::string			path;
	const std::string			name;
	const std::size_t			volume_size;
	std::size_t					shift_after;
	FILE							*file;
	bool							toterm;
	std::string					prefix;
	yal::level					level;
	std::size_t					writen_bytes;
	std::size_t					volume_number;
};

/***************************************************************************/

session::session(const std::string &path, const std::string &name, std::size_t volume_size, std::size_t shift_after)
	:pimpl(new impl(path, name, volume_size, shift_after))
{}

session::~session()
{ delete pimpl; }

const std::string &session::name() const { return pimpl->name; }

const char *session::date_str() { return session::impl::date_str(); }

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

void session::write(const char *fileline, const char *func, const std::string &data, level lvl) {
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

	template<typename Cont, typename F>
	static void iterate(Cont &cont, F func) {
		for ( auto it = cont.begin(), end = cont.end(); it != end; ) {
			if ( auto session = it->second.lock() ) {
				func(session);
				++it;
			} else {
				it = cont.erase(it);
			}
		}
	}

	std::mutex mutex;
	std::string root_path;
	sessions_map sessions;
}; // struct impl

/***************************************************************************/

session_manager::session_manager()
	:pimpl(new impl)
{}

session_manager::~session_manager() {
	flush();
	delete pimpl;
}

/***************************************************************************/

const std::string &session_manager::root_path() const {
	std::lock_guard<std::mutex> lock(pimpl->mutex);

	return pimpl->root_path;
}

void session_manager::root_path(const std::string &path) {
	std::lock_guard<std::mutex> lock(pimpl->mutex);

	if ( !boost::filesystem::exists(path) )
		boost::filesystem::create_directories(path);

	pimpl->root_path = path;
}

std::shared_ptr<session>
session_manager::create(const std::string &name, std::size_t volume_size, std::size_t shift_after) {
	std::lock_guard<std::mutex> lock(pimpl->mutex);

	if ( !name.empty() && name[0] == '/' )
		throw std::runtime_error("yal: session name cannot be a full path name");
	if ( !shift_after )
		throw std::runtime_error("yal: shift_after can be 1 or greater");

	impl::iterate(
		 pimpl->sessions
		,[&name](yal::session s) {
			if ( s->name() == name )
				throw std::runtime_error("yal: session \""+name+"\" already exists");
		}
	);

	const auto pos = name.find_last_of('/');
	if ( pos != std::string::npos ) {
		const std::string &path = pimpl->root_path+"/"+name.substr(0, pos);
		//std::cout << "path:" << path << std::endl;
		if ( !boost::filesystem::exists(path) )
			boost::filesystem::create_directories(path);
	}

	yal::session session = std::make_shared<detail::session>(pimpl->root_path, name, volume_size, shift_after);
	pimpl->sessions.insert({name, session});

	return session;
}

/***************************************************************************/

void session_manager::write(const char *fileline, const char *func, const std::string &data, level lvl) {
	std::lock_guard<std::mutex> lock(pimpl->mutex);

	impl::iterate(
		 pimpl->sessions
		,[fileline, func, &data, lvl](yal::session s) { s->write(fileline, func, data, lvl); }
	);
}

std::shared_ptr<session>
session_manager::get(const std::string &name) const {
	std::lock_guard<std::mutex> lock(pimpl->mutex);

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
	std::lock_guard<std::mutex> lock(pimpl->mutex);

	impl::iterate(
		 pimpl->sessions
		,[](yal::session s) { s->flush(); }
	);
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

} // ns detail

logger::logger_session_manager_ptr logger::instance() {
	std::call_once(flag, &init);
	return object;
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

void logger::init() { object = std::make_shared<logger_session_manager_ptr::element_type>(); }

void logger::root_path(const std::string &path) { instance()->root_path(path); }

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

} // ns yal
