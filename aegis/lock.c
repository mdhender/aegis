/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 * MANIFEST: functions to lock aegis' data
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <error.h>
#include <glue.h>
#include <gonzo.h>
#include <lock.h>
#include <mem.h>
#include <os.h>
#include <trace.h>


/*
 * how many bits of the hash to use in the mux
 */
#define BITS 16
#define BITS_SIZE (1L << BITS)
#define BITS_MASK (BITS_SIZE - 1)

enum lock_ty
{
	lock_master,
	lock_gstate,
	lock_MAX
};
typedef enum lock_ty lock_ty;

enum lock_mux_ty
{
	lock_mux_ustate,
	lock_mux_pstate,
	lock_mux_cstate,
	lock_mux_build,
	lock_mux_MAX
};
typedef enum lock_mux_ty lock_mux_ty;


static	string_ty	*path;
static	size_t		nplaces;
static	struct flock	*place;
static	int		fd = -1;
static	long		magic;
static	int	 	quitregd;


static void flock_construct _((struct flock *, int type, long start,
	long length));

static void
flock_construct(p, type, start, length)
	struct flock	*p;
	int		type;
	long		start;
	long		length;
{
	/*
	 * the memset is here because some systems have
	 * extra fields in the flock structure.
	 * These fields are frequently undocumented,
	 * and yet these systems give "EINVAL" if the
	 * undocumented fields are not zero!
	 */
	memset(p, 0, sizeof(*p));

	assert(start >= 0);
	assert(length > 0);
	p->l_type = type;
	p->l_whence = SEEK_SET;
	p->l_start = start;
	p->l_len = length;
	p->l_pid = 0;
}


static void lock_prepare _((long, long, int));

static void
lock_prepare(start, length, exclusive)
	long		start;
	long		length;
	int		exclusive;
{
	struct flock	p;

	trace(("lock_prepare(start = %ld, length = %ld, excl = %d)\n{\n"/*}*/,
		start, length, exclusive));
	assert(start > lock_master);
	assert(length > 0);
	flock_construct(&p, (exclusive ? F_WRLCK : F_RDLCK), start, length);
	*(struct flock *)
	enlarge(&nplaces, (char **)&place, sizeof(struct flock)) =
		p;
	trace((/*{*/"}\n"));
}


void
lock_prepare_gstate()
{
	trace(("lock_prepare_gstate()\n{\n"/*}*/));
	lock_prepare((long)lock_gstate, 1L, 1);
	trace((/*{*/"}\n"));
}


static void lock_prepare_mux _((lock_mux_ty, long, int));

static void
lock_prepare_mux(lock_mux, n, exclusive)
	lock_mux_ty	lock_mux;
	long		n;
	int		exclusive;
{
	trace(("lock_prepare_mux(lock_mux = %d, n = %ld, excl = %d)\n{\n"/*}*/,
		lock_mux, n, exclusive));
	lock_prepare
	(
		((lock_mux + 1L) << BITS) + (n & BITS_MASK),
		1L,
		exclusive
	);
	trace((/*{*/"}\n"));
}


static void lock_prepare_mux_all _((lock_mux_ty, int));

static void
lock_prepare_mux_all(lock_mux, exclusive)
	lock_mux_ty	lock_mux;
	int		exclusive;
{
	trace(("lock_prepare_mux_all(lock_mux = %d)\n{\n"/*}*/, lock_mux));
	lock_prepare(((lock_mux + 1L) << BITS), BITS_SIZE, exclusive);
	trace((/*{*/"}\n"));
}


void
lock_prepare_pstate(s)
	string_ty	*s;
{
	trace(("lock_prepare_pstate(s = \"%s\")\n{\n"/*}*/, s->str_text));
	lock_prepare_mux(lock_mux_pstate, (long)s->str_hash, 1);
	trace((/*{*/"}\n"));
}


void
lock_prepare_build_read(s)
	string_ty	*s;
{
	trace(("lock_prepare_pstate(s = \"%s\")\n{\n"/*}*/, s->str_text));
	lock_prepare_mux(lock_mux_build, (long)s->str_hash, 0);
	trace((/*{*/"}\n"));
}


void
lock_prepare_build_write(s)
	string_ty	*s;
{
	trace(("lock_prepare_pstate(s = \"%s\")\n{\n"/*}*/, s->str_text));
	lock_prepare_mux(lock_mux_build, (long)s->str_hash, 1);
	trace((/*{*/"}\n"));
}


void
lock_prepare_ustate(uid)
	int		uid;
{
	trace(("lock_prepare_ustate()\n{\n"/*}*/));
	lock_prepare_mux(lock_mux_ustate, (long)uid, 1);
	trace((/*{*/"}\n"));
}


void
lock_prepare_ustate_all()
{
	trace(("lock_prepare_ustate_all()\n{\n"/*}*/));
	lock_prepare_mux_all(lock_mux_ustate, 1);
	trace((/*{*/"}\n"));
}


void
lock_prepare_cstate(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	trace(("lock_prepare_cstate(project_name = \"%s\", change_number = \
%ld)\n{\n"/*}*/, project_name->str_text, change_number));
	lock_prepare_mux
	(
		lock_mux_cstate,
		change_number + project_name->str_hash,
		1
	);
	trace((/*{*/"}\n"));
}


static char *flock_type_string _((int));

static char *
flock_type_string(n)
	int		n;
{
	switch (n)
	{
	case F_RDLCK:
		return "F_RDLCK";

	case F_WRLCK:
		return "F_WRLCK";

	case F_UNLCK:
		return "F_UNLCK";
	}
	return "unknown";
}


static char *flock_whence_string _((int));

static char *
flock_whence_string(n)
	int		n;
{
	switch (n)
	{
	case SEEK_SET:
		return "SEEK_SET";

	case SEEK_CUR:
		return "SEEK_CUR";

	case SEEK_END:
		return "SEEK_END";
	}
	return "unknown";
}


static char *flock_string _((struct flock *));

static char *
flock_string(p)
	struct flock	*p;
{
	static char	buffer[120];

	sprintf
	(
		buffer,
		"&{type = %s, whence = %s, start = %ld, len = %ld, pid = %d}",
		flock_type_string(p->l_type),
		flock_whence_string(p->l_whence),
		p->l_start,
		p->l_len,
		p->l_pid
	);
	return buffer;
}


static char *lock_description _((struct flock *));

static char *
lock_description(p)
	struct flock	*p;
{
	switch (p->l_start)
	{
	case lock_master:
		return "master";

	case lock_gstate:
		return "global state";
	}
	switch ((p->l_start >> BITS) - 1)
	{
	case lock_mux_ustate:
		return "user";

	case lock_mux_cstate:
		return "change";

	case lock_mux_pstate:
		return "project";

	case lock_mux_build:
		return "build";
	}
	return "unknown";
}


static void quitter _((int));

static void
quitter(n)
	int	n;
{
	if (fd >= 0)
	{
		while (os_become_active())
			os_become_undo();
		lock_release();
	}
}


void
lock_take()
{
	int		flags;
	struct flock	p;
	int		j, k;

	/*
	 * get the file descriptor of the lock file
	 *
	 * If it isn't there, create it.  If it is there, truncate it (prevent
	 * abuse of universally writable file).
	 */
	trace(("lock_take()\n{\n"/*}*/));
	assert(fd < 0);
	assert(nplaces);
	if (!quitregd)
	{
		quitregd = 1;
		quit_register(quitter);
	}
	if (!path)
		path = gonzo_lockpath_get();
	gonzo_become();
	fd = glue_open(path->str_text, O_RDWR | O_CREAT | O_TRUNC, 0600);
	if (fd < 0)
		nfatal("open(\"%s\")", path->str_text);
	trace_int(fd);

	/*
	 * make sure the file is closed when a child exec's
	 */
#ifndef CONF_NO_seteuid
	if (fcntl(fd, F_GETFD, &flags))
		nfatal("fcntl(\"%s\", F_GETFD)", path->str_text);
	flags |= 1;
	if (fcntl(fd, F_SETFD, flags))
		nfatal("fcntl(\"%s\", F_SETFD, %d)", path->str_text, flags);
	trace(("mark\n"));
#endif

	/*
	 * Block for the master lock.
	 * Take the locks we really want.
	 * If we got them, release master and return.
	 * If did not get all of the locks,
	 * release the locks we could get and then the master.
	 * Block on the lock we could not get,
	 * release that lock and start again.
	 *
	 * Unfortunately, this scheme isn't fair
	 * (i.e. no guarantee of success in finite time)
	 * it would be nice if I could atomically lock (and block)
	 * an entire vector of locks.  (vfcntl like berkeley's vwrite?)
	 */
	for (;;)
	{
		/*
		 * get the master lock
		 * (block if necessary,
		 * it should never be held for long)
		 */
		trace(("mark\n"));
		flock_construct(&p, F_WRLCK, (long)lock_master, 1L);
		if (glue_fcntl(fd, F_SETLKW, &p))
		{
			nfatal
			(
				"fcntl(\"%s\", F_SETLKW, %s)",
				path->str_text,
				flock_string(&p)
			);
		}

		/*
		 * get each of the locks we really wanted
		 */
		for (j = 0; j < nplaces; ++j)
		{
			trace(("mark\n"));
			p = place[j];
			if (glue_fcntl(fd, F_SETLK, &p))
			{
				if (errno != EACCES && errno != EAGAIN)
				{
					nfatal
					(
						"fcntl(\"%s\", F_SETLK, %s)",
						path->str_text,
						flock_string(&p)
					);
				}
				break;
			}
		}

		/*
		 * return if were successful.
		 */
		if (j >= nplaces)
		{
			trace(("mark\n"));
			flock_construct(&p, F_UNLCK, (long)lock_master, 1L);
			if (glue_fcntl(fd, F_SETLKW, &p))
			{
				nfatal
				(
					"fcntl(\"%s\", F_SETLKW, %s)",
					path->str_text,
					flock_string(&p)
				);
			}
			break;
		}

		/*
		 * give all the locks back
		 * that we got so far
		 */
		for (k = 0; k < j; ++k)
		{
			trace(("mark\n"));
			p = place[k];
			p.l_type = F_UNLCK;
			if (glue_fcntl(fd, F_SETLKW, &p))
			{
				nfatal
				(
					"fcntl(\"%s\", F_SETLKW, %s)",
					path->str_text,
					flock_string(&p)
				);
			}
		}

		/*
		 * release the master lock
		 */
		flock_construct(&p, F_UNLCK, (long)lock_master, 1L);
		if (glue_fcntl(fd, F_SETLKW, &p))
		{
			nfatal
			(
				"fcntl(\"%s\", F_SETLKW, %s)",
				path->str_text,
				flock_string(&p)
			);
		}
		trace(("mark\n"));

		/*
		 * verbose message about why we are blocking
		 */
		p = place[j];
		verbose("waiting for %s lock", lock_description(&p));

		/*
		 * block on the lock that stopped us before
		 */
		if (glue_fcntl(fd, F_SETLKW, &p))
		{
			nfatal
			(
				"fcntl(\"%s\", F_SETLKW, %s)",
				path->str_text,
				flock_string(&p)
			);
		}

		/*
		 * and then release it,
		 * before trying all over again.
		 */
		p = place[j];
		p.l_type = F_UNLCK;
		if (glue_fcntl(fd, F_SETLKW, &p))
		{
			nfatal
			(
				"fcntl(\"%s\", F_SETLKW, %s)",
				path->str_text,
				flock_string(&p)
			);
		}
		trace(("mark\n"));
	}
	gonzo_become_undo();
	magic++;
	trace((/*{*/"}\n"));
}


void
lock_release()
{
	struct flock	p;
	int		j;
	int		fildes;

	/*
	 * set the file descriptor to -1
	 * so that we will not be invoked again should
	 * a fatal error happen here.
	 */
	trace(("lock_release()\n{\n"/*}*/));
	trace_int(fd);
	assert(fd >= 0);
	assert(nplaces);
	assert(place);
	assert(path);
	gonzo_become();
	fildes = fd;
	fd = -1;

	/*
	 * Release each of the locks.
	 * This should be unnecessary, because we then close the file,
	 * but SunOS hangs onto some of them, even after the process dies.
	 */
	for (j = 0; j < nplaces; ++j)
	{
		p = place[j];
		p.l_type = F_UNLCK;
		if (glue_fcntl(fildes, F_SETLKW, &p))
		{
			nfatal
			(
				"fcntl(\"%s\", F_SETLKW, %s)",
				path->str_text,
				flock_string(&p)
			);
		}
	}

	/*
	 * close the file
	 * (this *should* have been enough)
	 */
	glue_close(fildes);
	gonzo_become_undo();
	nplaces = 0;
	mem_free((char *)place);
	place = 0;
	trace((/*{*/"}\n"));
}


long
lock_magic()
{
	return magic;
}


static void lock_walk_hunt _((long, long, lock_walk_callback));

static void
lock_walk_hunt(min, max, callback)
	long		min;
	long		max;
	lock_walk_callback callback;
{
	struct flock	flock;
	int		j;
	lock_walk_found	found;

	/*
	 * look for a lock in the given range
	 */
	flock.l_type = F_WRLCK;
	flock.l_whence = SEEK_SET;
	flock.l_start = min;
	flock.l_len = max - min;
	flock.l_pid = 0;
	gonzo_become();
	if (glue_fcntl(fd, F_GETLK, &flock))
		nfatal("getlock \"%s\"", path->str_text);
	gonzo_become_undo();
	if (flock.l_type == F_UNLCK)
		return;

	/*
	 * aegis only uses byte locks,
	 * so multi-byte ranges are separate locks
	 */
	for (j = 0; j < flock.l_len; ++j)
	{
		/*
		 * figure the name and address
		 */
		found.address = flock.l_start + j;
		found.subset = 0;
		switch (found.address)
		{
		case lock_master:
			found.name = lock_walk_name_master;
			break;

		case lock_gstate:
			found.name = lock_walk_name_gstate;
			break;

		default:
			switch ((found.address >> BITS) - 1)
			{
			case lock_mux_ustate:
				found.name = lock_walk_name_ustate;
				found.subset = found.address & BITS_MASK;
				break;

			case lock_mux_pstate:
				found.name = lock_walk_name_ustate;
				found.subset = found.address & BITS_MASK;
				break;

			case lock_mux_cstate:
				found.name = lock_walk_name_cstate;
				found.subset = found.address & BITS_MASK;
				break;

			case lock_mux_build:
				found.name = lock_walk_name_build;
				found.subset = found.address & BITS_MASK;
				break;

			default:
				found.name = lock_walk_name_unknown;
				break;
			}
		}

		/*
		 * figure the type
		 */
		switch (flock.l_type)
		{
		case F_RDLCK:
			found.type = lock_walk_type_shared;
			break;

		case F_WRLCK:
			found.type = lock_walk_type_exclusive;
			break;

		default:
			found.type = lock_walk_type_unknown;
			break;
		}

		/*
		 * Process holding the lock.
		 * Workout if it is local or remote.
		 */
		found.pid = flock.l_pid;
		if (kill(found.pid, 0))
		{
			switch (errno)
			{
			default:
				fatal("kill(%d, 0)", found.pid);

			case EPERM:
				found.pid_is_local = 1;
				break;

			case ESRCH:
				found.pid_is_local = 0;
				break;
			}
		}
		else
			found.pid_is_local = 1;

		/*
		 * do something with it
		 */
		callback(&found);
	}

	/*
	 * look for more locks on either side
	 */
	if (flock.l_start)
		lock_walk_hunt(min, flock.l_start, callback);
	if (flock.l_len)
		lock_walk_hunt(flock.l_start + flock.l_len, max, callback);
}


void
lock_walk(callback)
	lock_walk_callback callback;
{
	int	flags;
	int	fildes;

	trace(("lock_walk()\n{\n"/*}*/));
	assert(fd < 0);
	assert(!nplaces);
	if (!quitregd)
	{
		quitregd = 1;
		quit_register(quitter);
	}
	if (!path)
		path = gonzo_lockpath_get();
	gonzo_become();
	fd = glue_open(path->str_text, O_RDWR | O_CREAT | O_TRUNC, 0600);
	if (fd < 0)
		nfatal("open(\"%s\")", path->str_text);
	trace_int(fd);

	/*
	 * make sure the file is closed when a child exec's
	 */
#ifndef CONF_NO_seteuid
	if (fcntl(fd, F_GETFD, &flags))
		nfatal("fcntl(\"%s\", F_GETFD)", path->str_text);
	flags |= 1;
	if (fcntl(fd, F_SETFD, flags))
		nfatal("fcntl(\"%s\", F_SETFD, %d)", path->str_text, flags);
	trace(("mark\n"));
#endif
	gonzo_become_undo();

	/*
	 * chase all of the locks
	 */
	lock_walk_hunt(0L, ((lock_mux_MAX + 1) << BITS), callback);

	/*
	 * close the file
	 */
	gonzo_become();
	trace_int(fd);
	assert(fd >= 0);
	assert(path);
	fildes = fd;
	fd = -1;
	glue_close(fildes);
	gonzo_become_undo();
	trace((/*{*/"}\n"));
}
