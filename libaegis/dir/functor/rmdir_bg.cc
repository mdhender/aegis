//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: implementation of the dir_functor_rmdir_bg class
//

#pragma implementation "dir_functor_rmdir_bg"

#include <ac/errno.h>
#include <ac/stddef.h>
#include <ac/sys/types.h>
#include <sys/stat.h>

#include <dir/functor/rmdir_bg.h>
#include <nstring.h>
#include <sub.h>


dir_functor_rmdir_bg::~dir_functor_rmdir_bg()
{
}


dir_functor_rmdir_bg::dir_functor_rmdir_bg()
{
}


void
dir_functor_rmdir_bg::operator()(msg_t msg, const nstring &path,
    const struct stat &st)
{
    switch (msg)
    {
    case msg_dir_before:
	if (!(st.st_mode & 0200))
    	    chmod(path.c_str(), ((st.st_mode & 07777) | 0200));
	break;

    case msg_dir_after:
	if (rmdir(path.c_str()))
	{
	    int errno_old = errno;
    	    if (errno_old == ENOENT)
       		break;
    	    sub_context_ty sc;
    	    sc.errno_setx(errno_old);
    	    sc.var_set_string("File_Name", path);
    	    sc.error_intl(i18n("warning: rmdir $filename: $errno"));
	}
	break;

    case msg_file:
    case msg_special:
    case msg_symlink:
	if (unlink(path.c_str()))
	{
	    int errno_old = errno;
	    if (errno_old == ENOENT)
		break;
	    sub_context_ty sc;
	    sc.errno_setx(errno_old);
	    sc.var_set_string("File_Name", path);
	    sc.error_intl(i18n("warning: unlink $filename: $errno"));
	}
	break;
    }
}
