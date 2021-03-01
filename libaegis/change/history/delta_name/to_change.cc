//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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

#include <libaegis/change/branch.h>
#include <common/fstrcmp.h>
#include <libaegis/sub.h>


long
change_history_change_by_name(change::pointer cp, string_ty *delta_name,
    int errok)
{
    cstate_ty       *cstate_data;
    cstate_branch_history_list_ty *h;
    size_t          j;
    string_ty       *best;
    double          best_weight;

    cstate_data = cp->cstate_get();
    if (!cstate_data->branch)
	goto useless;
    h = cstate_data->branch->history;
    if (!h)
	goto useless;
    if (!h->length)
	goto useless;
    best = 0;
    best_weight = 0.6;
    for (j = 0; j < h->length; ++j)
    {
	cstate_branch_history_ty *he;
	cstate_branch_history_name_list_ty *nlp;
	size_t		k;

	he = h->list[j];
	nlp = he->name;
	if (!nlp || !nlp->length)
		continue;
	for (k = 0; k < nlp->length; ++k)
	{
	    string_ty       *s;
	    double          weight;

	    s = nlp->list[k];
	    if (str_equal(s, delta_name))
		return he->change_number;
	    weight = fstrcmp(s->str_text, delta_name->str_text);
	    if (weight > best_weight)
		best = s;
	}
    }
    if (errok)
	return 0;
    if (best)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "Name", delta_name);
	sub_var_set_string(scp, "Guess", best);
	change_fatal(cp, scp, i18n("no delta $name, closest is $guess"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    else
    {
	useless:
	if (!errok)
	{
	    sub_context_ty	*scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", delta_name);
	    change_fatal(cp, scp, i18n("no delta $name"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
    }
    return 0;
}
