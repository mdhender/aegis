/*
 *	aegis - project change supervisor
 *	Copyright (C) 1993 Peter Miller.
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to perform systems calls in subprocesses
 *
 * Most of the functions in this file are only used when
 * the CONF_NO_seteuid  symbol is defined in common/conf.h.
 *
 * The various system calls wgich must be perfomed as a specific user
 * are given to a "proxy" process to perform, where this proxy process
 * has the appropriate real (and effective) user id and group id.
 * These processes terminate when they see end-of-file on their command
 * streams, which only happens when the parent process exits.
 *
 * The overhead of communicating with the proxy process will
 * naturally result in a performance hit for systems without
 * a seteuid system call.
 *
 * Note that some systems, notably AIX, have a seteuid system call
 * which is broken.  These systems will also need to use this glue.
 */

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utime.h>

#include <error.h>
#include <glue.h>
#include <mem.h>
#include <os.h>
#include <trace.h>
#include <undo.h>

#ifdef CONF_NO_seteuid

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
	command_getcwd,
	command_link,
	command_lstat,
	command_mkdir,
	command_open,
	command_read,
	command_readlink,
	command_read_whole_dir,
	command_rename,
	command_rmdir,
	command_stat,
	command_unlink,
	command_utime,
	command_write,
};


typedef struct proxy_ty proxy_ty;
struct proxy_ty
{
	int		hash;
	int		uid;
	int		gid;
	int		umask;
	int		pid;
	FILE		*command;
	FILE		*reply;
	proxy_ty	*next;

};

#define proxy_hash(u, g, m) (((u) * 16 + (g) * 4 + (m)) & 0x7FFF)

/*
 * This table will be sparsely filled.
 * The maximum number of entries expected is 4.
 * The size MUST be prime.
 */
static proxy_ty	*proxy_table[17];


#ifdef DEBUG

static char *command_name _((int));

static char *
command_name(n)
	int		n;
{
	static char	buf[12];

	switch (n)
	{
	case EOF:		return "quit";
	case command_access:	return "access";
	case command_catfile:	return "catfile";
	case command_chmod:	return "chmod";
	case command_chown:	return "chown";
	case command_close:	return "close";
	case command_copyfile:	return "copyfile";
	case command_creat:	return "creat";
	case command_fcntl:	return "fcntl";
	case command_getcwd:	return "getcwd";
	case command_link:	return "link";
	case command_lstat:	return "lstat";
	case command_mkdir:	return "mkdir";
	case command_open:	return "open";
	case command_read:	return "read";
	case command_readlink:	return "readlink";
	case command_read_whole_dir: return "read_whole_dir";
	case command_rename:	return "rename";
	case command_rmdir:	return "rmdir";
	case command_stat:	return "stat";
	case command_unlink:	return "unlink";
	case command_utime:	return "utime";
	case command_write:	return "write";
	}
	sprintf(buf, "%d", n);
	return buf;
}

#endif


static void put_int _((FILE *, int));

static void
put_int(fp, n)
	FILE		*fp;
	int		n;
{
	int		j;
	unsigned char	*ptr;

	trace(("put_int(%d)\n{\n"/*}*/, n));
	ptr = (unsigned char *)&n; 
	for (j = 0; j < sizeof(int); ++j)
		putc(ptr[j], fp);
	if (ferror(fp))
		nfatal("writing pipe");
	trace((/*{*/"}\n"));
}


static int get_int _((FILE *));

static int
get_int(fp)
	FILE		*fp;
{
	int		result;
	int		j;
	unsigned char	*ptr;

	trace(("get_int()\n{\n"/*}*/));
	ptr = (unsigned char *)&result;
	for (j = 0; j < sizeof(int); ++j)
	{
		int	c;

		c = getc(fp);
		if (c == EOF)
		{
			if (ferror(fp))
				nfatal("reading pipe");
			fatal("reading pipe: proxy protocol error (%d)", getpid());
		}
		ptr[j] = c;
	}
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


static void put_long _((FILE *, long));

static void
put_long(fp, n)
	FILE		*fp;
	long		n;
{
	int		j;
	unsigned char	*ptr;

	trace(("put_long(%ld)\n{\n"/*}*/, n));
	ptr = (unsigned char *)&n; 
	for (j = 0; j < sizeof(long); ++j)
		putc(ptr[j], fp);
	if (ferror(fp))
		nfatal("writing pipe");
	trace((/*{*/"}\n"));
}


static long get_long _((FILE *));

static long
get_long(fp)
	FILE		*fp;
{
	long		result;
	int		j;
	unsigned char	*ptr;

	trace(("get_long()\n{\n"/*}*/));
	ptr = (unsigned char *)&result;
	for (j = 0; j < sizeof(long); ++j)
	{
		int	c;

		c = getc(fp);
		if (c == EOF)
		{
			if (ferror(fp))
				nfatal("reading pipe");
			fatal("reading pipe: proxy protocol error (%d)", getpid());
		}
		ptr[j] = c;
	}
	trace(("return %ld;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


static void put_binary _((FILE *, void *, size_t));

static void
put_binary(fp, ptr, len)
	FILE		*fp;
	void		*ptr;
	size_t		len;
{
	trace(("put_binary(%ld)\n{\n"/*}*/, len));
	fwrite(ptr, 1, len, fp);
	if (ferror(fp))
		nfatal("writing pipe");
	trace((/*{*/"}\n"));
}


static void get_binary _((FILE *, void *, size_t));

static void
get_binary(fp, ptr, len)
	FILE		*fp;
	void		*ptr;
	size_t		len;
{
	long		n;

	trace(("get_binary(%ld)\n{\n"/*}*/, len));
	n = fread(ptr, 1, len, fp);
	if (n != len)
	{
		if (ferror(fp))
			nfatal("reading pipe");
		fatal("reading pipe: proxy protocol error (%d)", getpid());
	}
	trace((/*{*/"}\n"));
}


static void put_string _((FILE *, char *));

static void
put_string(fp, s)
	FILE		*fp;
	char		*s;
{
	trace(("put_string(\"%s\")\n{\n"/*}*/, s));
	for (;;)
	{
		putc(*s, fp);
		if (ferror(fp))
			nfatal("writing pipe");
		if (!*s)
			break;
		++s;
	}
	trace((/*{*/"}\n"));
}


static void *get_string _((FILE *));

static void *
get_string(fp)
	FILE		*fp;
{
	static char	*result;
	static size_t	result_max;
	size_t		pos;

	trace(("get_string()\n{\n"/*}*/));
	if (!result)
	{
		result_max = (1L << 10 ) - 32;
		result = mem_alloc(result_max);
	}

	pos = 0;
	for (;;)
	{
		int	c;

		c = getc(fp);
		if (c == EOF)
		{
			if (ferror(fp))
				nfatal("reading pipe");
			fatal("reading pipe: proxy protocol error (%d)", getpid());
		}
		if (pos >= result_max)
		{
			result_max += (1L << 10);
			mem_change_size(&result, result_max);
		}
		result[pos] = c;
		if (!c)
			break;
		++pos;
	}
	trace(("return \"%s\";\n", result));
	trace((/*{*/"}\n"));
	return result;
}


static void proxy _((int rd, int wr));

static void
proxy(rd_fd, wr_fd)
	int		rd_fd;
	int		wr_fd;
{
	FILE		*command;
	FILE		*reply;
	char		*path;
	char		*path1;
	int		mode;
	struct stat	st;
	struct utimbuf	utb;
	int		uid;
	int		gid;
	char		*buf;
	int		max;
	int		perm;
	int		result;
	int		fd;
	long		nbytes;
	long		nbytes2;
	struct flock	flock;

	trace(("proxy(%d, %d)\n{\n"/*}*/, rd_fd, wr_fd));
	errno = 0;
	command = fdopen(rd_fd, "r");
	if (!command)
	{
		if (!errno)
			errno = ENOMEM;
		exit(errno);
	}

	errno = 0;
	reply = fdopen(wr_fd, "w");
	if (!reply)
	{
		if (!errno)
			errno = ENOMEM;
		exit(errno);
	}

	for (;;)
	{
		int	c;

		c = getc(command);
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
			fatal("proxy: unknown %d command (bug)", c);

		case command_access:
			path = get_string(command);
			mode = get_int(command);
			if (access(path, mode))
				result = errno;
			else
				result = 0;
			put_int(reply, result);
			break;

		case command_catfile:
			path = get_string(command);
			if (catfile(path))
				result = errno;
			else
				result = 0;
			put_int(reply, result);
			break;

		case command_chmod:
			path = get_string(command);
			mode = get_int(command);
			if (chmod(path, mode))
				result = errno;
			else
				result = 0;
			put_int(reply, result);
			break;

		case command_chown:
			path = get_string(command);
			uid = get_int(command);
			gid = get_int(command);
			if (chown(path, uid, gid))
				result = errno;
			else
				result = 0;
			put_int(reply, result);
			break;

		case command_close:
			fd = get_int(command);
			if (close(fd))
				result = errno;
			else
				result = 0;
			put_int(reply, result);
			break;

		case command_copyfile:
			path = get_string(command);
			path1 = mem_copy_string(path);
			path = get_string(command);
			result = copyfile(path1, path);
			if (result)
				result = errno;
			mem_free(path1);
			put_int(reply, result);
			break;

		case command_creat:
			path = get_string(command);
			mode = get_int(command);
			result = creat(path, mode);
			put_int(reply, result);
			if (result < 0)
				put_int(reply, errno);
			break;

		case command_getcwd:
			max = get_int(command);
			path = mem_alloc(max);
			if (!getcwd(path, max))
				put_int(reply, errno);
			else
			{
				put_int(reply, 0);
				put_string(reply, path);
			}
			mem_free(path);
			break;

		case command_fcntl:
			fd = get_int(command);
			mode = get_int(command);
			get_binary(command, &flock, sizeof(flock));
			result = fcntl(fd, mode, &flock);
			if (result)
				result = errno;
			put_int(reply, result);
			put_binary(reply, &flock, sizeof(flock));
			break;

		case command_lstat:
			path = get_string(command);
#ifdef S_IFLNK
			result = lstat(path, &st);
#else
			result = stat(path, &st);
#endif
			if (result)
				put_int(reply, errno);
			else
			{
				put_int(reply, 0);
				put_binary(reply, &st, sizeof(st));
			}
			break;


		case command_link:
			path = get_string(command);
			path1 = mem_copy_string(path);
			path = get_string(command);
			result = link(path1, path);
			if (result)
				result = errno;
			mem_free(path1);
			put_int(reply, result);
			break;

		case command_mkdir:
			path = get_string(command);
			mode = get_int(command);
			if (mkdir(path, mode))
				result = errno;
			else
				result = 0;
			put_int(reply, result);
			break;

		case command_open:
			path = get_string(command);
			mode = get_int(command);
			perm = get_int(command);
			result = open(path, mode, perm);
			put_int(reply, result);
			if (result < 0)
				put_int(reply, errno);
			break;

		case command_rename:
			path = get_string(command);
			path1 = mem_copy_string(path);
			path = get_string(command);
			result = rename(path1, path);
			if (result)
				result = errno;
			mem_free(path1);
			put_int(reply, result);
			break;

		case command_read:
			fd = get_int(command);
			nbytes = get_long(command);
			buf = mem_alloc(nbytes);
			nbytes2 = read(fd, buf, nbytes);
			put_long(reply, nbytes2);
			if (nbytes2 > 0)
				put_binary(reply, buf, nbytes2);
			else if (nbytes2 < 0)
				put_int(reply, errno);
			mem_free(buf);
			break;

		case command_readlink:
			path = get_string(command);
			max = get_int(command);
			path1 = mem_alloc(max + 1);
			result = readlink(path, path1, max);
			put_int(reply, result);
			if (result < 0)
				put_int(reply, errno);
			else
				put_binary(reply, path1, result);
			mem_free(path1);
			break;

		case command_read_whole_dir:
			path = get_string(command);
			result = read_whole_dir(path, &buf, &nbytes);
			if (result < 0)
				put_int(reply, errno);
			else
			{
				put_int(reply, 0);
				put_long(reply, nbytes);
				put_binary(reply, buf, nbytes);
			}
			break;

		case command_rmdir:
			path = get_string(command);
			if (rmdir(path))
				result = errno;
			else
				result = 0;
			put_int(reply, result);
			break;

		case command_stat:
			path = get_string(command);
			result = stat(path, &st);
			if (result)
				put_int(reply, errno);
			else
			{
				put_int(reply, 0);
				put_binary(reply, &st, sizeof(st));
			}
			break;

		case command_unlink:
			path = get_string(command);
			result = unlink(path);
			if (result)
				result = errno;
			put_int(reply, result);
			break;

		case command_utime:
			path = get_string(command);
			get_binary(command, &utb, sizeof(utb));
			result = utime(path, &utb);
			if (result)
				result = errno;
			put_int(reply, result);
			break;

		case command_write:
			fd = get_int(command);
			nbytes = get_long(command);
			buf = mem_alloc(nbytes);
			get_binary(command, buf, nbytes);
			nbytes2 = write(fd, buf, nbytes);
			put_long(reply, nbytes2);
			if (nbytes2 < 0)
				put_int(reply, errno);
			mem_free(buf);
			break;
		}
		fflush(reply);
		if (ferror(reply))
			exit(errno);
	}
	trace((/*{*/"}\n"));
}


static void get_pipe _((int *, int *));

static void
get_pipe(rd, wr)
	int	*rd;
	int	*wr;
{
	int	fd[2];

	trace(("get_pipe()\n{\n"/*}*/));
	if (pipe(fd))
		nfatal("pipe");
	*rd = fd[0];
	*wr = fd[1];
	trace(("read %d\n", fd[0]));
	trace(("write %d\n", fd[1]));
	trace((/*{*/"}\n"));
}


static  void proxy_close _((void));

static void
proxy_close()
{
	proxy_ty	*p;
	int		j;

	trace(("proxy_close()\n{\n"/*}*/));
	trace(("pid = %d;\n", getpid()));
	for (j = 0; j < SIZEOF(proxy_table); ++j)
	{
		for (;;)
		{
			p = proxy_table[j];
			if (!p)
				break;
			trace(("p->pid %d; uid %d; gid %d\n", p->pid, p->uid, p->gid));
			proxy_table[j] = p->next;
			fclose(p->command);
			fclose(p->reply);
			mem_free((char *)p);
		}
	}
	trace((/*{*/"}\n"));
}


static void proxy_spawn _((proxy_ty *));

static void
proxy_spawn(pp)
	proxy_ty	*pp;
{
	int		command_read_fd;
	int		command_write_fd;
	int		reply_read_fd;
	int		reply_write_fd;
	int		pid;
	static int	quitregd;

	trace(("proxy_spawn()\n{\n"/*}*/));
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
		/*
		 * close the ends of the pipes the proxy will not be using
		 */
		close(command_write_fd);
		close(reply_read_fd);
		error_set_id_func((error_id_ty)0);

		/*
		 * the proxy now assumes the appropriate ID
		 */
		if (setgid(pp->gid))
			exit(errno);
		if (setuid(pp->uid))
			exit(errno);
		umask(pp->umask);

		/*
		 * close all of the master ends of all the other proxys
		 * otherwise they will keep each other alive
		 * after the master dies
		 */
		trace_indent_reset();
		proxy_close();

		/*
		 * normally the proxys are silent,
		 * returning all errors to the master.
		 * Should one of the error functions be called,
		 * make sure the proxy does not perform the undo.
		 */
		undo_cancel();

		/*
		 * do whatever is asked
		 */
		proxy(command_read_fd, reply_write_fd);
		exit(0);

	default:
		/*
		 * close the ends of the pipes the master will not be using
		 */
		close(command_read_fd);
		close(reply_write_fd);

		/*
		 * remember who the child is
		 * (even though we don't have a use for it at the moment)
		 */
		pp->pid = pid;
		trace(("child pid %d\n", getpid()));

		/*
		 * open a buffered stream for commands
		 */
		errno = 0;
		pp->command = fdopen(command_write_fd, "w");
		if (!pp->command)
		{
			if (!errno)
				errno = ENOMEM;
			nfatal("fdopen");
		}

		/*
		 * open a buffered stream for replies
		 */
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
	trace((/*{*/"}\n"));
}


static proxy_ty *proxy_find _((void));

static proxy_ty *
proxy_find()
{
	int		uid;
	int		gid;
	int		um;
	int		hash;
	int		pix;
	proxy_ty	*pp;

	/*
	 * search for an existing proxy
	 */
	trace(("proxy_find()\n{\n"/*}*/));
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

	/*
	 * no such proxy, so create a new one
	 */
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

	/*
	 * glue into the table AFTER the spawn,
	 * so the child doesn't close the wrong things.
	 */
	pp->next = proxy_table[pix];
	proxy_table[pix] = pp;

	/*
	 * here for all exits
	 */
	done:
	trace(("return %08lX;\n", (long)pp));
	trace((/*{*/"}\n"));
	return pp;
}


static void end_of_command _((proxy_ty *));

static void
end_of_command(pp)
	proxy_ty	*pp;
{
	trace(("end_of_command()\n{\n"/*}*/));
	if (fflush(pp->command))
		nfatal("write pipe");
	trace((/*{*/"}\n"));
}


int
glue_stat(path, st)
	char		*path;
	struct stat	*st;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_stat()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_stat, pp->command);
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
	trace((/*{*/"}\n"));
	return result;
}


int
glue_lstat(path, st)
	char		*path;
	struct stat	*st;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_lstat()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_lstat, pp->command);
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
	trace((/*{*/"}\n"));
	return result;
}


int
glue_mkdir(path, mode)
	char		*path;
	int		mode;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_mkdir()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_mkdir, pp->command);
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
	trace((/*{*/"}\n"));
	return result;
}


int
glue_chown(path, uid, gid)
	char		*path;
	int		uid;
	int		gid;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_chown()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_chown, pp->command);
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
	trace((/*{*/"}\n"));
	return result;
}


int
glue_catfile(path)
	char		*path;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_catfile()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_catfile, pp->command);
	put_string(pp->command, path);
	end_of_command(pp);
	result = get_int(pp->reply);
	if (result)
	{
		errno = result;
		result = -1;
	}
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace((/*{*/"}\n"));
	return result;
}


int
glue_chmod(path, mode)
	char		*path;
	int		mode;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_chmod()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_chmod, pp->command);
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
	trace((/*{*/"}\n"));
	return result;
}


int
glue_rmdir(path)
	char		*path;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_rmdir()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_rmdir, pp->command);
	put_string(pp->command, path);
	end_of_command(pp);
	result = get_int(pp->reply);
	if (result)
	{
		errno = result;
		result = -1;
	}
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace((/*{*/"}\n"));
	return result;
}


int
glue_rename(p1, p2)
	char		*p1;
	char		*p2;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_rename()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_rename, pp->command);
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
	trace((/*{*/"}\n"));
	return result;
}


int
glue_unlink(path)
	char		*path;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_unlink()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_unlink, pp->command);
	put_string(pp->command, path);
	end_of_command(pp);
	result = get_int(pp->reply);
	if (result)
	{
		errno = result;
		result = -1;
	}
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace((/*{*/"}\n"));
	return result;
}


int
glue_link(p1, p2)
	char		*p1;
	char		*p2;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_link()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_link, pp->command);
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
	trace((/*{*/"}\n"));
	return result;
}


int
glue_access(path, mode)
	char		*path;
	int		mode;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_access()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_access, pp->command);
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
	trace((/*{*/"}\n"));
	return result;
}


char *
glue_getcwd(buf, max)
	char		*buf;
	int		max;
{
	proxy_ty	*pp;
	char		*s;
	int		result;

	/*
	 * don't bother with the case where buf
	 * is the NULL pointer, aegis never uses it.
	 */
	trace(("glue_getcwd()\n{\n"/*}*/));
	assert(buf);
	pp = proxy_find();
	putc(command_getcwd, pp->command);
	put_int(pp->command, max);
	end_of_command(pp);
	result = get_int(pp->reply);
	if (result)
	{
		s = 0;
		trace(("return NULL; /* errno = %d */\n", result));
		errno = result;
	}
	else
	{
		s = get_string(pp->reply);
		strcpy(buf, s);
		s = buf;
		trace(("return \"%s\";\n", s));
	}
	trace((/*{*/"}\n"));
	return s;
}


int
glue_readlink(path, buf, max)
	char		*path;
	char		*buf;
	int		max;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_readlink()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_readlink, pp->command);
	put_string(pp->command, path);
	put_int(pp->command, max);
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
	trace((/*{*/"}\n"));
	return result;
}


int
glue_utime(path, values)
	char		*path;
	struct utimbuf	*values;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_utime()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_utime, pp->command);
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
	trace((/*{*/"}\n"));
	return result;
}


int
glue_copyfile(p1, p2)
	char		*p1;
	char		*p2;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_copyfile()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_copyfile, pp->command);
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
	trace((/*{*/"}\n"));
	return result;
}


typedef struct glue_file_ty glue_file_ty;
struct glue_file_ty
{
	long		guard1;
	char		*path;
	int		fmode;
	char		*buffer_end;
	char		*buffer_pos;
	int		fd;
	int		errno;
	int		pushback;
	proxy_ty	*pp;
	char		buffer[1 << 11]; /* fits within knl pipe buf */
	long		guard2;
};


FILE *
glue_fopen(path, mode)
	char		*path;
	char		*mode;
{
	glue_file_ty	*gfp;
	proxy_ty	*pp;
	int		fmode;
	int		fd;

	trace(("glue_fopen()\n{\n"/*}*/));
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
	putc(command_open, pp->command);
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

	/*
	 * build our file structure
	 */
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
	gfp->errno = 0;
	gfp->pushback = EOF;

	/*
	 * NOTE: this is veeerrry nasty.
	 * The return value is not really a valid FILE,
	 * but is only useful to the glue file functions.
	 */
	done:
	trace(("return %08lX; /* errno = %d */\n", (long)gfp, errno));
	trace((/*{*/"}\n"));
	return (FILE *)gfp;
}


int
glue_fclose(fp)
	FILE		*fp;
{
	proxy_ty	*pp;
	glue_file_ty	*gfp;
	int		result;
	int		result2;

	/*
	 * Leave the standard file streams alone.
	 * There is a chance these will be seen here.
	 */
	if (fp == stdout || fp == stdin || fp == stderr)
		return fclose(fp);

	/*
	 * flush the buffers
	 */
	trace(("glue_fclose()\n{\n"/*}*/));
	result = glue_fflush(fp) ? errno : 0;

	/*
	 * locate the appropriate proxy
	 */
	gfp = (glue_file_ty *)fp;
	assert(gfp->guard1 == GUARD1);
	assert(gfp->guard2 == GUARD2);
	pp = gfp->pp;

	/*
	 * tell the proxy to close
	 */
	putc(command_close, pp->command);
	put_int(pp->command, gfp->fd);
	end_of_command(pp);
	result2 = get_int(pp->reply);
	if (!result)
		result = result2;

	/*
	 * Fclose always closes the file,
	 * even when the implicit write fails.
	 * Always dispose of our data.
	 */
	mem_free(gfp->path);
	mem_free((char *)gfp);
	
	/*
	 * set errno and get out of here
	 */
	if (result)
	{
		errno = result;
		result = -1;
	}
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace((/*{*/"}\n"));
	return result;
}


int
glue_fgetc(fp)
	FILE		*fp;
{
	proxy_ty	*pp;
	glue_file_ty	*gfp;
	int		result;
	long		nbytes;

	/*
	 * Leave the standard file streams alone.
	 * There is a chance these will be seen here.
	 */
	if (fp == stdout || fp == stdin || fp == stderr)
		return getc(fp);

	/*
	 * locate the appropriate proxy
	 */
#if 0
	trace(("glue_fgetc()\n{\n"/*}*/));
#endif
	gfp = (glue_file_ty *)fp;
	assert(gfp->guard1 == GUARD1);
	assert(gfp->guard2 == GUARD2);
	pp = gfp->pp;

	/*
	 * complain if we are in an error state,
	 * or they asked for something stupid
	 */
	if (gfp->errno)
	{
		errno = gfp->errno;
		result = EOF;
		goto done;
	}
	if (gfp->fmode != O_RDONLY)
	{
		gfp->errno = EINVAL;
		errno = EINVAL;
		result = EOF;
		goto done;
	}

	/*
	 * use pushback if there is anything in it
	 */
	if (gfp->pushback != EOF)
	{
		result = gfp->pushback;
		gfp->pushback = EOF;
		goto done;
	}

	/*
	 * use the buffer if there is anything in it
	 */
	if (gfp->buffer_pos && gfp->buffer_pos < gfp->buffer_end)
	{
		result = (unsigned char)*gfp->buffer_pos++;
		goto done;
	}
	gfp->buffer_pos = 0;
	gfp->buffer_end = 0;

	/*
	 * tell the proxy to read another buffer-full
	 */
	pp = gfp->pp;
	putc(command_read, pp->command);
	put_int(pp->command, gfp->fd);
	put_long(pp->command, (long)sizeof(gfp->buffer));
	end_of_command(pp);
	nbytes = get_long(pp->reply);
	if (nbytes < 0)
	{
		gfp->errno = get_int(pp->reply);
		errno = gfp->errno;
		result = EOF;
		goto done;
	}
	if (nbytes == 0)
	{
		errno = 0;
		result = EOF;
		goto done;
	}
	assert(nbytes <= sizeof(gfp->buffer));
	get_binary(pp->reply, gfp->buffer, nbytes);
	gfp->buffer_pos = gfp->buffer;
	gfp->buffer_end = gfp->buffer + nbytes;
	result = (unsigned char)*gfp->buffer_pos++;

	/*
	 * here for all exits
	 */
	done:
#if 0
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace((/*{*/"}\n"));
#endif
	return result;
}


int
glue_ungetc(c, fp)
	int		c;
	FILE		*fp;
{
	glue_file_ty	*gfp;
	int		result;

	/*
	 * Leave the standard file streams alone.
	 * There is a chance these will be seen here.
	 */
	if (fp == stdout || fp == stdin || fp == stderr)
		return ungetc(c, fp);

	/*
	 * make a pointer to our data
	 */
	trace(("glue_ungetc()\n{\n"/*}*/));
	result = EOF;
	gfp = (glue_file_ty *)fp;
	assert(gfp->guard1 == GUARD1);
	assert(gfp->guard2 == GUARD2);

	/*
	 * make sure not too many
	 */
	if (gfp->pushback != EOF || c == EOF)
	{
		gfp->errno = EINVAL;
		errno = EINVAL;
		gfp->pushback = EOF;
		goto done;
	}

	/*
	 * stash the returned char
	 */
	gfp->pushback = (unsigned char)c;
	result = (unsigned char)c;

	/*
	 * here for all exits
	 */
	done:
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace((/*{*/"}\n"));
	return result;
}


int
glue_fputc(c, fp)
	int		c;
	FILE		*fp;
{
	proxy_ty	*pp;
	glue_file_ty	*gfp;
	int		result;

	/*
	 * Leave the standard file streams alone.
	 * There is a chance these will be seen here.
	 */
	if (fp == stdout || fp == stdin || fp == stderr)
		return putc(c, fp);

	/*
	 * locate the appropriate proxy
	 */
#if 0
	trace(("glue_fputc()\n{\n"/*}*/));
#endif
	result = EOF;
	gfp = (glue_file_ty *)fp;
	assert(gfp->guard1 == GUARD1);
	assert(gfp->guard2 == GUARD2);
	pp = gfp->pp;

	/*
	 * if the stream is in an error state,
	 * or we were asked to do something stupid,
	 * return the error
	 */
	if (gfp->errno)
	{
		errno = gfp->errno;
		goto done;
	}
	if (gfp->fmode == O_RDONLY)
	{
		gfp->errno = EINVAL;
		errno = EINVAL;
		goto done;
	}

	/*
	 * if there is no room in the buffer,
	 * flush it to the proxy
	 */
	assert(gfp->buffer_pos);
	if (gfp->buffer_pos >= gfp->buffer_end)
	{
		long	nbytes;
		long	nbytes2;

		putc(command_write, pp->command);
		put_int(pp->command, gfp->fd);
		nbytes = gfp->buffer_pos - gfp->buffer;
		put_long(pp->command, nbytes);
		put_binary(pp->command, gfp->buffer, nbytes);
		end_of_command(pp);
		nbytes2 = get_long(pp->reply);
		if (nbytes2 < 0)
		{
			gfp->errno = get_int(pp->reply);
			errno = gfp->errno;
			goto done;
		}
		if (nbytes2 != nbytes)
		{
			gfp->errno = EIO;
			errno = EIO;
			goto done;
		}
		gfp->buffer_pos = gfp->buffer;
		gfp->buffer_end = gfp->buffer + sizeof(gfp->buffer);
	}

	/*
	 * stash the character
	 */
	*gfp->buffer_pos++ = c;
	result = (unsigned char)c;

	/*
	 * here for all exits
	 */
	done:
#if 0
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace((/*{*/"}\n"));
#endif
	return result;
}


int
glue_ferror(fp)
	FILE		*fp;
{
	glue_file_ty	*gfp;

	/*
	 * Leave the standard file streams alone.
	 * There is a chance these will be seen here.
	 */
	if (fp == stdout || fp == stdin || fp == stderr)
		return ferror(fp);

	/*
	 * locate the appropriate proxy
	 */
	gfp = (glue_file_ty *)fp;
	assert(gfp->guard1 == GUARD1);
	assert(gfp->guard2 == GUARD2);

	/*
	 * set errno depending on
	 * the error for this stream.
	 */
	if (gfp->errno)
	{
		errno = gfp->errno;
		gfp->errno = 0;
		return 1;
	}
	return 0;
}


int
glue_fflush(fp)
	FILE		*fp;
{
	glue_file_ty	*gfp;
	proxy_ty	*pp;
	int		result;

	/*
	 * Leave the standard file streams alone.
	 * There is a chance these will be seen here.
	 */
	if (fp == stdout || fp == stdin || fp == stderr)
		return fflush(fp);

	/*
	 * locate the appropriate proxy
	 */
	trace(("glue_fflush()\n{\n"/*}*/));
	result = EOF;
	gfp = (glue_file_ty *)fp;
	assert(gfp->guard1 == GUARD1);
	assert(gfp->guard2 == GUARD2);
	pp = gfp->pp;

	/*
	 * if the stream is in an error state,
	 * don't do anything
	 */
	if (gfp->errno)
	{
		errno = gfp->errno;
		goto done;
	}
	if (gfp->fmode == O_RDONLY)
	{
		gfp->errno = EINVAL;
		errno = EINVAL;
		goto done;
	}

	/*
	 * if there is anything in the buffer,
	 * send it to the proxy
	 */
	if (gfp->buffer_pos && gfp->buffer_pos > gfp->buffer)
	{
		long	nbytes;
		long	nbytes2;

		putc(command_write, pp->command);
		put_int(pp->command, gfp->fd);
		nbytes = gfp->buffer_pos - gfp->buffer;
		put_long(pp->command, nbytes);
		put_binary(pp->command, gfp->buffer, nbytes);
		end_of_command(pp);
		nbytes2 = get_long(pp->reply);
		if (nbytes2 < 0)
		{
			gfp->errno = get_int(pp->reply);
			errno = gfp->errno;
			goto done;
		}
		if (nbytes2 != nbytes)
		{
			gfp->errno = EIO;
			errno = EIO;
			goto done;
		}
		gfp->buffer_pos = gfp->buffer;
		gfp->buffer_end = gfp->buffer + sizeof(gfp->buffer);
	}
	result = 0;

	/*
	 * here for all exits
	 */
	done:
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace((/*{*/"}\n"));
	return result;
}


int
glue_open(path, mode, perm)
	char		*path;
	int		mode;
	int		perm;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_open()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_open, pp->command);
	put_string(pp->command, path);
	put_int(pp->command, mode);
	put_int(pp->command, perm);
	end_of_command(pp);
	result = get_int(pp->reply);
	if (result < 0)
		errno = get_int(pp->reply);
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace((/*{*/"}\n"));
	return result;
}


int
glue_creat(path, mode)
	char		*path;
	int		mode;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_creat()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_creat, pp->command);
	put_string(pp->command, path);
	put_int(pp->command, mode);
	end_of_command(pp);
	result = get_int(pp->reply);
	if (result < 0)
		errno = get_int(pp->reply);
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace((/*{*/"}\n"));
	return result;
}


int
glue_close(fd)
	int		fd;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_close()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_close, pp->command);
	put_int(pp->command, fd);
	end_of_command(pp);
	result = get_int(pp->reply);
	if (result)
	{
		errno = result;
		result = -1;
	}
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace((/*{*/"}\n"));
	return result;
}


int
glue_write(fd, buf, len)
	int		fd;
	char		*buf;
	long		len;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_write()\n{\n"/*}*/));
	pp = proxy_find();
	putc(command_write, pp->command);
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
	trace((/*{*/"}\n"));
	return result;
}


int
glue_fcntl(fd, cmd, data)
	int		fd;
	int		cmd;
	struct flock	*data;
{
	proxy_ty	*pp;
	int		result;

	trace(("glue_fcntl()\n{\n"/*}*/));
	assert(cmd == F_SETLKW || cmd == F_SETLK || cmd == F_UNLCK ||
		cmd == F_GETLK);
	pp = proxy_find();
	putc(command_fcntl, pp->command);
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
	trace((/*{*/"}\n"));
	return result;
}


int
glue_read_whole_dir(path, data_p, data_len_p)
	char		*path;
	char		**data_p;
	long		*data_len_p;
{
	static char	*data;
	static long	data_max;
	long		data_len;
	proxy_ty	*pp;
	int		result;

	trace(("glue_read_whole_dir(path = \"%s\")\n{\n"/*}*/, path));
	pp = proxy_find();
	putc(command_read_whole_dir, pp->command);
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
			data_max = data_len;
			if (!data)
				data = mem_alloc(data_max);
			else
				mem_change_size(&data, data_max);
		}
		get_binary(pp->reply, data, data_len);
		*data_len_p = data_len;
		*data_p = data;
	}
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace((/*{*/"}\n"));
	return result;
}


#endif /* CONF_NO_seteuid */


/*
 * NAME
 *	copyfile - copy a file
 *
 * SYNOPSIS
 *	int copyfile(char *src, char *dst);
 *
 * DESCRIPTION
 *	The copyfile function complements the link and rename functions.
 *
 * ARGUMENTS
 *	src	- pathname of source file
 *	dst	- pathname of destination file
 *
 * RETURNS
 *	0	on success
 *	-1	on error, setting errno appropriately
 */

int
copyfile(src, dst)
	char	*src;
	char	*dst;
{
	int	src_fd;
	int	dst_fd;
	char	*buffer;
	long	max;
	long	nbytes;
	long	nbytes2;
	int	err;
	int	result;

	trace(("copyfile(\"%s\", \"%s\")\n{\n"/*}*/, src, dst));
	result = -1;
	src_fd = open(src, O_RDONLY, 0666);
	if (src_fd < 0)
		goto done;
	dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (dst_fd < 0)
	{
		err = errno;
		close(src_fd);
		errno = err;
		goto done;
	}

	max = 1L << 13;
	errno = 0;
	buffer = malloc(max);
	if (!buffer)
	{
		err = errno ? errno : ENOMEM;
		close(dst_fd);
		close(src_fd);
		errno = err;
		goto done;
	}

	for (;;)
	{
		nbytes = read(src_fd, buffer, max);
		if (nbytes < 0)
		{
			err = errno;
			close(src_fd);
			close(dst_fd);
			free(buffer);
			errno = err;
			goto done;
		}
		if (nbytes == 0)
			break;
		
		nbytes2 = write(dst_fd, buffer, nbytes);
		if (nbytes2 < 0)
		{
			err = errno;
			close(src_fd);
			close(dst_fd);
			free(buffer);
			errno = err;
			goto done;
		}
		if (nbytes2 != nbytes)
		{
			close(src_fd);
			close(dst_fd);
			free(buffer);
			errno = EIO; /* weird device, probably */
			goto done;
		}
	}
	free(buffer);
	if (close(src_fd))
	{
		err = errno;
		close(dst_fd);
		errno = err;
		goto done;
	}
	result = close(dst_fd);

	/*
	 * here for all exits
	 */
	done:
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	catfile - copy a file
 *
 * SYNOPSIS
 *	int catfile(char *path);
 *
 * DESCRIPTION
 *	The catfile function is used to print the contents of
 *	a file on the standard output.
 *
 * ARGUMENTS
 *	path	- pathname of source file
 *
 * RETURNS
 *	0	on success
 *	-1	on error, setting errno appropriately
 */

int
catfile(path)
	char	*path;
{
	int	fd;
	char	*buffer;
	long	max;
	long	nbytes;
	long	nbytes2;
	int	err;
	int	result;

	trace(("catfile(\"%s\")\n{\n"/*}*/, path));
	result = -1;
	fd = open(path, O_RDONLY, 0666);
	if (fd < 0)
		goto done;

	max = 1L << 13;
	errno = 0;
	buffer = malloc(max);
	if (!buffer)
	{
		err = errno ? errno : ENOMEM;
		close(fd);
		errno = err;
		goto done;
	}

	for (;;)
	{
		nbytes = read(fd, buffer, max);
		if (nbytes < 0)
		{
			err = errno;
			close(fd);
			free(buffer);
			errno = err;
			goto done;
		}
		if (nbytes == 0)
			break;
		
		nbytes2 = write(fileno(stdout), buffer, nbytes);
		if (nbytes2 < 0)
		{
			err = errno;
			close(fd);
			free(buffer);
			errno = err;
			goto done;
		}
		if (nbytes2 != nbytes)
		{
			close(fd);
			free(buffer);
			errno = EIO; /* weird device, probably */
			goto done;
		}
	}
	free(buffer);
	result = close(fd);

	/*
	 * here for all exits
	 */
	done:
	trace(("return %d; /* errno = %d */\n", result, errno));
	trace((/*{*/"}\n"));
	return result;
}


int
read_whole_dir(path, data_p, data_len_p)
	char		*path;
	char		**data_p;
	long		*data_len_p;
{
	DIR		*dp;
	struct dirent	*de;
	static char	*data;
	static size_t	data_len;
	static size_t	data_max;
	char		*np;
	size_t		len;

	errno = ENOMEM;
	dp = opendir(path);
	if (!dp)
		return -1;
	errno = 0;
	if (!data)
	{
		data_max = 1000;
		data = mem_alloc(data_max);
	}
	data_len = 0;
	for (;;)
	{
		de = readdir(dp);
		if (!de)
			break;
		np = de->d_name;
#ifdef CONF_pyramid_broken_readdir
		np -= 2;
#endif
		if (np[0] == '.' && (!np[1] || (np[1] == '.' && !np[2])))
			continue;
		len = strlen(np) + 1;
		if (data_len + len > data_max)
		{
			data_max += 1000;
			mem_change_size(&data, data_max);
		}
		memcpy(data + data_len, np, len);
		data_len += len;
	}
	closedir(dp);
	*data_p = data;
	*data_len_p = data_len;
	return 0;
}
