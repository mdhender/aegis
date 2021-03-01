/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-2003 Peter Miller;
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
 * MANIFEST: wrappers around operating system functions
 */

#include <ac/errno.h>
#include <ac/unistd.h>

#include <glue.h>
#include <os.h>
#include <sub.h>


static long
pathconf_inner(const char *path, int arg)
{
#ifdef HAVE_PATHCONF
    long            result;

    errno = EINVAL;	/* IRIX 5.2 fails to set on errors */
    result = glue_pathconf(path, arg);
    if (result < 0)
    {
	switch (errno)
	{
	case ENOSYS:	/* lotsa systems say this for EINVAL */
#ifdef EOPNOTSUPP
	case EOPNOTSUPP:	/* HPUX says this for EINVAL */
#endif
	    errno = EINVAL;
	    break;
	}
    }
    else
    {
#if INT_MAX < LONG_MAX
	if (result > INT_MAX)
	    result = INT_MAX;
#endif
    }
    return result;
#else
    /*
     * This system does not have the pathconf system call.
     * (Return EINVAL rather than ENOSYS, because the outer wrapper
     * expects EINVAL.)
     */
    errno = EINVAL;
    return -1;
#endif
}


static long
pathconf_wrapper(char *path, int arg, long default_value)
{
#ifdef HAVE_PATHCONF
    long            result;

    result = pathconf_inner(path, arg);
    if (result < 0 && errno == EINVAL)
    {
	/*
	 * Probably NFS/2 mounted (NFS/3 added pathconf), so
	 * we will *guess* it's the same as the root filesystem.
	 * Default if root is also NFS mounted.
	 */
	result = pathconf_inner("/", arg);
	if (result < 0 && errno == EINVAL)
	    result = default_value;
    }
    return result;
#else
    /*
     * This system does not have the pathconf system call.
     */
    return default_value;
#endif
}


int
os_pathconf_path_max(string_ty *path)
{
    long            result;

    os_become_must_be_active();
    result = 1024;
    result = pathconf_wrapper(path->str_text, _PC_PATH_MAX, result);
    if (result < 0)
    {
	sub_context_ty *scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("pathconf(\"$filename\", {PATH_MAX}): $errno"));
	/* NOTREACHED */
    }
    return result;
}


int
os_pathconf_name_max(string_ty *path)
{
    long            result;

    os_become_must_be_active();
#ifdef HAVE_LONG_FILE_NAMES
    result = 255;
#else
    result = 14;
#endif
    result = pathconf_wrapper(path->str_text, _PC_NAME_MAX, result);
    if (result < 0)
    {
	sub_context_ty *scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("pathconf(\"$filename\", {NAME_MAX}): $errno"));
	/* NOTREACHED */
    }
    return result;
}
