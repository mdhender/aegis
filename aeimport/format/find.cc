//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>

#include <aeimport/format/rcs.h>
#include <aeimport/format/sccs.h>
#include <libaegis/sub.h>


format_ty *
format_find(const char *name)
{
    sub_context_ty  *scp;

    //
    // The default is RCS format (I'm guessing most folks are
    // importing a CVS archive).
    //
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

    //
    // Oops.
    //
    scp = sub_context_new();
    sub_var_set_charstar(scp, "Name", name);
    fatal_intl(scp, i18n("history format \"$name\" unknown"));
    return 0;
}
