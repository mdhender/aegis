//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate fatals
//

#include <change.h>
#include <project.h>
#include <sub.h>


void
change_fatal(change_ty *cp, sub_context_ty *scp, const char *s)
{
	string_ty	*msg;
	int		need_to_delete;

	if (!scp)
	{
		scp = sub_context_new();
		need_to_delete = 1;
	}
	else
		need_to_delete = 0;

	//
	// asemble the message
	//
	subst_intl_change(scp, cp);
	msg = subst_intl(scp, s);

	//
	// get ready to pass the message to the project error function
	//
	// re-use substitution context
	sub_var_set_string(scp, "Message", msg);
	str_free(msg);

	//
	// the form of the project error message depends on what kind of
	// change this is
	//
	if (cp->bogus)
		project_fatal(cp->pp, scp, i18n("$message"));
	else if (cp->number == TRUNK_CHANGE_NUMBER)
		project_fatal(cp->pp, scp, i18n("trunk: $message"));
	else
	{
		subst_intl_change(scp, cp);
		project_fatal(cp->pp, scp, i18n("change $change: $message"));
	}
	// NOTREACHED

	if (need_to_delete)
		sub_context_delete(scp);
}
