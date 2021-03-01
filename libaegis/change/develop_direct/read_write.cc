//
// aegis - project change supervisor
// Copyright (C) 2002-2009, 2011, 2012 Peter Miller
// Copyright (C) 2008 Walter Franzini
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <libaegis/change/branch.h>
#include <libaegis/change/develop_direct/read_write.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/dir.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>


struct auxilliary
{
    change::pointer cp;
    string_ty       *dd;
    mode_t          umask;
    user_ty::pointer up;
    uid_t           uid;
    bool            protect;
};


static void
func(void *arg, dir_walk_message_ty msg, string_ty *path,
    const struct stat *st)
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
    mode &= ~aux->umask;

    //
    // What to do depends on what kind of file it is.
    //
    switch (msg)
    {
    case dir_walk_dir_after:
    case dir_walk_special:
    case dir_walk_symlink:
        break;

    case dir_walk_dir_before:
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
        aux->up->become_end();
        string_ty *rpath = os_below_dir(aux->dd, path);
        fstate_src_ty *src =
            aux->cp->file_find(nstring(rpath), view_path_first);
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
            src = aux->cp->pp->file_find(rpath, view_path_simple);
            if (src)
                mode &= ~0222;
        }
        str_free(rpath);
        aux->up->become_begin();

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
change_development_directory_chmod_read_write(change::pointer cp)
{
    if (cp->was_a_branch())
        return;
    cstate_ty *cstate_data = cp->cstate_get();
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
    aux.umask = cp->umask_get();
    aux.up = user_ty::create(nstring(cp->developer_name()));
    aux.uid = aux.up->get_uid();
    aux.protect = project_protect_development_directory_get(cp->pp);
    if (aux.protect)
        change_verbose(cp, 0, i18n("making dev dir writable"));
    aux.up->become_begin();
    dir_walk(aux.dd, func, &aux);
    aux.up->become_end();
}


// vim: set ts=8 sw=4 et :
