
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

#include <yal/yal.hpp>
#include <yal/index.hpp>
#include <yal/throw.hpp>

#include <cstdio>
#include <cctype>
#include <cmath>

#include <unordered_map>
#include <algorithm>
#include <mutex>

/***************************************************************************/

#if defined(_WIN32) || defined(_MSC_VER)

bool exists(const char *fname);

#else

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

bool exists(const char *fname) {
    return ::access(fname, F_OK) == 0;
}

bool create_dir_tree(const char* dirname) {
    char temp[1024] = "\0";
    char* pname = &temp[0];
    const char* end = std::strchr(dirname+1, '/');
    while ( true ) {
        std::strncpy(pname, dirname, end-dirname);
        if ( 0 == ::access(pname, F_OK) ) {
            if ( 0 == std::strcmp(pname, dirname) )
                break;

            end = std::strchr(end+1, '/');
            if ( !end )
                end = dirname + std::strlen(dirname);

            continue;
        }
        if ( 0 != ::mkdir(pname, S_IRWXU|S_IRWXG|S_IRWXO) )
            return false;

        if ( 0 == std::strcmp(pname, dirname) )
            break;

        end = strchr(end+1, '/');
        if ( !end )
            end = dirname + std::strlen(dirname);
    }

    return true;
}

bool is_directory(const char *path) {
    struct ::stat st{};
    return ::stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

std::size_t file_size(const char *fname) {
    struct ::stat st{};
    ::stat(fname, &st);

    return static_cast<std::size_t>(st.st_size);
}

#endif // WIN32

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

const char *level_str(const level lvl) {
    return (
        lvl == yal::info ? "I"
            : lvl == yal::debug ? "D"
                : lvl == yal::warning ? "W"
                    : lvl == yal::error ? "E"
                        : "X"
    );
}

char level_chr(const level lvl) {
    const char *str = level_str(lvl);

    return str[0];
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

static const char active_ext[] = ".active";

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

            ::rename(fname.c_str(), normalize_fname(fname).c_str());
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

            ::rename(fname.c_str(), normalize_fname(fname).c_str());
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
        ,std::size_t opts
        ,process_buffer proc
    )
        :m_path(path)
        ,m_name(name)
        ,m_volume_size(volume_size)
        ,m_options(opts)
        ,m_proc(std::move(proc))
        ,m_shift_after(YAL_MAX_VOLUME_NUMBER)
        ,m_logfile(create_io(opts))
        ,m_idxfile(opts & create_index_file ? (create_io(opts)) : nullptr)
        ,m_toterm(false)
        ,m_prefix()
        ,m_level(yal::info)
        ,m_recbuf()
        ,m_writen_bytes(0)
        ,m_volume_number(0)
    {
        if ( m_name != "disable" ) {
            m_volume_number = get_last_volume_number(m_path, m_name, ((m_options & yal::remove_empty_logs)>0));
            create_volume();
        } else {
            m_level = yal::disable;
        }
    }
    ~impl() {
        flush();
    }

    static std::string final_log_fname(const std::string &fname) {
        return fname.substr(0, fname.length()-sizeof(active_ext)-1);
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

        struct dirent* dirent;
        DIR *dir = ::opendir(logpath.c_str());
        if ( !dir ) return 0;
        std::vector<std::string> empty_logs;
        std::vector<std::string> for_rename;
        while ( (dirent = readdir(dir)) != nullptr ) {
            if ( (dirent->d_name[0] == '.' && dirent->d_name[1] == 0) ||
                 (dirent->d_name[0] == '.' && dirent->d_name[1] == '.' && dirent->d_name[2] == 0)
            ) {
                continue;
            }

            std::string fname = dirent->d_name;
            std::string fpath = logpath;
            fpath += "/";
            fpath += fname;

            if ( is_directory(fpath.c_str()) )
                continue;

            if ( fpath.find(logfname+"-") == std::string::npos )
                continue;

            if ( remove_empty ) {
                const auto filesize = file_size(fpath.c_str());
                if ( filesize == 0 ) {
                    empty_logs.push_back(fpath);
                    continue;
                }
            }

            if ( fname.find(active_ext) != std::string::npos )
                for_rename.push_back(fpath);

            auto beg = std::find(fname.begin(), fname.end(), '-');
            if ( beg == fname.end() || beg+1 == fname.end() )
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

        for ( const auto &it: empty_logs ) {
            int ok = ::remove(it.c_str());
            __YAL_THROW_IF(ok, "can't remove empty volume");
        }

        for ( const auto &it: for_rename ) {
            const char *oldfname = it.c_str();
            const std::string newfname = final_log_fname(it);
            int ok = ::rename(oldfname, newfname.c_str());
            __YAL_THROW_IF(ok, "can't rename unfinished volume");
        }

        return volnum;
    }

    void create_volume() {
        char fmt[64];
        char pathbuf[1024*4];

        if ( m_volume_number > m_shift_after ) {
            m_shift_after = m_shift_after * 10 + 9;
        }

        const int digits = static_cast<int>(std::log10(m_shift_after)+1);
        std::snprintf(fmt, sizeof(fmt), "%s%d%s", "%s/%s-%0", digits, "d-%s");

        char datebuf[dtf::bufsize];
        const auto flags = dtf::flags::yyyy_mm_dd|dtf::flags::secs|dtf::flags::sep2;
        const auto n = dtf::timestamp_to_chars(datebuf, dtf::timestamp(), flags);
        datebuf[n] = 0;
        std::snprintf(
             pathbuf
            ,sizeof(pathbuf)
            ,fmt
            ,m_path.c_str()
            ,m_name.c_str()
            ,m_volume_number
            ,datebuf
        );

        if ( const char *pos = std::strchr(m_name.c_str(), '.') ) {
            std::strcat(pathbuf, pos);
        }

        m_logfile->create(pathbuf);

        if ( m_options & create_index_file ) {
            std::strcat(pathbuf, ".idx");
            m_idxfile->create(pathbuf);
        }
    }

    void flush() {
        m_logfile->fsync();
        if ( m_options & create_index_file )
            m_idxfile->fsync();
    }
    void to_term(bool ok, const std::string &pref) { m_toterm = ok; m_prefix = pref; }

    void write(
         const char *fileline
        ,std::size_t fileline_len
        ,const char *sfileline
        ,std::size_t sfileline_len
        ,const char *sfunc
        ,std::size_t sfunc_len
        ,const char *func
        ,std::size_t func_len
        ,const std::string &data
        ,const level lvl)
    {
        const auto dtres = (m_options & sec_res) ? dtf::flags::secs
            : (m_options & msec_res) ? dtf::flags::msecs
                : (m_options & usec_res) ? dtf::flags::usecs
                    : dtf::flags::nsecs
        ;
        const auto dtflags = dtf::flags::yyyy_mm_dd|dtf::flags::sep3|dtres;
        char dtbuf[dtf::bufsize];
        const auto dt = dtf::timestamp();
        const auto dtlen = dtf::timestamp_to_chars(dtbuf, dt, dtflags);

        if ( !(m_options & full_source_name) ) {
            fileline = sfileline;
            fileline_len = sfileline_len;
        }

        std::size_t lfunc_len = sfunc_len;
        const char *lfunc_name = sfunc;
        if ( m_options & full_func_name ) {
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

        if ( reclen > m_recbuf.size() )
            m_recbuf.resize(reclen);

        const char lvlchr = level_chr(lvl);

        /*********************************************/
        char *p = const_cast<char*>(m_recbuf.c_str());
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

        if ( m_toterm ) {
            FILE *term = ((lvl == yal::info || lvl == yal::debug) ? stdout : stderr);
            if ( !m_prefix.empty() ) {
                std::fprintf(term, "<%s>%s", m_prefix.c_str(), m_recbuf.c_str());
            } else {
                std::fprintf(term, "%s", m_recbuf.c_str());
            }
            std::fflush(term);
        }

        if ( m_options & create_index_file ) {
            const std::uint32_t off = static_cast<std::uint32_t>(m_logfile->fpos());
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

            m_idxfile->write(&record, sizeof(record));
        }

        if ( m_proc ) {
            const auto proc_res = m_proc(m_recbuf.c_str(), reclen);
            m_logfile->write(proc_res.first, proc_res.second);
        } else {
            m_logfile->write(m_recbuf.c_str(), reclen);
        }

        if ( m_options & fsync_each_record ) {
            m_logfile->fsync();
            if ( m_options & create_index_file ) {
                m_idxfile->fsync();
            }
        }

        m_writen_bytes += reclen;
        if ( m_writen_bytes >= m_volume_size ) {
            m_writen_bytes = 0;
            m_volume_number += 1;
            create_volume();
        }
    }

    const std::string        m_path;
    const std::string        m_name;
    const std::size_t        m_volume_size;
    const std::size_t        m_options;
    const process_buffer     m_proc;
    std::size_t              m_shift_after;
    std::unique_ptr<io_base> m_logfile;
    std::unique_ptr<io_base> m_idxfile;
    bool                     m_toterm;
    std::string              m_prefix;
    yal::level               m_level;
    std::string              m_recbuf;
    std::size_t              m_writen_bytes;
    std::size_t              m_volume_number;
};

/***************************************************************************/

session::session(
     const std::string &path
    ,const std::string &name
    ,std::size_t volume_size
    ,std::size_t opts
    ,process_buffer proc
)
    :pimpl(new impl(path, name, volume_size, opts, std::move(proc)))
{}

session::~session()
{}

/***************************************************************************/

const std::string& session::name() const { return pimpl->m_name; }
std::size_t session::flags() const { return pimpl->m_options; }
void session::to_term(const bool ok, const std::string &pref) { pimpl->to_term(ok, pref); }
void session::set_level(const level lvl) { pimpl->m_level = lvl; }
level session::get_level() const { return pimpl->m_level; }

void session::write(
     const char *fileline
    ,const std::size_t fileline_len
    ,const char *sfileline
    ,const std::size_t sfileline_len
    ,const char *sfunc
    ,const std::size_t sfunc_len
    ,const char *func
    ,const std::size_t func_len
    ,const std::string &data
    ,const level lvl)
{
    pimpl->write(fileline, fileline_len, sfileline, sfileline_len, sfunc, sfunc_len, func, func_len, data, lvl);
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
{}

/***************************************************************************/

const std::string& session_manager::root_path() const {
    guard_t lock(pimpl->mutex);

    return pimpl->root_path;
}

void session_manager::root_path(const std::string &path) {
    guard_t lock(pimpl->mutex);

    if ( !exists(path.c_str()) ) {
        bool ok = create_dir_tree(path.c_str());
        __YAL_THROW_IF(!ok, "can't create logs root directory");
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
        if ( !exists(path.c_str()) ) {
            bool ok = create_dir_tree(path.c_str());
            __YAL_THROW_IF(!ok, "can't create volume path");
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
    ,const char *sfileline
    ,const std::size_t sfileline_len
    ,const char *sfunc
    ,const std::size_t sfunc_len
    ,const char *func
    ,const std::size_t func_len
    ,const std::string &data
    ,const level lvl)
{
    guard_t lock(pimpl->mutex);

    pimpl->iterate(
        [fileline, fileline_len, sfileline, sfileline_len, sfunc, sfunc_len, func, func_len, &data, lvl](yal::session s) {
            if ( s->get_level() >= lvl ) {
                s->write(fileline, fileline_len, sfileline, sfileline_len, sfunc, sfunc_len, func, func_len, data, lvl);
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
    ,const char *sfileline
    ,const std::size_t sfileline_len
    ,const char *sfunc
    ,const std::size_t sfunc_len
    ,const char *func
    ,const std::size_t func_len
    ,const std::string &data
    ,const level lvl)
{
    instance()->write(fileline, fileline_len, sfileline, sfileline_len, sfunc, sfunc_len, func, func_len, data, lvl);
}

void logger::flush() { instance()->flush(); }

void logger::root_path(const std::string &path) { instance()->root_path(path); }

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

} // ns yal
