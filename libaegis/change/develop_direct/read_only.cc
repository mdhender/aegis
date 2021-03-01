//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate read_onlys
//

#include <ac/stddef.h>
#include <ac/sys/types.h>
#include <sys/stat.h>

#include <cstate.h>
#include <change.h>
#include <change/branch.h>
#include <change/develop_direct/read_only.h>
#include <change/file.h>
#include <dir.h>
#include <os.h>
#include <project/file.h>
#include <project/history.h>
#include <sub.h>
#include <undo.h>
#include <user.h>


struct auxilliary
{
    change_ty       *cp;
    string_ty       *dd;
    mode_t          umask;
    user_ty         *up;
    uid_t           uid;
    bool            protect;
};


static void
func(void *arg, dir_walk_message_ty msg, string_ty *path, struct stat *st)
{
    //
    // If they are using one of the more interesting development
    // directory styles then the file could be a hard link to
    // the baseline.  In such a case the file may be owned by a
    // different user, and we are supposed to leave it alone.
    //
    // Any other case of a different UID means they have done something
    // creative with their build system, and we don't want to know.
    //
    auxilliary *aux = (auxilliary *)arg;
    if (st->st_uid != aux->uid)
	return;

    //
    // Figure out the correct file mode.
    //
    // Note that there are time when you are using Samba and editing
    // files on Windoze that the group and other bits get cleared,
    // meaning that the reviewers will be unable to read the files.
    // This code will correct this if it sees it.
    //
    mode_t mode = (st->st_mode & 07777) | 0644;
    if (S_ISDIR(st->st_mode))
	mode |= 0111;
    if (aux->protect)
	mode &= ~0222;
    mode &= ~aux->umask;

    //
    // What to do depends on what kind of file it is.
    //
    switch (msg)
    {
    case dir_walk_dir_before:
    case dir_walk_special:
    case dir_walk_symlink:
	break;

    case dir_walk_dir_after:
	if ((st->st_mode & 07777) != mode)
	{
	    undo_chmod(path, st->st_mode & 07777);
	    os_chmod_errok(path, mode);
	}
	break;

    case dir_walk_file:
	if (st->st_nlink > 1)
	{
	    //
            // This file has more than one link.  That means if we
            // chmod this file, we will change something else, somewhere
            // else.  That could be bad, so we will be conservative and
            // leave this file alone.
	    //
            // It's probably a link to the ocrresponding file in the
            // baseline (which should already be read-only, and should
            // be left alone) but we can't be sure.
	    //
	    break;
	}

	//
        // We have to figure out what mode the file is supposed to have.
        // If it is a source file of the change, or it is a derived
        // file, it should be read write.  But if they are using one of
        // the more interesting development directory styles then the
        // file could be meant to be read-only.
	//
	user_become_undo();
	string_ty *rpath = os_below_dir(aux->dd, path);
	fstate_src_ty *src = change_file_find(aux->cp, rpath, view_path_first);
	if (src)
	{
	    switch (src->action)
	    {
	    case file_action_remove:
		// probably whiteout
		break;

	    case file_action_insulate:
	    case file_action_transparent:
		mode &= ~0222;
		break;

	    case file_action_create:
	    case file_action_modify:
		break;
	    }
	}
	else
	{
	    //
            // Links to (copies of) project files are supposed to be
            // read-only.  It remonds the developer to aecp the file
            // first.
	    //
	    src = project_file_find(aux->cp->pp, rpath, view_path_simple);
	    if (src)
		mode &= ~0222;
	}
	str_free(rpath);
	user_become(aux->up);

	//
	// Verify the mode.
	//
	if ((st->st_mode & 07777) != mode)
	{
	    undo_chmod(path, st->st_mode & 07777);
	    os_chmod_errok(path, mode);
	}
	break;
    }
}


void
change_development_directory_chmod_read_only(change_ty *cp)
{
    if (change_was_a_branch(cp))
	return;
    cstate_ty *cstate_data = change_cstate_get(cp);
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_completed:
	return;

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
    case cstate_state_being_integrated:
	break;
    }

    auxilliary aux;
    aux.cp = cp;
    aux.dd = change_development_directory_get(cp, 0);
    aux.umask = change_umask(cp);
    aux.up = user_symbolic(cp->pp, change_developer_name(cp));
    aux.uid = user_id(aux.up);
    aux.protect = project_protect_development_directory_get(cp->pp);
    if (aux.protect)
	change_verbose(cp, 0, i18n("making dev dir read only"));
    user_become(aux.up);
    dir_walk(aux.dd, func, &aux);
    user_become_undo();
}
