/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995, 1996, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate change_brans
 */

#include <ac/stdlib.h>

#include <change_bran.h>
#include <change/file.h>
#include <error.h>
#include <fstrcmp.h>
#include <interval.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <skip_unlucky.h>
#include <str_list.h>
#include <zero.h>


void
change_branch_new(cp)
	change_ty	*cp;
{
	cstate		cstate_data;

	cstate_data = change_cstate_get(cp);
	assert(!cstate_data->branch);
	cstate_data->cause = change_cause_internal_enhancement;
	cstate_data->branch = cstate_branch_type.alloc();
	cstate_data->branch->change = cstate_branch_change_list_type.alloc();
}


int
change_history_delta_validate(cp, delta_number)
	change_ty	*cp;
	long		delta_number;
{
	cstate		cstate_data;
	cstate_branch_history_list h;
	size_t		j;

	cstate_data = change_cstate_get(cp);
	if (!cstate_data->branch)
		return 0;
	h = cstate_data->branch->history;
	if (!h)
		return 0;
	for (j = 0; j < h->length; ++j)
	{
		if (h->list[j]->delta_number == delta_number)
			return 1;
	}
	return 0;
}


static time_t change_completion_timestamp _((change_ty *));

static time_t
change_completion_timestamp(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_history	chp;
	cstate_history_list chlp;

	cstate_data = change_cstate_get(cp);
	assert(cstate_data->state == cstate_state_completed);
	chlp = cstate_data->history;
	assert(chlp);
	assert(chlp->length);
	chp = chlp->list[chlp->length - 1];
	assert(chp);
	return chp->when;
}


time_t
change_history_delta_to_timestamp(pp, delta_number)
	project_ty	*pp;
	long		delta_number;
{
	cstate		cstate_data;
	cstate_branch_history_list hl;
	size_t		j;
	change_ty	*cp;

	cp = project_change_get(pp);
	cstate_data = change_cstate_get(cp);
	if (!cstate_data->branch)
		return (time_t)-1;
	if (delta_number == 0)
	{
		/*
		 * special case, meaning
		 * ``when the branch was created''
		 */
		if (cstate_data->history && cstate_data->history->length)
			return cstate_data->history->list[0]->when;
		return (time_t)-1;
	}
	hl = cstate_data->branch->history;
	if (!hl)
		return (time_t)-1;
	for (j = 0; j < hl->length; ++j)
	{
		cstate_branch_history bh;
		change_ty	*cp2;
		time_t		result;

		bh = hl->list[j];
		assert(bh);
		if (!bh)
			continue;
		if (bh->delta_number != delta_number)
			continue;
		cp2 = change_alloc(pp, bh->change_number);
		change_bind_existing(cp2);
		result = change_completion_timestamp(cp2);
		change_free(cp2);
		return result;
	}
	return (time_t)-1;
}


long
change_history_delta_latest(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch_history_list h;

	cstate_data = change_cstate_get(cp);
	if (!cstate_data->branch)
		return 0;
	h = cstate_data->branch->history;
	if (!h)
		return 0;
	if (!h->length)
		return 0;
	return h->list[h->length - 1]->delta_number;
}


long
change_history_last_change_integrated(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;
	cstate_branch_history	history_data;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	if (!bp || !bp->history || !bp->history->length)
		return 0;
	history_data = bp->history->list[bp->history->length - 1];
	return history_data->change_number;
}


long
change_history_delta_by_name(cp, delta_name, errok)
	change_ty	*cp;
	string_ty	*delta_name;
	int		errok;
{
	cstate		cstate_data;
	cstate_branch_history_list h;
	size_t		j;
	string_ty	*best;
	double		best_weight;

	cstate_data = change_cstate_get(cp);
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
		cstate_branch_history he;
		cstate_branch_history_name_list nlp;
		size_t		k;

		he = h->list[j];
		nlp = he->name;
		if (!nlp || !nlp->length)
			continue;
		for (k = 0; k < nlp->length; ++k)
		{
			string_ty	*s;
			double		weight;

			s = nlp->list[k];
			if (str_equal(s, delta_name))
				return he->delta_number;
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
		sub_var_set(scp, "Name", "%S", delta_name);
		sub_var_set(scp, "Guess", "%S", best);
		change_fatal(cp, scp, i18n("no delta $name, closest is $guess"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	else
	{
		useless:
		if (!errok)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%S", delta_name);
			change_fatal(cp, scp, i18n("no delta $name"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
	}
	return 0;
}


void
change_history_delta_name_delete(cp, delta_name)
	change_ty	*cp;
	string_ty	*delta_name;
{
	cstate		cstate_data;
	cstate_branch_history_list h;
	size_t		j;

	trace(("change_history_delta_name_delete(cp = %8.8lX, delta_name = \
\"%s\")\n{\n"/*}{*/, (long)cp, delta_name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	h = cstate_data->branch->history;
	assert(h);
	assert(h->length);
	for (j = 0; j < h->length; ++j)
	{
		cstate_branch_history he;
		cstate_branch_history_name_list nlp;
		size_t		k, m;

		he = h->list[j];
		nlp = he->name;
		if (!nlp || !nlp->length)
			continue;
		for (k = 0; k < nlp->length; ++k)
		{
			if (!str_equal(nlp->list[k], delta_name))
				continue;

			/*
			 * remove the name from the list
			 */
			str_free(nlp->list[k]);
			for (m = k + 1; m < nlp->length; ++m)
				nlp->list[m - 1] = nlp->list[m];
			k--;

			nlp->length--;
			if (nlp->length == 0)
			{
				he->name = 0;
				cstate_branch_history_name_list_type.free(nlp);
				break;
			}
		}
	}
	trace((/*{*/"}\n"));
}


void
change_history_delta_name_add(cp, delta_number, delta_name)
	change_ty	*cp;
	long		delta_number;
	string_ty	*delta_name;
{
	cstate		cstate_data;
	cstate_branch_history_list h;
	size_t		j;
	cstate_branch_history he;

	trace(("change_history_delta_name_add(cp = %8.8lX, delta_number = %ld, \
delta_name = \"%s\")\n{\n"/*}{*/, (long)cp, delta_number, delta_name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	h = cstate_data->branch->history;
	assert(h);
	assert(h->length);
	for (j = 0; j < h->length; ++j)
	{
		type_ty		*type_p;
		string_ty	**addr_p;

		he = h->list[j];
		if (he->delta_number != delta_number)
			continue;

		/*
		 * add the name to the selected history entry
		 */
		if (!he->name)
			he->name = cstate_branch_history_name_list_type.alloc();
		addr_p =
			cstate_branch_history_name_list_type.list_parse
			(
				he->name,
				&type_p
			);
		assert(type_p == &string_type);
		*addr_p = str_copy(delta_name);
		break;
	}
	trace((/*{*/"}\n"));
}


long
change_current_integration_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;

	cstate_data = change_cstate_get(cp);
	if (!cstate_data->branch)
		return 0;
	return cstate_data->branch->currently_integrating_change;
}


void
change_current_integration_set(cp, change_number)
	change_ty	*cp;
	long		change_number;
{
	cstate		cstate_data;
	cstate_branch	bp;

	trace(("change_current_integration_set(cp = %8.8lX, change_number = %ld)\n{\n"/*}*/,
		(long)cp, change_number));
	assert(change_number >= 0 || change_number == MAGIC_ZERO);
	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (change_number)
	{
		assert(!(bp->mask & cstate_branch_currently_integrating_change_mask));
		bp->currently_integrating_change = change_number;
		bp->mask |= cstate_branch_currently_integrating_change_mask;
	}
	else
	{
		assert(bp->mask & cstate_branch_currently_integrating_change_mask);
		bp->currently_integrating_change = 0;
		bp->mask &= ~cstate_branch_currently_integrating_change_mask;
	}
	trace((/*{*/"}\n"));
}


long
change_delta_number_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;
	long		result;
	size_t		j;

	trace(("change_delta_number_get(cp = %8.8lX)\n{\n"/*}*/, (long)cp));
	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (!bp || !bp->history || !bp->history->length)
	{
		trace(("return 1;\n"));
		trace((/*{*/"}\n"));
		return 1;
	}
	result = bp->history->length + 1;
	for (j = 0; j < bp->history->length; ++j)
	{
		cstate_branch_history hp;

		hp = bp->history->list[j];
		if (hp->delta_number >= result)
			result = hp->delta_number + 1;
	}
	trace(("return %ld;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


void
change_copyright_year_append(cp, year)
	change_ty	*cp;
	int		year;
{
	cstate		cstate_data;
	size_t		j;
	long		*year_p;
	type_ty		*type_p;

	cstate_data = change_cstate_get(cp);
	if (!cstate_data->copyright_years)
		cstate_data->copyright_years =
			cstate_copyright_years_list_type.alloc();
	for (j = 0; j < cstate_data->copyright_years->length; ++j)
		if (cstate_data->copyright_years->list[j] == year)
			return;
	year_p =
		cstate_copyright_years_list_type.list_parse
		(
			cstate_data->copyright_years,
			&type_p
		);
	assert(type_p == &integer_type);
	*year_p = year;
}


void
change_copyright_years_now(cp)
	change_ty	*cp;
{
	time_t		now;
	struct tm	*tm;
	int		year;

	time(&now);
	tm = localtime(&now);
	year = tm->tm_year + 1900;
	change_copyright_year_append(cp, year);
}


void
change_copyright_years_merge(cp1, cp2)
	change_ty	*cp1;
	change_ty	*cp2;
{
	cstate		cstate_data;
	size_t		j;

	cstate_data = change_cstate_get(cp2);
	if (!cstate_data->copyright_years)
		return;
	for (j = 0; j < cstate_data->copyright_years->length; ++j)
	{
		change_copyright_year_append
		(
			cp1,
			cstate_data->copyright_years->list[j]
		);
	}
}


static void change_copyright_years_slurp _((change_ty *, int *, int , int *));

static void
change_copyright_years_slurp(cp, a, amax, alen_p)
	change_ty	*cp;
	int		*a;
	int		amax;
	int		*alen_p;
{
	int		j, k;
	int		n;
	cstate		cstate_data;
	cstate_copyright_years_list cylp;

	cstate_data = change_cstate_get(cp);
	cylp = cstate_data->copyright_years;
	if (!cylp)
		return;
	for (j = 0; j < cylp->length; ++j)
	{
		if (*alen_p >= amax)
			return;
		n = cylp->list[j];
		for (k = 0; k < *alen_p; ++k)
			if (a[k] == n)
				break;
		if (k >= *alen_p)
		{
			a[*alen_p] = n;
			++*alen_p;
		}
	}
}


static int change_copyright_years_cmp _((const void *, const void *));

static int
change_copyright_years_cmp(va, vb)
	const void	*va;
	const void	*vb;
{
	const int	*a;
	const int	*b;

	a = va;
	b = vb;
	return (*a - *b);
}


void
change_copyright_years_get(cp, a, amax, alen_p)
	change_ty	*cp;
	int		*a;
	int		amax;
	int		*alen_p;
{
	project_ty	*pp;

	/*
	 * Get the years specific to this change.
	 */
	assert(alen_p);
	*alen_p = 0;
	change_copyright_years_slurp(cp, a, amax, alen_p);

	/*
	 * Walk up the list of ancestors until we get to the trunk
	 * extracting the years specific to each branch.
	 */
	for (pp = cp->pp; pp; pp = pp->parent)
	{
		change_copyright_years_slurp
		(
			project_change_get(pp),
			a,
			amax,
			alen_p
		);
	}

	/*
	 * sort the years into ascending order
	 */
	if (*alen_p >= 2)
		qsort(a, *alen_p, sizeof(a[0]), change_copyright_years_cmp);
}


void
change_branch_history_new(cp, delta_number, change_number)
	change_ty	*cp;
	long		delta_number;
	long		change_number;
{
	cstate		cstate_data;
	cstate_branch_history hp;
	cstate_branch_history *hpp;
	type_ty		*type_p;

	trace(("change_branch_history_new(cp = %8.8lX, delta_number = %ld, \
change_number = %ld)\n{\n"/*}*/, (long)cp, delta_number, change_number));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->history)
		cstate_data->branch->history =
			cstate_branch_history_list_type.alloc();
	hpp =
		cstate_branch_history_list_type.list_parse
		(
			cstate_data->branch->history,
			&type_p
		);
	assert(type_p == &cstate_branch_history_type);
	hp = cstate_branch_history_type.alloc();
	*hpp = hp;
	hp->delta_number = delta_number;
	hp->change_number = change_number;
	trace((/*{*/"}\n"));
}


int
change_branch_history_nth(cp, n, cnp, dnp, name)
	change_ty	*cp;
	long		n;
	long		*cnp;
	long		*dnp;
	string_list_ty		*name;
{
	cstate		cstate_data;
	cstate_branch_history_list lp;
	int		result;

	trace(("change_branch_history_nth(cp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)cp, n));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->history)
		cstate_data->branch->history =
			cstate_branch_history_list_type.alloc();
	lp = cstate_data->branch->history;
	if (n < 0 || n >= lp->length)
		result = 0;
	else
	{
		cstate_branch_history hp;
		hp = lp->list[n];
		*cnp = hp->change_number;
		*dnp = hp->delta_number;
		string_list_constructor(name);
		if (hp->name)
		{
			size_t		j;

			for (j = 0; j < hp->name->length; ++j)
				string_list_append(name, hp->name->list[j]);
		}
		result = 1;
	}
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


void
change_branch_administrator_add(cp, user_name)
	change_ty	*cp;
	string_ty	*user_name;
{
	cstate		cstate_data;
	type_ty		*type_p;
	string_ty	**spp;
	cstate_branch_administrator_list lp;
	size_t		j;

	trace(("change_branch_administrator_add(cp = %8.8lX, user_name = \
\"%s\")\n{\n"/*}*/, (long)cp, user_name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->administrator)
		cstate_data->branch->administrator =
			cstate_branch_administrator_list_type.alloc();
	lp = cstate_data->branch->administrator;

	/*
	 * make sure we don't have her already
	 */
	for (j = 0; j < lp->length; ++j)
	{
		if (str_equal(user_name, lp->list[j]))
		{
			trace((/*{*/"}\n"));
			return;
		}
	}

	/*
	 * append her to the list
	 */
	spp =
		cstate_branch_administrator_list_type.list_parse
		(
			cstate_data->branch->administrator,
			&type_p
		);
	assert(type_p == &string_type);
	*spp = str_copy(user_name);
	trace((/*{*/"}\n"));
}


void
change_branch_administrator_remove(cp, user_name)
	change_ty	*cp;
	string_ty	*user_name;
{
	cstate		cstate_data;
	cstate_branch_administrator_list lp;
	size_t		j;

	trace(("change_branch_administrator_remove(cp = %8.8lX, user_name = \
\"%s\")\n{\n"/*}*/, (long)cp, user_name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->administrator)
		cstate_data->branch->administrator =
			cstate_branch_administrator_list_type.alloc();
	lp = cstate_data->branch->administrator;

	/*
	 * Remove the name from the list, if it is on the list.
	 * Be conservative, look for duplicates.
	 */
	for (j = 0; j < lp->length; ++j)
	{
		if (str_equal(user_name, lp->list[j]))
		{
			size_t		k;

			str_free(lp->list[j]);
			for (k = j + 1; k < lp->length; ++k)
				lp->list[k - 1] = lp->list[k];
			lp->length--;
			j--;
		}
	}
	trace((/*{*/"}\n"));
}


string_ty *
change_branch_administrator_nth(cp, n)
	change_ty	*cp;
	long		n;
{
	cstate		cstate_data;
	cstate_branch_administrator_list lp;
	string_ty	*result;

	trace(("change_branch_administrator_nth(cp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)cp, n));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->administrator)
		cstate_data->branch->administrator =
			cstate_branch_administrator_list_type.alloc();
	lp = cstate_data->branch->administrator;

	if (n < 0 || n >= lp->length)
		result = 0;
	else
		result = lp->list[n];
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


int
change_branch_administrator_query(cp, name)
	change_ty	*cp;
	string_ty	*name;
{
	cstate		cstate_data;
	cstate_branch_administrator_list lp;
	size_t		j;

	trace(("change_branch_administrator_query(cp = %8.8lX, name = \"%s\")\n{\n"/*}*/,
		(long)cp, name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->administrator)
		cstate_data->branch->administrator =
			cstate_branch_administrator_list_type.alloc();
	lp = cstate_data->branch->administrator;
	for (j = 0; j < lp->length; ++j)
	{
		if (str_equal(lp->list[j], name))
		{
			trace(("return 1;\n"));
			trace((/*{*/"}\n"));
			return 1;
		}
	}
	trace(("return 0;\n"));
	trace((/*{*/"}\n"));
	return 0;
}


void
change_branch_developer_add(cp, user_name)
	change_ty	*cp;
	string_ty	*user_name;
{
	cstate		cstate_data;
	type_ty		*type_p;
	string_ty	**spp;
	cstate_branch_developer_list lp;
	size_t		j;

	trace(("change_branch_developer_add(cp = %8.8lX, user_name = \
\"%s\")\n{\n"/*}*/, (long)cp, user_name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->developer)
		cstate_data->branch->developer =
			cstate_branch_developer_list_type.alloc();
	lp = cstate_data->branch->developer;

	/*
	 * make sure we don't have her already
	 */
	for (j = 0; j < lp->length; ++j)
	{
		if (str_equal(user_name, lp->list[j]))
		{
			trace((/*{*/"}\n"));
			return;
		}
	}

	/*
	 * append her to the list
	 */
	spp =
		cstate_branch_developer_list_type.list_parse
		(
			cstate_data->branch->developer,
			&type_p
		);
	assert(type_p == &string_type);
	*spp = str_copy(user_name);
	trace((/*{*/"}\n"));
}


void
change_branch_developer_remove(cp, user_name)
	change_ty	*cp;
	string_ty	*user_name;
{
	cstate		cstate_data;
	cstate_branch_developer_list lp;
	size_t		j;

	trace(("change_branch_developer_remove(cp = %8.8lX, user_name = \
\"%s\")\n{\n"/*}*/, (long)cp, user_name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->developer)
		cstate_data->branch->developer =
			cstate_branch_developer_list_type.alloc();
	lp = cstate_data->branch->developer;

	/*
	 * Remove the name from the list, if it is on the list.
	 * Be conservative, look for duplicates.
	 */
	for (j = 0; j < lp->length; ++j)
	{
		if (str_equal(user_name, lp->list[j]))
		{
			size_t		k;

			str_free(lp->list[j]);
			for (k = j + 1; k < lp->length; ++k)
				lp->list[k - 1] = lp->list[k];
			lp->length--;
			j--;
		}
	}
	trace((/*{*/"}\n"));
}


string_ty *
change_branch_developer_nth(cp, n)
	change_ty	*cp;
	long		n;
{
	cstate		cstate_data;
	cstate_branch_developer_list lp;
	string_ty	*result;

	trace(("change_branch_developer_nth(cp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)cp, n));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->developer)
		cstate_data->branch->developer =
			cstate_branch_developer_list_type.alloc();
	lp = cstate_data->branch->developer;

	if (n < 0 || n >= lp->length)
		result = 0;
	else
		result = lp->list[n];
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


int
change_branch_developer_query(cp, name)
	change_ty	*cp;
	string_ty	*name;
{
	cstate		cstate_data;
	cstate_branch_developer_list lp;
	size_t		j;

	trace(("change_branch_developer_query(cp = %8.8lX, name = \"%s\")\n{\n"/*}*/,
		(long)cp, name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->developer)
		cstate_data->branch->developer =
			cstate_branch_developer_list_type.alloc();
	lp = cstate_data->branch->developer;
	for (j = 0; j < lp->length; ++j)
	{
		if (str_equal(lp->list[j], name))
		{
			trace(("return 1;\n"));
			trace((/*{*/"}\n"));
			return 1;
		}
	}
	trace(("return 0;\n"));
	trace((/*{*/"}\n"));
	return 0;
}


void
change_branch_reviewer_add(cp, user_name)
	change_ty	*cp;
	string_ty	*user_name;
{
	cstate		cstate_data;
	type_ty		*type_p;
	string_ty	**spp;
	cstate_branch_reviewer_list lp;
	size_t		j;

	trace(("change_branch_reviewer_add(cp = %8.8lX, user_name = \
\"%s\")\n{\n"/*}*/, (long)cp, user_name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->reviewer)
		cstate_data->branch->reviewer =
			cstate_branch_reviewer_list_type.alloc();
	lp = cstate_data->branch->reviewer;

	/*
	 * make sure we don't have her already
	 */
	for (j = 0; j < lp->length; ++j)
	{
		if (str_equal(user_name, lp->list[j]))
		{
			trace((/*{*/"}\n"));
			return;
		}
	}

	/*
	 * append her to the list
	 */
	spp =
		cstate_branch_reviewer_list_type.list_parse
		(
			cstate_data->branch->reviewer,
			&type_p
		);
	assert(type_p == &string_type);
	*spp = str_copy(user_name);
	trace((/*{*/"}\n"));
}


void
change_branch_reviewer_remove(cp, user_name)
	change_ty	*cp;
	string_ty	*user_name;
{
	cstate		cstate_data;
	cstate_branch_reviewer_list lp;
	size_t		j;

	trace(("change_branch_reviewer_remove(cp = %8.8lX, user_name = \
\"%s\")\n{\n"/*}*/, (long)cp, user_name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->reviewer)
		cstate_data->branch->reviewer =
			cstate_branch_reviewer_list_type.alloc();
	lp = cstate_data->branch->reviewer;

	/*
	 * Remove the name from the list, if it is on the list.
	 * Be conservative, look for duplicates.
	 */
	for (j = 0; j < lp->length; ++j)
	{
		if (str_equal(user_name, lp->list[j]))
		{
			size_t		k;

			str_free(lp->list[j]);
			for (k = j + 1; k < lp->length; ++k)
				lp->list[k - 1] = lp->list[k];
			lp->length--;
			j--;
		}
	}
	trace((/*{*/"}\n"));
}


string_ty *
change_branch_reviewer_nth(cp, n)
	change_ty	*cp;
	long		n;
{
	cstate		cstate_data;
	cstate_branch_reviewer_list lp;
	string_ty	*result;

	trace(("change_branch_reviewer_nth(cp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)cp, n));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->reviewer)
		cstate_data->branch->reviewer =
			cstate_branch_reviewer_list_type.alloc();
	lp = cstate_data->branch->reviewer;

	if (n < 0 || n >= lp->length)
		result = 0;
	else
		result = lp->list[n];
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


int
change_branch_reviewer_query(cp, name)
	change_ty	*cp;
	string_ty	*name;
{
	cstate		cstate_data;
	cstate_branch_reviewer_list lp;
	size_t		j;

	trace(("change_branch_reviewer_query(cp = %8.8lX, name = \"%s\")\n{\n"/*}*/,
		(long)cp, name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->reviewer)
		cstate_data->branch->reviewer =
			cstate_branch_reviewer_list_type.alloc();
	lp = cstate_data->branch->reviewer;
	for (j = 0; j < lp->length; ++j)
	{
		if (str_equal(lp->list[j], name))
		{
			trace(("return 1;\n"));
			trace((/*{*/"}\n"));
			return 1;
		}
	}
	trace(("return 0;\n"));
	trace((/*{*/"}\n"));
	return 0;
}


void
change_branch_integrator_add(cp, user_name)
	change_ty	*cp;
	string_ty	*user_name;
{
	cstate		cstate_data;
	type_ty		*type_p;
	string_ty	**spp;
	cstate_branch_integrator_list lp;
	size_t		j;

	trace(("change_branch_integrator_add(cp = %8.8lX, user_name = \
\"%s\")\n{\n"/*}*/, (long)cp, user_name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->integrator)
		cstate_data->branch->integrator =
			cstate_branch_integrator_list_type.alloc();
	lp = cstate_data->branch->integrator;

	/*
	 * make sure we don't have her already
	 */
	for (j = 0; j < lp->length; ++j)
	{
		if (str_equal(user_name, lp->list[j]))
		{
			trace((/*{*/"}\n"));
			return;
		}
	}

	/*
	 * append her to the list
	 */
	spp =
		cstate_branch_integrator_list_type.list_parse
		(
			cstate_data->branch->integrator,
			&type_p
		);
	assert(type_p == &string_type);
	*spp = str_copy(user_name);
	trace((/*{*/"}\n"));
}


void
change_branch_integrator_remove(cp, user_name)
	change_ty	*cp;
	string_ty	*user_name;
{
	cstate		cstate_data;
	cstate_branch_integrator_list lp;
	size_t		j;

	trace(("change_branch_integrator_remove(cp = %8.8lX, user_name = \
\"%s\")\n{\n"/*}*/, (long)cp, user_name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->integrator)
		cstate_data->branch->integrator =
			cstate_branch_integrator_list_type.alloc();
	lp = cstate_data->branch->integrator;

	/*
	 * Remove the name from the list, if it is on the list.
	 * Be conservative, look for duplicates.
	 */
	for (j = 0; j < lp->length; ++j)
	{
		if (str_equal(user_name, lp->list[j]))
		{
			size_t		k;

			str_free(lp->list[j]);
			for (k = j + 1; k < lp->length; ++k)
				lp->list[k - 1] = lp->list[k];
			lp->length--;
			j--;
		}
	}
	trace((/*{*/"}\n"));
}


string_ty *
change_branch_integrator_nth(cp, n)
	change_ty	*cp;
	long		n;
{
	cstate		cstate_data;
	cstate_branch_integrator_list lp;
	string_ty	*result;

	trace(("change_branch_integrator_nth(cp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)cp, n));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->integrator)
		cstate_data->branch->integrator =
			cstate_branch_integrator_list_type.alloc();
	lp = cstate_data->branch->integrator;

	if (n < 0 || n >= lp->length)
		result = 0;
	else
		result = lp->list[n];
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


int
change_branch_integrator_query(cp, name)
	change_ty	*cp;
	string_ty	*name;
{
	cstate		cstate_data;
	cstate_branch_integrator_list lp;
	size_t		j;

	trace(("change_branch_integrator_query(cp = %8.8lX, name = \"%s\")\n{\n"/*}*/,
		(long)cp, name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->integrator)
		cstate_data->branch->integrator =
			cstate_branch_integrator_list_type.alloc();
	lp = cstate_data->branch->integrator;
	for (j = 0; j < lp->length; ++j)
	{
		if (str_equal(lp->list[j], name))
		{
			trace(("return 1;\n"));
			trace((/*{*/"}\n"));
			return 1;
		}
	}
	trace(("return 0;\n"));
	trace((/*{*/"}\n"));
	return 0;
}


void
change_branch_change_add(cp, change_number, is_a_branch)
	change_ty	*cp;
	long		change_number;
	int		is_a_branch;
{
	cstate		cstate_data;
	type_ty		*type_p;
	long		*addr;
	cstate_branch_change_list lp;
	size_t		j;

	/*
	 * add it to the change list
	 */
	trace(("change_branch_change_add(cp = %8.8lX, change_number = %ld)\n{\n"/*}*/,
		(long)cp, change_number));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->change)
		cstate_data->branch->change =
			cstate_branch_change_list_type.alloc();
	lp = cstate_data->branch->change;

	/*
	 * make sure we don't have it already
	 */
	for (j = 0; j < lp->length; ++j)
		if (change_number == lp->list[j])
			break;
	/*
	 * append it to the list
	 */
	if (j >= lp->length)
	{
		addr =
			cstate_branch_change_list_type.list_parse
			(
				cstate_data->branch->change,
				&type_p
			);
		assert(type_p == &integer_type);
		*addr = change_number;
	}

	if (is_a_branch)
	{
		cstate_branch_sub_branch_list lp2;

		/*
		 * add it to the change list
		 */
		if (!cstate_data->branch->sub_branch)
			cstate_data->branch->sub_branch =
				cstate_branch_sub_branch_list_type.alloc();
		lp2 = cstate_data->branch->sub_branch;
	
		/*
		 * make sure we don't have it already
		 */
		for (j = 0; j < lp2->length; ++j)
			if (change_number == lp2->list[j])
				break;
		/*
		 * append it to the list
		 */
		if (j >= lp2->length)
		{
			addr =
				cstate_branch_change_list_type.list_parse
				(
				 cstate_data->branch->sub_branch,
					&type_p
				);
			assert(type_p == &integer_type);
			*addr = change_number;
		}
	}
	trace((/*{*/"}\n"));
}


void
change_branch_change_remove(cp, change_number)
	change_ty	*cp;
	long		change_number;
{
	cstate		cstate_data;
	cstate_branch_change_list lp;
	size_t		j;

	trace(("change_branch_change_remove(cp = %8.8lX, change_number = %ld)\n{\n"/*}*/,
		(long)cp, change_number));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->change)
		cstate_data->branch->change =
			cstate_branch_change_list_type.alloc();

	/*
	 * Remove the name from the list, if it is on the list.
	 * Be conservative, look for duplicates.
	 */
	lp = cstate_data->branch->change;
	for (j = 0; j < lp->length; ++j)
	{
		if (change_number == lp->list[j])
		{
			size_t		k;

			for (k = j + 1; k < lp->length; ++k)
				lp->list[k - 1] = lp->list[k];
			lp->length--;
			j--;
		}
	}

	if (cstate_data->branch->sub_branch)
	{
		cstate_branch_sub_branch_list lp2;
	
		/*
		 * Remove the name from the list, if it is on the list.
		 * Be conservative, look for duplicates.
		 */
		lp2 = cstate_data->branch->sub_branch;
		for (j = 0; j < lp2->length; ++j)
		{
			if (change_number == lp2->list[j])
			{
				size_t		k;
	
				for (k = j + 1; k < lp2->length; ++k)
					lp2->list[k - 1] = lp2->list[k];
				lp2->length--;
				j--;
			}
		}
	}
	trace((/*{*/"}\n"));
}


void
change_branch_sub_branch_list_get(cp, list, len)
	change_ty	*cp;
	long		**list;
	size_t		*len;
{
	cstate		cstate_data;
	cstate_branch_sub_branch_list lp;

	trace(("change_branch_list_get(cp = %8.8lX)\n{\n"/*}*/, (long)cp));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	lp = cstate_data->branch->sub_branch;
	if (!lp)
	{
		*list = 0;
		*len = 0;
	}
	else
	{
		*list = lp->list;
		*len = lp->length;
	}
	trace((/*{*/"}\n"));
}


int
change_branch_change_nth(cp, n, cnp)
	change_ty	*cp;
	long		n;
	long		*cnp;
{
	cstate		cstate_data;
	cstate_branch_change_list lp;

	trace(("change_branch_change_nth(cp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)cp, n));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->change)
		cstate_data->branch->change =
			cstate_branch_change_list_type.alloc();
	lp = cstate_data->branch->change;

	if (n < 0 || n >= lp->length)
	{
		trace(("return FALSE;\n"));
		trace((/*{*/"}\n"));
		return 0;
	}
	assert(cnp);
	*cnp = lp->list[n];
	trace(("return %ld;\n", *cnp));
	trace((/*{*/"}\n"));
	return 1;
}


long
change_branch_minimum_change_number_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (!bp)
		return 10;
	if (bp->minimum_change_number < 1)
		bp->minimum_change_number = 10;
	return bp->minimum_change_number;
}


void
change_branch_minimum_change_number_set(cp, n)
	change_ty	*cp;
	long		n;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (n < 1)
		n = 10;
	bp->minimum_change_number = n;
}


int
change_branch_reuse_change_numbers_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (!bp)
		return 1;
	if (!(bp->mask & cstate_branch_reuse_change_numbers_mask))
	{
		bp->reuse_change_numbers = 1;
		bp->mask |= cstate_branch_reuse_change_numbers_mask;
	}
	return bp->reuse_change_numbers;
}


void
change_branch_reuse_change_numbers_set(cp, n)
	change_ty	*cp;
	int		n;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	bp->reuse_change_numbers = !!n;
	bp->mask |= cstate_branch_reuse_change_numbers_mask;
}


long
change_branch_minimum_branch_number_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (!bp)
		return 1;
	if (bp->minimum_branch_number < 1)
		bp->minimum_branch_number = 1;
	return bp->minimum_branch_number;
}


void
change_branch_minimum_branch_number_set(cp, n)
	change_ty	*cp;
	long		n;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (n < 1)
		n = 1;
	bp->minimum_branch_number = n;
}


int
change_branch_skip_unlucky_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	return (bp && bp->skip_unlucky);
}


void
change_branch_skip_unlucky_set(cp, n)
	change_ty	*cp;
	int		n;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	bp->skip_unlucky = !!n;
}


int
change_branch_compress_database_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	return (bp && bp->compress_database);
}


void
change_branch_compress_database_set(cp, n)
	change_ty	*cp;
	int		n;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	bp->compress_database = !!n;
}


long
change_branch_next_change_number(cp, is_a_change)
	change_ty	*cp;
	int		is_a_change;
{
	cstate		cstate_data;
	cstate_branch_change_list lp;
	long		change_number;
	interval_ty	*ip1;
	interval_ty	*ip2;
	interval_ty	*ip3;
	size_t		j, k;
	long		min;
	int		reuse;

	trace(("change_branch_next_change_number(cp = %8.8lX)\n{\n"/*}*/,
		(long)cp));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->change)
		cstate_data->branch->change =
			cstate_branch_change_list_type.alloc();
	lp = cstate_data->branch->change;

	/*
	 * build an interval which contains the set of changes
	 * (it could be empty)
	 */
	ip1 = interval_create_empty();
	for (j = 0; j < lp->length; )
	{
		for (k = j + 1; k < lp->length; ++k)
			if (lp->list[k - 1] + 1 != lp->list[k])
				break;
		ip2 = interval_create_range(lp->list[j], lp->list[k - 1]);
		ip3 = interval_union(ip1, ip2);
		interval_free(ip1);
		interval_free(ip2);
		ip1 = ip3;
		j = k;
	}

	/*
	 * difference the above set from the interval [min..max)
	 * This avoids zero.  If the user wants a zero-numbered change,
	 * she must ask for it.
	 */
	if (is_a_change)
		min = change_branch_minimum_change_number_get(cp);
	else
		min = change_branch_minimum_branch_number_get(cp);
	ip2 = interval_create_range(min, TRUNK_CHANGE_NUMBER - 1);
	ip3 = interval_difference(ip2, ip1);
	interval_free(ip1);
	interval_free(ip2);
	assert(ip3->length);

	/*
	 * The result is the minimum acceptable number in the interval.
	 * But !reuse means the first number in the last interval.
	 */
	reuse = !is_a_change || change_branch_reuse_change_numbers_get(cp);
	if (!change_branch_skip_unlucky_get(cp))
	{
		if (reuse)
			change_number = ip3->data[0];
		else
			change_number = ip3->data[ip3->length - 2];
	}
	else
	{
		for (;;)
		{
			if (reuse)
				change_number = ip3->data[0];
			else
				change_number = ip3->data[ip3->length - 2];
			min = skip_unlucky(change_number);
			if (min == change_number)
				break;
			ip1 = interval_create_range(change_number, min - 1);
			ip2 = interval_difference(ip3, ip1);
			interval_free(ip3);
			interval_free(ip1);
			ip3 = ip2;
		}
	}
	interval_free(ip3);
	trace(("return %ld;\n", change_number));
	trace((/*{*/"}\n"));
	return change_number;
}


int
change_branch_change_number_in_use(cp, cn)
	change_ty	*cp;
	long		cn;
{
	int		result;
	cstate		cstate_data;
	cstate_branch_change_list lp;
	size_t		j;

	trace(("change_branch_change_number_in_use(cp = %8.8lX, cn = %ld)\n{\n"/*}*/, (long)cp, cn));
	result = 0;
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	lp = cstate_data->branch->change;
	if (lp)
	{
		for (j = 0; j < lp->length; ++j)
		{
			if (lp->list[j] == cn)
			{
				result = 1;
				break;
			}
		}
	}
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	change_branch_delta_to_edit
 *
 * SYNOPSIS
 *	string_ty *change_branch_delta_to_edit(project_ty *pp, long delta_number,
 *		string_ty *file_name);
 *
 * DESCRIPTION
 *	The change_branch_delta_to_edit function is used to map a delta number
 *	into a specific edit number for a project source file.  This requires
 *	roll-forward of the edits to the named file, until the relevant
 *	delta is reached.
 *
 * ARGUMENTS
 *	cp		- branch the file is in
 *	delta_number	- delta number wanted
 *	file_name	- name of file
 *
 * RETURNS
 *	string_ty *;	string containing edit number,
 *			NULL if file does not exist at this delta.
 *
 * CAVEAT
 *	It is the caller's responsibility to free the string returned
 *	when not futher required.
 */

string_ty *
change_branch_delta_to_edit(pp, delta_number, file_name)
	project_ty	*pp;
	long		delta_number;
	string_ty	*file_name;
{
	change_ty	*cp;
	string_ty	*edit_number;
	cstate		cstate_data;
	cstate_branch	bp;
	long		j;

	cp = project_change_get(pp);
	trace(("change_branch_delta_to_edit(cp = %08lX, delta_number = %ld)\n{\n"/*}*/, cp, delta_number));
	edit_number = 0;
	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	for (j = 0; j < bp->history->length; ++j)
	{
		cstate_branch_history hp;
		change_ty	*cp2;
		fstate_src	src_data;

		hp = bp->history->list[j];
		cp2 = change_alloc(pp, hp->change_number);
		src_data = change_file_find(cp2, file_name);
		if (src_data)
		{
			if (src_data->action == file_action_remove)
			{
				if (edit_number)
					str_free(edit_number);
				edit_number = 0;
			}
			else
			{
				/*
				 * there should always be an edit number,
				 * because it is checked for when the file
				 * is read in.
				 */
				assert(src_data->edit_number);
				if (edit_number)
					str_free(edit_number);
				edit_number = str_copy(src_data->edit_number);
			}
		}
		change_free(cp2);
		if (hp->delta_number == delta_number)
			break;
	}
	trace(("return %s;\n", edit_number ? edit_number->str_text : "NULL"));
	trace((/*{*/"}\n"));
	return edit_number;
}


/*
 * NAME
 *	change_branch_delta_to_edit
 *
 * SYNOPSIS
 *	string_ty *change_branch_delta_to_edit(project_ty *pp, long delta_number,
 *		string_ty *file_name);
 *
 * DESCRIPTION
 *	The change_branch_delta_to_edit function is used to map a delta number
 *	into a specific edit number for a project source file.  This requires
 *	roll-forward of the edits to the named file, until the relevant
 *	delta is reached.
 *
 * ARGUMENTS
 *	cp		- branch the file is in
 *	delta_number	- delta number wanted
 *	file_name	- name of file
 *
 * RETURNS
 *	string_ty *;	string containing edit number,
 *			NULL if file does not exist at this delta.
 *
 * CAVEAT
 *	It is the caller's responsibility to free the string returned
 *	when not futher required.
 */

string_ty *
change_branch_delta_date_to_edit(pp, delta_date, file_name)
	project_ty	*pp;
	time_t		delta_date;
	string_ty	*file_name;
{
	change_ty	*cp;
	string_ty	*edit_number;
	cstate		cstate_data;
	cstate_branch	bp;
	long		j;
	int		first_time_seen;

	trace(("change_branch_delta_to_edit(pp = %08lX, delta_date = %ld)\n{\n"/*}*/, (long)pp, (long)delta_date));
	cp = project_change_get(pp);
	edit_number = 0;
	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	first_time_seen = 1;
	if (!bp->history)
		bp->history = cstate_branch_history_list_type.alloc();
	for (j = 0; j < bp->history->length; ++j)
	{
		cstate_branch_history hp;
		change_ty	*cp2;
		time_t		when;
		fstate_src	src_data;

		hp = bp->history->list[j];
		cp2 = change_alloc(pp, hp->change_number);
		change_bind_existing(cp2);
		when = change_completion_timestamp(cp2);
		src_data = change_file_find(cp2, file_name);
		if (src_data)
		{
			if
			(
				when > delta_date
			&&
				first_time_seen
			&&
				src_data->action != file_action_create
			&&
				pp->parent
			)
			{
				if (edit_number)
					str_free(edit_number);
				edit_number =
					change_branch_delta_date_to_edit
					(
						pp->parent,
						delta_date,
						file_name
					);
			}
			else if (when <= delta_date)
			{
				if (edit_number)
					str_free(edit_number);
				if (src_data->action == file_action_remove)
					edit_number = 0;
				else
				{
					/*
					 * there should always be an edit number,
					 * because it is checked for when the
					 * file is read in.
					 */
					assert(src_data->edit_number);
					edit_number = str_copy(src_data->edit_number);
				}
			}
			first_time_seen = 0;
		}
		change_free(cp2);
		if (when > delta_date)
			break;
	}

	/*
	 * If we have not seen this file at all in this branch, try the
	 * parent branch.
	 */
	if (first_time_seen && pp->parent)
	{
		assert(!edit_number);
		edit_number =
			change_branch_delta_date_to_edit
			(
				pp->parent,
				delta_date,
				file_name
			);
	}

	/*
	 * report results
	 */
	trace(("return %s;\n", edit_number ? edit_number->str_text : "NULL"));
	trace((/*{*/"}\n"));
	return edit_number;
}


void
change_branch_umask_set(cp, n)
	change_ty	*cp;
	int		n;
{
	cstate		cstate_data;
	cstate_branch	bp;

	n = (n & 5) | 022;
	if (n == 023)
		n = 022;
	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	bp->umask = n;
}


int
change_branch_umask_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	return bp->umask;
}


void
change_branch_developer_may_review_set(cp, n)
	change_ty	*cp;
	int		n;
{
	cstate		cstate_data;
	cstate_branch	bp;

	trace(("change_branch_developer_may_review(cp = %8.8lX, n = %d)\n{\n"/*}*/,
		(long)cp, n));
	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	bp->developer_may_review = n;
	trace((/*{*/"}\n"));
}


int
change_branch_developer_may_review_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	return bp->developer_may_review;
}


void
change_branch_developer_may_integrate_set(cp, n)
	change_ty	*cp;
	int		n;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	bp->developer_may_integrate = n;
}


int
change_branch_developer_may_integrate_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	return bp->developer_may_integrate;
}


void
change_branch_reviewer_may_integrate_set(cp, n)
	change_ty	*cp;
	int		n;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	bp->reviewer_may_integrate = n;
}


int
change_branch_reviewer_may_integrate_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	return bp->reviewer_may_integrate;
}


void
change_branch_developers_may_create_changes_set(cp, n)
	change_ty	*cp;
	int		n;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	bp->developers_may_create_changes = n;
}


int
change_branch_developers_may_create_changes_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	return bp->developers_may_create_changes;
}


void
change_branch_forced_develop_begin_notify_command_set(cp, s)
	change_ty	*cp;
	string_ty	*s;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (bp->forced_develop_begin_notify_command)
	{
		str_free(bp->forced_develop_begin_notify_command);
		bp->forced_develop_begin_notify_command = 0;
	}
	if (s)
		bp->forced_develop_begin_notify_command = str_copy(s);
}


string_ty *
change_branch_forced_develop_begin_notify_command_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if
	(
		bp->forced_develop_begin_notify_command
	&&
		!bp->forced_develop_begin_notify_command->str_length
	)
	{
		str_free(bp->forced_develop_begin_notify_command);
		bp->forced_develop_begin_notify_command = 0;
	}
	return bp->forced_develop_begin_notify_command;
}


void
change_branch_develop_end_notify_command_set(cp, s)
	change_ty	*cp;
	string_ty	*s;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (bp->develop_end_notify_command)
	{
		str_free(bp->develop_end_notify_command);
		bp->develop_end_notify_command = 0;
	}
	if (s && s->str_length)
		bp->develop_end_notify_command = str_copy(s);
}


string_ty *
change_branch_develop_end_notify_command_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if
	(
		bp->develop_end_notify_command
	&&
		!bp->develop_end_notify_command->str_length
	)
	{
		str_free(bp->develop_end_notify_command);
		bp->develop_end_notify_command = 0;
	}
	return bp->develop_end_notify_command;
}


void
change_branch_develop_end_undo_notify_command_set(cp, s)
	change_ty	*cp;
	string_ty	*s;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (bp->develop_end_undo_notify_command)
	{
		str_free(bp->develop_end_undo_notify_command);
		bp->develop_end_undo_notify_command = 0;
	}
	if (s && s->str_length)
		bp->develop_end_undo_notify_command = str_copy(s);
}


string_ty *
change_branch_develop_end_undo_notify_command_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if
	(
		bp->develop_end_undo_notify_command
	&&
		!bp->develop_end_undo_notify_command->str_length
	)
	{
		str_free(bp->develop_end_undo_notify_command);
		bp->develop_end_undo_notify_command = 0;
	}
	return bp->develop_end_undo_notify_command;
}


void
change_branch_review_pass_notify_command_set(cp, s)
	change_ty	*cp;
	string_ty	*s;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (bp->review_pass_notify_command)
	{
		str_free(bp->review_pass_notify_command);
		bp->review_pass_notify_command = 0;
	}
	if (s && s->str_length)
		bp->review_pass_notify_command = str_copy(s);
}


string_ty *
change_branch_review_pass_notify_command_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if
	(
		bp->review_pass_notify_command
	&&
		!bp->review_pass_notify_command->str_length
	)
	{
		str_free(bp->review_pass_notify_command);
		bp->review_pass_notify_command = 0;
	}
	return bp->review_pass_notify_command;
}


void
change_branch_review_pass_undo_notify_command_set(cp, s)
	change_ty	*cp;
	string_ty	*s;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (bp->review_pass_undo_notify_command)
	{
		str_free(bp->review_pass_undo_notify_command);
		bp->review_pass_undo_notify_command = 0;
	}
	if (s && s->str_length)
		bp->review_pass_undo_notify_command = str_copy(s);
}


string_ty *
change_branch_review_pass_undo_notify_command_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if
	(
		bp->review_pass_undo_notify_command
	&&
		!bp->review_pass_undo_notify_command->str_length
	)
	{
		str_free(bp->review_pass_undo_notify_command);
		bp->review_pass_undo_notify_command = 0;
	}
	return bp->review_pass_undo_notify_command;
}


void
change_branch_review_fail_notify_command_set(cp, s)
	change_ty	*cp;
	string_ty	*s;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (bp->review_fail_notify_command)
	{
		str_free(bp->review_fail_notify_command);
		bp->review_fail_notify_command = 0;
	}
	if (s && s->str_length)
		bp->review_fail_notify_command = str_copy(s);
}


string_ty *
change_branch_review_fail_notify_command_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if
	(
		bp->review_fail_notify_command
	&&
		!bp->review_fail_notify_command->str_length
	)
	{
		str_free(bp->review_fail_notify_command);
		bp->review_fail_notify_command = 0;
	}
	return bp->review_fail_notify_command;
}


void
change_branch_integrate_pass_notify_command_set(cp, s)
	change_ty	*cp;
	string_ty	*s;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (bp->integrate_pass_notify_command)
	{
		str_free(bp->integrate_pass_notify_command);
		bp->integrate_pass_notify_command = 0;
	}
	if (s && s->str_length)
		bp->integrate_pass_notify_command = str_copy(s);
}


string_ty *
change_branch_integrate_pass_notify_command_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if
	(
		bp->integrate_pass_notify_command
	&&
		!bp->integrate_pass_notify_command->str_length
	)
	{
		str_free(bp->integrate_pass_notify_command);
		bp->integrate_pass_notify_command = 0;
	}
	return bp->integrate_pass_notify_command;
}


void
change_branch_integrate_fail_notify_command_set(cp, s)
	change_ty	*cp;
	string_ty	*s;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (bp->integrate_fail_notify_command)
	{
		str_free(bp->integrate_fail_notify_command);
		bp->integrate_fail_notify_command = 0;
	}
	if (s && s->str_length)
		bp->integrate_fail_notify_command = str_copy(s);
}


string_ty *
change_branch_integrate_fail_notify_command_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if
	(
		bp->integrate_fail_notify_command
	&&
		!bp->integrate_fail_notify_command->str_length
	)
	{
		str_free(bp->integrate_fail_notify_command);
		bp->integrate_fail_notify_command = 0;
	}
	return bp->integrate_fail_notify_command;
}


void
change_branch_default_development_directory_set(cp, s)
	change_ty	*cp;
	string_ty	*s;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	if (bp->default_development_directory)
	{
		str_free(bp->default_development_directory);
		bp->default_development_directory = 0;
	}
	if (s)
		bp->default_development_directory = str_copy(s);
}


string_ty *
change_branch_default_development_directory_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	return bp->default_development_directory;
}


void
change_branch_default_test_exemption_set(cp, n)
	change_ty	*cp;
	int		n;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	bp->default_test_exemption = n;
}


int
change_branch_default_test_exemption_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_branch	bp;

	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	return bp->default_test_exemption;
}


int
change_is_a_branch(cp)
	change_ty	*cp;
{
	cstate		cstate_data;

	cstate_data = change_cstate_get(cp);
	return
	(
		cstate_data->branch != 0
	&&
		cstate_data->state == cstate_state_being_developed
	);
}


int
change_was_a_branch(cp)
	change_ty	*cp;
{
	cstate		cstate_data;

	cstate_data = change_cstate_get(cp);
	return (cstate_data->branch != 0);
}


string_ty *
change_version_get(cp)
	change_ty	*cp;
{
	string_ty	*s1;
	string_ty	*s2;
	cstate		cstate_data;

	s1 = project_version_short_get(cp->pp);
	cstate_data = change_cstate_get(cp);
	if (cstate_data->state < cstate_state_being_integrated)
		s2 = str_format("%S.C%3.3ld", s1, cp->number);
	else
		s2 = str_format("%S.D%3.3ld", s1, cstate_data->delta_number);
	/* not not free s1 */
	return s2;
}
