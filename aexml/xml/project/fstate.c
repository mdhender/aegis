/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate fstates
 */

#include <ael/change/inappropriat.h>
#include <fstate.h>
#include <output.h>
#include <project.h>
#include <project/file.h>
#include <trace.h>
#include <user.h>
#include <xml/project/fstate.h>


void
xml_project_fstate(struct string_ty *project_name, long change_number,
    struct output_ty *op)
{
    project_ty      *pp;
    size_t          j;

    trace(("xml_project_fstate()\n{\n"));
    if (change_number)
	    list_change_inappropriate();

    /*
     * locate project data
     */
    if (!project_name)
	project_name = user_default_project();
    else
	project_name = str_copy(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    /*
     * We don't actually use project_fstate_get() because very soon it
     * is going to be broken up into one-file-per-file and this will no
     * longer exist.  So we synthesize it.
     */
    output_fputs(op, "<fstate>\n<src>\n");
    for (j = 0; ; ++j)
    {
	fstate_src      src;

	src = project_file_nth(pp, j, view_path_simple);
	if (!src)
	    break;
	fstate_src_write_xml(op, "fstate_src", src);
    }
    output_fputs(op, "</src>\n</fstate>\n");

    project_free(pp);
    trace(("}\n"));
}
