//
//	aegis - project change supervisor
//	Copyright (C) 1991-2008 Peter Miller
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

#include <common/ac/errno.h>
#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>
#include <common/ac/unistd.h>
#include <common/ac/sys/prctl.h>

#include <common/error.h>
#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/trace.h>

static int      become_orig_uid;
static int      become_orig_gid;
static int      become_orig_umask;
#ifdef DEBUG
static int      become_inited;
#endif

/**
  * The become_testing variable is used to remember the testing state of
  * the program.  It assumes one of three values:
  *
  * -1 means we are one of the non-set-uid-root programs (e.g. aereport)
  *  0 means set-uid-root program (e.g. aegis or aeimport)
  *  1 means Aegis is in testing (not set-uid-root) mode.
  */
static int      become_testing;

static int      become_active;
static int      become_active_uid;
static int      become_active_gid;
static int      become_active_umask;


void
os_setgid(int gid)
{
    os_become_must_not_be_active();
    if (become_testing < 0)
	return;
    if (setgid(gid))
    {
	sub_context_ty  *scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_long(scp, "Argument", gid);
	if (become_testing)
	{
	    //
	    // don't use os_testing_mode(),
	    // it could mess with errno
	    //
	    error_intl(scp, i18n("warning: setgid $arg: $errno"));
	}
	else
	    fatal_intl(scp, i18n("setgid $arg: $errno"));
	sub_context_delete(scp);
    }
#if defined(DEBUG) && defined(__linux__) && defined(HAVE_PRCTL)
    if (!prctl(PR_GET_DUMPABLE, 0, 0, 0, 0))
    {
        prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);
    }
#endif
}


void
os_chown_check(string_ty *path, int mode, int uid, int gid)
{
    struct stat     st;
    int             nerrs;
    int             oret;

    trace(("os_chown_check(path = \"%s\")\n{\n", path->str_text));
    os_become_must_be_active();
    nerrs = 0;
#ifdef S_IFLNK
    oret = glue_lstat(path->str_text, &st);
#else
    oret = glue_stat(path->str_text, &st);
#endif
    if (oret)
    {
	sub_context_ty  *scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("stat $filename: $errno"));
	// NOTREACHED
    }
    if (mode > 0 && ((int)st.st_mode & 07777) != mode)
    {
	sub_context_ty *scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", path);
	sub_var_set_format
        (
            scp,
            "Number1",
            "%5.5o",
            (unsigned int)(st.st_mode & 07777)
        );
	sub_var_set_format(scp, "Number2", "%5.5o", mode);
	error_intl
	(
	    scp,
	    i18n("$filename: mode is $number1, should be $number2")
	);
	sub_context_delete(scp);
	++nerrs;
    }
    if (become_testing > 0)
    {
	//
	// This is the test performed by aegis
	// in testing (not set-uid-root) mode.
	//
	if (st.st_uid != geteuid())
	{
	    sub_context_ty *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", path);
	    sub_var_set_long(scp, "Number1", st.st_uid);
	    sub_var_set_long(scp, "Number2", geteuid());
	    error_intl
		(scp, i18n("$filename: owner is $number1, should be $number2"));
	    sub_context_delete(scp);
	    ++nerrs;
	}
    }
    else if (become_testing == 0)
    {
	//
	// This is the test performed by aegis
	// when is set-uid-root mode.
	//
	if ((int)st.st_uid != uid)
	{
	    sub_context_ty *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", path);
	    sub_var_set_long(scp, "Number1", st.st_uid);
	    sub_var_set_long(scp, "Number2", uid);
	    error_intl
	    (
		scp,
		i18n("$filename: owner is $number1, should be $number2")
	    );
	    sub_context_delete(scp);
	    ++nerrs;
	}
	if (gid >= 0 && (int)st.st_gid != gid)
	{
	    sub_context_ty *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", path);
	    sub_var_set_long(scp, "Number1", st.st_gid);
	    sub_var_set_long(scp, "Number2", gid);
	    error_intl
	    (
		scp,
		i18n("$filename: group is $number1, should be $number2")
	    );
	    sub_context_delete(scp);
	    ++nerrs;
	}
    }
    else
    {
	//
	// No test is performed by aereport, aefind, etc.
	// This is because testing mode and non-testing mode
	// are indistinguishable.
	//
    }
    if (nerrs)
    {
	sub_context_ty  *scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("$filename: has been tampered with (fatal)"));
	// NOTREACHED
    }
    trace(("}\n"));
}


void
os_become_init(void)
{
    assert(!become_inited);
    become_active_umask = DEFAULT_UMASK;
    become_orig_umask = umask(DEFAULT_UMASK);
    become_orig_uid = getuid();
    become_orig_gid = getgid();
#ifdef SINGLE_USER
    become_testing = -1;
    become_active_uid = become_orig_uid;
    become_active_gid = become_orig_gid;
#else
    if (setuid(0))
    {
	become_testing = 1;
	become_active_uid = become_orig_uid;
	become_active_gid = become_orig_gid;
    }
    else {
        setgid(0);
#if defined (DEBUG) && defined (__linux__)
        if (!prctl (PR_GET_DUMPABLE, 0, 0, 0, 0)) {
            prctl (PR_SET_DUMPABLE, 1, 0, 0, 0);
        }
#endif
    }
#endif
#ifdef DEBUG
    become_inited = 1;
#endif
}


void
os_become_init_mortal(void)
{
    assert(!become_inited);
    become_active_umask = DEFAULT_UMASK;
    become_orig_umask = umask(DEFAULT_UMASK);
    become_orig_uid = getuid();
    become_orig_gid = getgid();
    become_testing = -1;
    become_active_uid = become_orig_uid;
    become_active_gid = become_orig_gid;
#ifdef DEBUG
    become_inited = 1;
#endif
}


void
os_become_reinit_mortal(void)
{
    assert(become_inited);
    become_active_umask = DEFAULT_UMASK;
    become_orig_umask = umask(DEFAULT_UMASK);
    become_orig_uid = getuid();
    become_orig_gid = getgid();
    become_testing = -1;
    become_active_uid = become_orig_uid;
    become_active_gid = become_orig_gid;
#ifdef DEBUG
    become_inited = 1;
#endif
}


int
os_testing_mode(void)
{
    static int      warned;

    assert(become_inited);
    if (become_testing <= 0)
	return 0;
    if (!warned)
    {
	sub_context_ty *scp;

	warned = 1;
	scp = sub_context_new();
	verbose_intl(scp, i18n("warning: test mode"));
	sub_context_delete(scp);
    }
    return 1;
}


void
os_become(int uid, int gid, int um)
{
    trace(("os_become(uid = %d, gid = %d, um = %03o)\n{\n", uid, gid, um));
    if (become_active)
	fatal_raw("multiple user permissions set (bug)");
    become_active = 1;
    if (os_testing_mode())
    {
	become_active_umask = um;
	umask(um);
    }
    else if (become_testing < 0)
    {
	// do nothing if we are mortal
	become_active_umask = um;
	umask(um);
    }
    else
    {
	become_active_uid = uid;
	become_active_gid = gid;
	become_active_umask = um;
#ifndef CONF_NO_seteuid
	if (setegid(gid))
	    nfatal("setegid(%d)", gid);
	if (seteuid(uid))
	    nfatal("seteuid(%d)", uid);
#if defined (DEBUG) && defined (__linux__)
        if (!prctl (PR_GET_DUMPABLE, 0, 0, 0, 0)) {
            prctl (PR_SET_DUMPABLE, 1, 0, 0, 0);
        }
#endif
	umask(um);
#endif
    }
    trace(("}\n"));
}


void
os_become_undo(void)
{
    trace(("os_become_undo()\n{\n"));
    os_become_must_be_active();
    os_become_undo_atexit();
    trace(("}\n"));
}


void
os_become_undo_atexit(void)
{
    trace(("os_become_undo_atexit()\n{\n"));
    assert(become_inited);
    if (become_active)
    {
	become_active = 0;
	if (!become_testing)
	{
#ifndef CONF_NO_seteuid
	    if (seteuid(0))
		nfatal("seteuid(0)");
	    if (setegid(0))
		nfatal("setegid(0)");
#if defined(DEBUG) && defined(__linux__)
	    if (!prctl(PR_GET_DUMPABLE, 0, 0, 0, 0))
	    {
		prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);
	    }
#endif
#endif
	    become_active_uid = 0;
	    become_active_gid = 0;
	}
    }
    trace(("}\n"));
}


void
os_become_undo(int uid, int gid)
{
    trace(("os_become_undo(uid = %d, gid = %d)\n{\n", uid, gid));
    assert(become_inited);
    assert(become_active);
    become_active = 0;
    if (!become_testing)
    {
        assert(become_active_uid == uid);
        assert(become_active_gid == gid);
        (void)uid;
        (void)gid;
#ifndef CONF_NO_seteuid
        if (seteuid(0))
            nfatal("seteuid(0)");
        if (setegid(0))
            nfatal("setegid(0)");
#if defined(DEBUG) && defined(__linux__)
        if (!prctl(PR_GET_DUMPABLE, 0, 0, 0, 0))
        {
            prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);
        }
#endif
#endif
        become_active_uid = 0;
        become_active_gid = 0;
    }
    trace(("}\n"));
}


void
os_become_orig(void)
{
    os_become(become_orig_uid, become_orig_gid, become_orig_umask);
}


void
os_become_query(int *uid, int *gid, int *umsk)
{
    os_become_must_be_active();
    *uid = become_active_uid;
    if (gid)
	*gid = become_active_gid;
    if (umsk)
	*umsk = become_active_umask;
}


void
os_become_orig_query(int *uid, int *gid, int *umsk)
{
    if (uid)
	*uid = become_orig_uid;
    if (gid)
	*gid = become_orig_gid;
    if (umsk)
	*umsk = become_orig_umask;
}


int
os_become_active(void)
{
    return become_active;
}


void
os_become_must_be_active_gizzards(const char *file, int line)
{
    if (!os_become_active())
	fatal_raw("%s: %d: user permissions not set (bug)", file, line);
}


void
os_become_must_not_be_active_gizzards(const char *file, int line)
{
    if (os_become_active())
    {
	fatal_raw
	(
	    "%s: %d: user permissions set when aught not (bug)",
	    file,
	    line
	);
    }
}
