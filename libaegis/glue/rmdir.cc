//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004-2006 Peter Miller
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
// MANIFEST: functions to manipulate rmdirs
//

#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>
#include <sys/stat.h>
#include <common/ac/stdlib.h>

#include <libaegis/dir.h>
#include <libaegis/dir/functor/rmdir_bg.h>
#include <common/error.h>
#include <libaegis/glue.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/undo.h>


int
rmdir_bg(const char *path)
{
    switch (fork())
    {
    case -1:
	nfatal("fork");

    case 0:
	{
	    // child
	    os_interrupt_ignore();
	    lock_release_child(); // don't hold locks
	    undo_cancel(); // don't do anything else!
	    dir_functor_rmdir_bg eraser;
	    dir_walk(nstring(path), eraser);
	    exit(0);
	}

    default:
	// parent
	break;
    }
    return 0;
}


int
rmdir_tree(const char *path)
{
    dir_functor_rmdir_bg eraser;
    dir_walk(nstring(path), eraser);
    return 0;
}
