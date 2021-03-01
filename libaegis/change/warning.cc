//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#include <libaegis/change.h>
#include <libaegis/project.h>
#include <common/nstring.h>
#include <libaegis/sub.h>


void
change_warning(change::pointer cp, sub_context_ty *scp, const char *s)
{
    bool need_to_delete = false;
    if (!scp)
    {
	scp = sub_context_new();
	need_to_delete = true;
    }

    //
    // assemble the message
    //
    subst_intl_change(scp, cp);
    nstring msg1(subst_intl(scp, s));

    //
    // get ready to pass the message to the project error function
    //
    // re-use substitution context
    sub_var_set_string(scp, "MeSsaGe", msg1.get_ref());

    //
    // assemble the warning
    //
    subst_intl_change(scp, cp);
    nstring msg2(subst_intl(scp, i18n("warning: $message")));

    //
    // get ready to pass the message to the project error function
    //
    // re-use substitution context
    sub_var_set_string(scp, "MeSsaGe", msg2.get_ref());

    //
    // the form of the project error message depends on what kind of
    // change this is
    //
    if (cp->bogus)
	project_verbose(cp->pp, scp, i18n("$message"));
    else if (cp->number == TRUNK_CHANGE_NUMBER)
	project_verbose(cp->pp, scp, i18n("trunk: $message"));
    else
    {
	subst_intl_change(scp, cp);
	project_verbose(cp->pp, scp, i18n("change $change: $message"));
    }

    if (need_to_delete)
	sub_context_delete(scp);
}
