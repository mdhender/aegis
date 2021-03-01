//
//	aegis - project change supervisor
//	Copyright (C) 1993-1995, 1997-1999, 2001-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//
// Most of the functions in this file are only used when
// the CONF_NO_seteuid symbol is defined in "[arch]/common/config.h".
//
// The various system calls wgich must be perfomed as a specific user
// are given to a "proxy" process to perform, where this proxy process
// has the appropriate real (and effective) user id and group id.
// These processes terminate when they see end-of-file on their command
// streams, which only happens when the parent process exits.
//
// The overhead of communicating with the proxy process will
// naturally result in a performance hit for systems without
// a seteuid system call.
//
// Note that some systems have a seteuid system call
// which is broken.  These systems will also need to use this glue.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/errno.h>
#include <common/ac/signal.h>

#include <common/ac/sys/types.h>
#include <common/ac/fcntl.h>
#include <common/ac/unistd.h>
#include <utime.h>
#include <common/ac/sys/stat.h>

//
// Turn off the function defines in aegis/glue.h so that we can
// define the glue functions without the macros getting in the way.
//
#define aegis_glue_disable

#include <common/error.h>
#include <common/mem.h>
#include <common/trace.h>
#include <libaegis/glue.h>
#include <libaegis/lock.h> // for lock_release_child
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>

#define GUARD1 0x416E6479
#define GUARD2 0x4C777279

enum
{
    command_access,
    command_catfile,
    command_chmod,
    command_chown,
    command_close,
    command_copyfile,
    command_creat,
    command_fcntl,
    command_file_compare,
    command_file_fingerprint,
    command_getcwd,
    command_link,
    command_lstat,
    command_lutime,
    command_mkdir,
    command_open,
    command_pathconf,
    command_read,
    command_readlink,
    command_read_whole_dir,
    command_rename,
    command_rmdir,
    command_rmdir_bg,
    command_rmdir_tree,
    command_stat,
    command_symlink,
    command_unlink,
    command_utime,
    command_write
};


struct proxy_ty
{
    int             hash;
    int             uid;
    int             gid;
    int             umask;
    int             pid;
    FILE            *command;
    FILE            *reply;
    proxy_ty        *next;

};

#define proxy_hash(u, g, m) (((u) * 16 + (g) * 4 + (m)) & 0x7FFF)

//
// This table will be sparsely filled.
// The maximum number of entries expected is 4.
// The size MUST be prime.
//
static proxy_ty	*proxy_table[17];


#ifdef DEBUG

static const char *
command_name(int n)
{
    static char	buf[12];

    switch (n)
    {
    case EOF: 	return "quit";
    case command_access: return "access";
    case command_catfile: return "catfile";
    case command_chmod: return "chmod";
    case command_chown: return "chown";
    case command_close: return "close";
    case command_copyfile: return "copyfile";
    case command_creat: return "creat";
    case command_fcntl: return "fcntl";
    case command_file_compare: return "file_compare";
    case command_file_fingerprint: return "file_fingerprint";
    case command_getcwd: return "getcwd";
    case command_link: return "link";
    case command_lstat: return "lstat";
    case command_lutime: return "lutime";
    case command_mkdir: return "mkdir";
    case command_open: return "open";
    case command_pathconf: return "pathconf";
    case command_read: return "read";
    case command_readlink: return "readlink";
    case command_read_whole_dir: return "read_whole_dir";
    case command_rename: return "rename";
    case command_rmdir: return "rmdir";
    case command_rmdir_bg: return "rmdir_bg";
    case command_rmdir_tree: return "rmdir_tree";
    case command_symlink: return "symlink";
    case command_stat: return "stat";
    case command_unlink: return "unlink";
    case command_utime: return "utime";
    case command_write: return "write";
    }
    snprintf(buf, sizeof(buf), "%d", n);
    return buf;
}

#endif // DEBUG


static void
put_int(FILE *fp, int n)
{
    trace(("put_int(%d)\n{\n", n));
    unsigned char *ptr = (unsigned char *)&n;
    for (size_t j = 0; j < sizeof(int); ++j)
	fputc(ptr[j], fp);
    if (ferror(fp))
	nfatal("writing pipe");
    trace(("}\n"));
}


static int
get_int(FILE *fp)
{
    int		result;
    size_t		j;
    unsigned char	*ptr;

    trace(("get_int()\n{\n"));
    ptr = (unsigned char *)&result;
    for (j = 0; j < sizeof(int); ++j)
    {
	int c = fgetc(fp);
	if (c == EOF)
	{
	    if (ferror(fp))
	       	nfatal("reading pipe");
	    fatal_raw("reading pipe: proxy protocol error (%d)", getpid());
	}
	ptr[j] = c;
    }
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


static void
put_long(FILE *fp, long n)
{
    size_t		j;
    unsigned char	*ptr;

    trace(("put_long(%ld)\n{\n", n));
    ptr = (unsigned char *)&n;
    for (j = 0; j < sizeof(long); ++j)
	fputc(ptr[j], fp);
    if (ferror(fp))
	nfatal("writing pipe");
    trace(("}\n"));
}


static long
get_long(FILE *fp)
{
    long		result;
    size_t		j;
    unsigned char	*ptr;

    trace(("get_long()\n{\n"));
    ptr = (unsigned char *)&result;
    for (j = 0; j < sizeof(long); ++j)
    {
	int c = fgetc(fp);
	if (c == EOF)
	{
	    if (ferror(fp))
	       	nfatal("reading pipe");
	    fatal_raw("reading pipe: proxy protocol error (%d)", getpid());
	}
	ptr[j] = c;
    }
    trace(("return %ld;\n", result));
    trace(("}\n"));
    return result;
}


static void
put_binary(FILE *fp, const void *ptr, size_t len)
{
    trace(("put_binary(%ld)\n{\n", (long)len));
    fwrite(ptr, 1, len, fp);
    if (ferror(fp))
	nfatal("writing pipe");
    trace(("}\n"));
}


static void
get_binary(FILE *fp, void *ptr, size_t len)
{
    long		n;

    trace(("get_binary(%ld)\n{\n", (long)len));
    n = fread(ptr, 1, len, fp);
    if ((size_t)n != len)
    {
	if (ferror(fp))
    	    nfatal("reading pipe");
	fatal_raw("reading pipe: proxy protocol error (%d)", getpid());
    }
    trace(("}\n"));
}


static void
put_string(FILE *fp, const char *s)
{
    trace(("put_string(\"%s\")\n{\n", s));
    for (;;)
    {
	fputc(*s, fp);
	if (ferror(fp))
    	    nfatal("writing pipe");
	if (!*s)
    	    break;
	++s;
    }
    trace(("}\n"));
}


static void *
get_string(FILE *fp)
{
    static char	*result;
    static size_t	result_max;
    size_t		pos;

    trace(("get_string()\n{\n"));
    if (!result)
    {
	result_max = (1L << 10 ) - 32;
	result = (char *)mem_alloc(result_max);
    }

    pos = 0;
    for (;;)
    {
	int c = fgetc(fp);
	if (c == EOF)
	{
	    if (ferror(fp))
	       	nfatal("reading pipe");
	    fatal_raw("reading pipe: proxy protocol error (%d)", getpid());
	}
	if (pos >= result_max)
	{
	    result_max = result_max * 2 + 16;
	    char *new_result = new char [result_max];
	    memcpy(new_result, result, result_max);
	    delete [] result;
	    result = new_result;
	}
	result[pos] = c;
	if (!c)
	    break;
	++pos;
    }
    trace(("return \"%s\";\n", result));
    trace(("}\n"));
    return result;
}


static void
proxy(int rd_fd, int wr_fd)
{
    trace(("proxy(%d, %d)\n{\n", rd_fd, wr_fd));
    errno = 0;
    FILE *command = fdopen(rd_fd, "r");
    if (!command)
    {
	if (!errno)
    	    errno = ENOMEM;
	exit(errno);
    }

    errno = 0;
    FILE *reply = fdopen(wr_fd, "w");
    if (!reply)
    {
	if (!errno)
    	    errno = ENOMEM;
	exit(errno);
    }

    for (;;)
    {
	int c = fgetc(command);
	trace(("command: %s\n", command_name(c)));
	trace(("uid = %d;\n", getuid()));
	trace(("gid = %d;\n", getgid()));
	switch (c)
	{
	case EOF:
	    if (ferror(command))
	       	exit(errno);
	    exit(0);

	default:
	    fatal_raw("proxy: unknown %d command (bug)", c);

	case command_access:
            {
                char *path = (char *)get_string(command);
                int mode = get_int(command);
                int result = 0;
                if (access(path, mode))
                    result = errno;
                put_int(reply, result);
            }
	    break;

	case command_catfile:
            {
                char *path = (char *)get_string(command);
                int result = 0;
                if (catfile(path))
                    result = errno;
                put_int(reply, result);
            }
	    break;

	case command_chmod:
            {
                char *path = (char *)get_string(command);
                int mode = get_int(command);
                int result = 0;
                if (chmod(path, mode))
                    result = errno;
                put_int(reply, result);
            }
	    break;

	case command_chown:
            {
                char *path = (char *)get_string(command);
                int uid = get_int(command);
                int gid = get_int(command);
                int result = 0;
                if (chown(path, uid, gid))
                    result = errno;
                put_int(reply, result);
            }
	    break;

	case command_close:
            {
                int fd = get_int(command);
                int result = 0;
                if (close(fd))
                    result = errno;
                put_int(reply, result);
            }
	    break;

	case command_copyfile:
            {
                char *path = (char *)get_string(command);
                char *path1 = mem_copy_string(path);
                path = (char *)get_string(command);
                int result = copyfile(path1, path);
                if (result)
                    result = errno;
                mem_free(path1);
                put_int(reply, result);
            }
	    break;

	case command_creat:
            {
                char *path = (char *)get_string(command);
                int mode = get_int(command);
                int result = creat(path, mode);
                put_int(reply, result);
                if (result < 0)
                    put_int(reply, errno);
            }
	    break;

	case command_getcwd:
            {
                int path_max = get_int(command);
                char *path = (char *)mem_alloc(path_max);
                if (!getcwd(path, path_max))
                    put_int(reply, errno);
                else
                {
                    put_int(reply, 0);
                    put_string(reply, path);
                }
                mem_free(path);
            }
	    break;

	case command_fcntl:
            {
                int fd = get_int(command);
                int mode = get_int(command);
                struct flock theFlock;
                get_binary(command, &theFlock, sizeof(theFlock));
                int result = fcntl(fd, mode, &theFlock);
                if (result)
                    result = errno;
                put_int(reply, result);
                put_binary(reply, &theFlock, sizeof(theFlock));
            }
	    break;

	case command_file_compare:
            {
                char *path = (char *)get_string(command);
                char *path1 = mem_copy_string(path);
                path = (char *)get_string(command);
                int result = file_compare(path1, path);
                if (result < 0)
                    result = -errno;
                mem_free(path1);
                put_int(reply, result);
            }
	    break;

	case command_file_fingerprint:
            {
                char *path = (char *)get_string(command);
                int path1_max = get_int(command);
                char *path1 = (char *)mem_alloc(path1_max + 1);
                int result = file_fingerprint(path, path1, path1_max);
                put_int(reply, result);
                if (result < 0)
                    put_int(reply, errno);
                else
                    put_binary(reply, path1, result);
                mem_free(path1);
            }
	    break;

	case command_link:
            {
                char *path = (char *)get_string(command);
                char *path1 = mem_copy_string(path);
                char *path2 = (char *)get_string(command);
                int result = link(path1, path2);
                if (result)
                    result = errno;
                mem_free(path1);
                put_int(reply, result);
            }
	    break;

	case command_lstat:
            {
                char *path = (char *)get_string(command);
                struct stat st;
#ifdef S_IFLNK
                int result = lstat(path, &st);
#else
                int result = stat(path, &st);
#endif
                if (result)
                    put_int(reply, errno);
                else
                {
                    put_int(reply, 0);
                    put_binary(reply, &st, sizeof(st));
                }
            }
	    break;

	case command_lutime:
            {
                char *path = (char *)get_string(command);
                struct utimbuf utb;
                get_binary(command, &utb, sizeof(utb));
#ifdef HAVE_LUTIME
                int result = utime(path, &utb);
                if (result)
                    result = errno;
#else
                (void)path;
                int result = EINVAL;
#endif
                put_int(reply, result);
            }
	    break;

	case command_mkdir:
            {
                char *path = (char *)get_string(command);
                int mode = get_int(command);
                int result = 0;
                if (mkdir(path, mode))
                    result = errno;
                put_int(reply, result);
            }
	    break;

	case command_open:
            {
                char *path = (char *)get_string(command);
                int mode = get_int(command);
                int perm = get_int(command);
                int result = open(path, mode, perm);
                put_int(reply, result);
                if (result < 0)
                    put_int(reply, errno);
            }
	    break;

	case command_pathconf:
            {
                char *path = (char *)get_string(command);
                int mode = get_int(command);
#ifndef _PC_NAME_MAX
                (void)path;
                (void)mode;
                put_long(reply, -1L);
                put_int(reply, EINVAL);
#else
                errno = EINVAL;
                long nbytes = pathconf(path, mode);
                put_long(reply, nbytes);
                if (nbytes < 0)
                    put_int(reply, errno);
#endif
            }
	    break;

	case command_rename:
            {
                char *path = (char *)get_string(command);
                char *path1 = mem_copy_string(path);
                path = (char *)get_string(command);
                int result = rename(path1, path);
                if (result)
                    result = errno;
                mem_free(path1);
                put_int(reply, result);
            }
	    break;

	case command_read:
            {
                int fd = get_int(command);
                long nbytes = get_long(command);
                char *buf = (char *)mem_alloc(nbytes);
                long nbytes2 = read(fd, buf, nbytes);
                put_long(reply, nbytes2);
                if (nbytes2 > 0)
                    put_binary(reply, buf, nbytes2);
                else if (nbytes2 < 0)
                    put_int(reply, errno);
                mem_free(buf);
            }
	    break;

	case command_readlink:
            {
                char *path = (char *)get_string(command);
                int path1_max = get_int(command);
                char *path1 = (char *)mem_alloc(path1_max + 1);
#ifdef S_IFLNK
                int result = readlink(path, path1, path1_max);
                put_int(reply, result);
                if (result < 0)
                    put_int(reply, errno);
                else
                    put_binary(reply, path1, result);
#else
                put_int(reply, -1);
                put_int(reply, EINVAL);
#endif
                mem_free(path1);
            }
	    break;

	case command_read_whole_dir:
            {
                char *path = (char *)get_string(command);
                long nbytes = 0;
                char *buf = 0;
                int result = read_whole_dir(path, &buf, &nbytes);
                if (result < 0)
                    put_int(reply, errno);
                else
                {
                    put_int(reply, 0);
                    put_long(reply, nbytes);
                    put_binary(reply, buf, nbytes);
                }
                // do not free *buf, or *buf[*]
            }
	    break;

	case command_rmdir:
            {
                char *path = (char *)get_string(command);
                int result = 0;
                if (rmdir(path))
                    result = errno;
                put_int(reply, result);
            }
	    break;

	case command_rmdir_bg:
            {
                char *path = (char *)get_string(command);
                int result = 0;
                if (rmdir_bg(path))
                    result = errno;
                put_int(reply, result);
            }
	    break;

	case command_rmdir_tree:
            {
                char *path = (char *)get_string(command);
                int result = 0;
                if (rmdir_tree(path))
                    result = errno;
                put_int(reply, result);
            }
	    break;

	case command_stat:
            {
                char *path = (char *)get_string(command);
                struct stat st;
                int result = stat(path, &st);
                if (result)
                    put_int(reply, errno);
                else
                {
                    put_int(reply, 0);
                    put_binary(reply, &st, sizeof(st));
                }
            }
	    break;

	case command_symlink:
            {
                char *path = (char *)get_string(command);
                char *path1 = mem_copy_string(path);
                char *path2 = (char *)get_string(command);
#ifdef S_IFLNK
                int result = symlink(path1, path2);
                if (result)
                    result = errno;
#else
                int result = EINVAL;
#endif
                mem_free(path1);
                put_int(reply, result);
            }
	    break;

	case command_unlink:
            {
                char *path = (char *)get_string(command);
                int result = unlink(path);
                if (result)
                    result = errno;
                put_int(reply, result);
            }
	    break;

	case command_utime:
            {
                char *path = (char *)get_string(command);
                struct utimbuf utb;
                get_binary(command, &utb, sizeof(utb));
                int result = utime(path, &utb);
                if (result)
                    result = errno;
                put_int(reply, result);
            }
	    break;

	case command_write:
            {
                int fd = get_int(command);
                long nbytes = get_long(command);
                char *buf = (char *)mem_alloc(nbytes);
                get_binary(command, buf, nbytes);
                long nbytes2 = write(fd, buf, nbytes);
                put_long(reply, nbytes2);
                if (nbytes2 < 0)
                    put_int(reply, errno);
                mem_free(buf);
            }
	    break;
	}
	fflush(reply);
	if (ferror(reply))
	    exit(errno);
    }
    trace(("}\n"));
}


static void
get_pipe(int *rd, int *wr)
{
    int	fd[2];

    trace(("get_pipe()\n{\n"));
    if (pipe(fd))
	nfatal("pipe");
    *rd = fd[0];
    *wr = fd[1];
    trace(("read %d\n", fd[0]));
    trace(("write %d\n", fd[1]));
    trace(("}\n"));
}


static void
proxy_close(void)
{
    proxy_ty	*p;
    size_t		j;

    trace(("proxy_close()\n{\n"));
    trace(("pid = %d;\n", getpid()));
    for (j = 0; j < SIZEOF(proxy_table); ++j)
    {
	for (;;)
	{
	    p = proxy_table[j];
	    if (!p)
		break;
	    trace(("p->pid %d; uid %d; gid %d\n",
		p->pid, p->uid, p->gid));
	    proxy_table[j] = p->next;
	    fclose(p->command);
	    fclose(p->reply);
	    mem_free((char *)p);
	}
    }
    trace(("}\n"));
}


static void
proxy_spawn(proxy_ty *pp)
{
    int		command_read_fd;
    int		command_write_fd;
    int		reply_read_fd;
    int		reply_write_fd;
    int		pid;
    static int	quitregd;

    trace(("proxy_spawn()\n{\n"));
    if (!quitregd)
    {
	quitregd = 1;
	signal(SIGPIPE, SIG_IGN);
	trace(("master pid %d;\n", getpid()));
    }

    get_pipe(&command_read_fd, &command_write_fd);
    get_pipe(&reply_read_fd, &reply_write_fd);
    switch (pid = fork())
    {
    case -1:
	nfatal("fork");

    case 0:
	//
	// close the ends of the pipes the proxy will not be using
	//
	close(command_write_fd);
	close(reply_read_fd);
	os_interrupt_ignore();

	//
	// the proxy now assumes the appropriate ID
	//
	if (setgid(pp->gid))
		exit(errno);
	if (setuid(pp->uid))
		exit(errno);
	umask(pp->umask);

	//
	// close all of the master ends of all the other proxys
	// otherwise they will keep each other alive
	// after the master dies
	//
	trace_indent_reset();
	proxy_close();

	//
	// normally the proxys are silent,
	// returning all errors to the master.
	// Should one of the error functions be called,
	// make sure the proxy does not perform the undo.
	//
	undo_cancel();

	//
	// do whatever is asked
	//
	proxy(command_read_fd, reply_write_fd);
	exit(0);

    default:
	//
	// close the ends of the pipes the master will not be using
	//
	close(command_read_fd);
	close(reply_write_fd);

	//
	// remember who the child is
	// (even though we don't have a use for it at the moment)
	//
	pp->pid = pid;
	trace(("child pid %d\n", getpid()));

	//
	// open a buffered stream for commands
	//
	errno = 0;
	pp->command = fdopen(command_write_fd, "w");
	if (!pp->command)
	{
	    if (!errno)
	       	errno = ENOMEM;
	    nfatal("fdopen");
	}

	//
	// open a buffered stream for replies
	//
	errno = 0;
	pp->reply = fdopen(reply_read_fd, "r");
	if (!pp->reply)
	{
	    if (!errno)
	       	errno = ENOMEM;
	    nfatal("fdopen");
	}
	break;
    }
    trace(("}\n"));
}


static proxy_ty *
proxy_find(void)
{
    int		uid;
    int		gid;
    int		um;
    int		hash;
    int		pix;
    proxy_ty	*pp;

    //
    // search for an existing proxy
    //
    trace(("proxy_find()\n{\n"));
    os_become_must_be_active();
    os_become_query(&uid, &gid, &um);
    hash = proxy_hash(uid, gid, um);
    pix = hash % SIZEOF(proxy_table);
    for (pp = proxy_table[pix]; pp; pp = pp->next)
    {
	if (pp->hash != hash)
    	    continue;
	if (pp->uid != uid || pp->gid != gid || pp->umask != um)
    	    continue;
	goto done;
    }

    //
    // no such proxy, so create a new one
    //
    trace(("uid = %d; gid = %d; umask = 0%o;\n", uid, gid, um));
    pp = (proxy_ty *)mem_alloc(sizeof(proxy_ty));
    pp->hash = hash;
    pp->uid = uid;
    pp->gid = gid;
    pp->umask = um;
    pp->pid = -1;
    pp->command = 0;
    pp->reply = 0;
    pp->next = 0;
    proxy_spawn(pp);

    //
    // glue into the table AFTER the spawn,
    // so the child doesn't close the wrong things.
    //
    pp->next = proxy_table[pix];
    proxy_table[pix] = pp;

    //
    // here for all exits
    //
    done:
    trace(("return %08lX;\n", (long)pp));
    trace(("}\n"));
    return pp;
}


static void
end_of_command(proxy_ty *pp)
{
    trace(("end_of_command()\n{\n"));
    if (fflush(pp->command))
	nfatal("write pipe");
    trace(("}\n"));
}


int
glue_stat(const char *path, struct stat *st)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_stat()\n{\n"));
    pp = proxy_find();
    fputc(command_stat, pp->command);
    put_string(pp->command, path);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    else
	get_binary(pp->reply, st, sizeof(*st));
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_lstat(const char *path, struct stat *st)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_lstat()\n{\n"));
    pp = proxy_find();
    fputc(command_lstat, pp->command);
    put_string(pp->command, path);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    else
	get_binary(pp->reply, st, sizeof(*st));
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_mkdir(const char *path, int mode)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_mkdir()\n{\n"));
    pp = proxy_find();
    fputc(command_mkdir, pp->command);
    put_string(pp->command, path);
    put_int(pp->command, mode);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_chown(const char *path, int uid, int gid)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_chown()\n{\n"));
    pp = proxy_find();
    fputc(command_chown, pp->command);
    put_string(pp->command, path);
    put_int(pp->command, uid);
    put_int(pp->command, gid);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_catfile(const char *path)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_catfile()\n{\n"));
    pp = proxy_find();
    fputc(command_catfile, pp->command);
    put_string(pp->command, path);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_chmod(const char *path, int mode)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_chmod()\n{\n"));
    pp = proxy_find();
    fputc(command_chmod, pp->command);
    put_string(pp->command, path);
    put_int(pp->command, mode);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_rmdir(const char *path)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_rmdir()\n{\n"));
    pp = proxy_find();
    fputc(command_rmdir, pp->command);
    put_string(pp->command, path);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_rmdir_bg(const char *path)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_rmdir_bg()\n{\n"));
    pp = proxy_find();
    fputc(command_rmdir_bg, pp->command);
    put_string(pp->command, path);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_rename(const char *p1, const char *p2)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_rename()\n{\n"));
    pp = proxy_find();
    fputc(command_rename, pp->command);
    put_string(pp->command, p1);
    put_string(pp->command, p2);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_symlink(const char *name1, const char *name2)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_symlink()\n{\n"));
    pp = proxy_find();
    fputc(command_symlink, pp->command);
    put_string(pp->command, name1);
    put_string(pp->command, name2);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_unlink(const char *path)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_unlink()\n{\n"));
    pp = proxy_find();
    fputc(command_unlink, pp->command);
    put_string(pp->command, path);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_link(const char *p1, const char *p2)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_link()\n{\n"));
    pp = proxy_find();
    fputc(command_link, pp->command);
    put_string(pp->command, p1);
    put_string(pp->command, p2);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_access(const char *path, int mode)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_access()\n{\n"));
    pp = proxy_find();
    fputc(command_access, pp->command);
    put_string(pp->command, path);
    put_int(pp->command, mode);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


char *
glue_getcwd(char *buf, int buf_len)
{
    //
    // don't bother with the case where buf
    // is the NULL pointer, aegis never uses it.
    //
    trace(("glue_getcwd()\n{\n"));
    assert(buf);
    proxy_ty *pp = proxy_find();
    fputc(command_getcwd, pp->command);
    put_int(pp->command, buf_len);
    end_of_command(pp);
    int result = get_int(pp->reply);
    char *s = 0;
    if (result)
    {
	trace(("return NULL; /* errno = %d */\n", result));
	errno = result;
    }
    else
    {
	s = (char *)get_string(pp->reply);
	strendcpy(buf, s, buf + buf_len);
	s = buf;
	trace(("return \"%s\";\n", s));
    }
    trace(("}\n"));
    return s;
}


int
glue_readlink(const char *path, char *buf, int buf_len)
{
    trace(("glue_readlink()\n{\n"));
    proxy_ty *pp = proxy_find();
    fputc(command_readlink, pp->command);
    put_string(pp->command, path);
    put_int(pp->command, buf_len);
    end_of_command(pp);
    int result = get_int(pp->reply);
    if (result < 0)
    {
	errno = get_int(pp->reply);
	result = -1;
    }
    else
    {
	get_binary(pp->reply, buf, result);
	trace(("buf = \"%.*s\";\n", result, buf));
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_utime(const char *path, struct utimbuf *values)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_utime()\n{\n"));
    pp = proxy_find();
    fputc(command_utime, pp->command);
    put_string(pp->command, path);
    put_binary(pp->command, values, sizeof(*values));
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_lutime(const char *path, struct utimbuf *values)
{
#ifdef HAVE_LUTIME
    proxy_ty	*pp;
    int		result;

    trace(("glue_utime()\n{\n"));
    pp = proxy_find();
    fputc(command_lutime, pp->command);
    put_string(pp->command, path);
    put_binary(pp->command, values, sizeof(*values));
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
#else
    (void)path;
    (void)values;
    errno = EINVAL;
    return -1;
#endif
}


int
glue_copyfile(const char *p1, const char *p2)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_copyfile()\n{\n"));
    pp = proxy_find();
    fputc(command_copyfile, pp->command);
    put_string(pp->command, p1);
    put_string(pp->command, p2);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


struct glue_file_ty
{
    long		guard1;
    char		*path;
    int		fmode;
    char		*buffer_end;
    char		*buffer_pos;
    int		fd;
    int		errno_sequester;
    int		pushback;
    proxy_ty	*pp;
    char		buffer[1 << 11]; // fits within knl pipe buf
    long		guard2;
};


FILE *
glue_fopen(const char *path, const char *mode)
{
    glue_file_ty	*gfp;
    proxy_ty	*pp;
    int		fmode;
    int		fd;

    trace(("glue_fopen()\n{\n"));
    if (!strcmp(mode, "r"))
	fmode = O_RDONLY;
    else if (!strcmp(mode, "w"))
	fmode = O_WRONLY | O_CREAT | O_TRUNC;
    else
    {
	errno = EINVAL;
	gfp = 0;
	goto done;
    }

    pp = proxy_find();
    fputc(command_open, pp->command);
    put_string(pp->command, path);
    put_int(pp->command, fmode);
    put_int(pp->command, 0666);
    end_of_command(pp);
    fd = get_int(pp->reply);
    if (fd < 0)
    {
	errno = get_int(pp->reply);
	gfp = 0;
	goto done;
    }

    //
    // build our file structure
    //
    gfp = (glue_file_ty *)mem_alloc(sizeof(glue_file_ty));
    gfp->pp = pp;
    gfp->path = mem_copy_string(path);
    gfp->fmode = fmode;
    gfp->fd = fd;
    gfp->guard1 = GUARD1;
    gfp->guard2 = GUARD2;
    if (gfp->fmode == O_RDONLY)
    {
	gfp->buffer_end = 0;
	gfp->buffer_pos = 0;
    }
    else
    {
	gfp->buffer_pos = gfp->buffer;
	gfp->buffer_end = gfp->buffer + sizeof(gfp->buffer);
    }
    gfp->errno_sequester = 0;
    gfp->pushback = EOF;

    //
    // NOTE: this is veeerrry nasty.
    // The return value is not really a valid FILE,
    // but is only useful to the glue file functions.
    //
    done:
    trace(("return %08lX; /* errno = %d */\n", (long)gfp, errno));
    trace(("}\n"));
    return (FILE *)gfp;
}


int
glue_fclose(FILE *fp)
{
    proxy_ty	*pp;
    glue_file_ty	*gfp;
    int		result;
    int		result2;

    //
    // Leave the standard file streams alone.
    // There is a chance these will be seen here.
    //
    if (fp == stdout || fp == stdin || fp == stderr)
	return fclose(fp);

    //
    // flush the buffers
    //
    trace(("glue_fclose()\n{\n"));
    gfp = (glue_file_ty *)fp;
    result = (gfp->fmode != O_RDONLY && glue_fflush(fp)) ? errno : 0;

    //
    // locate the appropriate proxy
    //
    assert(gfp->guard1 == GUARD1);
    assert(gfp->guard2 == GUARD2);
    pp = gfp->pp;

    //
    // tell the proxy to close
    //
    fputc(command_close, pp->command);
    put_int(pp->command, gfp->fd);
    end_of_command(pp);
    result2 = get_int(pp->reply);
    if (!result)
	result = result2;

    //
    // Fclose always closes the file,
    // even when the implicit write fails.
    // Always dispose of our data.
    //
    mem_free(gfp->path);
    mem_free((char *)gfp);

    //
    // set errno and get out of here
    //
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_fgetc(FILE *fp)
{
    proxy_ty	*pp;
    glue_file_ty	*gfp;
    int		result;
    long		nbytes;

    //
    // Leave the standard file streams alone.
    // There is a chance these will be seen here.
    //
    if (fp == stdout || fp == stdin || fp == stderr)
	return fgetc(fp);

    //
    // locate the appropriate proxy
    //
    gfp = (glue_file_ty *)fp;
    assert(gfp->guard1 == GUARD1);
    assert(gfp->guard2 == GUARD2);
    pp = gfp->pp;

    //
    // complain if we are in an error state,
    // or they asked for something stupid
    //
    if (gfp->errno_sequester)
    {
	errno = gfp->errno_sequester;
	result = EOF;
	goto done;
    }
    if (gfp->fmode != O_RDONLY)
    {
	gfp->errno_sequester = EINVAL;
	errno = EINVAL;
	result = EOF;
	goto done;
    }

    //
    // use pushback if there is anything in it
    //
    if (gfp->pushback != EOF)
    {
	result = gfp->pushback;
	gfp->pushback = EOF;
	goto done;
    }

    //
    // use the buffer if there is anything in it
    //
    if (gfp->buffer_pos && gfp->buffer_pos < gfp->buffer_end)
    {
	result = (unsigned char)*gfp->buffer_pos++;
	goto done;
    }
    gfp->buffer_pos = 0;
    gfp->buffer_end = 0;

    //
    // tell the proxy to read another buffer-full
    //
    pp = gfp->pp;
    fputc(command_read, pp->command);
    put_int(pp->command, gfp->fd);
    put_long(pp->command, (long)sizeof(gfp->buffer));
    end_of_command(pp);
    nbytes = get_long(pp->reply);
    if (nbytes < 0)
    {
	gfp->errno_sequester = get_int(pp->reply);
	errno = gfp->errno_sequester;
	result = EOF;
	goto done;
    }
    if (nbytes == 0)
    {
	errno = 0;
	result = EOF;
	goto done;
    }
    assert((size_t)nbytes <= sizeof(gfp->buffer));
    get_binary(pp->reply, gfp->buffer, nbytes);
    gfp->buffer_pos = gfp->buffer;
    gfp->buffer_end = gfp->buffer + nbytes;
    result = (unsigned char)*gfp->buffer_pos++;

    //
    // here for all exits
    //
    done:
    return result;
}


long
glue_read(int fd, void *data, size_t len)
{
    proxy_ty	*pp;
    long		nbytes;

    //
    // Leave the standard file streams alone.
    // There is a chance these will be seen here.
    //
    if (fd == fileno(stdout) || fd == fileno(stdin) || fd == fileno(stderr))
	return read(fd, data, len);

    //
    // locate the appropriate proxy
    //
    pp = proxy_find();

    //
    // tell the proxy to read another buffer-full
    //
    fputc(command_read, pp->command);
    put_int(pp->command, fd);
    put_long(pp->command, len);
    end_of_command(pp);
    nbytes = get_long(pp->reply);
    if (nbytes < 0)
    {
	errno = get_int(pp->reply);
	return -1;
    }
    if (nbytes == 0)
    {
	errno = 0;
	return 0;
    }
    assert((size_t)nbytes <= len);
    get_binary(pp->reply, data, nbytes);
    return nbytes;
}


int
glue_ungetc(int c, FILE *fp)
{
    glue_file_ty	*gfp;
    int		result;

    //
    // Leave the standard file streams alone.
    // There is a chance these will be seen here.
    //
    if (fp == stdout || fp == stdin || fp == stderr)
	return ungetc(c, fp);

    //
    // make a pointer to our data
    //
    trace(("glue_ungetc()\n{\n"));
    result = EOF;
    gfp = (glue_file_ty *)fp;
    assert(gfp->guard1 == GUARD1);
    assert(gfp->guard2 == GUARD2);

    //
    // make sure not too many
    //
    if (gfp->pushback != EOF || c == EOF)
    {
	gfp->errno_sequester = EINVAL;
	errno = EINVAL;
	gfp->pushback = EOF;
	goto done;
    }

    //
    // stash the returned char
    //
    gfp->pushback = (unsigned char)c;
    result = (unsigned char)c;

    //
    // here for all exits
    //
    done:
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_fputc(int c, FILE *fp)
{
    proxy_ty	*pp;
    glue_file_ty	*gfp;
    int		result;

    //
    // Leave the standard file streams alone.
    // There is a chance these will be seen here.
    //
    if (fp == stdout || fp == stdin || fp == stderr)
	return fputc(c, fp);

    //
    // locate the appropriate proxy
    //
    result = EOF;
    gfp = (glue_file_ty *)fp;
    assert(gfp->guard1 == GUARD1);
    assert(gfp->guard2 == GUARD2);
    pp = gfp->pp;

    //
    // if the stream is in an error state,
    // or we were asked to do something stupid,
    // return the error
    //
    if (gfp->errno_sequester)
    {
	errno = gfp->errno_sequester;
	goto done;
    }
    if (gfp->fmode == O_RDONLY)
    {
	gfp->errno_sequester = EINVAL;
	errno = EINVAL;
	goto done;
    }

    //
    // if there is no room in the buffer,
    // flush it to the proxy
    //
    assert(gfp->buffer_pos);
    if (gfp->buffer_pos >= gfp->buffer_end)
    {
	long	nbytes;
	long	nbytes2;

	fputc(command_write, pp->command);
	put_int(pp->command, gfp->fd);
	nbytes = gfp->buffer_pos - gfp->buffer;
	put_long(pp->command, nbytes);
	put_binary(pp->command, gfp->buffer, nbytes);
	end_of_command(pp);
	nbytes2 = get_long(pp->reply);
	if (nbytes2 < 0)
	{
	    gfp->errno_sequester = get_int(pp->reply);
	    errno = gfp->errno_sequester;
	    goto done;
	}
	if (nbytes2 != nbytes)
	{
	    gfp->errno_sequester = EIO;
	    errno = EIO;
	    goto done;
	}
	gfp->buffer_pos = gfp->buffer;
	gfp->buffer_end = gfp->buffer + sizeof(gfp->buffer);
    }

    //
    // stash the character
    //
    *gfp->buffer_pos++ = c;
    result = (unsigned char)c;

    //
    // here for all exits
    //
    done:
    return result;
}


int
glue_fwrite(char *buf, long len1, long len2, FILE *fp)
{
    proxy_ty	*pp;
    glue_file_ty	*gfp;
    int		result;
    long		len;

    //
    // Leave the standard file streams alone.
    // There is a chance these will be seen here.
    //
    if (fp == stdout || fp == stdin || fp == stderr)
	return fwrite(buf, len1, len2, fp);

    //
    // locate the appropriate proxy
    //
    result = EOF;
    gfp = (glue_file_ty *)fp;
    assert(gfp->guard1 == GUARD1);
    assert(gfp->guard2 == GUARD2);
    pp = gfp->pp;

    //
    // if the stream is in an error state,
    // or we were asked to do something stupid,
    // return the error
    //
    if (gfp->errno_sequester)
    {
	errno = gfp->errno_sequester;
	goto done;
    }
    if (gfp->fmode == O_RDONLY)
    {
	gfp->errno_sequester = EINVAL;
	errno = EINVAL;
	goto done;
    }

    //
    // push the bytes into the buffer
    //
    len = len1 * len2;
    while (len > 0)
    {
	int c = (unsigned char)*buf++;
	--len;

	//
	// if there is no room in the buffer,
	// flush it to the proxy
	//
	assert(gfp->buffer_pos);
	if (gfp->buffer_pos >= gfp->buffer_end)
	{
	    long	nbytes;
	    long	nbytes2;

	    fputc(command_write, pp->command);
	    put_int(pp->command, gfp->fd);
	    nbytes = gfp->buffer_pos - gfp->buffer;
	    put_long(pp->command, nbytes);
	    put_binary(pp->command, gfp->buffer, nbytes);
	    end_of_command(pp);
	    nbytes2 = get_long(pp->reply);
	    if (nbytes2 < 0)
	    {
		gfp->errno_sequester = get_int(pp->reply);
		errno = gfp->errno_sequester;
		goto done;
	    }
	    if (nbytes2 != nbytes)
	    {
		gfp->errno_sequester = EIO;
		errno = EIO;
		goto done;
	    }
	    gfp->buffer_pos = gfp->buffer;
	    gfp->buffer_end = gfp->buffer + sizeof(gfp->buffer);
	}

	//
	// stash the character
	//
	*gfp->buffer_pos++ = c;
    }
    result = len1;

    //
    // here for all exits
    //
    done:
    return result;
}


int
glue_ferror(FILE *fp)
{
    glue_file_ty	*gfp;

    //
    // Leave the standard file streams alone.
    // There is a chance these will be seen here.
    //
    if (fp == stdout || fp == stdin || fp == stderr)
	return ferror(fp);

    //
    // locate the appropriate proxy
    //
    gfp = (glue_file_ty *)fp;
    assert(gfp->guard1 == GUARD1);
    assert(gfp->guard2 == GUARD2);

    //
    // set errno depending on
    // the error for this stream.
    //
    if (gfp->errno_sequester)
    {
	errno = gfp->errno_sequester;
	gfp->errno_sequester = 0;
	return 1;
    }
    return 0;
}


int
glue_fflush(FILE *fp)
{
    glue_file_ty	*gfp;
    proxy_ty	*pp;
    int		result;

    //
    // Leave the standard file streams alone.
    // There is a chance these will be seen here.
    //
    if (fp == stdout || fp == stdin || fp == stderr)
	return fflush(fp);

    //
    // locate the appropriate proxy
    //
    trace(("glue_fflush()\n{\n"));
    result = EOF;
    gfp = (glue_file_ty *)fp;
    assert(gfp->guard1 == GUARD1);
    assert(gfp->guard2 == GUARD2);
    pp = gfp->pp;

    //
    // if the stream is in an error state,
    // don't do anything
    //
    if (gfp->errno_sequester)
    {
	errno = gfp->errno_sequester;
	goto done;
    }
    if (gfp->fmode == O_RDONLY)
    {
	gfp->errno_sequester = EINVAL;
	errno = EINVAL;
	goto done;
    }

    //
    // if there is anything in the buffer,
    // send it to the proxy
    //
    if (gfp->buffer_pos && gfp->buffer_pos > gfp->buffer)
    {
	long	nbytes;
	long	nbytes2;

	fputc(command_write, pp->command);
	put_int(pp->command, gfp->fd);
	nbytes = gfp->buffer_pos - gfp->buffer;
	put_long(pp->command, nbytes);
	put_binary(pp->command, gfp->buffer, nbytes);
	end_of_command(pp);
	nbytes2 = get_long(pp->reply);
	if (nbytes2 < 0)
	{
	    gfp->errno_sequester = get_int(pp->reply);
	    errno = gfp->errno_sequester;
	    goto done;
	}
	if (nbytes2 != nbytes)
	{
	    gfp->errno_sequester = EIO;
	    errno = EIO;
	    goto done;
	}
	gfp->buffer_pos = gfp->buffer;
	gfp->buffer_end = gfp->buffer + sizeof(gfp->buffer);
    }
    result = 0;

    //
    // here for all exits
    //
    done:
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_open(const char *path, int mode, int perm)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_open()\n{\n"));
    pp = proxy_find();
    fputc(command_open, pp->command);
    put_string(pp->command, path);
    put_int(pp->command, mode);
    put_int(pp->command, perm);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result < 0)
	errno = get_int(pp->reply);
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_creat(const char *path, int mode)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_creat()\n{\n"));
    pp = proxy_find();
    fputc(command_creat, pp->command);
    put_string(pp->command, path);
    put_int(pp->command, mode);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result < 0)
	errno = get_int(pp->reply);
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_close(int fd)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_close()\n{\n"));
    pp = proxy_find();
    fputc(command_close, pp->command);
    put_int(pp->command, fd);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_write(int fd, const void *buf, long len)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_write()\n{\n"));
    pp = proxy_find();
    fputc(command_write, pp->command);
    put_int(pp->command, fd);
    put_long(pp->command, len);
    put_binary(pp->command, buf, len);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_fcntl(int fd, int cmd, struct flock *data)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_fcntl()\n{\n"));
    assert(cmd == F_SETLKW || cmd == F_SETLK || cmd == F_UNLCK ||
	    cmd == F_GETLK);
    pp = proxy_find();
    fputc(command_fcntl, pp->command);
    put_int(pp->command, fd);
    put_int(pp->command, cmd);
    put_binary(pp->command, data, sizeof(*data));
    end_of_command(pp);
    result = get_int(pp->reply);
    get_binary(pp->reply, data, sizeof(*data));
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_file_compare(const char *p1, const char *p2)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_file_compare()\n{\n"));
    pp = proxy_find();
    fputc(command_rename, pp->command);
    put_string(pp->command, p1);
    put_string(pp->command, p2);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = -result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_file_fingerprint(const char *path, char *buf, int buf_len)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_file_fingerprint()\n{\n"));
    pp = proxy_find();
    fputc(command_file_fingerprint, pp->command);
    put_string(pp->command, path);
    put_int(pp->command, buf_len);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result < 0)
    {
	errno = get_int(pp->reply);
	result = -1;
    }
    else
    {
	get_binary(pp->reply, buf, result);
	trace(("buf = \"%.*s\";\n", result, buf));
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_read_whole_dir(const char *path, char **data_p, long *data_len_p)
{
    static char	*data;
    static long	data_max;
    long		data_len;
    proxy_ty	*pp;
    int		result;

    trace(("glue_read_whole_dir(path = \"%s\")\n{\n", path));
    pp = proxy_find();
    fputc(command_read_whole_dir, pp->command);
    put_string(pp->command, path);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    else
    {
	data_len = get_long(pp->reply);
	if (data_len > data_max)
	{
	    for (;;)
	    {
		data_max = data_max * 2 + 16;
		if (data_len <= data_max)
		    break;
	    }
	    delete [] data;
	    data = new char [data_max];
	}
	get_binary(pp->reply, data, data_len);
	*data_len_p = data_len;
	*data_p = data;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


long
glue_pathconf(const char *path, int cmd)
{
    proxy_ty	*pp;
    long		result;

    trace(("glue_pathconf()\n{\n"));
    pp = proxy_find();
    fputc(command_pathconf, pp->command);
    put_string(pp->command, path);
    put_int(pp->command, cmd);
    end_of_command(pp);
    result = get_long(pp->reply);
    if (result < 0)
	errno = get_int(pp->reply);
    trace(("return %ld; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}


int
glue_rmdir_tree(const char *path)
{
    proxy_ty	*pp;
    int		result;

    trace(("glue_open()\n{\n"));
    pp = proxy_find();
    fputc(command_rmdir_tree, pp->command);
    put_string(pp->command, path);
    end_of_command(pp);
    result = get_int(pp->reply);
    if (result)
    {
	errno = result;
	result = -1;
    }
    trace(("return %d; /* errno = %d */\n", result, errno));
    trace(("}\n"));
    return result;
}
