/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate changes
 */

#include <ac/ctype.h>

#include <change.h>
#include <cstate.h>
#include <str.h>
#include <sub.h>
#include <sub/change/number.h>
#include <symtab.h>
#include <trace.h>
#include <wstr.h>
#include <wstr_list.h>


typedef string_ty *(*func_ptr)_((change_ty *));

typedef struct table_ty table_ty;
struct table_ty
{
	char		*name;
	func_ptr	func;
};


static string_ty *change_number_get _((change_ty *));

static string_ty *
change_number_get(cp)
	change_ty	*cp;
{
	return str_format("%ld", magic_zero_decode(cp->number));
}


static string_ty *change_description_get _((change_ty *));

static string_ty *
change_description_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	string_ty	*s;
	size_t		len;

	/*
	 * We don't actually return the whole brief description, just the
	 * first line, or the first 80 characters, whichever is shortest.
	 */
	cstate_data = change_cstate_get(cp);
	s = cstate_data->brief_description;
	for (len = 0; len < s->str_length && len < 80; ++len)
		if (s->str_text[len] == '\n')
			break;
	return str_n_from_c(s->str_text, len);
}


static table_ty table[] =
{
	{ "brief_description", change_description_get, },
	{ "description",       change_description_get, },
	{ "number",            change_number_get,      },
};


static symtab_ty *stp;


static func_ptr find_func _((string_ty *));

static func_ptr
find_func(name)
	string_ty	*name;
{
	table_ty	*tp;
	string_ty	*s;
	func_ptr	result;

	if (!stp)
	{
		stp = symtab_alloc(SIZEOF(table));
		for (tp = table; tp < ENDOF(table); ++tp)
		{
			s = str_from_c(tp->name);
			symtab_assign(stp, s, tp->func);
			str_free(s);
		}
	}
	result = symtab_query(stp, name);
	if (!result)
	{
		s = symtab_query_fuzzy(stp, name);
		if (s)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set_string(scp, "Name", name);
			sub_var_set_string(scp, "Guess", s);
			error_intl(scp, i18n("no \"$name\", guessing \"$guess\""));
			sub_context_delete(scp);
		}
		return 0;
	}
	return result;
}


/*
 * NAME
 *	sub_change - the change substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_change(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_change function implements the change substitution.
 *	The change substitution is replaced by the change number.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_change_number(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	change_ty	*cp;
	wstring_ty	*result;
	string_ty	*s;
	func_ptr	func;

	trace(("sub_change()\n{\n"));
	cp = sub_context_change_get(scp);
	result = 0;
	if (!cp || cp->bogus)
	{
		sub_context_error_set
		(
			scp,
			i18n("not valid in current context")
		);
	}
	else if (arg->nitems == 1)
	{
		s = change_number_get(cp);
		result = str_to_wstr(s);
		str_free(s);
	}
	else if (arg->nitems == 2)
	{
		s = wstr_to_str(arg->item[1]);
		func = find_func(s);
		str_free(s);
		if (!func)
		{
			sub_context_error_set
			(
				scp,
				i18n("unknown substitution variant")
			);
		}
		else
		{
			s = func(cp);
			result = str_to_wstr(s);
			str_free(s);
		}
	}
	else
		sub_context_error_set(scp, i18n("requires one argument"));
	trace(("return %8.8lX;\n", (long)result));
	trace(("}\n"));
	return result;
}
