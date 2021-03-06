//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2002, 2004-2006, 2008, 2011, 2012 Peter Miller
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

#include <libaegis/project.h>
#include <libaegis/project/verbose.h>
#include <libaegis/sub.h>


void
project_verbose_directory(project *pp, string_ty *home)
{
    sub_context_ty  *scp;

    scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", home);
    project_verbose(pp, scp, i18n("proj dir $filename"));
    sub_context_delete(scp);
}


// vim: set ts=8 sw=4 et :
