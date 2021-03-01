//
//      aegis - project change supervisor
//      Copyright (C) 2004-2009, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/change/branch.h>
#include <libaegis/change.h>
#include <libaegis/pconf.fmtgen.h>
#include <libaegis/user.h>


void
change_maintain_symlinks_to_baseline(change::pointer cp, user_ty::pointer up,
    bool undoing)
{
    trace(("change_maintain_symlinks_to_baseline()\n{\n"));
    assert(cp->is_being_developed());
    assert(!cp->was_a_branch());
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    assert(pconf_data->development_directory_style);
    work_area_style_ty style = *pconf_data->development_directory_style;
    if (style.derived_at_start_only)
    {
        //
        // This function is not called by aedb (etc), so it is by
        // definition NOT at the start of development of a change set.
        // Therefore, clear all the bits that would have us do stuff to
        // derived (non-source) files.
        //
        style.derived_file_link = false;
        style.derived_file_symlink = false;
        style.derived_file_copy = false;
    }
    if (undoing && (style.source_file_link || style.source_file_symlink))
    {
        //
        // Warning: build tool fu.
        //
        // In order to have make (et al) behave sensibly around aecpu,
        // we need to make sure that the mod-time of the "uncopied"
        // source file is more recent than the file being uncopied,
        // otherwise the derived files (e.g. executables and .o files)
        // will not be rebuilt to reflect the "changed" source file.
        // This (almost always) means we need a file newer than the file
        // in the baseline.  This, in turn, means that a link doesn't
        // cut it, because we can't adjust the mod-time of the file in
        // the baseline, only in the development directory.
        //
        // The solution used here is to copy the baseline contents of
        // the file into the development directory, and touch the time
        // stamp of *that* file.
        //
        // The next time the file changes in the baseline, the copy will
        // be replaced by a {hard,sym}link, as a simple side-effect of
        // how change_create_symlinks_to_baseline is designed to work.
        //
        style.derived_file_link = false;
        style.derived_file_symlink = false;
        style.derived_file_copy = false;
        style.source_file_link = false;
        style.source_file_symlink = false;
        style.source_file_copy = true;
    }
    if
    (
        style.source_file_link
    ||
        style.source_file_symlink
    ||
        style.source_file_copy
    ||
        style.derived_file_link
    ||
        style.derived_file_symlink
    ||
        style.derived_file_copy
    )
    {
        if (up->symlink_pref(!style.during_build_only))
            change_create_symlinks_to_baseline(cp, up, style);
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
