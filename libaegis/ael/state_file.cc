//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2003-2008, 2012 Peter Miller
//      Copyright (C) 2008 Walter Franzini
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

#include <common/ac/stdio.h>

#include <common/str_list.h>
#include <libaegis/ael/state_file.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/project.h>
#include <libaegis/user.h>


void
list_state_file_name(change_identifier &cid, string_list_ty *)
{
    //
    // Print out the state file's path.
    //
    printf("%s\n", cid.get_cp()->cstate_filename_get()->str_text);
}


// vim: set ts=8 sw=4 et :
