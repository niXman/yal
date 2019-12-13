
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

#include <yal/yal.hpp>
#include <yal/index.hpp>
#include <yal/throw.hpp>

#include <cstdio>
#include <cctype>
#include <cmath>

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

namespace yal {

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

char level_chr(const level lvl) {
    return (
        lvl == yal::info ? 'I'
            : lvl == yal::debug ? 'D'
                : lvl == yal::warning ? 'W'
                    : lvl == yal::error ? 'E'
                        : 'X'
    );
}

/***************************************************************************/

namespace detail {

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#ifdef _WIN32

int fdatasync(int fd) {
    (void)fd;
    return 0;
}

#ifndef S_IRUSR
#   define S_IRUSR 0
#endif // S_IRUSR

#ifndef S_IWUSR
#   define S_IWUSR 0
#endif // S_IWUSR

#endif // _WIN32

/***************************************************************************/

static const char *active_ext = ".active";

struct io_base {
    virtual ~io_base() {}

    virtual void create(const std::string &fname) = 0;
    virtual void write(const void *ptr, const std::size_t size) = 0;
    virtual void close() = 0;
    virtual void fsync() = 0;
    virtual std::size_t fpos() = 0;

    static std::string normalize_fname(const std::string &fname) {
        return fname.substr(0, fname.length()-std::strlen(active_ext));
    }
};

struct file_io: io_base {
    file_io()
        :fd(-1)
        ,off(0)
        ,fname()
    {}
    virtual ~file_io() { close(); }

    void create(const std::string &fn) {
        close();
        fname = fn+active_ext;
        fd = ::open(fname.c_str(), O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
        __YAL_THROW_IF(fd == -1, "can't create file \"" +fname+ "\"");
    }
    void write(const void *ptr, const std::size_t size) {
        __YAL_THROW_IF(fd == -1, "file \"" +fname+ "\" is not open");
        __YAL_THROW_IF(size != static_cast<std::size_t>(::write(fd, ptr, size)), "write error");

        off += size;
    }
    void close() {
        if ( fd != -1 ) {
            ::close(fd);
            fd = -1;

            boost::system::error_code ec;
            boost::filesystem::rename(fname, normalize_fname(fname), ec);
        }

        off = 0;
    }
    void fsync() {
        __YAL_THROW_IF(fd == -1, "file \"" +fname+ "\" is not open");
        ::fdatasync(fd);
    }
    std::size_t fpos() { return off; }

private:
    int fd;
    std::size_t off;
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
        fd = ::open(fname.c_str(), O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
        __YAL_THROW_IF(fd == -1, "can't create file \"" +fname+ "\"");

        const char mode[4] = {'w','b','0'+YAL_COMPRESSION_LEVEL,0};
        gzfile = ::gzdopen(fd, mode);
        __YAL_THROW_IF(gzfile == nullptr, "can't create file \"" +fname+ "\"");
    }
    void write(const void *ptr, const std::size_t size) {
        __YAL_THROW_IF(gzfile == nullptr || fd == -1, "file \"" +fname+ "\" is not open");
        __YAL_THROW_IF(size != (std::size_t)::gzwrite(gzfile, ptr, size), "write error");
    }
    void close() {
        if ( gzfile ) {
            ::gzclose(gzfile);
            gzfile = nullptr;

            ::close(fd);
            fd = -1;

            boost::system::error_code ec;
            boost::filesystem::rename(fname, normalize_fname(fname), ec);
        }
    }
    void fsync() {
        __YAL_THROW_IF(gzfile == nullptr || fd == -1, "file \"" +fname+ "\" is not open");
        ::gzflush(gzfile, Z_FINISH);
        ::fdatasync(fd);
    }
    std::size_t fpos() {
        __YAL_THROW_IF(gzfile == nullptr || fd == -1, "file \"" +fname+ "\" is not open");
        return static_cast<std::size_t>(::gztell(gzfile));
    }

private:
    int fd;
    ::gzFile gzfile;
    std::string fname;
};
#else
struct gz_file_io: file_io {};
#endif // YAL_SUPPORT_COMPRESSION

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

struct session::impl {
    static io_base* create_io(std::uint32_t opts) {
        return (opts & yal::compress)
            ? static_cast<io_base*>(new gz_file_io)
            : static_cast<io_base*>(new file_io)
        ;
    }

    impl(
         const std::string &path
        ,const std::string &name
        ,std::size_t volume_size
        ,std::uint32_t opts
        ,process_buffer proc
    )
        :path(path)
        ,name(name)
        ,volume_size(volume_size)
        ,options(opts)
        ,proc(std::move(proc))
        ,shift_after(YAL_MAX_VOLUME_NUMBER)
        ,logfile(create_io(opts))
        ,idxfile(opts & create_index_file ? (create_io(opts)) : nullptr)
        ,toterm(false)
        ,prefix()
        ,level(yal::info)
        ,recbuf()
        ,writen_bytes(0)
        ,volume_number(0)
    {
        if ( name != "disable" ) {
            volume_number = get_last_volume_number(path, name, ((options & yal::remove_empty_logs)>0));
            create_volume();
        } else {
            level = yal::disable;
        }
    }
    ~impl() {
        flush();
    }

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
                __YAL_THROW_IF(ec, "can't get filesize(" +path+ "): " + ec.message());
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
            __YAL_THROW_IF(ec, "can't remove empty volume(" +it+ "): " + ec.message());
        }

        ec.clear();
        for ( const auto &it: for_rename ) {
            boost::filesystem::rename(it, final_log_fname(it), ec);
            __YAL_THROW_IF(ec, "can't rename unfinished volume(" +it+ "): " + ec.message());
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

        const int digits = static_cast<int>(std::log10(shift_after)+1);
        std::snprintf(fmt, sizeof(fmt), "%s%d%s", "%s/%s-%0", digits, "d-%s");

        sec_datetime_str(datebuf, sizeof(datebuf));
        datebuf[sec_res_len] = 0;
        std::snprintf(
             pathbuf
            ,sizeof(pathbuf)
            ,fmt
            ,path.c_str()
            ,name.c_str()
            ,volume_number
            ,datebuf
        );

        const char *pos = std::strchr(name.c_str(), '.');
        if ( pos ) {
            std::strcat(pathbuf, pos);
        }

        logfile->create(pathbuf);

        if ( options & create_index_file ) {
            std::strcat(pathbuf, ".idx");
            idxfile->create(pathbuf);
        }
    }

    void flush() {
        logfile->fsync();
        if ( options & create_index_file )
            idxfile->fsync();
    }
    void to_term(bool ok, const std::string &pref) { toterm = ok; prefix = pref; }

    void write(
             const char *fileline
            ,std::size_t fileline_len
            ,const char *sfunc
            ,std::size_t sfunc_len
            ,const char *func
            ,std::size_t func_len
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

        if ( !(options & full_source_name) ) {
            const char *p = std::strrchr(fileline, '/');
            if ( p ) {
                fileline = p+1;
                fileline_len = std::strlen(fileline);
            }
        }

        std::size_t lfunc_len = sfunc_len;
        const char *lfunc_name = sfunc;
        if ( options & full_func_name ) {
            lfunc_len = func_len;
            lfunc_name = func;
        }

        const std::size_t reclen =
            1 // '['
            +dtlen
            +2 // ']['
            +1 // log-level char
            +2 // ']['
            +fileline_len
            +2 // ']['
            +lfunc_len
            +3 // ']: '
            +data.length()
            +1 // '\n'
        ;

        if ( reclen > recbuf.size() )
            recbuf.resize(reclen);

        char dtbuf[32] = "\0";
        std::memset(dtbuf, ' ', sizeof(dtbuf));
        datetime_str(dtbuf, sizeof(dtbuf), options);
        const char lvlchr = level_chr(lvl);

        /*********************************************/
        char *p = const_cast<char*>(recbuf.c_str());
        *p++ = '[';
        std::memcpy(p, dtbuf, dtlen);
        p += dtlen;
        *p++ = ']';
        *p++ = '[';
        *p++ = lvlchr;
        *p++ = ']';
        *p++ = '[';
        std::memcpy(p, fileline, fileline_len);
        p += fileline_len;
        *p++ = ']';
        *p++ = '[';
        std::memcpy(p, lfunc_name, lfunc_len);
        p += lfunc_len;
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
            std::fflush(term);
        }

        if ( options & create_index_file ) {
            const std::uint32_t off = static_cast<std::uint32_t>(logfile->fpos());
            const index_record record = {
                off // start
                ,1 // dt_off
                ,static_cast<std::uint8_t>(dtlen) // dt_len
                ,2 // lvl_off
                ,1 // lvl_len
                ,2 // fl_off
                ,static_cast<std::uint8_t>(fileline_len) // fl_len
                ,2 // func_off
                ,static_cast<std::uint8_t>(lfunc_len) // func_len
                ,3 // data_off
                ,static_cast<std::uint32_t>(data.size()+1/*for '\n' */) // data_len
            };

            idxfile->write(&record, sizeof(record));
        }

        if ( proc ) {
            const auto proc_res = proc(recbuf.c_str(), reclen);
            logfile->write(proc_res.first, proc_res.second);
        } else {
            logfile->write(recbuf.c_str(), reclen);
        }

        if ( options & fsync_each_record ) {
            logfile->fsync();
            if ( options & create_index_file ) {
                idxfile->fsync();
            }
        }

        writen_bytes += reclen;
        if ( writen_bytes >= volume_size ) {
            writen_bytes = 0;
            volume_number += 1;
            create_volume();
        }
    }

    const std::string        path;
    const std::string        name;
    const std::size_t        volume_size;
    const std::uint32_t      options;
    const process_buffer     proc;
    std::size_t              shift_after;
    std::unique_ptr<io_base> logfile;
    std::unique_ptr<io_base> idxfile;
    bool                     toterm;
    std::string              prefix;
    yal::level               level;
    std::string              recbuf;
    std::size_t              writen_bytes;
    std::size_t              volume_number;
};

/***************************************************************************/

session::session(
     const std::string &path
    ,const std::string &name
    ,std::size_t volume_size
    ,std::uint32_t opts
    ,process_buffer proc
)
    :pimpl(new impl(path, name, volume_size, opts, std::move(proc)))
{}

session::~session()
{ delete pimpl; }

/***************************************************************************/

const std::string& session::name() const { return pimpl->name; }
void session::to_term(const bool ok, const std::string &pref) { pimpl->to_term(ok, pref); }
void session::set_level(const level lvl) { pimpl->level = lvl; }
level session::get_level() const { return pimpl->level; }

void session::write(
     const char *fileline
    ,const std::size_t fileline_len
    ,const char *sfunc
    ,const std::size_t sfunc_len
    ,const char *func
    ,const std::size_t func_len
    ,const std::string &data
    ,const level lvl)
{
    pimpl->write(fileline, fileline_len, sfunc, sfunc_len, func, func_len, data, lvl);
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
#ifndef YAL_DOESNT_USE_TIMEZONE
    // hack for setting the 'timezone' extern var
    std::time_t t = time(0);
    std::tm *lt = localtime(&t);
    (void)lt;
#endif // YAL_DOESNT_USE_TIMEZONE
}

session_manager::~session_manager()
{ delete pimpl; }

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
        __YAL_THROW_IF(ec, "can't create directory(" +path+ "): " + ec.message());
    }

    pimpl->root_path = path;
}

std::shared_ptr<session>
session_manager::create(const std::string &name, std::size_t volume_size, std::uint32_t opts, process_buffer proc) {
    guard_t lock(pimpl->mutex);

    __YAL_THROW_IF(!name.empty() && name[0] == '/', "session name can't be a full path");

    pimpl->iterate(
        [&name](yal::session s) {
            __YAL_THROW_IF(s->name() == name, "session \""+name+"\" already exists");
        }
    );

    const auto pos = name.find_last_of('/');
    if ( pos != std::string::npos ) {
        const std::string path = pimpl->root_path+"/"+name.substr(0, pos);
        //std::cout << "path:" << path << std::endl;
        if ( !boost::filesystem::exists(path) ) {
            boost::system::error_code ec;
            boost::filesystem::create_directories(path, ec);
            __YAL_THROW_IF(ec, "can't create directory(" +path+ "): " + ec.message());
        }
    }

    yal::session session = std::make_shared<detail::session>(
         pimpl->root_path
        ,name
        ,volume_size
        ,opts
        ,std::move(proc)
    );
    pimpl->sessions.insert({name, session});

    return session;
}

/***************************************************************************/

void session_manager::write(
     const char *fileline
    ,const std::size_t fileline_len
    ,const char *sfunc
    ,const std::size_t sfunc_len
    ,const char *func
    ,const std::size_t func_len
    ,const std::string &data
    ,const level lvl)
{
    guard_t lock(pimpl->mutex);

    pimpl->iterate(
        [fileline, fileline_len, sfunc, sfunc_len, func, func_len, &data, lvl](yal::session s) {
            if ( s->get_level() >= lvl ) {
                s->write(fileline, fileline_len, sfunc, sfunc_len, func, func_len, data, lvl);
            }
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

const std::string& logger::root_path() { return instance()->root_path(); }

session logger::create(
     const std::string &name
    ,std::size_t volume_size
    ,std::uint32_t opts
    ,detail::process_buffer proc)
{
    return instance()->create(name, volume_size, opts, std::move(proc));
}

yal::session logger::get(const std::string &name) {
    return instance()->get(name);
}

void logger::write(
     const char *fileline
    ,const std::size_t fileline_len
    ,const char *sfunc
    ,const std::size_t sfunc_len
    ,const char *func
    ,const std::size_t func_len
    ,const std::string &data
    ,const level lvl)
{
    instance()->write(fileline, fileline_len, sfunc, sfunc_len, func, func_len, data, lvl);
}

void logger::flush() { instance()->flush(); }

void logger::root_path(const std::string &path) { instance()->root_path(path); }

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

} // ns yal
