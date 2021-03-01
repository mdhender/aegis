/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to perform command substitutions
 */

#include <ctype.h>

#include <arglex.h>
#include <change.h>
#include <error.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <word.h>

typedef string_ty *(*fp)_((wlist *));

typedef struct table_ty table_ty;
struct table_ty
{
	char		*name;
	fp		func;
	string_ty	*value;
};

typedef struct diversion_ty diversion_ty;
struct diversion_ty
{
	long		pos;
	string_ty	*text;
	diversion_ty	*prev;
};


static	diversion_ty	*diversion;
static	char		*collect_buf;
static	size_t		collect_size;
static	size_t		collect_pos;
static	table_ty	*sub_var_list;
static	size_t		sub_var_size;
static	size_t		sub_var_pos;
static	change_ty	*cp;
static	project_ty	*pp;
static	char		*suberr;


/*
 * NAME
 *	sub_administrator_list - the administrator_list substitution
 *
 * SYNOPSIS
 *	string_ty *sub_administrator_list(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_administrator_list function implements the administrator_list
 * 	substitution.  The administrator_list substitution is replaced by
 *	a space separated list of project administrators.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_administrator_list _((wlist *));

static string_ty *
sub_administrator_list(arg)
	wlist		*arg;
{
	string_ty	*result;
	pstate		pstate_data;
	long		j;

	trace(("sub_administrator_list()\n{\n"/*}*/));
	if (arg->wl_nwords != 1)
	{
		suberr = "requires zero arguments";
		result = 0;
	}
	else
	{
		/*
		 * build a string containing all of the project administrators
		 */
		assert(pp);
		pstate_data = project_pstate_get(pp);
		assert(pstate_data->administrator);
		switch (pstate_data->administrator->length)
		{
		case 0:
			result = str_from_c("");
			break;
	
		case 1:
			result = str_copy(pstate_data->administrator->list[0]);
			break;
	
		default:
			assert(pstate_data->administrator->length >= 2);
			result = str_copy(pstate_data->administrator->list[0]);
			for (j = 1; j < pstate_data->administrator->length; ++j)
			{
				string_ty	*s;
	
				s =
					str_format
					(
						"%S %S",
						result,
						pstate_data->administrator->
							list[j]
					);
				str_free(result);
				result = s;
			}
			break;
		}
	}
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_baseline - the baseline substitution
 *
 * SYNOPSIS
 *	string_ty *sub_baseline(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_baseline function implements the baseline substitution.
 *	The baseline substitution is used to insert the absolute path
 *	of the baseline.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_baseline _((wlist *));

static string_ty *
sub_baseline(arg)
	wlist		*arg;
{
	string_ty	*result;

	trace(("sub_baseline()\n{\n"/*}*/));
	if (arg->wl_nwords != 1)
	{
		suberr = "requires zero arguments";
		result = 0;
	}
	else
		result = str_copy(project_baseline_path_get(pp, 0));
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_basename - the basename substitution
 *
 * SYNOPSIS
 *	string_ty *sub_basename(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_basename function implements the basename substitution.
 *	The basename substitution is replaced by the basename of
 *	the argument path, similar to the basename(1) command.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_basename _((wlist *));

static string_ty *
sub_basename(arg)
	wlist		*arg;
{
	string_ty	*result;
	string_ty	*suffix;
	string_ty	*s;
	long		len;

	trace(("sub_basename()\n{\n"/*}*/));
	switch (arg->wl_nwords)
	{
	default:
		suberr = "requires one or two arguments";
		result = 0;
		break;
	
	case 2:
		change_become(cp);
		result = os_entryname(arg->wl_word[1]);
		change_become_undo();
		break;
	
	case 3:
		change_become(cp);
		s = os_entryname(arg->wl_word[1]);
		change_become_undo();
		suffix = arg->wl_word[2];
		len = s->str_length - suffix->str_length;
		if
		(
			len > 0
		&&
			!memcmp
			(
				s->str_text + len,
				suffix->str_text,
				suffix->str_length
			)
		)
		{
			result = str_n_from_c(s->str_text, len);
			str_free(s);
		}
		else
			result = s;
		break;
	}
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_change - the change substitution
 *
 * SYNOPSIS
 *	string_ty *sub_change(wlist *arg);
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

static string_ty *sub_change _((wlist *));

static string_ty *
sub_change(arg)
	wlist		*arg;
{
	string_ty	*result;

	trace(("sub_change()\n{\n"/*}*/));
	if (arg->wl_nwords != 1)
	{
		suberr = "requires zero arguments";
		result = 0;
	}
	else
		result = str_format("%ld", cp->number);
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_date - the date substitution
 *
 * SYNOPSIS
 *	string_ty *sub_date(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_date function implements the date substitution.
 *	The date substitution is replaced by the current date.
 *	The optional arguments may construct format string,
 *	similar to the date(1) command.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_date _((wlist *));

static string_ty *
sub_date(arg)
	wlist		*arg;
{
	string_ty	*result;
	time_t		now;

	trace(("sub_date()\n{\n"/*}*/));
	assert(cp);
	time(&now);
	if (arg->wl_nwords < 2)
	{
		char	*cp;

		cp = ctime(&now);
		result = str_n_from_c(cp, 24);
	}
	else
	{
		struct tm	*tm;
		char		buf[1000];
		size_t		nbytes;
		string_ty	*fmt;
		extern size_t strftime _((char *, size_t, char *, struct tm *));

		fmt = wl2str(arg, 1, 32767);
		tm = localtime(&now);
		nbytes = strftime(buf, sizeof(buf) - 1, fmt->str_text, tm);
		if (!nbytes && fmt->str_length)
		{
			suberr = "strftime output too large";
			result = 0;
		}
		else
			result = str_n_from_c(buf, nbytes);
		str_free(fmt);
	}
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_delta - the delta substitution
 *
 * SYNOPSIS
 *	string_ty *sub_delta(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_delta function implements the delta substitution.
 *	The delta substitution is replaced by the delta number of the project.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_delta _((wlist *));

static string_ty *
sub_delta(arg)
	wlist		*arg;
{
	string_ty	*result;
	cstate		cstate_data;

	trace(("sub_delta()\n{\n"/*}*/));
	assert(cp);
	cstate_data = change_cstate_get(cp);
	if (arg->wl_nwords != 1)
	{
		suberr = "requires zero arguments";
		result = 0;
	}
	else if (cstate_data->state != cstate_state_being_integrated)
	{
		suberr = "not meaningful in current state";
		result = 0;
	}
	else
		result = str_format("%ld", cstate_data->delta_number);
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_developer - the developer substitution
 *
 * SYNOPSIS
 *	string_ty *sub_developer(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_developer function implements the developer substitution.
 *	The developer substitution is replaced by the name of the developer
 *	of the project.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_developer _((wlist *arg));

static string_ty *
sub_developer(arg)
	wlist		*arg;
{
	string_ty	*result;
	string_ty	*s;

	trace(("sub_developer()\n{\n"/*}*/));
	assert(cp);
	if (arg->wl_nwords != 1)
	{
		suberr = "requires zero arguments";
		result = 0;
	}
	else
	{
		s = change_developer_name(cp);
		if (!s)
		{
			suberr = "not meaningful in current state";
			result = 0;
		}
		else
			result = str_copy(s);
	}
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_developer_list - the developer_list substitution
 *
 * SYNOPSIS
 *	string_ty *sub_developer_list(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_developer_list function implements the developer_list substitution.
 *	The developer_list substitution is replaced by a space separated list
 *	of the project's developers.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_developer_list _((wlist *));

static string_ty *
sub_developer_list(arg)
	wlist		*arg;
{
	string_ty	*result;
	pstate		pstate_data;
	long		j;

	trace(("sub_developer_list()\n{\n"/*}*/));
	if (arg->wl_nwords != 1)
	{
		suberr = "requires zero arguments";
		result = 0;
	}
	else
	{
		/*
		 * build a string containing all of the project developers
		 */
		assert(pp);
		pstate_data = project_pstate_get(pp);
		assert(pstate_data->developer);
		switch (pstate_data->developer->length)
		{
		case 0:
			result = str_from_c("");
			break;
	
		case 1:
			result = str_copy(pstate_data->developer->list[0]);
			break;
	
		default:
			assert(pstate_data->developer->length >= 2);
			result = str_copy(pstate_data->developer->list[0]);
			for (j = 1; j < pstate_data->developer->length; ++j)
			{
				string_ty	*s;
	
				s =
					str_format
					(
						"%S %S",
						result,
						pstate_data->developer->list[j]
					);
				str_free(result);
				result = s;
			}
			break;
		}
	}
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_development_directory - the development_directory substitution
 *
 * SYNOPSIS
 *	string_ty *sub_development_directory(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_development_directory function implements the development_directory substitution.
 *	The development_directory substitution is used to insert the absolute path
 *	of the development_directory.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_development_directory _((wlist *));

static string_ty *
sub_development_directory(arg)
	wlist		*arg;
{
	string_ty	*result;
	cstate		cstate_data;

	trace(("sub_development_directory()\n{\n"/*}*/));
	assert(cp);
	cstate_data = change_cstate_get(cp);
	if (arg->wl_nwords != 1)
	{
		suberr = "requires zero arguments";
		result = 0;
	}
	else if
	(
		cstate_data->state < cstate_state_being_developed
	||
		cstate_data->state > cstate_state_being_integrated
	)
	{
		suberr = "not meaningful in current state";
		result = 0;
	}
	else
		result = str_copy(change_development_directory_get(cp, 0));
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_dirname - the dirname substitution
 *
 * SYNOPSIS
 *	string_ty *sub_dirname(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_dirname function implements the dirname substitution.
 *	The dirname substitution is replaced by the dirname of
 *	the argument path, similar to the dirname(1) command.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_dirname _((wlist *));

static string_ty *
sub_dirname(arg)
	wlist		*arg;
{
	string_ty	*result;

	trace(("sub_dirname()\n{\n"/*}*/));
	if (arg->wl_nwords != 2)
	{
		suberr = "requires one argument";
		result = 0;
	}
	else
	{
		change_become(cp);
		result = os_dirname(arg->wl_word[1]);
		change_become_undo();
	}
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_downcase - the downcase substitution
 *
 * SYNOPSIS
 *	string_ty *sub_downcase(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_downcase function implements the downcase substitution.
 *	The downcase substitution is replaced by the single argument
 *	mapped to lower case.
 *
 *	Requires exactly one argument.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_downcase _((wlist *arg));

static string_ty *
sub_downcase(arg)
	wlist		*arg;
{
	string_ty	*result;

	trace(("sub_downcase()\n{\n"/*}*/));
	if (arg->wl_nwords != 2)
	{
		suberr = "requires one argument";
		result = 0;
	}
	else
		result = str_downcase(arg->wl_word[1]);
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_identifier - the identifier substitution
 *
 * SYNOPSIS
 *	string_ty *sub_identifier(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_identifier function implements the identifier substitution.
 *	The identifier substitution is replaced by the single argument
 *	mapped to a C identifier.  All characters which are not legal in
 *	a C identifier are replaced by an underscore.
 *
 *	Requires exactly one argument.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_identifier _((wlist *arg));

static string_ty *
sub_identifier(arg)
	wlist		*arg;
{
	string_ty	*result;

	trace(("sub_identifier()\n{\n"/*}*/));
	if (arg->wl_nwords != 2)
	{
		suberr = "requires one argument";
		result = 0;
	}
	else if (arg->wl_word[1]->str_length == 0)
		result = str_from_c("_");
	else
	{
		string_ty	*s;
		size_t		len;
		char		*tmp;
		char		*cp1;
		char		*cp2;

		s = arg->wl_word[1];
		len = s->str_length;
		tmp = mem_alloc(len);
		for
		(
			cp1 = s->str_text, cp2 = tmp;
			cp1 < s->str_text + len;
			++cp1, ++cp2
		)
		{
			if (isalnum(*cp1))
				*cp2 = *cp1;
			else
				*cp2 = '_';
		}
		if (isdigit(tmp[0]))
			tmp[0] = '_';
		result = str_n_from_c(tmp, len);
		mem_free(tmp);
	}
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_integration_directory - the integration_directory substitution
 *
 * SYNOPSIS
 *	string_ty *sub_integration_directory(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_integration_directory function implements the integration_directory substitution.
 *	The integration_directory substitution is used to insert the absolute path
 *	of the integration_directory.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_integration_directory _((wlist *));

static string_ty *
sub_integration_directory(arg)
	wlist		*arg;
{
	string_ty	*result;
	cstate		cstate_data;

	trace(("sub_integration_directory()\n{\n"/*}*/));
	assert(cp);
	cstate_data = change_cstate_get(cp);
	if (arg->wl_nwords != 1)
	{
		suberr = "requires zero arguments";
		result = 0;
	}
	else if (cstate_data->state != cstate_state_being_integrated)
	{
		suberr = "not meaningful in current state";
		result = 0;
	}
	else
		result = str_copy(change_integration_directory_get(cp, 0));
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_integrator - the integrator substitution
 *
 * SYNOPSIS
 *	string_ty *sub_integrator(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_integrator function implements the integrator substitution.
 *	The integrator substitution is replaced by the name of the integrator
 *	of the project.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_integrator _((wlist *arg));

static string_ty *
sub_integrator(arg)
	wlist		*arg;
{
	string_ty	*result;
	string_ty	*s;

	trace(("sub_integrator()\n{\n"/*}*/));
	assert(cp);
	if (arg->wl_nwords != 1)
	{
		suberr = "requires zero arguments";
		result = 0;
	}
	else
	{
		s = change_integrator_name(cp);
		if (!s)
		{
			suberr = "not meaningful in current state";
			result = 0;
		}
		else
			result = str_copy(s);
	}
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_integrator_list - the integrator_list substitution
 *
 * SYNOPSIS
 *	string_ty *sub_integrator_list(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_integrator_list function implements the integrator_list substitution.
 *	The integrator_list substitution is replaced by a space separated list
 *	of the project's integrators.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_integrator_list _((wlist *));

static string_ty *
sub_integrator_list(arg)
	wlist		*arg;
{
	string_ty	*result;
	pstate		pstate_data;
	long		j;

	trace(("sub_integrator_list()\n{\n"/*}*/));
	if (arg->wl_nwords != 1)
	{
		suberr = "requires zero arguments";
		result = 0;
	}
	else
	{
		/*
		 * build a string containing all of the project integrators
		 */
		assert(pp);
		pstate_data = project_pstate_get(pp);
		assert(pstate_data->integrator);
		switch (pstate_data->integrator->length)
		{
		case 0:
			result = str_from_c("");
			break;
	
		case 1:
			result = str_copy(pstate_data->integrator->list[0]);
			break;
	
		default:
			assert(pstate_data->integrator->length >= 2);
			result = str_copy(pstate_data->integrator->list[0]);
			for (j = 1; j < pstate_data->integrator->length; ++j)
			{
				string_ty	*s;
	
				s =
					str_format
					(
						"%S %S",
						result,
						pstate_data->integrator->list[j]
					);
				str_free(result);
				result = s;
			}
			break;
		}
	}
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_project - the project substitution
 *
 * SYNOPSIS
 *	string_ty *sub_project(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_project function implements the project substitution.
 *	The project substitution is replaced by the project name.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_project _((wlist *));

static string_ty *
sub_project(arg)
	wlist		*arg;
{
	string_ty	*result;

	trace(("sub_project()\n{\n"/*}*/));
	if (arg->wl_nwords != 1)
	{
		suberr = "requires zero arguments";
		result = 0;
	}
	else
		result = str_copy(project_name_get(pp));
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_reviewer - the reviewer substitution
 *
 * SYNOPSIS
 *	string_ty *sub_reviewer(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_reviewer function implements the reviewer substitution.
 *	The reviewer substitution is replaced by the name of the reviewer
 *	of the project.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_reviewer _((wlist *arg));

static string_ty *
sub_reviewer(arg)
	wlist		*arg;
{
	string_ty	*result;
	string_ty	*s;

	trace(("sub_reviewer()\n{\n"/*}*/));
	assert(cp);
	if (arg->wl_nwords != 1)
	{
		suberr = "requires zero arguments";
		result = 0;
	}
	else
	{
		s = change_reviewer_name(cp);
		if (!s)
		{
			suberr = "not meaningful in current state";
			result = 0;
		}
		else
			result = str_copy(s);
	}
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_reviewer_list - the reviewer_list substitution
 *
 * SYNOPSIS
 *	string_ty *sub_reviewer_list(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_reviewer_list function implements the reviewer_list substitution.
 *	The reviewer_list substitution is replaced by a space separated list
 *	of the project's reviewers.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_reviewer_list _((wlist *));

static string_ty *
sub_reviewer_list(arg)
	wlist		*arg;
{
	string_ty	*result;
	pstate		pstate_data;
	long		j;

	trace(("sub_reviewer_list()\n{\n"/*}*/));
	if (arg->wl_nwords != 1)
	{
		suberr = "requires zero arguments";
		result = 0;
	}
	else
	{
		/*
		 * build a string containing all of the project reviewers
		 */
		assert(pp);
		pstate_data = project_pstate_get(pp);
		assert(pstate_data->reviewer);
		switch (pstate_data->reviewer->length)
		{
		case 0:
			result = str_from_c("");
			break;
	
		case 1:
			result = str_copy(pstate_data->reviewer->list[0]);
			break;
	
		default:
			assert(pstate_data->reviewer->length >= 2);
			result = str_copy(pstate_data->reviewer->list[0]);
			for (j = 1; j < pstate_data->reviewer->length; ++j)
			{
				string_ty	*s;
	
				s =
					str_format
					(
						"%S %S",
						result,
						pstate_data->reviewer->list[j]
					);
				str_free(result);
				result = s;
			}
			break;
		}
	}
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_source - the source substitution
 *
 * SYNOPSIS
 *	string_ty *sub_source(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_source function implements the source substitution.
 *	The source substitution is replaced by the path of the source file,
 *	depending on wether it is in the baseline or the change.
 *	If the file named in the argument is in the change,
 *	the name will be left unchanged,
 *	but if the file is in the baseline, an absolute path will resiult.
 *	If the change is being integrated, it will always be left untouched.
 *
 *	Requires exactly one argument.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_source _((wlist *arg));

static string_ty *
sub_source(arg)
	wlist		*arg;
{
	string_ty	*result;
	cstate		cstate_data;

	trace(("sub_source()\n{\n"/*}*/));
	cstate_data = change_cstate_get(cp);
	if (arg->wl_nwords != 2)
	{
		suberr = "requires one argument";
		result = 0;
	}
	else if (cstate_data->state == cstate_state_awaiting_development)
	{
		suberr = "not meaningful in current state";
		result = 0;
	}
	else
	{
		/*
		 * Make sure the named file exists
		 * either in the change or in the project.
		 */
		if (!change_src_find(cp, arg->wl_word[1]))
		{
			if (project_src_find(cp->pp, arg->wl_word[1]))
			{
				/*
				 * if the project is is the 'being
				 * developed' state, need to qualify
				 * the file name with the baseline
				 * directory name.
				 *
				 * If the change is in the 'being
				 * integrated' state, don't do
				 * anything, as files in the
				 * project AND files in the change
				 * are below the current directory.
				 */
				if
				(
					cstate_data->state
				<
					cstate_state_being_integrated
				)
				{
					result =
						str_format
						(
							"%S/%S",
					project_baseline_path_get(cp->pp, 0),
							arg->wl_word[1]
						);
				}
				else
					result = str_copy(arg->wl_word[1]);
			}
			else
			{
				suberr = "source file unknown";
				result = 0;
			}
		}
		else
			result = str_copy(arg->wl_word[1]);
	}
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_upcase - the upcase substitution
 *
 * SYNOPSIS
 *	string_ty *sub_upcase(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_upcase function implements the upcase substitution.
 *	The upcase substitution is replaced by the single argument
 *	mapped to upper case.
 *
 *	Requires exactly one argument.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_upcase _((wlist *arg));

static string_ty *
sub_upcase(arg)
	wlist		*arg;
{
	string_ty	*result;

	trace(("sub_upcase()\n{\n"/*}*/));
	if (arg->wl_nwords != 2)
	{
		suberr = "requires one argument";
		result = 0;
	}
	else
		result = str_upcase(arg->wl_word[1]);
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_version - the version substitution
 *
 * SYNOPSIS
 *	string_ty *sub_version(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_version function implements the version substitution.
 *	The version substitution is replaced by the version of the chnage.
 *
 *	If the change is not in the being_integrated or completed states,
 *	the version will be major.minor.Cnumber, where
 *	"major" is the project major version number,
 *	"minor" is the project minor version number, and
 *	"change" is a 3 digit change number, possibly zero padded.
 *
 *	If the change is in the being_integrated or completed state,
 *	the version will be major.minor.Cdelta, where
 *	"delta" is a 3 digit delta number, possibly zero padded.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static string_ty *sub_version _((wlist *arg));

static string_ty *
sub_version(arg)
	wlist		*arg;
{
	string_ty	*result;
	pstate		pstate_data;
	cstate		cstate_data;

	trace(("sub_version()\n{\n"/*}*/));
	assert(cp);
	if (arg->wl_nwords != 1)
	{
		suberr = "requires zero arguments";
		result = 0;
	}
	else
	{
		pstate_data = project_pstate_get(pp);
		cstate_data = change_cstate_get(cp);
		if (cstate_data->state < cstate_state_being_integrated)
		{
			result =
				str_format
				(
					"%ld.%ld.C%3.3ld",
					pstate_data->version_major,
					pstate_data->version_minor,
					cp->number
				);
		}
		else
		{
			result =
				str_format
				(
					"%ld.%ld.D%3.3ld",
					pstate_data->version_major,
					pstate_data->version_minor,
					cstate_data->delta_number
				);
		}
	}
	trace(("return \"%s\";\n", result ? result->str_text : suberr));
	trace((/*{*/"}\n"));
	return result;
}


static table_ty	table[] =
{
	{ "Administrator_List",		sub_administrator_list,		},
	{ "BaseLine",			sub_baseline,			},
	{ "Basename",			sub_basename,			},
	{ "Change",			sub_change,			},
	{ "DAte",			sub_date,			},
	{ "DELta",			sub_delta,			},
	{ "DEVeloper",			sub_developer,			},
	{ "DEVeloper_List",		sub_developer_list,		},
	{ "Development_Directory",	sub_development_directory,	},
	{ "Dirname",			sub_dirname,			},
	{ "DownCase",			sub_downcase,			},
	/* Edit								*/
	/* File_Name							*/
	/* History							*/
	/* Input							*/
	{ "IDentifier",			sub_identifier,			},
	{ "INTegration_Directory",	sub_integration_directory,	},
	{ "INTegrator",			sub_integrator,			},
	{ "INTegrator_List",		sub_integrator_list,		},
	/* Most_Recent							*/
	/* Output							*/
	/* ORiginal							*/
	{ "Project",			sub_project,			},
	{ "Reviewer",			sub_reviewer,			},
	{ "Reviewer_List",		sub_reviewer_list,		},
	{ "Source",			sub_source,			},
	{ "UpCase",			sub_upcase,			},
	{ "Version",			sub_version,			},
};


/*
 * NAME
 *	sub_diversion
 *
 * SYNOPSIS
 *	void sub_diversion(string_ty *s);
 *
 * DESCRIPTION
 *	The sub_diversion function is used to divert input
 *	to a string, until that string is exhausted.
 *	When the string is exhausted, input will resume
 *	from the previous string.
 *
 * ARGUMENTS
 *	s - string to take as input
 */

static void sub_diversion _((string_ty *));

static void
sub_diversion(s)
	string_ty	*s;
{
	diversion_ty	*dp;

	trace(("sub_diversion(\"%s\")\n{\n"/*}*/, s->str_text));
	dp = (diversion_ty *)mem_alloc(sizeof(diversion_ty));
	dp->text = str_copy(s);
	dp->pos = 0;
	dp->prev = diversion;
	diversion = dp;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	sub_diversion_close
 *
 * SYNOPSIS
 *	void sub_diversion_close(void);
 *
 * DESCRIPTION
 *	The sub_diversion_close function is used to
 *	release a diversion when it has been exhausted.
 */

static void sub_diversion_close _((void));

static void
sub_diversion_close()
{
	diversion_ty	*dp;

	trace(("sub_diversion_close()\n{\n"/*}*/));
	assert(diversion);
	dp = diversion;
	diversion = dp->prev;
	assert(dp->pos == dp->text->str_length);
	str_free(dp->text);
	mem_free((char *)dp);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	execute
 *
 * SYNOPSIS
 *	void execute(wlist *args);
 *
 * DESCRIPTION
 *	The execute function is used to perform the substitution
 *	described by the argument list.
 *
 * ARGUMENTS
 *	args - the name and arguments of the substitution
 */

static void execute _((wlist *));

static void
execute(arg)
	wlist		*arg;
{
	char		*cmd;
	string_ty	*s;
	table_ty	*hit[20];
	int		nhits;
	table_ty	*tp;
	long		j;

	trace(("execute()\n{\n"/*}*/));
	if (arg->wl_nwords == 0)
		fatal("empty ${} substitution");
	
	/*
	 * scan the variables
	 */
	nhits = 0;
	cmd = arg->wl_word[0]->str_text;
	for (j = 0; j < sub_var_pos; ++j)
	{
		tp = &sub_var_list[j];
		if (arglex_compare(tp->name, cmd))
		{
			if (nhits < SIZEOF(hit))
				hit[nhits++] = tp;
		}
	}

	/*
	 * scan the functions
	 */
	for (tp = table; tp < ENDOF(table); ++tp)
	{
		if (arglex_compare(tp->name, cmd))
		{
			if (nhits < SIZEOF(hit))
				hit[nhits++] = tp;
		}
	}

	/*
	 * figure what to do
	 */
	switch (nhits)
	{
	case 0:
		suberr = "unknown substitution name";
		s = 0;
		break;

	case 1:
		tp = hit[0];
		if (tp->value)
		{
			s = str_copy(tp->value);
			if (isdigit(tp->name[0]))
			{
				error
				(
       "warning: the %%%s substitution has been replaced by %s in this context",
					tp->name,
					s->str_text
				);
			}
			trace(("%s -> %s\n", tp->name, s->str_text));
		}
		else
		{
			str_free(arg->wl_word[0]);
			arg->wl_word[0] = str_from_c(tp->name);
			s = tp->func(arg);
		}
		break;

	default:
		suberr = "ambiguous substitution name";
		s = 0;
		break;
	}

	/*
	 * deal with the result
	 */
	if (s)
	{
		sub_diversion(s);
		str_free(s);
	}
	else
	{
		string_ty	*s2;

		s2 = wl2str(arg, 0, 32767);
		fatal
		(
			"substitution ${%s} failed: %s",
			s2->str_text,
			suberr
		);
		str_free(s2);
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	sub_getc_meta
 *
 * SYNOPSIS
 *	void sub_getc_meta(void);
 *
 * DESCRIPTION
 *	The sub_getc_meta function is used to get a character from
 *	the current input string.  When the current string is exhaused,
 *	the previous string is resumed.
 *
 * RETURNS
 *	int - the chacater, or NUL to indicate end of input
 */

static int sub_getc_meta _((void));

static int
sub_getc_meta()
{
	int		result;
	diversion_ty	*dp;

	trace(("sub_getc_meta()\n{\n"/*}*/));
	dp = diversion;
	if (!dp)
		result = 0;
	else if (dp->pos >= dp->text->str_length)
		result = 0;
	else
		result = dp->text->str_text[dp->pos++];
	trace(("return '%c';\n", result ? result : 0x80));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_getc_meta_undo
 *
 * SYNOPSIS
 *	void sub_getc_meta_undo(int c);
 *
 * DESCRIPTION
 *	The sub_getc_meta_undo function is used to give back
 *	a character output by sub_getc_meta.
 *
 * ARGUMENTS
 *	c - character being given back
 *
 * CAVEAT
 *	Only push back what was read.
 */

static void sub_getc_meta_undo _((int));

static void
sub_getc_meta_undo(c)
	int		c;
{
	diversion_ty	*dp;

	trace(("sub_getc_meta_undo('%c')\n{\n"/*}*/, c ? c : 0x80));
	dp = diversion;
	assert(dp);
	if (!c)
	{
		assert(dp->pos == dp->text->str_length);
	}
	else
	{
		assert(dp->pos >= 1);
		dp->pos--;
		assert(c == dp->text->str_text[dp->pos]);
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	collect
 *
 * SYNOPSIS
 *	void collect(int c);
 *
 * DESCRIPTION
 *	The collect function is used to accumulate a string
 *	one character at a time.  No size limit.
 *
 * ARGUMENTS
 *	c - the character being collected
 */

static void collect _((int));

static void
collect(c)
	int	c;
{
	if (collect_pos >= collect_size)
	{
		collect_size += (1L << 10);
		if (!collect_buf)
			collect_buf = mem_alloc(collect_size);
		else
			mem_change_size(&collect_buf, collect_size);
	}
	collect_buf[collect_pos++] = c;
}


/*
 * NAME
 *	collect_end
 *
 * SYNOPSIS
 *	string_ty *collect_end(void);
 *
 * DESCRIPTION
 *	The collect_end function is used to fetch the string
 *	accumulated with the collect function.
 *	The bufferer for the collect function is cleared.
 *
 * RETURNS
 *	string_ty *; pointer to the string in dynamic memory.
 */

static string_ty *collect_end _((void));

static string_ty *
collect_end()
{
	string_ty	*result;

	result = str_n_from_c(collect_buf, collect_pos);
	collect_pos = 0;
	return result;
}


/*
 * NAME
 *	percent
 *
 * SYNOPSIS
 *	int percent(void);
 *
 * DESCRIPTION
 *	The percent function is used to perform percent (%) substitutions.
 *	On entry, the % is expected to have been consumed.
 *
 *	The substitution is usually achieved as a side-effect,
 *	by using the sub_diversion function.
 *
 * RETURNS
 *	int	a character to deliver as output,
 *		or NUL if none.
 */

static int percent _((void));

static int
percent()
{
	int		c;
	int		result;
	int		c2;
	string_ty	*s;
	static int	warned;
	string_ty	*s2;

	trace(("percent()\n{\n"/*}*/));
	result = 0;
	c = sub_getc_meta();
	switch (c)
	{
	case 'P':
		c2 = sub_getc_meta();
		if (!isdigit(c2))
		{
			sub_getc_meta_undo(c2);
			sub_getc_meta_undo(c);
			result = '%';
			break;
		}
		s = str_format("${dirname ${%c}}", c2);
		sub_diversion(s);
		str_free(s);
		break;

	case 'F':
		c2 = sub_getc_meta();
		if (!isdigit(c2))
		{
			sub_getc_meta_undo(c2);
			sub_getc_meta_undo(c);
			result = '%';
			break;
		}
		s = str_format("${basename ${%c}}", c2);
		sub_diversion(s);
		str_free(s);
		break;

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		s = str_format("${%c}", c);
		sub_diversion(s);
		str_free(s);
		break;

	case '('/*)*/:
		for (;;)
		{
			c = sub_getc_meta();
			if (!c)
				fatal("unterminated %%() substitution");
			if (c == /*(*/')')
				break;
			collect(c);
		}
		s2 = collect_end();
		s = str_format("${source %S}", s2);
		str_free(s2);
		sub_diversion(s);
		str_free(s);
		break;

	default:
		sub_getc_meta_undo(c);
		result = '%';
		break;
	}
	if (!result && !warned)
	{
		error
		(
"warning: the percent (%%) substitutions are obsolescent, \
please use the equivalent dollar ($) substitution"
		);
		warned = 1;
	}
	trace(("return '%c'\n", result ? result : 0x80));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	dollar
 *
 * SYNOPSIS
 *	void dollar(void);
 *
 * DESCRIPTION
 *	The dollar function is used to perform dollar ($) substitutions.
 *	On entry, the $ is expected to have been consumed.
 *
 *	The substitution is usually achieved as a side-effect,
 *	by using the sub_diversion function.
 *
 * RETURNS
 *	int	a character to deliver as output,
 *		or NUL if none.
 */

static int sub_getc _((void)); /* forward */

static int dollar _((void));

static int
dollar()
{
	wlist		arg;
	int		result;
	int		c;
	string_ty	*s;

	trace(("dollar()\n{\n"/*}*/));
	wl_zero(&arg);
	result = 0;
	c = sub_getc_meta();
	switch (c)
	{
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9': 
		for (;;)
		{
			collect(c);
			c = sub_getc_meta();
			if (!isdigit(c))
			{
				sub_getc_meta_undo(c);
				break;
			}
		}
		s = collect_end();
		wl_append(&arg, s);
		str_free(s);
		execute(&arg);
		wl_free(&arg);
		break;

	case 'a': case 'b': case 'c': case 'd': case 'e':
	case 'f': case 'g': case 'h': case 'i': case 'j':
	case 'k': case 'l': case 'm': case 'n': case 'o':
	case 'p': case 'q': case 'r': case 's': case 't':
	case 'u': case 'v': case 'w': case 'x': case 'y': case 'z': 
	case 'A': case 'B': case 'C': case 'D': case 'E':
	case 'F': case 'G': case 'H': case 'I': case 'J':
	case 'K': case 'L': case 'M': case 'N': case 'O':
	case 'P': case 'Q': case 'R': case 'S': case 'T':
	case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z': 
		for (;;)
		{
			collect(c);
			c = sub_getc_meta();
			if (!isalnum(c) && c != '_' && c != '-')
			{
				sub_getc_meta_undo(c);
				break;
			}
		}
		s = collect_end();
		wl_append(&arg, s);
		str_free(s);
		execute(&arg);
		wl_free(&arg);
		break;

	case '{'/*}*/:
		c = sub_getc();
		for (;;)
		{
			/*
			 * look for terminator
			 */
			if (c == /*{*/'}')
				break;

			/*
			 * watch out for unterminated substitutions
			 */
			if (!c)
			{
				fatal("unterminated ${} substitution");
				break;
			}

			/*
			 * skip white space separating the arguments
			 */
			if (isspace(c))
			{
				c = sub_getc();
				continue;
			}

			/*
			 * collect the argument
			 *	any run of non-white-space characters
			 */
			for (;;)
			{
				collect(c);
				c = sub_getc();
				if (!c || isspace(c) || c == /*{*/'}')
					break;
			}
			s = collect_end();
			wl_append(&arg, s);
			str_free(s);
		}
		execute(&arg);
		wl_free(&arg);
		break;

	case '$':
		result = '$';
		break;

	default:
		sub_getc_meta_undo(c);
		result = '$';
		break;
	}
	trace(("return '%c';\n", result ? result : 0x80));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_getc
 *
 * SYNOPSIS
 *	int sub_getc(void);
 *
 * DESCRIPTION
 *	The sub_getc function is used to get a character from
 *	the substitution stream.  This is used both for the final output
 *	and for fetching arguments to dollar ($) substitutions.
 *
 * RETURNS
 *	int	a character from the stream,
 *		or NUL to indicate end of input.
 */

static int sub_getc _((void));

static int
sub_getc()
{
	int	c;

	trace(("sub_getc()\n{\n"/*}*/));
	for (;;)
	{
		c = sub_getc_meta();
		switch (c)
		{
		default:
			break;

		case 0:
			if (!diversion)
				break;
			sub_diversion_close();
			continue;

		case '%':
			c = percent();
			if (!c)
				continue;
			break;

		case '$':
			c = dollar();
			if (!c)
				continue;
			break;
		}
		break;
	}
	trace(("return '%c';\n", c ? c : 0x80));
	trace((/*{*/"}\n"));
	return c;
}


/*
 * NAME
 *	substitute
 *
 * SYNOPSIS
 *	string_ty *substitute(change_ty *cp, string_ty *s);
 *
 * DESCRIPTION
 *	The substitute function is used to perform substitutions on
 *	strings.  Usually command strings, but not always.
 *
 *	The format of substitutions, and the commonly available
 *	substitutions, are described in aesub(5).
 *
 * ARGUMENTS
 *	cp	- the aegis change involved with the command
 *		  This may never be NULL.
 *	s	- the string to be substituted.
 *
 * RETURNS
 *	string_ty *; pointer to string in dynamic memory
 */

string_ty *
substitute(acp, s)
	change_ty	*acp;
	string_ty	*s;
{
	static char	*buffer;
	static size_t	buflen;
	size_t		pos;
	int		c;
	string_ty	*result;

	trace(("substitute(acp = %08lX, s = \"%s\")\n{\n"/*}*/,
		(long)acp, s->str_text));
	assert(acp);
	cp = acp;
	pp = acp->pp;

	sub_diversion(s);
	pos = 0;
	if (!buffer)
	{
		buflen = (1L << 10);
		buffer = mem_alloc(buflen);
	}
	for (;;)
	{
		/*
		 * get the next character
		 */
		c = sub_getc();
		if (!c)
			break;

		/*
		 * make sure there is room
		 */
		if (pos >= buflen)
		{
			buflen += (1L << 10);
			mem_change_size(&buffer, buflen);
		}

		/*
		 * save the character
		 */
		buffer[pos++] = c;
	}
	sub_var_clear();
	cp = 0;
	pp = 0;
	result = str_n_from_c(buffer, pos);
	trace(("return \"%s\";\n", result->str_text));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_var_clear
 *
 * SYNOPSIS
 *	void sub_var_clear(void);
 *
 * DESCRIPTION
 *	The sub_var_clear function is used to clear all of
 *	the substitution variables.  Not usually needed manually,
 *	as this is done automatically at the end of every substitute().
 */

void
sub_var_clear()
{
	size_t	j;

	for (j = 0; j < sub_var_pos; ++j)
		str_free(sub_var_list[j].value);
	sub_var_pos = 0;
}


/*
 * NAME
 *	sub_var_set
 *
 * SYNOPSIS
 *	void sub_var_set(char *name, char *fmt, ...);
 *
 * DESCRIPTION
 *	The sub_var_set function is used to set the value of a
 *	substitution variable.  These variables are command specific,
 *	as opposed to the functions which are always present.
 *	The user documentation does NOT make this distinction by
 *	using the names "variable" and "function", they are always referred
 *	to as "substitutions".
 *
 * ARGUMENTS
 *	name	- the name of the variable
 *	fmt,...	- a format string and arguments to construct the value.
 *		  Handed to str_vformat to make a (string_ty *) out of it.
 *
 * CAVEAT
 *	remains in scope until the next invokation of sub_var_clear,
 *	or until the end of the next invokation of substitute.
 */

void
sub_var_set(name, fmt sva_last)
	char		*name;
	char		*fmt;
	sva_last_decl
{
	va_list		ap;
	string_ty	*s;
	table_ty	*svp;

	sva_init(ap, fmt);
	s = str_vformat(fmt, ap);
	va_end(ap);

	if (sub_var_pos >= sub_var_size)
	{
		size_t	nbytes;

		sub_var_size += 10;
		nbytes = sub_var_size * sizeof(table_ty);
		if (!sub_var_list)
			sub_var_list = (table_ty *)mem_alloc(nbytes);
		else
			mem_change_size((char **)&sub_var_list, nbytes);
	}
	svp = &sub_var_list[sub_var_pos++];
	svp->name = name;
	svp->value = s;
}
