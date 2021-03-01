/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1999, 2001 Peter Miller;
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
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to lock aegis' data
 */

#include <ac/errno.h>
#include <ac/fcntl.h>
#include <ac/stdio.h>
#include <ac/unistd.h>
#include <ac/string.h>
#include <signal.h>

#include <error.h>
#include <glue.h>
#include <gonzo.h>
#include <lock.h>
#include <mem.h>
#include <os.h>
#include <r250.h>
#include <sub.h>
#include <trace.h>
#include <user.h>

/*
 * Define this symbol if you want the lock poriority scheme, so
 * that integrate pass has a guarantee of succeeeding eventually.
 * (Without it, aeipass can be blocked indefinitely by rolling builds
 * by multiple developers.)
 */
#define LOCK_PRIORITY_SCHEME

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
	lock_mux_baseline_priority,
	lock_mux_ustate,
	lock_mux_pstate,
	lock_mux_cstate,
	lock_mux_baseline,
	lock_mux_history,
	lock_mux_MAX
};
typedef enum lock_mux_ty lock_mux_ty;


typedef struct lock_place_ty lock_place_ty;
struct lock_place_ty
{
	struct flock	location;
	int		release_immediately;
	lock_callback_ty callback;
	void		*callback_arg;
};

static	string_ty	*path;
static	size_t		nplaces;
static	size_t		nplaces_max;
static	lock_place_ty	*place;
static	int		fd = -1;
static	long		magic;
static	int	 	quitregd;

/*
 * Values for the lock_prepare::exclusive argument
 */
#define LOCK_PREP_SHARED    0
#define LOCK_PREP_EXCLUSIVE 1
#define LOCK_PREP_PRIORITY  2
#define LOCK_PREP_EXCL_PRIO (LOCK_PREP_EXCLUSIVE | LOCK_PREP_PRIORITY)


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


static int flock_equal _((struct flock *, struct flock *));

static int
flock_equal(p1, p2)
	struct flock	*p1;
	struct flock	*p2;
{
	return
	(
		p1->l_type == p2->l_type
	&&
		p1->l_start == p2->l_start
	&&
		p1->l_len == p2->l_len
	);
}


static void lock_prepare _((long, long, int, lock_callback_ty, void *));

static void
lock_prepare(start, length, exclusive, callback, arg)
	long		start;
	long		length;
	int		exclusive;
	lock_callback_ty callback;
	void		*arg;
{
	int		j;
	lock_place_ty	p;

	/*
	 * construct the lock structure
	 */
	trace(("lock_prepare(start = %ld, length = %ld, excl = %d)\n{\n"/*}*/,
		start, length, exclusive));
	assert(start > lock_master);
	assert(length > 0);
	flock_construct
	(
		&p.location,
		((exclusive & LOCK_PREP_EXCLUSIVE) ? F_WRLCK : F_RDLCK),
		start,
		length
	);
	p.release_immediately = (exclusive & LOCK_PREP_PRIORITY);
	p.callback = callback;
	p.callback_arg = arg;

	/*
	 * if we have already got this one, don't add it to the list.
	 *
	 * While most systems are tolerant of asking for a lock twice,
	 * some are not equally tolerant of releasing it more then once.
	 */
	for (j = 0; j < nplaces; ++j)
	{
		if (flock_equal(&p.location, &place[j].location))
		{
			trace((/*{*/"}\n"));
			return;
		}
	}

	/*
	 * Append the lock to the list of
	 * locks to be taken.
	 *
	 * The locks are taken in one go
	 * to avoid deadlocks.
	 */
	if (nplaces >= nplaces_max)
	{
		size_t		nbytes;

		nplaces_max = nplaces_max * 2 + 4;
		nbytes = nplaces_max * sizeof(p);
		place = mem_change_size(place, nbytes);
	}
	place[nplaces++] = p;
	trace((/*{*/"}\n"));
}


void
lock_prepare_gstate(callback, arg)
	lock_callback_ty callback;
	void		*arg;
{
	trace(("lock_prepare_gstate()\n{\n"/*}*/));
	lock_prepare((long)lock_gstate, 1L, 1, callback, arg);
	trace((/*{*/"}\n"));
}


static void lock_prepare_mux _((lock_mux_ty, long, int, lock_callback_ty,
	void *));

static void
lock_prepare_mux(lock_mux, n, exclusive, callback, arg)
	lock_mux_ty	lock_mux;
	long		n;
	int		exclusive;
	lock_callback_ty callback;
	void		*arg;
{
	trace(("lock_prepare_mux(lock_mux = %d, n = %ld, excl = %d)\n{\n"/*}*/,
		lock_mux, n, exclusive));
	lock_prepare
	(
		((lock_mux + 1L) << BITS) + (n & BITS_MASK),
		1L,
		exclusive,
		callback,
		arg
	);
	trace((/*{*/"}\n"));
}


static void lock_prepare_mux_all _((lock_mux_ty, int, lock_callback_ty,
	void *));

static void
lock_prepare_mux_all(lock_mux, exclusive, callback, arg)
	lock_mux_ty	lock_mux;
	int		exclusive;
	lock_callback_ty callback;
	void		*arg;
{
	trace(("lock_prepare_mux_all(lock_mux = %d)\n{\n"/*}*/, lock_mux));
	lock_prepare
	(
		((lock_mux + 1L) << BITS),
		BITS_SIZE,
		exclusive,
		callback,
		arg
	);
	trace((/*{*/"}\n"));
}


void
lock_prepare_pstate(s, callback, arg)
	string_ty	*s;
	lock_callback_ty callback;
	void		*arg;
{
	trace(("lock_prepare_pstate(s = \"%s\")\n{\n"/*}*/, s->str_text));
	lock_prepare_mux
	(
		lock_mux_pstate,
		(long)s->str_hash,
		LOCK_PREP_EXCLUSIVE,
		callback,
		arg
	);
	trace((/*{*/"}\n"));
}


void
lock_prepare_baseline_read(s, callback, arg)
	string_ty	*s;
	lock_callback_ty callback;
	void		*arg;
{
	trace(("lock_prepare_pstate(s = \"%s\")\n{\n"/*}*/, s->str_text));
#ifdef LOCK_PRIORITY_SCHEME
	lock_prepare_mux
	(
		lock_mux_baseline_priority,
		(long)s->str_hash,
		LOCK_PREP_EXCL_PRIO,
		callback,
		arg
	);
#endif
	lock_prepare_mux
	(
		lock_mux_baseline,
		(long)s->str_hash,
		LOCK_PREP_SHARED,
		callback,
		arg
	);
	trace((/*{*/"}\n"));
}


void
lock_prepare_baseline_write(s, callback, arg)
	string_ty	*s;
	lock_callback_ty callback;
	void		*arg;
{
	trace(("lock_prepare_pstate(s = \"%s\")\n{\n"/*}*/, s->str_text));
#ifdef LOCK_PRIORITY_SCHEME
	lock_prepare_mux
	(
		lock_mux_baseline_priority,
		(long)s->str_hash,
		LOCK_PREP_EXCLUSIVE,
		callback,
		arg
	);
#endif
	lock_prepare_mux
	(
		lock_mux_baseline,
		(long)s->str_hash,
		LOCK_PREP_EXCLUSIVE,
		callback,
		arg
	);
	trace((/*{*/"}\n"));
}


void
lock_prepare_history(s, callback, arg)
	string_ty	*s;
	lock_callback_ty callback;
	void		*arg;
{
	trace(("lock_prepare_pstate(s = \"%s\")\n{\n"/*}*/, s->str_text));
	lock_prepare_mux
	(
		lock_mux_history,
		(long)s->str_hash,
		LOCK_PREP_EXCLUSIVE,
		callback,
		arg
	);
	trace((/*{*/"}\n"));
}


void
lock_prepare_ustate(uid, callback, arg)
	int		uid;
	lock_callback_ty callback;
	void		*arg;
{
	trace(("lock_prepare_ustate()\n{\n"/*}*/));
	lock_prepare_mux
	(
		lock_mux_ustate,
		(long)uid,
		LOCK_PREP_EXCLUSIVE,
		callback,
		arg
	);
	trace((/*{*/"}\n"));
}


void
lock_prepare_ustate_all(callback, arg)
	lock_callback_ty callback;
	void		*arg;
{
	trace(("lock_prepare_ustate_all()\n{\n"/*}*/));
	lock_prepare_mux_all
	(
		lock_mux_ustate,
		LOCK_PREP_EXCLUSIVE,
		callback,
		arg
	);
	trace((/*{*/"}\n"));
}


void
lock_prepare_cstate(project_name, change_number, callback, arg)
	string_ty	*project_name;
	long		change_number;
	lock_callback_ty callback;
	void		*arg;
{
	trace(("lock_prepare_cstate(project_name = \"%s\", change_number = \
%ld)\n{\n"/*}*/, project_name->str_text, change_number));
	lock_prepare_mux
	(
		lock_mux_cstate,
		change_number + project_name->str_hash,
		LOCK_PREP_EXCLUSIVE,
		callback,
		arg
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
	case lock_mux_baseline_priority:
		return "baseline priority";

	case lock_mux_ustate:
		return "user";

	case lock_mux_cstate:
		return "change";

	case lock_mux_pstate:
		return "project";

	case lock_mux_baseline:
		return "baseline";

	case lock_mux_history:
		return "history";
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
	int		j;
	int		nsecs;
	int		wait_for_locks;

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

	/*
	 * See if we will wait for locks.
	 */
	wait_for_locks = user_lock_wait(0);

	gonzo_become();
#if defined(__CYGWIN__) || defined(__hpux__)
	/*
	 * There isn't a major security problem here, because the user
	 * can't park weird files here, and we truncate them anyway.
	 * Also, the directory is supposed to be read-only for just
	 * about everyone.
	 *
	 * CYGWIN is only really useful in single user, so that isn't
	 * an issue.
	 *
	 * The problem comes when you are using HP/UX over NFS.
	 * It gets errors like "no locks available" if the file isn't
	 * world writable.  I could understand if we were the wrong user
	 * id (many NFS implementations have kittens when you open file
	 * file as one user, and the read or write it as another user)
	 * but we are using the same user id as when we opened the file.
	 *
	 * But making the file world-writable means we have a potential
	 * denial of service problem.  A process could take a lock in
	 * the file, preventing all Aegis processes from obtaining locks
	 * they need.
	 *
	 * Unfortunately, the #ifdef __hpux__ probably isn't enough,
	 * because you need a world writable file if there are *any*
	 * HP/UX clients, and the file will probably be created by some
	 * other (inevitably, better) UNIX.
	 */
	fd = glue_open(path->str_text, O_RDWR | O_CREAT | O_TRUNC, 0666);
#else
	fd = glue_open(path->str_text, O_RDWR | O_CREAT | O_TRUNC, 0600);
#endif
	if (fd < 0)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set_string(scp, "File_Name", path);
		fatal_intl(scp, i18n("open $filename: $errno"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	trace_int(fd);

	/*
	 * make sure the file is closed when a child exec's
	 */
#ifndef CONF_NO_seteuid
	flags = 0;
	if (fcntl(fd, F_GETFD, &flags))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set_string(scp, "File_Name", path);
		fatal_intl(scp, i18n("fcntl(\"$filename\", F_GETFD): $errno"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	flags |= 1;
	if (fcntl(fd, F_SETFD, flags))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set_string(scp, "File_Name", path);
		sub_var_set_long(scp, "Argument", flags);
		fatal_intl(scp, i18n("fcntl(\"$filename\", F_SETFD, $arg): $errno"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
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
		 * Cope with interrupts while trying to obtain locks.
		 */
		os_interrupt_cope();

		/*
		 * get the master lock
		 * (block if necessary,
		 * it should never be held for long)
		 */
		trace(("mark\n"));
		flock_construct(&p, F_WRLCK, (long)lock_master, 1L);
		if (glue_fcntl(fd, F_SETLKW, &p))
		{
			sub_context_ty	*scp;

			if (errno == EINTR)
				os_interrupt_cope();
			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set_string(scp, "File_Name", path);
			sub_var_set_charstar(scp, "Argument", flock_string(&p));
			fatal_intl
			(
				scp,
			    i18n("fcntl(\"$filename\", F_SETLKW, $arg): $errno")
			);
			/* NOTREACHED */
			sub_context_delete(scp);
		}

		/*
		 * get each of the locks we really wanted
		 */
		for (j = 0; j < nplaces; ++j)
		{
			trace(("mark\n"));
			p = place[j].location; /* yes, copy it */
			if (glue_fcntl(fd, F_SETLK, &p))
			{
				if (errno != EACCES && errno != EAGAIN)
				{
					sub_context_ty	*scp;

					scp = sub_context_new();
					sub_errno_set(scp);
					sub_var_set_string(scp, "File_Name", path);
					sub_var_set_charstar(scp, "Argument", flock_string(&p));
					fatal_intl
					(
						scp,
			     i18n("fcntl(\"$filename\", F_SETLK, $arg): $errno")
					);
					/* NOTREACHED */
					sub_context_delete(scp);
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
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_errno_set(scp);
				sub_var_set_string(scp, "File_Name", path);
				sub_var_set_charstar(scp, "Argument", flock_string(&p));
				fatal_intl
				(
					scp,
			    i18n("fcntl(\"$filename\", F_SETLKW, $arg): $errno")
				);
				/* NOTREACHED */
				sub_context_delete(scp);
			}

			/*
			 * Unlock any of the "release immediately" locks.
			 * (These are used to gain a modicum of fairness.)
			 */
			for (j = 0; j < nplaces; ++j)
			{
				if (!place[j].release_immediately)
					continue;
				trace(("mark\n"));
				p = place[j].location; /* yes, copy it */
				p.l_type = F_UNLCK;
				if (glue_fcntl(fd, F_SETLKW, &p))
				{
					sub_context_ty	*scp;
	
					scp = sub_context_new();
					sub_errno_set(scp);
					sub_var_set_string
					(
						scp,
						"File_Name",
						path
					);
					sub_var_set_charstar
					(
						scp,
						"Argument",
						flock_string(&p)
					);
					fatal_intl
					(
						scp,
			    i18n("fcntl(\"$filename\", F_SETLKW, $arg): $errno")
					);
					/* NOTREACHED */
					sub_context_delete(scp);
				}
			}
			break;
		}

		/*
		 * give all the locks back
		 * that we got so far
		 */
		flock_construct(&p, F_UNLCK, 0L, 0x7FFFFFFF);
		if (glue_fcntl(fd, F_SETLKW, &p))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set_string(scp, "File_Name", path);
			sub_var_set_charstar(scp, "Argument", flock_string(&p));
			fatal_intl
			(
				scp,
			    i18n("fcntl(\"$filename\", F_SETLKW, $arg): $errno")
			);
		}

		/*
		 * Cope with interrupts while trying to obtain locks.
		 */
		os_interrupt_cope();
		trace(("mark\n"));

		/*
		 * verbose message about why we are blocking
		 */
		if (place[j].callback)
		{
			place[j].callback(place[j].callback_arg);
			if (!wait_for_locks)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();

				/* should never be reached */
				sub_var_set_charstar(scp, "Name", lock_description(&p));
				fatal_intl(scp, i18n("$name lock not available"));
				sub_context_delete(scp);
			}
		}
		else
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			p = place[j].location; /* yes, copy it */
			sub_var_set_charstar(scp, "Name", lock_description(&p));
			if (wait_for_locks)
				verbose_intl(scp, i18n("waiting for $name lock"));
			else
				fatal_intl(scp, i18n("$name lock not available"));
			sub_context_delete(scp);
		}

		/*
		 * sleep for a random number of seconds.
		 * This is necessary, because it is possible for two
		 * processes to be trying for the same lock
		 * and they alternate in the next wait.
		 */
		nsecs = r250() % 5;
		if (nsecs)
		{
			sleep(nsecs);
			os_interrupt_cope();
		}

		/*
		 * block on the lock that stopped us before
		 */
		if (j > 0 && place[j-1].release_immediately)
		{
			/*
			 * If the lock is preceeded by a priority lock,
			 * take that too, to stop them getting through.
			 * We can probably get it without waiting,
			 * because we already got it once before.
			 */
			trace(("mark\n"));
			p = place[j-1].location; /* yes, copy it */
			if (glue_fcntl(fd, F_SETLKW, &p))
			{
				sub_context_ty	*scp;
	
				if (errno == EINTR)
					os_interrupt_cope();
				scp = sub_context_new();
				sub_errno_set(scp);
				sub_var_set_string
				(
					scp,
					"File_Name",
					path
				);
				sub_var_set_charstar
				(
					scp,
					"Argument",
					flock_string(&p)
				);
				fatal_intl
				(
					scp,
			    i18n("fcntl(\"$filename\", F_SETLKW, $arg): $errno")
				);
				/* NOTREACHED */
				sub_context_delete(scp);
			}
		}
		trace(("mark\n"));
		p = place[j].location; /* yes, copy it */
		if (glue_fcntl(fd, F_SETLKW, &p))
		{
			sub_context_ty	*scp;

			if (errno == EINTR)
				os_interrupt_cope();
			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set_string(scp, "File_Name", path);
			sub_var_set_charstar(scp, "Argument", flock_string(&p));
			fatal_intl
			(
				scp,
			    i18n("fcntl(\"$filename\", F_SETLKW, $arg): $errno")
			);
			/* NOTREACHED */
			sub_context_delete(scp);
		}

		/*
		 * and then release it,
		 * before trying all over again.
		 */
		flock_construct(&p, F_UNLCK, 0L, 0x7FFFFFFF);
		if (glue_fcntl(fd, F_SETLKW, &p))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set_string(scp, "File_Name", path);
			sub_var_set_charstar(scp, "Argument", flock_string(&p));
			fatal_intl
			(
				scp,
			    i18n("fcntl(\"$filename\", F_SETLKW, $arg): $errno")
			);
			/* NOTREACHED */
			sub_context_delete(scp);
		}
		trace(("mark\n"));
	}
	gonzo_become_undo();
	magic++;
	trace(("}\n"));
}


void
lock_release()
{
	struct flock	p;
	int		fildes;
	int		err;

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
	 * Release all of the locks.
	 *
	 * This should be unnecessary because we close the file anyway,
	 * but some operating systems hang onto them if we don't.
	 */
	flock_construct(&p, F_UNLCK, 0L, 0x7FFFFFFF);
	err = glue_fcntl(fildes, F_SETLKW, &p);
#ifdef __CYGWIN__
	if (err && errno == EACCES)
	{
		/*
		 * For some reason Cygwin gives an error if you attempt
		 * to release all locks when you have no locks taken.
		 * Unix implementations, on the other hand, simply
		 * return success.
		 */
		err = 0;
	}
#endif
	if (err)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set_string(scp, "File_Name", path);
		sub_var_set_charstar(scp, "Argument", flock_string(&p));
		fatal_intl
		(
			scp,
			i18n("fcntl(\"$filename\", F_SETLKW, $arg): $errno")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * close the file
	 * (this *should* have been enough)
	 */
	glue_close(fildes);
	gonzo_become_undo();
	nplaces = 0;
	nplaces_max = 0;
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set_string(scp, "File_Name", path);
		sub_var_set_charstar(scp, "Argument", flock_string(&flock));
		fatal_intl(scp, i18n("fcntl(\"$filename\", F_GETLK, $arg): $errno"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	gonzo_become_undo();
	if (flock.l_type == F_UNLCK)
		return;

	/*
	 * Some operating systems return more than we asked for.
	 */
	if (flock.l_start < min)
	{
		flock.l_len -= (min - flock.l_start);
		flock.l_start = min;
	}
	if (flock.l_len <= 0)
		return;
	if (flock.l_start + flock.l_len > max)
		flock.l_len = max - flock.l_start;
	if (flock.l_len <= 0)
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
				found.name = lock_walk_name_pstate;
				found.subset = found.address & BITS_MASK;
				break;

			case lock_mux_cstate:
				found.name = lock_walk_name_cstate;
				found.subset = found.address & BITS_MASK;
				break;

			case lock_mux_baseline_priority:
				found.name = lock_walk_name_baseline_priority;
				found.subset = found.address & BITS_MASK;
				break;

			case lock_mux_baseline:
				found.name = lock_walk_name_baseline;
				found.subset = found.address & BITS_MASK;
				break;

			case lock_mux_history:
				found.name = lock_walk_name_history;
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
				nfatal("kill(%d, 0)", found.pid);

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
	if (min < flock.l_start)
		lock_walk_hunt(min, flock.l_start, callback);
	if (flock.l_start + flock.l_len < max)
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set_string(scp, "File_Name", path);
		fatal_intl(scp, i18n("open $filename: $errno"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	trace_int(fd);

	/*
	 * make sure the file is closed when a child exec's
	 */
#ifndef CONF_NO_seteuid
	if (fcntl(fd, F_GETFD, &flags))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set_string(scp, "File_Name", path);
		fatal_intl(scp, i18n("fcntl(\"$filename\", F_GETFD): $errno"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	flags |= 1;
	if (fcntl(fd, F_SETFD, flags))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set_string(scp, "File_Name", path);
		sub_var_set_long(scp, "Argument", flags);
		fatal_intl(scp, i18n("fcntl(\"$filename\", F_SETFD, $arg): $errno"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
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


void
lock_release_child()
{
#ifdef glue_close
	/*
	 * Only do this if and only if we are NOT using a proxy.
	 */
	if (fd >= 0)
		close(fd);
	fd = -1;
#endif
}
