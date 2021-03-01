/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998 Peter Miller;
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
 * MANIFEST: functions to perform command substitutions
 */

#include <ac/ctype.h>
#include <errno.h>
#include <ac/stdio.h>
#include <ac/stddef.h>
#include <ac/stdlib.h>
#include <ac/libintl.h>
#include <ac/limits.h>
#include <ac/string.h>
#include <ac/wchar.h>
#include <ac/wctype.h>

#include <ac/grp.h>
#include <ac/pwd.h>

#include <arglex.h>
#include <change.h>
#include <change_bran.h>
#include <change_file.h>
#include <column_width.h>
#include <error.h>
#include <file.h>
#include <gonzo.h>
#include <language.h>
#include <libdir.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project_file.h>
#include <project_hist.h>
#include <str_list.h>
#include <sub.h>
#include <sub/basename.h>
#include <sub/comment.h>
#include <sub/dollar.h>
#include <sub/expr.h>
#include <sub/left.h>
#include <sub/length.h>
#include <sub/namemax.h>
#include <sub/quote.h>
#include <sub/right.h>
#include <sub/trim_directo.h>
#include <sub/trim_extensi.h>
#include <sub/zero_pad.h>
#include <trace.h>
#include <user.h>
#include <wstr_list.h>


#define RESUB_DOLLARS 2
#define RESUB_PERCENT 1
#define RESUB_BOTH (RESUB_DOLLARS | RESUB_PERCENT)

enum getc_type
{
	getc_type_control,
	getc_type_data
};
typedef enum getc_type getc_type;


/* typedef struct sub_context_ty sub_context_ty; */
struct sub_context_ty
{
	struct sub_diversion_ty *diversion;
	struct sub_table_ty *sub_var_list;
	size_t		sub_var_size;
	size_t		sub_var_pos;
	struct change_ty *cp;
	struct project_ty *pp;
	char		*suberr;
	int		errno_sequester;
	char		*file_name;
	int		line_number;
};


typedef wstring_ty *(*fp)_((sub_context_ty *, wstring_list_ty *));

typedef struct sub_table_ty sub_table_ty;
struct sub_table_ty
{
	char		*name;
	fp		func;
	int		resubstitute;
	wstring_ty	*value;
	int		must_be_used	:1;
	int		append_if_unused :1;
	int		override	:1;
};

typedef struct sub_diversion_ty sub_diversion_ty;
struct sub_diversion_ty
{
	long		pos;
	wstring_ty	*text;
	sub_diversion_ty *prev;
	int		resubstitute;
};


typedef struct collect_ty collect_ty;
struct collect_ty
{
	size_t	pos;
	size_t	size;
	wchar_t	*buf;
};


static void collect_constructor _((collect_ty *));

static void
collect_constructor(cp)
	collect_ty	*cp;
{
	cp->buf = 0;
	cp->size = 0;
	cp->pos = 0;
}


static void collect_destructor _((collect_ty *));

static void
collect_destructor(cp)
	collect_ty	*cp;
{
	if (cp->buf)
		mem_free(cp->buf);
	cp->buf = 0;
	cp->size = 0;
	cp->pos = 0;
}


sub_context_ty *
sub_context_New(file_name, line_number)
	char		*file_name;
	int		line_number;
{
	sub_context_ty	*p;
	int		hold;

	hold = errno;	/* must preserve! */
	p = mem_alloc(sizeof(sub_context_ty));
	p->file_name = file_name;
	p->line_number = line_number;
	p->diversion = 0;
	p->sub_var_list = 0;
	p->sub_var_size = 0;
	p->sub_var_pos = 0;
	p->cp = 0;
	p->pp = 0;
	p->suberr = 0;
	p->errno_sequester = 0;
	errno = hold;	/* must preserve! */
	return p;
}


void
sub_context_delete(p)
	sub_context_ty	*p;
{
	size_t	j;

	for (j = 0; j < p->sub_var_pos; ++j)
		wstr_free(p->sub_var_list[j].value);
	if (p->sub_var_list)
		mem_free(p->sub_var_list);
	mem_free(p);
}


void
sub_context_error_set(scp, s)
	sub_context_ty	*scp;
	char		*s;
{
	scp->suberr = s;
}


/*
 * NAME
 *	sub_administrator_list - the administrator_list substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_administrator_list(wstring_list_ty *arg);
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

static wstring_ty *sub_administrator_list _((sub_context_ty *,
	wstring_list_ty *));

static wstring_ty *
sub_administrator_list(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	pstate		pstate_data;
	long		j;

	trace(("sub_administrator_list()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else if (!scp->pp)
	{
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
	{
		string_list_ty		wl;
		string_ty	*s;

		/*
		 * build a string containing all of the project administrators
		 */
		pstate_data = project_pstate_get(scp->pp);
		string_list_constructor(&wl);
		for (j = 0; ; ++j)
		{
			s = project_administrator_nth(scp->pp, j);
			if (!s)
				break;
			string_list_append(&wl, s);
		}
		s = wl2str(&wl, 0, wl.nstrings, " ");
		trace(("s = \"%s\"\n", s->str_text));
		string_list_destructor(&wl);
		result = str_to_wstr(s);
		str_free(s);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_baseline - the baseline substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_baseline(wstring_list_ty *arg);
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

static wstring_ty *sub_baseline _((sub_context_ty *, wstring_list_ty *));

static wstring_ty *
sub_baseline(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_baseline()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else if (!scp->pp)
	{
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
		result = str_to_wstr(project_baseline_path_get(scp->pp, 0));
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
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

static wstring_ty *sub_change _((sub_context_ty *, wstring_list_ty *));

static wstring_ty *
sub_change(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_change()\n{\n"/*}*/));
	result = 0;
	if (arg->nitems != 1)
		scp->suberr = i18n("requires zero arguments");
	else if (!scp->cp || scp->cp->bogus)
		scp->suberr = i18n("not valid in current context");
	else
	{
		string_ty	*s;

		s = str_format("%ld", magic_zero_decode(scp->cp->number));
		result = str_to_wstr(s);
		str_free(s);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static int icmp _((const void *, const void *));

static int
icmp(va, vb)
	const void	*va;
	const void	*vb;
{
	const int	*a;
	const int	*b;

	a = va;
	b = vb;
	if (*a == *b)
		return 0;
	if (*a < *b)
		return -1;
	return 1;
}


/*
 * NAME
 *	sub_copyright_years - the change substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_copyright_years(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_copyright_years function implements the change
 *	substitution.  The copyright_years substitution is replaced by
 *	the range of copyright years in the project's state, and
 *	maintained at integrate begin time.  Do not use this to insert
 *	into new files, it is not guaranteed to be up-to-date until the
 *	integrate build, use ${date %Y} instead.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static wstring_ty *sub_copyright_years _((sub_context_ty *, wstring_list_ty *));

static wstring_ty *
sub_copyright_years(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	int		ary[100]; /* a century should be enough  :-) */
	int		ary_len;
	int		ary_len2;
	string_list_ty		wl;
	size_t		j;
	string_ty	*s;

	trace(("sub_copyright_years()\n{\n"/*}*/));
	result = 0;
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		goto done;
	}
	if (!scp->cp || scp->cp->bogus)
	{
		scp->suberr = i18n("not valid in current context");
		goto done;
	}

	/*
	 * Extract the copyright years from the project
	 * and the change.  Don't worry about duplicates.
	 */
	assert(scp->pp);
	assert(scp->pp == scp->cp->pp);
	project_copyright_years_get(scp->pp, ary, SIZEOF(ary), &ary_len);
	change_copyright_years_get(scp->cp, ary + ary_len, SIZEOF(ary) - ary_len, &ary_len2);
	ary_len += ary_len2;

	/*
	 * sort the array
	 */
	qsort(ary, ary_len, sizeof(ary[0]), icmp);

	/*
	 * build the text string for the result
	 * this is where duplicates are removed
	 */
	string_list_constructor(&wl);
	for (j = 0; j < ary_len; ++j)
	{
		s = str_format("%ld", ary[j]);
		string_list_append(&wl, s);
		str_free(s);
	}
	s = wl2str(&wl, 0, wl.nstrings, ", ");
	string_list_destructor(&wl);
	result = str_to_wstr(s);
	str_free(s);

	/*
	 * here for all exits
	 */
	done:
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_date - the date substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_date(wstring_list_ty *arg);
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

static wstring_ty *sub_date _((sub_context_ty *, wstring_list_ty *));

static wstring_ty *
sub_date(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	time_t		now;

	trace(("sub_date()\n{\n"/*}*/));
	time(&now);
	if (arg->nitems < 2)
	{
		char	*time_string;

		time_string = ctime(&now);
		result = wstr_n_from_c(time_string, 24);
	}
	else
	{
		struct tm	*tm;
		char		buf[1000];
		size_t		nbytes;
		wstring_ty	*wfmt;
		string_ty	*fmt;

		wfmt = wstring_list_to_wstring(arg, 1, 32767, (char *)0);
		fmt = wstr_to_str(wfmt);
		wstr_free(wfmt);
		tm = localtime(&now);

		/*
		 * The strftime is locale dependent.
		 */
		language_human();
		nbytes = strftime(buf, sizeof(buf) - 1, fmt->str_text, tm);
		language_C();

		if (!nbytes && fmt->str_length)
		{
			scp->suberr = i18n("strftime output too large");
			result = 0;
		}
		else
			result = wstr_n_from_c(buf, nbytes);
		str_free(fmt);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_delta - the delta substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_delta(wstring_list_ty *arg);
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

static wstring_ty *sub_delta _((sub_context_ty *, wstring_list_ty *));

static wstring_ty *
sub_delta(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	cstate		cstate_data;

	trace(("sub_delta()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else if (!scp->cp)
	{
		yuck:
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
	{
		string_ty	*s;

		cstate_data = change_cstate_get(scp->cp);
		if (cstate_data->state != cstate_state_being_integrated)
			goto yuck;
		s = str_format("%ld", cstate_data->delta_number);
		result = str_to_wstr(s);
		str_free(s);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_developer - the developer substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_developer(wstring_list_ty *arg);
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

static wstring_ty *sub_developer _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_developer(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	string_ty	*s;

	trace(("sub_developer()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else if (!scp->cp)
	{
		yuck:
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
	{
		s = change_developer_name(scp->cp);
		if (!s)
			goto yuck;
		result = str_to_wstr(s);
		/* do not free s */
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_developer_list - the developer_list substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_developer_list(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_developer_list function implements the developer_list
 *	substitution.  The developer_list substitution is replaced by a
 *	space separated list of the project's developers.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static wstring_ty *sub_developer_list _((sub_context_ty *, wstring_list_ty *));

static wstring_ty *
sub_developer_list(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	long		j;
	string_list_ty		wl;
	string_ty	*s;

	trace(("sub_developer_list()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else if (!scp->pp)
	{
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
	{
		/*
		 * build a string containing all of the project developers
		 */
		assert(scp->pp);
		string_list_constructor(&wl);
		for (j = 0; ; ++j)
		{
			s = project_developer_nth(scp->pp, j);
			if (!s)
				break;
			string_list_append(&wl, s);
		}
		s = wl2str(&wl, 0, wl.nstrings, " ");
		string_list_destructor(&wl);
		result = str_to_wstr(s);
		str_free(s);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_development_directory - the development_directory substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_development_directory(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_development_directory function implements the
 *	development_directory substitution.  The development_directory
 *	substitution is used to insert the absolute path of the
 *	development_directory.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static wstring_ty *sub_development_directory _((sub_context_ty *,
	wstring_list_ty *));

static wstring_ty *
sub_development_directory(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	cstate		cstate_data;

	trace(("sub_development_directory()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else if (!scp->cp)
	{
		yuck:
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
	{
		cstate_data = change_cstate_get(scp->cp);
		if
		(
			cstate_data->state < cstate_state_being_developed
		||
			cstate_data->state > cstate_state_being_integrated
		)
			goto yuck;
		result =
			str_to_wstr
			(
				change_development_directory_get(scp->cp, 0)
			);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_dirname - the dirname substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_dirname(wstring_list_ty *arg);
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

static wstring_ty *sub_dirname _((sub_context_ty *, wstring_list_ty *));

static wstring_ty *
sub_dirname(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_dirname()\n{\n"/*}*/));
	if (arg->nitems != 2)
	{
		scp->suberr = i18n("requires one argument");
		result = 0;
	}
	else
	{
		string_ty	*s1;
		string_ty	*s2;

		s1 = wstr_to_str(arg->item[1]);
		os_become_orig();
		s2 = os_dirname(s1);
		os_become_undo();
		str_free(s1);
		result = str_to_wstr(s2);
		str_free(s2);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_errno - the errno substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_errno(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_errno function implements the errno substitution.
 *	The errno substitution is replaced by the value if th errno
 *	variable provided by the system, as mapped through the strerror function.
 *
 *	Requires exactly zero arguments.
 *
 *	The sub_errno_set() function may be used to remember errno,
 *	and thus isolate the error from subsequest system calls.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static wstring_ty *sub_errno _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_errno(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_errno()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else
	{
		if (scp->errno_sequester == 0)
			scp->errno_sequester = errno;
		if
		(
			scp->errno_sequester == EPERM
		||
			scp->errno_sequester == EACCES
		)
		{
			int		uid;
			struct passwd	*pw;
			char		uidn[20];
			int		gid;
			struct group	*gr;
			char		gidn[20];
			string_ty	*s;

			os_become_query(&uid, &gid, (int *)0);
			pw = getpwuid(uid);
			if (pw)
				sprintf(uidn, "user \"%.8s\"", pw->pw_name);
			else
				sprintf(uidn, "uid %d", uid);

			gr = getgrgid(gid);
			if (gr)
				sprintf(gidn, "group \"%.8s\"", gr->gr_name);
			else
				sprintf(gidn, "gid %d", gid);

			s =
				str_format
				(
					"%s [%s, %s]",
					strerror(scp->errno_sequester),
					uidn,
					gidn
				);
			result = str_to_wstr(s);
			str_free(s);
		}
		else
			result = wstr_from_c(strerror(scp->errno_sequester));
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_downcase - the downcase substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_downcase(wstring_list_ty *arg);
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

static wstring_ty *sub_downcase _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_downcase(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_downcase()\n{\n"/*}*/));
	if (arg->nitems != 2)
	{
		scp->suberr = i18n("requires one argument");
		result = 0;
	}
	else
		result = wstr_to_lower(arg->item[1]);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_identifier - the identifier substitution
 *
 * SYNOPSIS
 *	string_ty *sub_identifier(wstring_list_ty *arg);
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

static wstring_ty *sub_identifier _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_identifier(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_identifier()\n{\n"/*}*/));
	if (arg->nitems != 2)
	{
		scp->suberr = i18n("requires one argument");
		result = 0;
	}
	else
		result = wstr_to_ident(arg->item[1]);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_integration_directory - the integration_directory substitution
 *
 * SYNOPSIS
 *	string_ty *sub_integration_directory(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *      The sub_integration_directory function implements the
 *	integration_directory substitution.  The integration_directory
 *	substitution is used to insert the absolute path of the
 *	integration_directory.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static wstring_ty *sub_integration_directory _((sub_context_ty *,
	wstring_list_ty *));

static wstring_ty *
sub_integration_directory(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	cstate		cstate_data;

	trace(("sub_integration_directory()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else if (!scp->cp)
	{
		yuck:
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
	{
		cstate_data = change_cstate_get(scp->cp);
		if (cstate_data->state != cstate_state_being_integrated)
			goto yuck;
		result =
			str_to_wstr
			(
				change_integration_directory_get(scp->cp, 0)
			);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_integrator - the integrator substitution
 *
 * SYNOPSIS
 *	string_ty *sub_integrator(wstring_list_ty *arg);
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

static wstring_ty *sub_integrator _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_integrator(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	string_ty	*s;

	trace(("sub_integrator()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else if (!scp->cp)
	{
		yuck:
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
	{
		s = change_integrator_name(scp->cp);
		if (!s)
			goto yuck;
		result = str_to_wstr(s);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_integrator_list - the integrator_list substitution
 *
 * SYNOPSIS
 *	string_ty *sub_integrator_list(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_integrator_list function implements the integrator_list
 *	substitution.  The integrator_list substitution is replaced by
 *	a space separated list of the project's integrators.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static wstring_ty *sub_integrator_list _((sub_context_ty *, wstring_list_ty *));

static wstring_ty *
sub_integrator_list(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	long		j;
	string_list_ty		wl;
	string_ty	*s;

	trace(("sub_integrator_list()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else if (!scp->pp)
	{
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
	{
		/*
		 * build a string containing all of the project integrators
		 */
		string_list_constructor(&wl);
		for (j = 0; ; ++j)
		{
			s = project_integrator_nth(scp->pp, j);
			if (!s)
				break;
			string_list_append(&wl, s);
		}
		s = wl2str(&wl, 0, wl.nstrings, " ");
		string_list_destructor(&wl);
		result = str_to_wstr(s);
		str_free(s);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_library - the library substitution
 *
 * SYNOPSIS
 *	string_ty *sub_library(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_library function implements the library substitution.
 *	The library substitution is replaced by the absolute path of
 *	aegis' library directory.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static wstring_ty *sub_library _((sub_context_ty *, wstring_list_ty *));

static wstring_ty *
sub_library(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_library()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else
		result = wstr_from_c(configured_datadir());
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static wstring_ty *sub_plural _((sub_context_ty *, wstring_list_ty *));

static wstring_ty *
sub_plural(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	string_ty	*s;
	wstring_ty	*result;
	long		n;

	trace(("sub_plural()\n{\n"/*}*/));
	switch (arg->nitems)
	{
	default:
		scp->suberr = i18n("requires two or three arguments");
		result = 0;
		break;

	case 3:
		wstring_list_append(arg, wstr_from_c(""));
		/* fall through... */

	case 4:
		s = wstr_to_str(arg->item[1]);
		n = atol(s->str_text);
		str_free(s);
		if (n != 1)
			result = wstr_copy(arg->item[2]);
		else
			result = wstr_copy(arg->item[3]);
		break;
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_project - the project substitution
 *
 * SYNOPSIS
 *	string_ty *sub_project(wstring_list_ty *arg);
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

static wstring_ty *sub_project _((sub_context_ty *, wstring_list_ty *));

static wstring_ty *
sub_project(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_project()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else if (!scp->pp)
	{
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
		result = str_to_wstr(project_name_get(scp->pp));
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_read_file - the read_file substitution
 *
 * SYNOPSIS
 *	string_ty *sub_read_file(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_read_file function implements the read_file substitution.
 *	The read_file substitution is replaced by the contents of the
 *	named file.  An absolute path must be supplied.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static wstring_ty *sub_read_file _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_read_file(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_read_file()\n{\n"/*}*/));
	if (arg->nitems != 2)
	{
		scp->suberr = i18n("requires one argument");
		result = 0;
	}
	else
	{
		if (arg->item[1]->wstr_text[0] != '/')
		{
			scp->suberr = i18n("absolute path required");
			result = 0;
		}
		else
		{
			string_ty	*s1;
			string_ty	*s2;

			s1 = wstr_to_str(arg->item[1]);
			os_become_orig();
			s2 = read_whole_file(s1->str_text);
			os_become_undo();
			str_free(s1);
			result = str_to_wstr(s2);
			str_free(s2);
		}
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_reviewer - the reviewer substitution
 *
 * SYNOPSIS
 *	string_ty *sub_reviewer(wstring_list_ty *arg);
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

static wstring_ty *sub_reviewer _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_reviewer(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	string_ty	*s;

	trace(("sub_reviewer()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else if (!scp->cp)
	{
		yuck:
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
	{
		s = change_reviewer_name(scp->cp);
		if (!s)
			goto yuck;
		result = str_to_wstr(s);
		/* do not free s */
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_reviewer_list - the reviewer_list substitution
 *
 * SYNOPSIS
 *	string_ty *sub_reviewer_list(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_reviewer_list function implements the reviewer_list
 *	substitution.  The reviewer_list substitution is replaced by a
 *	space separated list of the project's reviewers.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static wstring_ty *sub_reviewer_list _((sub_context_ty *, wstring_list_ty *));

static wstring_ty *
sub_reviewer_list(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	long		j;
	string_list_ty		wl;
	string_ty	*s;

	trace(("sub_reviewer_list()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else if (!scp->pp)
	{
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
	{
		/*
		 * build a string containing all of the project reviewers
		 */
		string_list_constructor(&wl);
		for (j = 0; ; ++j)
		{
			s = project_reviewer_nth(scp->pp, j);
			if (!s)
				break;
			string_list_append(&wl, s);
		}
		s = wl2str(&wl, 0, wl.nstrings, " ");
		string_list_destructor(&wl);
		result = str_to_wstr(s);
		str_free(s);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_search_path - the search_path substitution
 *
 * SYNOPSIS
 *	string_ty *sub_search_path(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_search_path function implements the search_path
 *	substitution.  The search_path substitution is replaced by a
 *	colon separated list of absolute paths to search when building a
 *	change, it will point from a change to its branch and so on up
 *	to the project trunk.
 *
 *	Requires exactly zero arguments.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static wstring_ty *sub_search_path _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_search_path(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_shell()\n{\n"/*}*/));
	result = 0;
	if (arg->nitems != 1)
		scp->suberr = i18n("requires zero arguments");
	else if (!scp->cp)
	{
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
	{
		string_list_ty		tmp;
		string_ty	*s;

		change_search_path_get(scp->cp, &tmp, 0);
		s = wl2str(&tmp, 0, tmp.nstrings, ":");
		string_list_destructor(&tmp);
		result = str_to_wstr(s);
		str_free(s);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_shell - the shell substitution
 *
 * SYNOPSIS
 *	string_ty *sub_shell(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *      The sub_shell function implements the shell substitution.
 *      The shell substitution is replaced by the absolute path of a
 *	Bourne shell which understands functions.
 *
 *	Requires exactly zero arguments.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static wstring_ty *sub_shell _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_shell(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_shell()\n{\n"/*}*/));
	result = 0;
	if (arg->nitems != 1)
		scp->suberr = i18n("requires zero arguments");
	else
		result = wstr_from_c(os_shell());
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_source - the source substitution
 *
 * SYNOPSIS
 *	string_ty *sub_source(wstring_list_ty *arg);
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

static wstring_ty *sub_source _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_source(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	cstate		cstate_data;
	int		absolute;
	string_ty	*fn;
	string_ty	*s;

	trace(("sub_source()\n{\n"/*}*/));
	absolute = 0;
	result = 0;
	if (!scp->cp)
	{
		scp->suberr = i18n("not valid in current context");
		goto done;
	}
	switch (arg->nitems)
	{
	default:
		scp->suberr = i18n("requires one argument");
		goto done;

	case 2:
		break;

	case 3:
		s = wstr_to_str(arg->item[2]);
		if (arglex_compare("Relative", s->str_text))
		{
			str_free(s);
			break;
		}
		if (arglex_compare("Absolute", s->str_text))
		{
			str_free(s);
			absolute = 1;
			break;
		}
		str_free(s);
		scp->suberr = i18n("second argument must be \"Absolute\" or \"Relative\"");
		goto done;
		break;
	}

	/*
	 * make sure we are in an appropriate state
	 */
	cstate_data = change_cstate_get(scp->cp);
	if (cstate_data->state == cstate_state_awaiting_development)
	{
		scp->suberr = i18n("not valid in current context");
		goto done;
	}

	/*
	 * find the file's path
	 */
	fn = wstr_to_str(arg->item[1]);
	if (cstate_data->state == cstate_state_completed)
		s = project_file_path(scp->pp, fn);
	else
		s = change_file_source(scp->cp, fn);
	if (!s)
	{
		str_free(fn);
		scp->suberr = i18n("source file unknown");
		goto done;
	}

	/*
	 * To turn absolute paths into relative ones, we need to see if
	 * the file is in the first element of the search path.
	 */
	if (!absolute)
	{
		string_list_ty		search_path;
		string_ty	*s2;

		if (cstate_data->state == cstate_state_completed)
		{
			string_list_constructor(&search_path);
			project_search_path_get(scp->pp, &search_path, 0);
		}
		else
			change_search_path_get(scp->cp, &search_path, 0);
		s2 = os_below_dir(search_path.string[0], s);
		if (s2)
		{
			str_free(s2);
			str_free(s);
			s = str_copy(fn);
		}
		string_list_destructor(&search_path);
	}

	/*
	 * build the result
	 */
	result = str_to_wstr(s);
	str_free(fn);
	str_free(s);

	/*
	 * here for all exits
	 */
	done:
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_state - the state substitution
 *
 * SYNOPSIS
 *	string_ty *sub_state(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_state function implements the state substitution.
 *	The state substitution is replaced by the name of the state the current change is in.
 *
 *	Requires exactly zero arguments.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static wstring_ty *sub_state _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_state(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_state()\n{\n"/*}*/));
	result = 0;
	if (!scp->cp)
		scp->suberr = i18n("not valid in current context");
	else
	{
		cstate		cstate_data;

		cstate_data = change_cstate_get(scp->cp);
		result = wstr_from_c(cstate_state_ename(cstate_data->state));
	}

	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_architecture - the architecture substitution
 *
 * SYNOPSIS
 *	string_ty *sub_architecture(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_architecture function implements the architecture
 *	substitution.  The architecture substitution is replaced by the
 *	architecture variant pattern appropriate for the current
 *	execution environment.  Requires no arguments.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static wstring_ty *sub_architecture _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_architecture(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_architecture()\n{\n"/*}*/));
	if (arg->nitems > 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else if (!scp->cp)
	{
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
		result = str_to_wstr(change_architecture_name(scp->cp, 1));

	/*
	 * here for all exits
	 */
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_upcase - the upcase substitution
 *
 * SYNOPSIS
 *	string_ty *sub_upcase(wstring_list_ty *arg);
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

static wstring_ty *sub_upcase _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_upcase(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_upcase()\n{\n"/*}*/));
	if (arg->nitems != 2)
	{
		scp->suberr = i18n("requires one argument");
		result = 0;
	}
	else
		result = wstr_to_upper(arg->item[1]);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_user - the user substitution
 *
 * SYNOPSIS
 *	string_ty *sub_user(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_user function implements the user substitution.
 *	The user substitution is replaced by the login name of the user
 *	who executed the current command.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

static wstring_ty *sub_user _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_user(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	user_ty		*up;

	trace(("sub_user()\n{\n"/*}*/));
	if (arg->nitems != 1)
	{
		scp->suberr = i18n("requires zero arguments");
		result = 0;
	}
	else
	{
		up = user_executing((project_ty *)0);
		result = str_to_wstr(user_name(up));
		user_free(up);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_version - the version substitution
 *
 * SYNOPSIS
 *	string_ty *sub_version(wstring_list_ty *arg);
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

static wstring_ty *sub_version _((sub_context_ty *, wstring_list_ty *arg));

static wstring_ty *
sub_version(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	string_ty	*s2;

	trace(("sub_version()\n{\n"/*}*/));
	result = 0;
	if (arg->nitems != 1)
		scp->suberr = i18n("requires zero arguments");
	else if (!scp->cp || scp->cp->bogus)
	{
		scp->suberr = i18n("not valid in current context");
		result = 0;
	}
	else
	{
		s2 = change_version_get(scp->cp);
		result = str_to_wstr(s2);
		str_free(s2);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static sub_table_ty table[] =
{
	{ "$",				sub_dollar,			},
	{ "#",				sub_comment,			},
	{ "Administrator_List",		sub_administrator_list,		},
	{ "ARCHitecture",		sub_architecture,		},
	{ "BaseLine",			sub_baseline,			},
	{ "Basename",			sub_basename,			},
	{ "Change",			sub_change,			},
	{ "Copyright_Years",		sub_copyright_years,		},
	{ "COMment",			sub_comment,			},
	{ "DAte",			sub_date,			},
	{ "DELta",			sub_delta,			},
	{ "DEVeloper",			sub_developer,			},
	{ "DEVeloper_List",		sub_developer_list,		},
	/* Default_Development_Directory				*/
	{ "Development_Directory",	sub_development_directory,	},
	{ "Dirname",			sub_dirname,			},
	{ "DownCase",			sub_downcase,			},
	{ "DOLlar",			sub_dollar,			},
	/* Edit								*/
	{ "ERrno",			sub_errno,			},
	{ "EXpression",			sub_expression,			},
	/* FieLD_List							*/
	/* File_List							*/
	/* File_Name							*/
	/* Guess							*/
	/* History							*/
	/* Input							*/
	{ "IDentifier",			sub_identifier,			},
	{ "INTegration_Directory",	sub_integration_directory,	},
	{ "INTegrator",			sub_integrator,			},
	{ "INTegrator_List",		sub_integrator_list,		},
	{ "LEFt",			sub_left,			},
	{ "LENgth",			sub_length,			},
	{ "LIBrary",			sub_library,			},
	/* MAgic							*/
	/* MeSsaGe							*/
	/* Most_Recent							*/
	{ "Name_Maximum",		sub_namemax,			},
	/* Number							*/
	/* Output							*/
	/* ORiginal							*/
	{ "PLural",			sub_plural,			},
	{ "Project",			sub_project,			},
	{ "QUote",			sub_quote,			},
	{ "Read_File",			sub_read_file, RESUB_DOLLARS,	},
	{ "Reviewer",			sub_reviewer,			},
	{ "Reviewer_List",		sub_reviewer_list,		},
	{ "RIght",			sub_right,			},
	{ "Search_Path",		sub_search_path,		},
	{ "SHell",			sub_shell,			},
	{ "Source",			sub_source,			},
	{ "STate",			sub_state,			},
	{ "Trim_DIRectory",		sub_trim_directory,		},
	{ "Trim_EXTension",		sub_trim_extension,		},
	{ "UName", /* undocumented */	sub_architecture,		},
	{ "UpCase",			sub_upcase,			},
	{ "USer",			sub_user,			},
	{ "Version",			sub_version,			},
	{ "Zero_Pad",			sub_zero_pad,			},
};


/*
 * NAME
 *	sub_diversion
 *
 * SYNOPSIS
 *	void sub_diversion(sub_context_ty *, wstring_ty *s, int resub);
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

static void sub_diversion _((sub_context_ty *, wstring_ty *, int));

static void
sub_diversion(scp, s, resubstitute)
	sub_context_ty	*scp;
	wstring_ty	*s;
	int		resubstitute;
{
	sub_diversion_ty *dp;

	trace(("sub_diversion(s = %8.8lX, resub = %d)\n{\n"/*}*/,
		(long)s, resubstitute));
	dp = (sub_diversion_ty *)mem_alloc(sizeof(sub_diversion_ty));
	dp->text = wstr_copy(s);
	dp->pos = 0;
	dp->resubstitute = resubstitute;
	dp->prev = scp->diversion;
	scp->diversion = dp;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	sub_diversion_close
 *
 * SYNOPSIS
 *	void sub_diversion_close(sub_context_ty *);
 *
 * DESCRIPTION
 *	The sub_diversion_close function is used to
 *	release a diversion when it has been exhausted.
 */

static void sub_diversion_close _((sub_context_ty *));

static void
sub_diversion_close(scp)
	sub_context_ty	*scp;
{
	sub_diversion_ty *dp;

	trace(("sub_diversion_close()\n{\n"/*}*/));
	assert(scp->diversion);
	dp = scp->diversion;
	scp->diversion = dp->prev;
	wstr_free(dp->text);
	mem_free(dp);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	execute
 *
 * SYNOPSIS
 *	void execute(wstring_list_ty *args);
 *
 * DESCRIPTION
 *	The execute function is used to perform the substitution
 *	described by the argument list.
 *
 * ARGUMENTS
 *	args - the name and arguments of the substitution
 */

static void execute _((sub_context_ty *scp, wstring_list_ty *));

static void
execute(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	string_ty	*cmd;
	wstring_ty	*s;
	sub_table_ty	*hit[20];
	int		nhits;
	sub_table_ty	*tp;
	long		j;

	trace(("execute()\n{\n"/*}*/));
	if (arg->nitems == 0)
	{
		sub_context_ty	*inner;

		inner = sub_context_new();
		sub_var_set(inner, "File_Name", "%s", scp->file_name);
		sub_var_set(inner, "Line_Number", "%d", scp->line_number);
		fatal_intl
		(
			inner,
			i18n("$filename: $linenumber: empty $${} substitution")
		);
		/* NOTREACHED */
		sub_context_delete(inner);
	}
	
	/*
	 * scan the variables
	 */
	nhits = 0;
	cmd = wstr_to_str(arg->item[0]);
	for (j = 0; j < scp->sub_var_pos; ++j)
	{
		tp = &scp->sub_var_list[j];
		if (arglex_compare(tp->name, cmd->str_text))
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
		if (arglex_compare(tp->name, cmd->str_text))
		{
			if (tp->override)
				goto override;
			if (nhits < SIZEOF(hit))
				hit[nhits++] = tp;
		}
	}
	str_free(cmd);

	/*
	 * figure what to do
	 */
	switch (nhits)
	{
	case 0:
		scp->suberr = i18n("unknown substitution name");
		s = 0;
		break;

	case 1:
		tp = hit[0];
		override:
		if (tp->value)
		{
			if (isdigit(tp->name[0]))
			{
				sub_context_ty	*inner;
				string_ty	*vs;

				inner = sub_context_new();
				sub_var_set(inner, "Name1", "%%%s", tp->name);
				vs = wstr_to_str(tp->value);
				sub_var_set(inner, "Name2", "%S", vs);
				trace(("%s -> %s\n", tp->name, vs->str_text));
				str_free(vs);
				error_intl
				(
					inner,
		    i18n("warning: $name1 is obsolete, use $name2 substitution")
				);
				sub_context_delete(inner);
			}

			/*
			 * flag that the variable has been used
			 */
			tp->must_be_used = 0;
			s = wstr_copy(tp->value);
		}
		else
		{
			wstr_free(arg->item[0]);
			arg->item[0] = wstr_from_c(tp->name);
			s = tp->func(scp, arg);
		}
		break;

	default:
		scp->suberr = i18n("ambiguous substitution name");
		s = 0;
		break;
	}

	/*
	 * deal with the result
	 */
	if (s)
	{
		sub_diversion(scp, s, tp->resubstitute);
		wstr_free(s);
	}
	else
	{
		wstring_ty	*s2;
		string_ty	*s3;
		sub_context_ty	*inner;
		char		*the_error;

		assert(scp->suberr);
		s2 = wstring_list_to_wstring(arg, 0, arg->nitems, (char *)0);
		s3 = wstr_to_str(s2);
		wstr_free(s2);
		the_error = scp->suberr ? scp->suberr : "this is a bug";
		inner = sub_context_new();
		sub_var_set(inner, "File_Name", "%s", scp->file_name);
		sub_var_set(inner, "Line_Number", "%d", scp->line_number);
		sub_var_set(inner, "Name", "%S", s3);
		sub_var_set(inner, "MeSsaGe", "%s", gettext(the_error));
		fatal_intl
		(
			inner,
	 i18n("$filename: $linenumber: substitution $${$name} failed: $message")
		);
		/* NOTREACHED */
		sub_context_delete(inner);
		str_free(s3);
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	sub_getc_meta
 *
 * SYNOPSIS
 *	wchar_t sub_getc_meta(sub_context_ty *, getc_type *);
 *
 * DESCRIPTION
 *	The sub_getc_meta function is used to get a character from
 *	the current input string.  When the current string is exhaused,
 *	the previous string is resumed.
 *
 * RETURNS
 *	int - the chacater, or NUL to indicate end of input
 */

static wchar_t sub_getc_meta _((sub_context_ty *, getc_type *));

static wchar_t
sub_getc_meta(scp, tp)
	sub_context_ty	*scp;
	getc_type	*tp;
{
	wchar_t		result;
	sub_diversion_ty *dp;

	trace(("sub_getc_meta()\n{\n"/*}*/));
	dp = scp->diversion;
	if (dp && (dp->resubstitute & RESUB_BOTH))
		*tp = getc_type_control;
	else
		*tp = getc_type_data;
	if (!dp)
		result = 0;
	else if (dp->pos >= dp->text->wstr_length)
		result = 0;
	else
		result = dp->text->wstr_text[dp->pos++];
#ifdef DEBUG
	if (iswprint(result) && result >= CHAR_MIN && result <= CHAR_MAX)
		trace(("return '%c' %s;\n", (char)result,
		(*tp == getc_type_control ? "control" : "data")));
	else
		trace(("return %4.4lX %s;\n", (long)result,
		(*tp == getc_type_control ? "control" : "data")));
#endif
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_getc_meta_undo
 *
 * SYNOPSIS
 *	void sub_getc_meta_undo(sub_context_ty *, wchar_t c);
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

static void sub_getc_meta_undo _((sub_context_ty *, wchar_t));

static void
sub_getc_meta_undo(scp, c)
	sub_context_ty	*scp;
	wchar_t		c;
{
	sub_diversion_ty *dp;

	trace(("sub_getc_meta_undo(%ld)\n{\n"/*}*/, (long)c));
#ifdef DEBUG
	if (iswprint(c) && c >= CHAR_MIN && c <= CHAR_MAX)
		trace(("c = '%c'\n", (char)c));
#endif
	dp = scp->diversion;
	assert(dp);
	if (!c)
	{
		assert(dp->pos == dp->text->wstr_length);
	}
	else
	{
		assert(dp->pos >= 1);
		dp->pos--;
		assert(c == dp->text->wstr_text[dp->pos]);
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	collect
 *
 * SYNOPSIS
 *	void collect(collect_ty *, int c);
 *
 * DESCRIPTION
 *	The collect function is used to accumulate a string
 *	one character at a time.  No size limit.
 *
 * ARGUMENTS
 *	c - the character being collected
 */

static void collect _((collect_ty *, wchar_t));

static void
collect(cp, c)
	collect_ty	*cp;
	wchar_t		c;
{
	if (cp->pos >= cp->size)
	{
		size_t	nbytes;

		cp->size += (1L << 10);
		nbytes = cp->size * sizeof(wchar_t);
		cp->buf = mem_change_size(cp->buf, nbytes);
	}
	cp->buf[cp->pos++] = c;
}


static void collect_n _((collect_ty *, wchar_t *, size_t));

static void
collect_n(cp, s, n)
	collect_ty	*cp;
	wchar_t		*s;
	size_t		n;
{
	while (n > 0)
	{
		collect(cp, *s++);
		--n;
	}
}


/*
 * NAME
 *	collect_end
 *
 * SYNOPSIS
 *	wstring_ty *collect_end(collect_ty *);
 *
 * DESCRIPTION
 *	The collect_end function is used to fetch the string
 *	accumulated with the collect function.
 *	The bufferer for the collect function is cleared.
 *
 * RETURNS
 *	wstring_ty *; pointer to the string in dynamic memory.
 */

static wstring_ty *collect_end _((collect_ty *));

static wstring_ty *
collect_end(cp)
	collect_ty	*cp;
{
	wstring_ty	*result;

	result = wstr_n_from_wc(cp->buf, cp->pos);
	cp->pos = 0;
	return result;
}


/*
 * NAME
 *	percent
 *
 * SYNOPSIS
 *	wchar_t percent(void);
 *
 * DESCRIPTION
 *	The percent function is used to perform percent (%) substitutions.
 *	On entry, the % is expected to have been consumed.
 *
 *	The substitution is usually achieved as a side-effect,
 *	by using the sub_diversion function.
 *
 * RETURNS
 *	wchar_t	a character to deliver as output,
 *		or NUL if none.
 */

static wchar_t percent _((sub_context_ty *));

static wchar_t
percent(scp)
	sub_context_ty	*scp;
{
	wchar_t		c;
	wchar_t		result;
	wchar_t		c2;
	string_ty	*s;
	wstring_ty	*ws;
	static int	warned;
	string_ty	*s2;
	collect_ty	tmp;
	getc_type	ct;

	trace(("percent()\n{\n"/*}*/));
	collect_constructor(&tmp);
	result = 0;
	c = sub_getc_meta(scp, &ct);
	if (ct != getc_type_control)
		goto normal;
	switch (c)
	{
	case 'P':
		c2 = sub_getc_meta(scp, &ct);
		switch (c2)
		{
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			s = str_format("${dirname ${%c}}", (char)c2);
			ws = str_to_wstr(s);
			str_free(s);
			sub_diversion(scp, ws, RESUB_DOLLARS);
			wstr_free(ws);
			break;

		default:
			sub_getc_meta_undo(scp, c2);
			sub_getc_meta_undo(scp, c);
			result = '%';
			break;
		}
		break;

	case 'F':
		c2 = sub_getc_meta(scp, &ct);
		switch (c2)
		{
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			s = str_format("${basename ${%c}}", (char)c2);
			ws = str_to_wstr(s);
			str_free(s);
			sub_diversion(scp, ws, RESUB_DOLLARS);
			wstr_free(ws);
			break;

		default:
			sub_getc_meta_undo(scp, c2);
			sub_getc_meta_undo(scp, c);
			result = '%';
			break;
		}
		break;

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		s = str_format("${%c}", (char)c);
		ws = str_to_wstr(s);
		str_free(s);
		sub_diversion(scp, ws, RESUB_DOLLARS);
		wstr_free(ws);
		break;

	case '('/*)*/:
		for (;;)
		{
			c = sub_getc_meta(scp, &ct);
			if (!c)
			{
				sub_context_ty	*inner;

				inner = sub_context_new();
				sub_var_set(inner, "File_Name", "%s", scp->file_name);
				sub_var_set(inner, "Line_Number", "%d", scp->line_number);
				fatal_intl
				(
					inner,
		  i18n("$filename: $linenumber: unterminated %%() substitution")
				);
				/* NOTREACHED */
				sub_context_delete(inner);
			}
			if (c == /*(*/')' && ct == getc_type_control)
				break;
			collect(&tmp, c);
		}
		ws = collect_end(&tmp);
		s2 = wstr_to_str(ws);
		wstr_free(ws);
		s = str_format("${source %S}", s2);
		str_free(s2);
		ws = str_to_wstr(s);
		str_free(s);
		sub_diversion(scp, ws, RESUB_DOLLARS);
		wstr_free(ws);
		break;

	case '%':
		result = '%';
		break;

	default:
		normal:
		sub_getc_meta_undo(scp, c);
		result = '%';
#ifdef DEBUG
		error_raw("warning: probable un-internationalized error message");
#endif
		break;
	}
	if (!result && !warned)
	{
		sub_context_ty	*inner;

		inner = sub_context_new();
		sub_var_set(inner, "File_Name", "%s", scp->file_name);
		sub_var_set(inner, "Line_Number", "%d", scp->line_number);
		error_intl
		(
			inner,
		      i18n("$filename: $linenumber: don't use %% substitutions")
		);
		sub_context_delete(inner);
		warned = 1;
	}
	collect_destructor(&tmp);
	trace(("return %4.4lX\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	dollar
 *
 * SYNOPSIS
 *	wchar_t dollar(sub_context_ty *);
 *
 * DESCRIPTION
 *	The dollar function is used to perform dollar ($) substitutions.
 *	On entry, the $ is expected to have been consumed.
 *
 *	The substitution is usually achieved as a side-effect,
 *	by using the sub_diversion function.
 *
 * RETURNS
 *	wchar_t	a character to deliver as output,
 *		or NUL if none.
 */

static wchar_t sub_getc _((sub_context_ty *, getc_type *)); /* forward */

static wchar_t dollar _((sub_context_ty *));

static wchar_t
dollar(scp)
	sub_context_ty	*scp;
{
	wstring_list_ty	arg;
	int		result;
	wchar_t		c;
	wstring_ty	*s;
	wchar_t		quoted;
	collect_ty	tmp;
	getc_type	ct;

	trace(("dollar()\n{\n"/*}*/));
	collect_constructor(&tmp);
	wstring_list_zero(&arg);
	result = 0;
	c = sub_getc_meta(scp, &ct);
	if (ct != getc_type_control)
		goto normal;
	switch (c)
	{
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9': 
		for (;;)
		{
			collect(&tmp, c);
			c = sub_getc_meta(scp, &ct);
			switch (c)
			{
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				continue;

			default:
				sub_getc_meta_undo(scp, c);
				break;
			}
			break;
		}
		s = collect_end(&tmp);
		trace(("push arg\n"));
		wstring_list_append(&arg, s);
		wstr_free(s);
		execute(scp, &arg);
		wstring_list_free(&arg);
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
			collect(&tmp, c);
			c = sub_getc_meta(scp, &ct);
			switch (c)
			{
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f': case 'g': case 'h': case 'i': case 'j':
			case 'k': case 'l': case 'm': case 'n': case 'o':
			case 'p': case 'q': case 'r': case 's': case 't':
			case 'u': case 'v': case 'w': case 'x': case 'y':
			case 'z': 
			case 'A': case 'B': case 'C': case 'D': case 'E':
			case 'F': case 'G': case 'H': case 'I': case 'J':
			case 'K': case 'L': case 'M': case 'N': case 'O':
			case 'P': case 'Q': case 'R': case 'S': case 'T':
			case 'U': case 'V': case 'W': case 'X': case 'Y':
			case 'Z': 
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9': 
			case '_': case '-':
				continue;

			default:
				sub_getc_meta_undo(scp, c);
				break;
			}
			break;
		}
		s = collect_end(&tmp);
		trace(("push arg\n"));
		wstring_list_append(&arg, s);
		wstr_free(s);
		execute(scp, &arg);
		wstring_list_free(&arg);
		break;

	case '{'/*}*/:
		c = sub_getc(scp, &ct);
		for (;;)
		{
			/*
			 * look for terminator
			 */
			if (c == /*{*/'}' && ct == getc_type_control)
				break;

			/*
			 * watch out for unterminated substitutions
			 */
			if (!c)
			{
				sub_context_ty	*inner;

				inner = sub_context_new();
				sub_var_set(inner, "File_Name", "%s", scp->file_name);
				sub_var_set(inner, "Line_Number", "%d", scp->line_number);
				fatal_intl
				(
					inner,
		  i18n("$filename: $linenumber: unterminated $${} substitution")
				);
				/* NOTREACHED */
				sub_context_delete(inner);
				break;
			}

			/*
			 * skip white space separating the arguments
			 */
			if (ct == getc_type_control && iswspace(c))
			{
				c = sub_getc(scp, &ct);
				continue;
			}

			/*
			 * collect the argument
			 *	any run of non-white-space characters
			 */
			quoted = 0;
			for (;;)
			{
				if (!c)
				{
					if (quoted)
					{
						sub_context_ty	*inner;

						inner = sub_context_new();
						sub_var_set(inner, "File_Name", "%s", scp->file_name);
						sub_var_set(inner, "Line_Number", "%d", scp->line_number);
						fatal_intl
						(
							inner,
			i18n("$filename: $linenumber: unterminated $${} quotes")
						);
						/* NOTREACHED */
						sub_context_delete(inner);
					}
					break;
				}
				if (!quoted && ct == getc_type_control && (iswspace(c) || c == /*{*/'}'))
					break;
				if (c == quoted)
				{
					assert(quoted);
					quoted = 0;
				}
				else if (c == '\'' && ct == getc_type_control)
				{
					assert(!quoted);
					quoted = c;
				}
				else if (c == '\\' && ct == getc_type_control)
				{
					c = sub_getc(scp, &ct);
					if (!c)
					{
						sub_context_ty	*inner;

						inner = sub_context_new();
						sub_var_set(inner, "File_Name", "%s", scp->file_name);
						sub_var_set(inner, "Line_Number", "%d", scp->line_number);
						fatal_intl
						(
							inner,
		   i18n("$filename: $linenumber: unterminated $${} \\ sequence")
						);
						/* NOTREACHED */
						sub_context_delete(inner);
					}
					collect(&tmp, c);
				}
				else
					collect(&tmp, c);
				c = sub_getc(scp, &ct);
			}
			s = collect_end(&tmp);
			trace(("push arg\n"));
			wstring_list_append(&arg, s);
			wstr_free(s);
		}
		execute(scp, &arg);
		wstring_list_free(&arg);
		break;

	case '$':
		result = '$';
		break;

	case '#':
		for (;;)
		{
			c = sub_getc_meta(scp, &ct);
			if (!c || (c == '\n' && ct == getc_type_control))
				break;
		}
		result = 0;
		break;

	default:
		normal:
		sub_getc_meta_undo(scp, c);
		result = '$';
		break;
	}
	collect_destructor(&tmp);
#ifdef DEBUG
	if (iswprint(result) && result >= CHAR_MIN && result <= CHAR_MAX)
		trace(("return '%c';\n", (char)result));
	else
		trace(("return %4.4lX;\n", (long)result));
#endif
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_getc
 *
 * SYNOPSIS
 *	wchar_t sub_getc(sub_context_ty *);
 *
 * DESCRIPTION
 *	The sub_getc function is used to get a character from
 *	the substitution stream.  This is used both for the final output
 *	and for fetching arguments to dollar ($) substitutions.
 *
 * RETURNS
 *	wchar_t	a character from the stream,
 *		or NUL to indicate end of input.
 */

static wchar_t sub_getc _((sub_context_ty *, getc_type *));

static wchar_t
sub_getc(scp, tp)
	sub_context_ty	*scp;
	getc_type	*tp;
{
	wchar_t		c;

	trace(("sub_getc()\n{\n"/*}*/));
	for (;;)
	{
		c = sub_getc_meta(scp, tp);
		if (c && *tp != getc_type_control)
			break;
		switch (c)
		{
		default:
			break;

		case 0:
			if (!scp->diversion)
				break;
			sub_diversion_close(scp);
			continue;

		case '%':
			if (!(scp->diversion->resubstitute & RESUB_PERCENT))
				break;
			c = percent(scp);
			if (!c)
				continue;
			*tp = getc_type_data;
			break;

		case '$':
			if (!(scp->diversion->resubstitute & RESUB_DOLLARS))
				break;
			c = dollar(scp);
			if (!c)
				continue;
			*tp = getc_type_data;
			break;
		}
		break;
	}
#ifdef DEBUG
	if (iswprint(c) && c >= CHAR_MIN && c <= CHAR_MAX)
		trace(("return '%c' %s;\n", (char)c,
		(*tp == getc_type_control ? "control" : "data")));
	else
		trace(("return %4.4lX;\n", (long)c,
		(*tp == getc_type_control ? "control" : "data")));
#endif
	trace((/*{*/"}\n"));
	return c;
}


void
subst_intl_project(scp, a)
	sub_context_ty	*scp;
	project_ty	*a;
{
	if (a != scp->pp)
	{
		assert(!scp->pp);
		assert(!scp->cp);
		scp->pp = a;
		scp->cp = 0;
	}
}


void
subst_intl_change(scp, a)
	sub_context_ty	*scp;
	change_ty	*a;
{
	assert(!scp->pp);
	assert(!scp->cp);
	scp->pp = a->pp;
	scp->cp = a;
}


static wstring_ty *subst _((sub_context_ty *, wstring_ty *));

static wstring_ty *
subst(scp, s)
	sub_context_ty	*scp;
	wstring_ty	*s;
{
	collect_ty	buf;
	wchar_t		c;
	wstring_ty	*result;
	sub_table_ty	*tp;
	sub_table_ty	*the_end;
	int		error_count;
	getc_type	ct;

	trace(("subst(s = %8.8lX)\n{\n"/*}*/, (long)s));
	collect_constructor(&buf);
	sub_diversion(scp, s, RESUB_BOTH);
	for (;;)
	{
		/*
		 * get the next character
		 */
		c = sub_getc(scp, &ct);
		if (!c)
			break;

		/*
		 * save the character
		 */
		collect(&buf, c);
	}

	/*
	 * find any unused variables marked "append if unused"
	 */
	the_end = scp->sub_var_list + scp->sub_var_pos;
	for (tp = scp->sub_var_list; tp < the_end; ++tp)
	{
		if (!tp->append_if_unused)
			continue;
		if (!tp->must_be_used)
			continue;
		assert(tp->value);

		/*
		 * flag that the variable has been used
		 */
		tp->must_be_used = 0;
		if (!tp->value->wstr_length)
			continue;

		/*
		 * append to the buffer, separated by a space
		 */
		collect(&buf, (wchar_t)' ');
		collect_n(&buf, tp->value->wstr_text, tp->value->wstr_length);
	}

	/*
	 * find any unused variables
	 * and complain about them
	 */
	error_count = 0;
	for (tp = scp->sub_var_list; tp < the_end; ++tp)
	{
		sub_context_ty	*inner;
		string_ty	*tmp;

		if (!tp->must_be_used)
			continue;

		/*
		 * Make sure the variables of this message are optional,
		 * to avoid infinite loops if there is a mistake in the
		 * translation string.
		 */
		inner = sub_context_new();
		sub_var_set(inner, "File_Name", "%s", scp->file_name);
		sub_var_set(inner, "Line_Number", "%d", scp->line_number);
		tmp = wstr_to_str(s);
		sub_var_set(inner, "Message", "%S", tmp);
		sub_var_optional(inner, "Message");
		sub_var_set(inner, "Name", "$%s", tp->name);
		sub_var_optional(inner, "Name");
		error_intl
		(
			inner,
i18n("$filename: $linenumber: \
in substitution \"$message\" variable \"$name\" unused")
		);
		sub_context_delete(inner);
		str_free(tmp);
		++error_count;
	}
	if (error_count > 0)
	{
		sub_context_ty	*inner;
		string_ty	*tmp;

		/*
		 * Make sure the variables of this message are optional,
		 * to avoid infinite loops if there is a mistake in the
		 * translation string.
		 */
		inner = sub_context_new();
		sub_var_set(inner, "File_Name", "%s", scp->file_name);
		sub_var_set(inner, "Line_Number", "%d", scp->line_number);
		tmp = wstr_to_str(s);
		sub_var_set(inner, "Message", "%S", tmp);
		sub_var_optional(inner, "Message");
		sub_var_set(inner, "Number", "%d", error_count);
		sub_var_optional(inner, "Number");
		fatal_intl
		(
			inner,
i18n("$filename: $linenumber: \
in substitution \"$message\" found unused variables")
		);
		/* NOTREACHED */
		sub_context_delete(inner);
		str_free(tmp);
	}

	/*
	 * clear the slate, ready for the next run
	 */
	sub_var_clear(scp);
	result = collect_end(&buf);
	collect_destructor(&buf);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static wstring_ty *subst_intl_wide _((sub_context_ty *, char *));

static wstring_ty *
subst_intl_wide(scp, msg)
	sub_context_ty	*scp;
	char		*msg;
{
	char		*tmp;
	wstring_ty	*s;
	wstring_ty	*result;

	trace(("subst_intl_wide(msg = \"%s\")\n{\n"/*}*/, msg));
	language_human();
	tmp = gettext(msg);
	language_C();
#ifdef DEBUG
#ifdef HAVE_GETTEXT
	if (tmp == msg)
	{
		error_raw
		(
			"%s: %d: warning: message \"%s\" has no translation",
			scp->file_name,
			scp->line_number,
			msg
		);
	}
#endif /* HAVE_GETTEXT */
#endif /* DEBUG */
	s = wstr_from_c(tmp);
	result = subst(scp, s);
	wstr_free(s);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


string_ty *
subst_intl(scp, s)
	sub_context_ty	*scp;
	char		*s;
{
	wstring_ty	*result_wide;
	string_ty	*result;

	trace(("subst_intl(s = \"%s\")\n{\n"/*}*/, s));
	result_wide = subst_intl_wide(scp, s);
	result = wstr_to_str(result_wide);
	wstr_free(result_wide);
	trace(("return \"%s\";\n", result->str_text));
	trace((/*{*/"}\n"));
	return result;
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
substitute(scp, acp, s)
	sub_context_ty	*scp;
	change_ty	*acp;
	string_ty	*s;
{
	wstring_ty	*ws;
	wstring_ty	*result_wide;
	string_ty	*result;
	int		need_to_delete;

	trace(("substitute(acp = %08lX, s = \"%s\")\n{\n"/*}*/,
		(long)acp, s->str_text));
	if (!scp)
	{
		scp = sub_context_new();
		need_to_delete = 1;
	}
	else
		need_to_delete = 0;
	assert(acp);
	subst_intl_change(scp, acp);
	ws = str_to_wstr(s);
	result_wide = subst(scp, ws);
	wstr_free(ws);
	result = wstr_to_str(result_wide);
	wstr_free(result_wide);
	if (need_to_delete)
		sub_context_delete(scp);
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
sub_var_clear(scp)
	sub_context_ty	*scp;
{
	size_t	j;

	for (j = 0; j < scp->sub_var_pos; ++j)
		wstr_free(scp->sub_var_list[j].value);
	scp->sub_var_pos = 0;
	scp->cp = 0;
	scp->pp = 0;
	scp->errno_sequester = 0;
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
sub_var_set(scp, name, fmt sva_last)
	sub_context_ty	*scp;
	char		*name;
	char		*fmt;
	sva_last_decl
{
	va_list		ap;
	string_ty	*s;
	sub_table_ty	*svp;

	sva_init(ap, fmt);
	s = str_vformat(fmt, ap);
	va_end(ap);

	if (scp->sub_var_pos >= scp->sub_var_size)
	{
		size_t		nbytes;

		scp->sub_var_size += 10;
		nbytes = scp->sub_var_size * sizeof(sub_table_ty);
		scp->sub_var_list = mem_change_size(scp->sub_var_list, nbytes);
	}
	svp = &scp->sub_var_list[scp->sub_var_pos++];
	svp->name = name;
	svp->value = str_to_wstr(s);
	str_free(s);
	svp->must_be_used = !isdigit(name[0]);
	svp->append_if_unused = 0;
	svp->override = 0;
	svp->resubstitute = !svp->must_be_used;
}


void
sub_var_resubstitute(scp, name)
	sub_context_ty	*scp;
	char	*name;
{
	sub_table_ty	*the_end;
	sub_table_ty	*svp;

	the_end = scp->sub_var_list + scp->sub_var_pos;
	for (svp = scp->sub_var_list; svp < the_end; ++svp)
		if (!strcmp(svp->name, name))
			break;
	if (svp >= the_end)
		this_is_a_bug();
	else
		svp->resubstitute = 1;
}


void
sub_var_override(scp, name)
	sub_context_ty	*scp;
	char		*name;
{
	sub_table_ty	*the_end;
	sub_table_ty	*svp;

	the_end = scp->sub_var_list + scp->sub_var_pos;
	for (svp = scp->sub_var_list; svp < the_end; ++svp)
		if (!strcmp(svp->name, name))
			break;
	if (svp >= the_end)
		this_is_a_bug();
	else
		svp->override = 1;
}


void
sub_var_optional(scp, name)
	sub_context_ty	*scp;
	char		*name;
{
	sub_table_ty	*the_end;
	sub_table_ty	*svp;

	the_end = scp->sub_var_list + scp->sub_var_pos;
	for (svp = scp->sub_var_list; svp < the_end; ++svp)
		if (!strcmp(svp->name, name))
			break;
	if (svp >= the_end)
		this_is_a_bug();
	else
		svp->must_be_used = 0;
}


void
sub_var_append_if_unused(scp, name)
	sub_context_ty	*scp;
	char		*name;
{
	sub_table_ty	*the_end;
	sub_table_ty	*svp;

	the_end = scp->sub_var_list + scp->sub_var_pos;
	for (svp = scp->sub_var_list; svp < the_end; ++svp)
		if (!strcmp(svp->name, name))
			break;
	if (svp >= the_end)
		this_is_a_bug();
	else
	{
		svp->must_be_used = 1;
		svp->append_if_unused = 1;
	}
}


/*
 * NAME
 *	wrap - wrap s string over lines
 *
 * SYNOPSIS
 *	void wrap(wstring_ty *);
 *
 * DESCRIPTION
 *	The wrap function is used to print error messages onto stderr
 *	wrapping ling lines.  Be very careful of multi-byte characters
 *	in international character sets.
 *
 * CAVEATS
 *	Line length is assumed to be 80 characters.
 */

static void wrap _((const wchar_t *));

static void
wrap(s)
	const wchar_t	*s;
{
	char		*progname;
	int		page_width;
	char		tmp[(MAX_PAGE_WIDTH + 2) * MB_LEN_MAX];
	int		first_line;
	char		*tp;
	int		nbytes;
	static int	progname_width;
	int		midway;

	/*
	 * flush any pending output,
	 * so the error message appears in a sensible place.
	 */
	if (fflush(stdout) || ferror(stdout))
		nfatal("(stdout)");

	/*
	 * Ask the system how wide the terminal is.
	 * Don't use last column, many terminals are dumb.
	 */
	page_width = option_page_width_get() - 1;
	midway = (page_width + 8) / 2;

	/*
	 * Because it must be a legal UNIX file name, it is unlikely to
	 * be stupid - unprintable characters are hard to type, and most
	 * file systems don't allow high-bit-on characters in file
	 * names.  Thus, assume progname is all legal characters.
	 */
	progname = progname_get();
	if (!progname_width)
	{
		wstring_ty	*ws;

		ws = wstr_from_c(progname);
		progname_width = wcs_column_width(ws->wstr_text);
		wstr_free(ws);
	}

	/*
	 * the message is for a human, so
	 * use the human's locale
	 */
	language_human();

	/*
	 * Emit the message a line at a time, wrapping as we go.  The
	 * first line starts with the program name, subsequent lines are
	 * indented by a tab.
	 */
	first_line = 1;
	while (*s)
	{
		const wchar_t	*ep;
		int		ocol;
		const wchar_t	*break_space;
		int		break_space_col;
		const wchar_t	*break_punct;
		int		break_punct_col;

		/*
		 * Work out how many characters fit on the line.
		 */
		if (first_line)
			ocol = progname_width + 2;
		else
			ocol = 8;

		wctomb(NULL, 0);
		ep = s;
		break_space = 0;
		break_space_col = 0;
		break_punct = 0;
		break_punct_col = 0;
		while (*ep)
		{
			char		dummy[MB_LEN_MAX];
			int		cw;
			wchar_t		c;

			/*
			 * Keep printing characters.  Use a dummy
			 * character for unprintable sequences (which
			 * should not happen).
			 */
			c = *ep;
			if (!iswprint(c))
				c = '?';
			nbytes = wctomb(dummy, c);

			cw = column_width(c);
			if (nbytes <= 0)
			{
				/*
				 * This should not happen!  All
				 * unprintable characters should have
				 * been turned into C escapes inside the
				 * common/wstr.c file when converting from C
				 * string to wide strings.
				 *
				 * Replace invalid wide characters with
				 * a C escape.
				 */
				cw = 4;
				nbytes = 4;

				/*
				 * The wctomb state will be ``error'',
				 * so reset it and brave the worst.  No
				 * need to reset the wctomb state, it is
				 * not broken.
				 */
				wctomb(NULL, 0);
			}

			/*
			 * Keep track of good places to break the line,
			 * but try to avoid runs of white space.  There
			 * is a pathological case where the line is
			 * entirely composed of white space, but it does
			 * not happen often.
			 */
			if (c == ' ')
			{
				break_space = ep;
				break_space_col = ocol;
				while (break_space > s && break_space[-1] == ' ')
				{
					--break_space;
					--break_space_col;
				}
			}
			if (iswpunct(c) && ocol + cw <= page_width)
			{
				break_punct = ep + 1;
				break_punct_col = ocol + cw;
			}

			/*
			 * if we have run out of room, break here
			 */
			if (ocol + cw > page_width)
				break;
			ocol += cw;
			++ep;
		}

		/*
		 * see if there is a better place to break the line
		 *
		 * Break the line at space characters, otherwise break
		 * at punctuator characters.  If it is possible to break
		 * on either a space or a punctuator, choose the space.
		 *
		 * However, if the space is in the left half of the
		 * line, things look very unbalanced, so break on a
		 * punctuator in that case.
		 */
		if (*ep && *ep != ' ')
		{
			if (break_space == s)
				break_space = 0;
			if
			(
				break_space
			&&
				break_punct
			&&
				break_space_col < midway
			&&
				break_punct_col >= midway
			)
				ep = break_punct;
			else if (break_space)
				ep = break_space;
			else if (break_punct)
				ep = break_punct;
		}

		/*
		 * print the line
		 */
		if (first_line)
		{
			strcpy(tmp, progname);
			strcat(tmp, ": ");
		}
		else
			strcpy(tmp, "\t");
		tp = tmp + strlen(tmp);

		/*
		 * Turn the input into a multi bytes chacacters.
		 */
		wctomb(NULL, 0);
		while (s < ep)
		{
			wchar_t		c;

			/*
			 * Keep printing characters.  Use a dummy
			 * character for unprintable sequences (which
			 * should not happen).
			 */
			c = *s++;
			if (!iswprint(c))
				c = '?';
			nbytes = wctomb(tp, c);

			if (nbytes <= 0)
			{
				/*
				 * This should not happen!  All
				 * unprintable characters should have
				 * been turned into C escapes inside the
				 * wstring.c file when converting from C
				 * string to wide strings.
				 *
				 * Replace invalid wide characters with
				 * a C escape.
				 */
				nbytes = 4;
				tp[0] = '\\';
				tp[1] = '0' + ((c >> 6) & 7);
				tp[2] = '0' + ((c >> 3) & 7);
				tp[3] = '0' + ( c       & 7);

				/*
				 * The wctomb state will be ``error'',
				 * so reset it and brave the worst.  No
				 * need to reset the wctomb state, it is
				 * not broken.
				 */
				wctomb(NULL, 0);
			}
			tp += nbytes;
		}

		/*
		 * Add a newline and end any outstanding shift state and
		 * add a NUL character.
		 */
		nbytes = wctomb(tp, (wchar_t)'\n');
		if (nbytes > 0)
			tp += nbytes;
		nbytes = wctomb(tp, (wchar_t)0);
		if (nbytes > 0)
			tp += nbytes;

		/*
		 * Emit the line to stderr.  It is important to do this
		 * a whole line at a time, otherwise performance is
		 * terrible - stderr by default is character buffered.
		 */
		fputs(tmp, stderr);
		if (ferror(stderr))
			break;

		/*
		 * skip leading spaces for subsequent lines
		 */
		while (*s == ' ')
			++s;
		first_line = 0;
	}

	/*
	 * done with humans
	 */
	language_C();

	/*
	 * make sure nothing went wrong
	 */
	if (fflush(stderr) || ferror(stderr))
		nfatal("(stderr)");
}


void
error_intl(scp, s)
	sub_context_ty	*scp;
	char		*s;
{
	wstring_ty	*message;
	int		need_to_delete;

	if (!scp)
	{
		scp = sub_context_new();
		need_to_delete = 0;
	}
	else
		need_to_delete = 0;

	message = subst_intl_wide(scp, s);
	wrap(message->wstr_text);
	wstr_free(message);

	if (need_to_delete)
		sub_context_delete(scp);
}


void
fatal_intl(scp, s)
	sub_context_ty	*scp;
	char		*s;
{
	wstring_ty	*message;
	static	char	*double_jeopardy;
	int		need_to_delete;

	if (!scp)
	{
		scp = sub_context_new();
		need_to_delete = 0;
	}
	else
		need_to_delete = 0;

	/*
	 * Make sure that there isn't an infinite loop,
	 * if there is a problem with a substitution
	 * in an error message.
	 */
	if (double_jeopardy)
	{
		/*
		 * this error message can't be internationalized
		 */
		fatal_raw
		(
"a fatal_intl error (\"%s\") happened while \
attempting to report an earlier fatal_intl error (\"%s\").  \
This is a probably bug.",
			s,
			double_jeopardy
		);
	}
	double_jeopardy = s;

	message = subst_intl_wide(scp, s);
	wrap(message->wstr_text);
	double_jeopardy = 0;
	quit(1);
	/* NOTREACHED */

	if (need_to_delete)
		sub_context_delete(scp);
}


void
verbose_intl(scp, s)
	sub_context_ty	*scp;
	char		*s;
{
	wstring_ty	*message;
	int		need_to_delete;

	if (!scp)
	{
		scp = sub_context_new();
		need_to_delete = 0;
	}
	else
		need_to_delete = 0;

	if (option_verbose_get())
	{
		message = subst_intl_wide(scp, s);
		wrap(message->wstr_text);
		wstr_free(message);
	}
	else
		sub_var_clear(scp);
	
	if (need_to_delete)
		sub_context_delete(scp);
}


void
sub_errno_setx(scp, x)
	sub_context_ty	*scp;
	int		x;
{
	scp->errno_sequester = x;
}


void
sub_errno_set(scp)
	sub_context_ty	*scp;
{
	sub_errno_setx(scp, errno);
}
