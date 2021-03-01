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
#include <common/ac/unistd.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>


static long
pathconf_inner(const char *path, int arg)
{
#ifdef HAVE_PATHCONF
    long            result;

    errno = EINVAL;	// IRIX 5.2 fails to set on errors
    result = glue_pathconf(path, arg);
    if (result < 0)
    {
	switch (errno)
	{
	case ENOSYS:	// lotsa systems say this for EINVAL
#ifdef EOPNOTSUPP
	case EOPNOTSUPP:	// HPUX says this for EINVAL
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
    //
    // This system does not have the pathconf system call.
    // (Return EINVAL rather than ENOSYS, because the outer wrapper
    // expects EINVAL.)
    //
    errno = EINVAL;
    return -1;
#endif
}


static long
pathconf_wrapper(const char *path, int arg, long default_value)
{
#ifdef HAVE_PATHCONF
    long            result;

    result = pathconf_inner(path, arg);
    if (result < 0 && errno == EINVAL)
    {
	//
	// Probably NFS/2 mounted (NFS/3 added pathconf), so
	// we will *guess* it's the same as the root filesystem.
	// Default if root is also NFS mounted.
	//
	result = pathconf_inner("/", arg);
	if (result < 0 && errno == EINVAL)
	    result = default_value;
    }
    return result;
#else
    //
    // This system does not have the pathconf system call.
    //
    return default_value;
#endif
}


int
os_pathconf_path_max(string_ty *path)
{
    return os_pathconf_path_max(nstring(path));
}


int
os_pathconf_path_max(const nstring &path)
{
    os_become_must_be_active();
    long result = 1024;
    result = pathconf_wrapper(path.c_str(), _PC_PATH_MAX, result);
    if (result < 0)
    {
	int errno_old = errno;
	sub_context_ty sc;
	sc.errno_setx(errno_old);
	sc.var_set_string("File_Name", path);
	sc.fatal_intl(i18n("pathconf(\"$filename\", {PATH_MAX}): $errno"));
	// NOTREACHED
    }
    return result;
}


int
os_pathconf_name_max(string_ty *path)
{
    return os_pathconf_name_max(nstring(path));
}


int
os_pathconf_name_max(const nstring &path)
{
    os_become_must_be_active();
#ifdef HAVE_LONG_FILE_NAMES
    long result = 255;
#else
    long result = 14;
#endif
    result = pathconf_wrapper(path.c_str(), _PC_NAME_MAX, result);
    if (result < 0)
    {
	int errno_old = errno;
	sub_context_ty sc;
	sc.errno_setx(errno_old);
	sc.var_set_string("File_Name", path);
	sc.fatal_intl(i18n("pathconf(\"$filename\", {NAME_MAX}): $errno"));
	// NOTREACHED
    }
    return result;
}
