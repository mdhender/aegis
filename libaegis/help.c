/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997 Peter Miller;
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
 * MANIFEST: functions to provide consistent treatment of -Help options
 */

#include <ac/stdlib.h>
#include <ac/unistd.h>

#include <arglex2.h>
#include <env.h>
#include <error.h>
#include <gonzo.h>
#include <help.h>
#include <libdir.h>
#include <option.h>
#include <os.h>
#include <pager.h>
#include <progname.h>
#include <sub.h>
#include <trace.h>
#include <str_list.h>


static void help_env _((void));

static void
help_env()
{
	char		*cp;
	string_list_ty	manpath;
	string_ty	*s;
	string_list_ty	lib;
	string_list_ty	lang;
	size_t		j, k;
	string_list_ty	wl;

	/*
	 * Honour any existing MANPATH setting by appending only.
	 * Read the MANPATH to set the initial path.
	 */
	cp = getenv("MANPATH");
	if (cp)
	{
		s = str_from_c(cp);
		str2wl(&manpath, s, ":", 0);
		str_free(s);
	}
	else
	{
		string_list_constructor(&manpath);
		s = str_from_c("/usr/man");
		string_list_append(&manpath, s);
		str_free(s);
	}

	s = str_from_c(configured_mandir());
	string_list_append_unique(&manpath, s);
	str_free(s);

	/*
	 * Read the AEGIS_PATH environment variable for the list of
	 * places to look for Aegis libraries.
	 */
	gonzo_report_path(&lib);

	/*
	 * Use the AEGIS_LIB and LANGUAGE (or LANG) environment
	 * variables to know which languages to add.
	 * Default to "en" if not set.
	 */
	string_list_constructor(&lang);
	cp = getenv("LANGUAGE");
	if (cp)
	{
		s = str_from_c(cp);
		str2wl(&wl, s, ":", 0);
		for (j = 0; j < wl.nstrings; ++j)
			string_list_append_unique(&lang, wl.string[j]);
		string_list_destructor(&wl);
	}
	cp = getenv("LANG");
	if (cp)
	{
		s = str_from_c(cp);
		str2wl(&wl, s, ":", 0);
		for (j = 0; j < wl.nstrings; ++j)
			string_list_append_unique(&lang, wl.string[j]);
		string_list_destructor(&wl);
	}
	if (!lang.nstrings)
	{
		s = str_from_c("en");
		string_list_append(&lang, s);
		str_free(s);
	}

	/*
	 * convolve the lib and lang lists and append them to the manpath
	 */
	for (j = 0; j < lib.nstrings; ++j)
	{
		string_list_append_unique(&manpath, lib.string[j]);
		for (k = 0; k < lang.nstrings; ++k)
		{
			s = str_format("%S/%S", lib.string[j], lang.string[k]);
			string_list_append_unique(&manpath, s);
			str_free(s);
		}
	}
	string_list_destructor(&lib);
	string_list_destructor(&lang);

	/*
	 * set the MANPATH environment variable
	 */
	s = wl2str(&manpath, 0, manpath.nstrings, ":");
	string_list_destructor(&manpath);
	env_set("MANPATH", s->str_text);
	str_free(s);
}


void
help(progname, usage)
	char		*progname;
	void		(*usage)_((void));
{
	int		flags;
	string_ty	*s;

	/*
	 * collect the rest of the command line,
	 * if necessary
	 */
	if (usage)
	{
		arglex();
		while (arglex_token != arglex_token_eoln)
			generic_argument(usage);
	}
	if (!progname)
		progname = progname_get();

	/*
	 * set the MANPATH environment variable
	 * to point into the libraries
	 */
	help_env();

	/*
	 * become the user who invoked this program
	 */
	os_become_orig();

	/*
	 * Invoke the appropriate ``man'' command.  This will find the
	 * right language on the search path, and it already knows how
	 * to translate the *roff into text.
	 */
	s = str_format("man %s", progname);
	flags = OS_EXEC_FLAG_INPUT | OS_EXEC_FLAG_ERROK;
	os_execute(s, flags, (string_ty *)0);
	str_free(s);
}


void
generic_argument(usage)
	void	(*usage)_((void));
{
	trace(("generic_argument()\n{\n"/*}*/));
	switch (arglex_token)
	{
	default:
		bad_argument(usage);
		/* NOTREACHED */

	case arglex_token_library:
		if (arglex() != arglex_token_string)
			option_needs_dir(arglex_token_library, usage);
		gonzo_library_append(arglex_value.alv_string);
		arglex();
		break;

	case arglex_token_pager:
		option_pager_set(1, usage);
		arglex();
		break;

	case arglex_token_no_pager:
		option_pager_set(0, usage);
		arglex();
		break;

	case arglex_token_page_length:
		if (arglex() != arglex_token_number)
			option_needs_number(arglex_token_page_length, usage);
		option_page_length_set(arglex_value.alv_number, usage);
		arglex();
		break;

	case arglex_token_page_width:
		if (arglex() != arglex_token_number)
			option_needs_number(arglex_token_page_width, usage);
		option_page_width_set(arglex_value.alv_number, usage);
		arglex();
		break;

	case arglex_token_tab_width:
		if (arglex() != arglex_token_number)
			option_needs_number(arglex_token_tab_width, usage);
		option_tab_width_set(arglex_value.alv_number, usage);
		arglex();
		break;

	case arglex_token_terse:
		option_terse_set(usage);
		arglex();
		break;

	case arglex_token_trace:
		if (arglex() != arglex_token_string)
			option_needs_files(arglex_token_trace, usage);
		for (;;)
		{
#ifdef DEBUG
			trace_enable(arglex_value.alv_string);
#endif
			if (arglex() != arglex_token_string)
				break;
		}
#ifndef DEBUG
		error_intl(0, i18n("-TRace needs DEBUG"));
#endif
		break;

	case arglex_token_unformatted:
		option_unformatted_set(usage);
		arglex();
		break;

	case arglex_token_verbose:
		option_verbose_set(usage);
		arglex();
		break;
	}
	trace((/*{*/"}\n"));
}


void
bad_argument(usage)
	void	(*usage)_((void));
{
	sub_context_ty	*scp;

	trace(("bad_argument()\n{\n"/*}*/));
	switch (arglex_token)
	{
	case arglex_token_string:
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%s", arglex_value.alv_string);
		error_intl(scp, i18n("misplaced file name (\"$filename\")"));
		sub_context_delete(scp);
		break;

	case arglex_token_number:
		scp = sub_context_new();
		sub_var_set(scp, "Number", "%s", arglex_value.alv_string);
		error_intl(scp, i18n("misplaced number ($number)"));
		sub_context_delete(scp);
		break;

	case arglex_token_option:
		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_value.alv_string);
		error_intl(scp, i18n("unknown \"$name\" option"));
		sub_context_delete(scp);
		break;

	case arglex_token_eoln:
		error_intl(0, i18n("command line too short"));
		break;

	default:
		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_value.alv_string);
		error_intl(scp, i18n("misplaced \"$name\" option"));
		sub_context_delete(scp);
		break;
	}
	usage();
	trace((/*{*/"}\n"));
	quit(1);
	/* NOTREACHED */
}


void
mutually_exclusive_options(a1, a2, usage)
	int		a1;
	int		a2;
	void		(*usage)_((void));
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set(scp, "Name1", "%s", arglex_token_name(a1));
	sub_var_set(scp, "Name2", "%s", arglex_token_name(a2));
	error_intl(scp, i18n("not $name1 and $name2 together"));
	sub_context_delete(scp);

	usage();
}


void
mutually_exclusive_options3(a1, a2, a3, usage)
	int		a1;
	int		a2;
	int		a3;
	void		(*usage)_((void));
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set(scp, "Name1", "%s", arglex_token_name(a1));
	sub_var_set(scp, "Name2", "%s", arglex_token_name(a2));
	sub_var_set(scp, "Name3", "%s", arglex_token_name(a3));
	error_intl(scp, i18n("not $name1 and $name2 and $name3 together"));
	sub_context_delete(scp);

	usage();
}


void
duplicate_option_by_name(name, usage)
	int		name;
	void		(*usage)_((void));
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set(scp, "Name", "%s", arglex_token_name(name));
	error_intl(scp, i18n("duplicate $name option"));
	sub_context_delete(scp);

	usage();
}


void
duplicate_option(usage)
	void		(*usage)_((void));
{
	duplicate_option_by_name(arglex_token, usage);
}


void
option_needs_number(name, usage)
	int		name;
	void		(*usage)_((void));
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set(scp, "Name", "%s", arglex_token_name(name));
	error_intl(scp, i18n("$name needs number"));
	sub_context_delete(scp);

	usage();
}


void
option_needs_string(name, usage)
	int		name;
	void		(*usage)_((void));
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set(scp, "Name", "%s", arglex_token_name(name));
	error_intl(scp, i18n("$name needs string"));
	sub_context_delete(scp);

	usage();
}


void
option_needs_name(name, usage)
	int		name;
	void		(*usage)_((void));
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set(scp, "Name", "%s", arglex_token_name(name));
	error_intl(scp, i18n("$name needs name"));
	sub_context_delete(scp);

	usage();
}


void
option_needs_file(name, usage)
	int		name;
	void		(*usage)_((void));
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set(scp, "Name", "%s", arglex_token_name(name));
	error_intl(scp, i18n("$name needs file"));
	sub_context_delete(scp);

	usage();
}


void
option_needs_dir(name, usage)
	int		name;
	void		(*usage)_((void));
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set(scp, "Name", "%s", arglex_token_name(name));
	error_intl(scp, i18n("$name needs dir"));
	sub_context_delete(scp);

	usage();
}


void
option_needs_files(name, usage)
	int		name;
	void		(*usage)_((void));
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set(scp, "Name", "%s", arglex_token_name(name));
	error_intl(scp, i18n("$name needs files"));
	sub_context_delete(scp);

	usage();
}
