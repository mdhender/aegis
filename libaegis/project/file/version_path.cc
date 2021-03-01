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
// MANIFEST: functions to manipulate version_paths
//

#include <change.h>
#include <change/file.h>
#include <error.h>
#include <fstate.h>
#include <os.h>
#include <project.h>
#include <project/file.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


string_ty *
project_file_version_path(project_ty *pp, fstate_src_ty *src, int *unlink_p)
{
    fstate_src_ty   *old_src;
    project_ty      *ppp;
    change_ty       *cp;
    string_ty       *filename;
    history_version_ty *ed;
    fstate_src_ty   *reconstruct;

    trace(("project_file_version_path(pp = %08lX, src = %08lX, "
	"unlink_p = %08lX)\n{\n", (long)pp, (long)src, (long)unlink_p));
    assert(src);
    assert(src->file_name);
    trace(("fn \"%s\"\n", src->file_name->str_text));
    assert(src->edit || src->edit_origin);
    ed = src->edit ? src->edit : src->edit_origin;
    assert(ed->revision);
    trace(("rev \"%s\"\n", ed->revision->str_text));
    if (unlink_p)
	*unlink_p = 0;
    for (ppp = pp; ppp; ppp = ppp->parent)
    {
	cp = project_change_get(ppp);
        if (change_is_completed(cp))
	    continue;
	old_src = change_file_find(cp, src->file_name, view_path_first);
	if (!old_src)
	    continue;
        switch (old_src->action)
	{
	case file_action_remove:
	case file_action_transparent:
	    continue;

	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
#ifndef DEBUG
	default:
#endif
	    // should be file_action_remove
	    assert(!old_src->deleted_by);
	    if (old_src->deleted_by)
		continue;

	    // should be file_action_transparent
	    assert(!old_src->about_to_be_created_by);
	    if (old_src->about_to_be_created_by)
		continue;

	    // should be file_action_transparent
	    assert(!old_src->about_to_be_copied_by);
	    if (old_src->about_to_be_copied_by)
		continue;
	    break;
	}
	assert(old_src->edit);
	assert(old_src->edit->revision);
	if (str_equal(old_src->edit->revision, ed->revision))
	{
	    filename = change_file_path(cp, src->file_name);
            //
            // The following check is needed to make aegis work even
            // when filename does'not exists.
            //
            os_become_orig();
            int file_exists = os_exists(filename);
            os_become_undo();
            assert(file_exists);
            if (!file_exists)
                break;
            trace(("return \"%s\";\n", filename->str_text));
	    trace(("}\n"));
	    return filename;
	}
    }

    filename = os_edit_filename(0);
    os_become_orig();
    undo_unlink_errok(filename);
    os_become_undo();
    if (unlink_p)
	*unlink_p = 1;

    reconstruct = (fstate_src_ty *)fstate_src_type.alloc();
    reconstruct->file_name = str_copy(src->file_name);
    reconstruct->edit = history_version_copy(ed);
    change_file_copy_basic_attributes(reconstruct, src);

    cp = project_change_get(pp);
    change_run_history_get_command
    (
	cp,
	reconstruct,
	filename,
	user_executing(pp)
    );
    history_version_type.free(reconstruct);
    trace(("return \"%s\";\n", filename->str_text));
    trace(("}\n"));
    return filename;
}
