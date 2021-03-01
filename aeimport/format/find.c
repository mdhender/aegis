/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate finds
 */

#include <ac/string.h>

#include <format/rcs.h>
#include <format/sccs.h>
#include <sub.h>


format_ty *
format_find(const char *name)
{
    sub_context_ty  *scp;

    /*
     * The default is RCS format (I'm guessing most folks are
     * importing a CVS archive).
     */
    if (!name || 0 == strcasecmp(name, "rcs") || 0 == strcasecmp(name, "cvs"))
	return format_rcs_new();

    if
    (
	0 == strcasecmp(name, "sccs")
    ||
	0 == strcasecmp(name, "cssc")
    ||
	0 == strcasecmp(name, "bitsccs")
    ||
	0 == strcasecmp(name, "bitkeeper")
    ||
	0 == strcasecmp(name, "bk")
    )
	return format_sccs_new();

    /*
     * Oops.
     */
    scp = sub_context_new();
    sub_var_set_charstar(scp, "Name", name);
    fatal_intl(scp, i18n("history format \"$name\" unknown"));
    return 0;
}
