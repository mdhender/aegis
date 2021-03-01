//
//	aegis - project change supervisor
//	Copyright (C) 1991-2005 Peter Miller;
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
// MANIFEST: functions to perform command substitutions
//

#pragma implementation "sub_context_ty"

#include <ac/errno.h>
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
#include <change/branch.h>
#include <change/file.h>
#include <collect.h>
#include <error.h>
#include <file.h>
#include <gonzo.h>
#include <language.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <page.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <project/history.h>
#include <quit.h>
#include <str_list.h>
#include <sub.h>
#include <sub/addpathsuffi.h>
#include <sub/architecture.h>
#include <sub/base_relativ.h>
#include <sub/basename.h>
#include <sub/binary_direc.h>
#include <sub/capitalize.h>
#include <sub/change/attribute.h>
#include <sub/change/delta.h>
#include <sub/change/develo_direc.h>
#include <sub/change/developer.h>
#include <sub/change/develop_list.h>
#include <sub/change/files.h>
#include <sub/change/integr_direc.h>
#include <sub/change/integrator.h>
#include <sub/change/number.h>
#include <sub/change/reviewer.h>
#include <sub/change/reviewr_list.h>
#include <sub/change/state.h>
#include <sub/change/version.h>
#include <sub/comment.h>
#include <sub/common_direc.h>
#include <sub/copyri_years.h>
#include <sub/data_directo.h>
#include <sub/date.h>
#include <sub/diff.h>
#include <sub/dirname.h>
#include <sub/dirname_rel.h>
#include <sub/dollar.h>
#include <sub/downcase.h>
#include <sub/email_addres.h>
#include <sub/expr.h>
#include <sub/getenv.h>
#include <sub/histo_direc.h>
#include <sub/identifier.h>
#include <sub/left.h>
#include <sub/length.h>
#include <sub/librar_direc.h>
#include <sub/namemax.h>
#include <sub/perl.h>
#include <sub/plural.h>
#include <sub/plural_forms.h>
#include <sub/project.h>
#include <sub/project/adminis_list.h>
#include <sub/project/baseline.h>
#include <sub/project/develop_list.h>
#include <sub/project/integra_list.h>
#include <sub/project/reviewe_list.h>
#include <sub/project/specific.h>
#include <sub/quote.h>
#include <sub/read_file.h>
#include <sub/right.h>
#include <sub/search_path.h>
#include <sub/shell.h>
#include <sub/source.h>
#include <sub/split.h>
#include <sub/substitute.h>
#include <sub/substr.h>
#include <sub/switch.h>
#include <sub/trim_directo.h>
#include <sub/trim_extensi.h>
#include <sub/unsplit.h>
#include <sub/upcase.h>
#include <sub/user.h>
#include <sub/zero_pad.h>
#include <trace.h>
#include <wstr/list.h>


enum getc_type
{
    getc_type_control,
    getc_type_data
};


typedef wstring_ty *(*fp)(sub_context_ty *, wstring_list_ty *);

struct sub_table_ty
{
    const char	    *name;
    fp		    func;
    bool	    resubstitute;
    bool	    must_be_used;
    bool	    append_if_unused;
    bool	    override;
    wstring_ty	    *value;
};


sub_context_ty::sub_context_ty(const char *arg1, int arg2) :
    var_list(0),
    var_size(0),
    var_pos(0),
    cp(0),
    pp(0),
    suberr(0),
    errno_sequester(errno),
    file_name(arg1 ? arg1 : __FILE__),
    line_number(arg1 ? (arg2 > 0 ? arg2 : 1) : __LINE__)
{
}


sub_context_ty::~sub_context_ty()
{
    clear();
    delete [] var_list;
}


void
sub_context_ty::error_set(const char *s)
{
    suberr = s;
}


project_ty *
sub_context_ty::project_get()
{
    return pp;
}


change_ty *
sub_context_ty::change_get()
{
    return cp;
}


//
// NAME
//	sub_errno - the errno substitution
//
// SYNOPSIS
//	wstring_ty *sub_errno(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_errno function implements the errno substitution.  The
//	errno substitution is replaced by the value if th errno variable
//	provided by the system, as mapped through the strerror function.
//
//	Requires exactly zero arguments.
//
//	The sub_errno_setx() function may be used to remember errno,
//	and thus isolate the error from subsequest system calls.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

static wstring_ty *
sub_errno(sub_context_ty *scp, wstring_list_ty *arg)
{
    trace(("sub_errno()\n{\n"));
    if (arg->size() != 1)
    {
	scp->error_set(i18n("requires zero arguments"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    int err = scp->errno_sequester_get();
    if (err == EPERM || err == EACCES)
    {
	int uid;
	int gid;
	os_become_query(&uid, &gid, (int *)0);
	struct passwd *pw = getpwuid(uid);
	char uidn[20];
	if (pw)
	    snprintf(uidn, sizeof(uidn), "user \"%.8s\"", pw->pw_name);
	else
	    snprintf(uidn, sizeof(uidn), "uid %d", uid);

	struct group *gr = getgrgid(gid);
	char gidn[20];
	if (gr)
	    snprintf(gidn, sizeof(gidn), "group \"%.8s\"", gr->gr_name);
	else
	    snprintf(gidn, sizeof(gidn), "gid %d", gid);

	string_ty *s =
	    str_format
	    (
		"%s [%s, %s]",
		strerror(err),
		uidn,
		gidn
	    );
	wstring_ty *result = str_to_wstr(s);
	str_free(s);
	trace(("return %8.8lX;\n", (long)result));
	trace(("}\n"));
	return result;
    }

    wstring_ty *result = wstr_from_c(strerror(err));
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static sub_table_ty table[] =
{
    {"$", sub_dollar, },
    {"#", sub_comment, },
    {"Add_Path_Suffix", sub_add_path_suffix, },
    {"Administrator_List", sub_administrator_list, },
    {"ARCHitecture", sub_architecture, },
    {"BaseLine", sub_baseline, },
    {"Basename", sub_basename, },
    {"BAse_RElative", sub_base_relative, },
    {"BINary_DIRectory", sub_binary_directory, },
    {"CAPitalize", sub_capitalize, },
    {"Change", sub_change_number, },
    {"Change_Attribute", sub_change_attribute, },
    {"Change_Developer_List", sub_change_developer_list, },
    {"Change_Files", sub_change_files, },
    {"Change_Reviewer_List", sub_change_reviewer_list, },
    {"Copyright_Years", sub_copyright_years, },
    {"COMment", sub_comment, },
    {"COMmon_DIRectory", sub_common_directory, },
    {"DATa_DIRectory", sub_data_directory, },
    {"DAte", sub_date, },
    {"DELta", sub_delta, },
    {"DEVeloper", sub_developer, },
    {"DEVeloper_List", sub_developer_list, },
    // Default_Development_Directory
    {"Development_Directory", sub_development_directory, },
    {"DIFference", sub_diff, },
    {"Dirname", sub_dirname, },
    {"Dirname_RELative", sub_dirname_relative, },
    {"DownCase", sub_downcase, },
    {"DOLlar", sub_dollar, },
    // Edit
    {"EMail_Address", sub_email_address, },
    {"ENVironment", sub_getenv, },
    {"ERrno", sub_errno, },
    {"EXpression", sub_expression, },
    // FieLD_List
    // File_List
    // File_Name
    {"Get_Environment", sub_getenv, },
    // Guess
    // History
    {"History_Directory", sub_history_directory, },
    // Input
    {"IDentifier", sub_identifier, },
    {"INTegration_Directory", sub_integration_directory, },
    {"INTegrator", sub_integrator, },
    {"INTegrator_List", sub_integrator_list, },
    {"LEFt", sub_left, },
    {"LENgth", sub_length, },
    {"LIBrary", sub_data_directory, },
    {"LIBrary_DIRectory", sub_library_directory, },
    // MAgic
    // MeSsaGe
    // Most_Recent
    {"Name_Maximum", sub_namemax, },
    // Number
    // Output
    // ORiginal
    {"PERL", sub_perl, },
    {"PLural", sub_plural, },
    {"PLural_Forms", sub_plural_forms, },
    {"Project", sub_project, },
    {"Project_Specific", sub_project_specific, },
    {"QUote", sub_quote, },
    {"Read_File", sub_read_file, true, },
    {"Read_File_Simple", sub_read_file, },
    {"Reviewer", sub_reviewer, },
    {"Reviewer_List", sub_reviewer_list, },
    {"RIght", sub_right, },
    {"Search_Path", sub_search_path, },
    {"Search_Path_Executable", sub_search_path, },
    {"SHell", sub_shell, },
    {"Source", sub_source, },
    {"SPLit", sub_split, },
    {"STate", sub_state, },
    {"SUBSTitute", sub_substitute, },
    {"SUBSTRing", sub_substr, },
    {"SWitch", sub_switch, },
    {"Trim_DIRectory", sub_trim_directory, },
    {"Trim_EXTension", sub_trim_extension, },
    {"UName", sub_architecture, }, // undocumented
    {"UNSplit", sub_unsplit, },
    {"UpCase", sub_upcase, },
    {"USer", sub_user, },
    {"Version", sub_version, },
    {"Zero_Pad", sub_zero_pad, },
};


void
sub_context_ty::execute(const wstring_list_ty &arg)
{
    trace(("execute()\n{\n"));
    if (arg.size() == 0)
    {
	sub_context_ty inner;
	inner.var_set_charstar("File_Name", file_name);
	inner.var_set_long("Line_Number", line_number);
	inner.fatal_intl
	(
	    i18n("$filename: $linenumber: empty $${} substitution")
	);
	// NOTREACHED
    }

    //
    // scan the variables
    //
    wstring_ty *s = 0;
    sub_table_ty *tp = 0;
    size_t nhits = 0;
    sub_table_ty *hit[20];
    string_ty *cmd = wstr_to_str(arg[0]);
    for (size_t j = 0; j < var_pos; ++j)
    {
	tp = &var_list[j];
	if (arglex_compare(tp->name, cmd->str_text, 0))
	{
	    if (tp->override)
	    {
		str_free(cmd);
		goto override;
	    }
	    if (nhits < SIZEOF(hit))
		hit[nhits++] = tp;
	}
    }

    //
    // scan the functions
    //
    for (tp = table; tp < ENDOF(table); ++tp)
    {
	if (arglex_compare(tp->name, cmd->str_text, 0))
	{
	    if (tp->override)
	    {
		str_free(cmd);
		goto override;
	    }
	    if ((size_t)nhits < SIZEOF(hit))
		hit[nhits++] = tp;
	}
    }
    str_free(cmd);

    //
    // figure what to do
    //
    switch (nhits)
    {
    case 0:
	suberr = i18n("unknown substitution name");
	break;

    case 1:
	tp = hit[0];
	override:
	if (tp->value)
	{
	    //
	    // flag that the variable has been used
	    //
	    tp->must_be_used = false;
	    s = wstr_copy(tp->value);
	}
	else
	{
	    wstring_ty *name = wstr_from_c(tp->name);
	    wstring_list_ty tmp;
	    tmp.push_back(name);
	    wstr_free(name);
	    for (size_t k = 1; k < arg.size(); ++k)
		tmp.push_back(arg[k]);
	    s = tp->func(this, &tmp);
	}
	break;

    default:
	suberr = i18n("ambiguous substitution name");
	break;
    }

    //
    // deal with the result
    //
    if (s)
    {
	diversion_stack.push_back(s, tp->resubstitute);
	wstr_free(s);
    }
    else
    {
	assert(suberr);
	wstring_ty *s2 = arg.unsplit();
	string_ty *s3 = wstr_to_str(s2);
	wstr_free(s2);
	const char *the_error = suberr ? suberr : "this is a bug";
	sub_context_ty inner(__FILE__, __LINE__);
	inner.var_set_charstar("File_Name", file_name);
	inner.var_set_long("Line_Number", line_number);
	inner.var_set_string("Name", s3);
	inner.var_set_charstar("MeSsaGe", gettext(the_error));
	inner.fatal_intl
	(
	 i18n("$filename: $linenumber: substitution $${$name} failed: $message")
	);
	// NOTREACHED
	str_free(s3);
    }
    trace(("}\n"));
}


wchar_t
sub_context_ty::getc_meta(getc_type &tr)
{
    trace(("sub_getc_meta()\n{\n"));
    if (diversion_stack.resub_both())
	tr = getc_type_control;
    else
	tr = getc_type_data;
    wchar_t result = diversion_stack.getch();
#ifdef DEBUG
    if (iswprint(result) && result >= CHAR_MIN && result <= CHAR_MAX)
    {
        trace(("return '%c' %s;\n", (char)result,
	    (tr == getc_type_control ? "control" : "data")));
    }
    else
    {
        trace(("return %4.4lX %s;\n", (long)result,
	    (tr == getc_type_control ? "control" : "data")));
    }
#endif
    trace(("}\n"));
    return result;
}


void
sub_context_ty::getc_meta_undo(wchar_t c)
{
    trace(("sub_getc_meta_undo(%ld)\n{\n", (long)c));
#ifdef DEBUG
    if (iswprint(c) && c >= CHAR_MIN && c <= CHAR_MAX)
        trace(("c = '%c'\n", (char)c));
#endif
    diversion_stack.ungetch(c);
    trace(("}\n"));
}


wchar_t
sub_context_ty::dollar()
{
    trace(("dollar()\n{\n"));
    collect tmp;
    wstring_list_ty arg;
    int result = 0;
    getc_type ct;
    wchar_t c = getc_meta(ct);
    if (ct != getc_type_control)
	goto normal;
    switch (c)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	{
	    for (;;)
	    {
		tmp.append(c);
		c = getc_meta(ct);
		switch (c)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		    continue;

		default:
		    getc_meta_undo(c);
		    break;
		}
		break;
	    }
	    wstring_ty *s = tmp.end();
	    trace(("push arg\n"));
	    arg.push_back(s);
	    wstr_free(s);
	    execute(arg);
	}
	break;

    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
	{
	    for (;;)
	    {
		tmp.append(c);
		c = getc_meta(ct);
		switch (c)
		{
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '_':
		case '-':
		    continue;

		default:
		    getc_meta_undo(c);
		    break;
		}
		break;
	    }
	    wstring_ty *s = tmp.end();
	    trace(("push arg\n"));
	    arg.push_back(s);
	    wstr_free(s);
	    execute(arg);
	}
	break;

    case '{':
	c = getch(ct);
	for (;;)
	{
	    //
	    // look for terminator
	    //
	    if (c == '}' && ct == getc_type_control)
		break;

	    //
	    // watch out for unterminated substitutions
	    //
	    if (!c)
	    {
		sub_context_ty inner(__FILE__, __LINE__);
		assert(file_name);
		inner.var_set_charstar("File_Name", file_name);
		assert(line_number);
		inner.var_set_long("Line_Number", line_number);
		inner.fatal_intl
		(
		  i18n("$filename: $linenumber: unterminated $${} substitution")
		);
		// NOTREACHED
		break;
	    }

	    //
	    // skip white space separating the arguments
	    //
	    if (iswspace(c))
	    {
		c = getch(ct);
		continue;
	    }

	    //
	    // collect the argument
	    //	    any run of non-white-space characters
	    //
	    wchar_t quoted = 0;
	    for (;;)
	    {
		if (!c)
		{
		    if (quoted)
		    {
			sub_context_ty inner(__FILE__, __LINE__);
			assert(file_name);
			inner.var_set_charstar("File_Name", file_name);
			assert(line_number);
			inner.var_set_long("Line_Number", line_number);
			inner.fatal_intl
			(
			i18n("$filename: $linenumber: unterminated $${} quotes")
			);
			// NOTREACHED
		    }
		    break;
		}
		if
		(
		    !quoted
		&&
		    (
			iswspace(c)
		    ||
			(ct == getc_type_control && c == '}')
		    )
		)
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
		    c = getch(ct);
		    if (!c)
		    {
			sub_context_ty inner(__FILE__, __LINE__);
			assert(file_name);
			inner.var_set_charstar("File_Name", file_name);
			assert(line_number);
			inner.var_set_long("Line_Number", line_number);
			inner.fatal_intl
			(
		   i18n("$filename: $linenumber: unterminated $${} \\ sequence")
			);
			// NOTREACHED
		    }
		    tmp.append(c);
		}
		else
		    tmp.append(c);
		c = getch(ct);
	    }
	    wstring_ty *s = tmp.end();
	    trace(("push arg\n"));
	    arg.push_back(s);
	    wstr_free(s);
	}
	execute(arg);
	break;

    case '$':
	result = '$';
	break;

    case '#':
	for (;;)
	{
	    c = getc_meta(ct);
	    if (!c || (c == '\n' && ct == getc_type_control))
		break;
	}
	result = 0;
	break;

    default:
	normal:
	getc_meta_undo(c);
	result = '$';
	break;
    }
#ifdef DEBUG
    if (iswprint(result) && result >= CHAR_MIN && result <= CHAR_MAX)
        trace(("return '%c';\n", (char)result));
    else
        trace(("return %4.4lX;\n", (long)result));
#endif
    trace(("}\n"));
    return result;
}


wchar_t
sub_context_ty::getch(getc_type &tr)
{
    trace(("sub_getc()\n{\n"));
    wchar_t c = 0;
    for (;;)
    {
	c = getc_meta(tr);
	if (c && tr != getc_type_control)
	    break;
	switch (c)
	{
	default:
	    break;

	case 0:
	    if (diversion_stack.empty())
		break;
	    diversion_stack.pop_back();
	    continue;

	case '$':
	    if (!diversion_stack.resub_both())
		break;
	    c = dollar();
	    if (!c)
		continue;
	    tr = getc_type_data;
	    break;
	}
	break;
    }
#ifdef DEBUG
    if (iswprint(c) && c >= CHAR_MIN && c <= CHAR_MAX)
    {
        trace(("return '%c' %s;\n", (char)c,
	    (tr == getc_type_control ? "control" : "data")));
    }
    else
    {
        trace(("return 0x%lX; /* %s */\n", (long)c,
	    (tr == getc_type_control ? "control" : "data")));
    }
#endif
    trace(("}\n"));
    return c;
}


void
sub_context_ty::subst_intl_project(project_ty *a)
{
    if (a != pp)
    {
	assert(!pp);
	assert(!cp);
	pp = a;
	cp = 0;
    }
}


void
sub_context_ty::subst_intl_change(change_ty *a)
{
    assert(!pp);
    assert(!cp);
    pp = a->pp;
    cp = a;
}


wstring_ty *
sub_context_ty::subst(wstring_ty *s)
{
    wchar_t	    c;
    wstring_ty	    *result;
    sub_table_ty    *tp;
    sub_table_ty    *the_end;
    int		    error_count;
    getc_type	    ct;

    trace(("subst(s = %8.8lX)\n{\n", (long)s));
    collect buf;
    diversion_stack.push_back(s, true);
    for (;;)
    {
	//
	// get the next character
	//
	c = getch(ct);
	if (!c)
	    break;

	//
	// save the character
	//
	buf.append(c);
    }

    //
    // find any unused variables marked "append if unused"
    //
    the_end = var_list + var_pos;
    for (tp = var_list; tp < the_end; ++tp)
    {
	if (!tp->append_if_unused)
	    continue;
	if (!tp->must_be_used)
	    continue;
	assert(tp->value);

	//
	// flag that the variable has been used
	//
	tp->must_be_used = false;
	if (!tp->value->wstr_length)
	    continue;

	//
	// append to the buffer, separated by a space
	//
	buf.append(L' ');
	buf.append(tp->value->wstr_text, tp->value->wstr_length);
    }

    //
    // find any unused variables
    // and complain about them
    //
    error_count = 0;
    for (tp = var_list; tp < the_end; ++tp)
    {
	string_ty	*tmp;

	if (!tp->must_be_used)
	    continue;

	//
	// Make sure the variables of this message are optional,
	// to avoid infinite loops if there is a mistake in the
	// translation string.
	//
	sub_context_ty inner(__FILE__, __LINE__);
	assert(file_name);
	inner.var_set_charstar("File_Name", file_name);
	assert(line_number);
	inner.var_set_long("Line_Number", line_number);
	tmp = wstr_to_str(s);
	inner.var_set_string("Message", tmp);
	inner.var_optional("Message");
	inner.var_set_format("Name", "$%s", tp->name);
	inner.var_optional("Name");
	inner.error_intl
	(
i18n("$filename: $linenumber: \
in substitution \"$message\" variable \"$name\" unused")
	);
	str_free(tmp);
	++error_count;
    }
    if (error_count > 0)
    {
	string_ty	*tmp;

	//
	// Make sure the variables of this message are optional,
	// to avoid infinite loops if there is a mistake in the
	// translation string.
	//
	sub_context_ty inner(__FILE__, __LINE__);
	assert(file_name);
	inner.var_set_charstar("File_Name", file_name);
	assert(line_number);
	inner.var_set_long("Line_Number", line_number);
	tmp = wstr_to_str(s);
	inner.var_set_string("Message", tmp);
	inner.var_optional("Message");
	inner.var_set_long("Number", error_count);
	inner.var_optional("Number");
	inner.fatal_intl
	(
i18n("$filename: $linenumber: \
in substitution \"$message\" found unused variables")
	);
	// NOTREACHED
	str_free(tmp);
    }

    //
    // clear the slate, ready for the next run
    //
    clear();
    result = buf.end();
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


wstring_ty *
sub_context_ty::subst_intl_wide(const char *msg)
{
    trace(("subst_intl_wide(msg = \"%s\")\n{\n", msg));
    language_human();
    const char *tmp = gettext(msg);
    language_C();
#if 0
#ifdef HAVE_GETTEXT
    if (tmp == msg)
    {
	error_raw
	(
	    "%s: %d: warning: message \"%s\" has no translation",
	    file_name,
	    line_number,
	    msg
	);
    }
#endif // HAVE_GETTEXT
#endif // DEBUG
    wstring_ty *s = wstr_from_c(tmp);
    wstring_ty *result = subst(s);
    wstr_free(s);
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


string_ty *
sub_context_ty::subst_intl(const char *s)
{
    trace(("subst_intl(s = \"%s\")\n{\n", s));
    wstring_ty *result_wide = subst_intl_wide(s);
    string_ty *result = wstr_to_str(result_wide);
    wstr_free(result_wide);
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}


string_ty *
sub_context_ty::substitute(change_ty *acp, string_ty *s)
{
    trace(("substitute(acp = %08lX, s = \"%s\")\n{\n", (long)acp, s->str_text));
    assert(acp);
    subst_intl_change(acp);
    wstring_ty *ws = str_to_wstr(s);
    wstring_ty *result_wide = subst(ws);
    wstr_free(ws);
    string_ty *result = wstr_to_str(result_wide);
    wstr_free(result_wide);
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}


string_ty *
sub_context_ty::substitute_p(project_ty *app, string_ty *s)
{
    trace(("substitute(pp = %08lX, s = \"%s\")\n{\n", (long)app, s->str_text));
    assert(app);
    subst_intl_project(app);
    wstring_ty *ws = str_to_wstr(s);
    wstring_ty *result_wide = subst(ws);
    wstr_free(ws);
    string_ty *result = wstr_to_str(result_wide);
    wstr_free(result_wide);
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}


void
sub_context_ty::clear()
{
    for (size_t j = 0; j < var_pos; ++j)
	wstr_free(var_list[j].value);
    var_pos = 0;
    cp = 0;
    pp = 0;
    errno_sequester = 0;
}


void
sub_context_ty::var_set_vformat(const char *name, const char *fmt, va_list ap)
{
    string_ty *s = str_vformat(fmt, ap);
    var_set_string(name, s);
    str_free(s);
}


void
sub_context_ty::var_set_format(const char *name, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    var_set_vformat(name, fmt, ap);
    va_end(ap);
}


void
sub_context_ty::var_set_string(const char *name, const nstring &value)
{
    if (var_pos >= var_size)
    {
	size_t new_var_size = var_size * 2 + 8;
	sub_table_ty *new_var_list = new sub_table_ty[new_var_size];
	for (size_t j = 0; j < var_pos; ++j)
	    new_var_list[j] = var_list[j];
	delete [] var_list;
	var_list = new_var_list;
	var_size = new_var_size;
    }
    sub_table_ty *svp = &var_list[var_pos++];
    svp->name = name;
    svp->value = wstr_n_from_c(value.c_str(), value.size());
    svp->must_be_used = true;
    svp->append_if_unused = false;
    svp->override = false;
    svp->resubstitute = !svp->must_be_used;
}


void
sub_context_ty::var_set_string(const char *name, string_ty *value)
{
    var_set_string(name, nstring(str_copy(value)));
}


void
sub_context_ty::var_resubstitute(const char *name)
{
    sub_table_ty *the_end = var_list + var_pos;
    for (sub_table_ty *svp = var_list; ; ++svp)
    {
	if (svp >= the_end)
	{
	    this_is_a_bug();
	    return;
	}
	if (!strcmp(svp->name, name))
	{
	    svp->resubstitute = true;
	    return;
	}
    }
}


void
sub_context_ty::var_override(const char *name)
{
    sub_table_ty *the_end = var_list + var_pos;
    for (sub_table_ty *svp = var_list; ; ++svp)
    {
	if (svp >= the_end)
	{
	    this_is_a_bug();
	    return;
	}
	if (!strcmp(svp->name, name))
	{
	    svp->override = true;
	    return;
	}
    }
}


void
sub_context_ty::var_optional(const char *name)
{
    sub_table_ty *the_end = var_list + var_pos;
    for (sub_table_ty *svp = var_list; ; ++svp)
    {
	if (svp >= the_end)
	{
	    this_is_a_bug();
	    return;
	}
	if (!strcmp(svp->name, name))
	{
	    svp->must_be_used = false;
	    return;
	}
    }
}


void
sub_context_ty::var_append_if_unused(const char *name)
{
    sub_table_ty *the_end = var_list + var_pos;
    for (sub_table_ty *svp = var_list; ; ++svp)
    {
	if (svp >= the_end)
	{
	    this_is_a_bug();
	    return;
	}
	if (!strcmp(svp->name, name))
	{
	    svp->must_be_used = true;
	    svp->append_if_unused = true;
	    return;
	}
    }
}


//
// NAME
//	wrap - wrap s string over lines
//
// SYNOPSIS
//	void wrap(wstring_ty *);
//
// DESCRIPTION
//	The wrap function is used to print error messages onto stderr
//	wrapping ling lines.  Be very careful of multi-byte characters
//	in international character sets.
//
// CAVEATS
//	Line length is assumed to be 80 characters.
//

static void
wrap(const wchar_t *s)
{
    const char      *progname;
    int		    page_width;
    char	    tmp[(MAX_PAGE_WIDTH + 2) * MB_LEN_MAX];
    int		    first_line;
    char	    *tp;
    int		    nbytes;
    static int	    progname_width;
    int		    midway;

    //
    // flush any pending output,
    // so the error message appears in a sensible place.
    //
    if (fflush(stdout) || ferror(stdout))
	nfatal("(stdout)");

    //
    // Ask the system how wide the terminal is.
    // Don't use last column, many terminals are dumb.
    //
    page_width = page_width_get(-1) - 1;
    midway = (page_width + 8) / 2;

    //
    // Because it must be a legal UNIX file name, it is unlikely to
    // be stupid - unprintable characters are hard to type, and most
    // file systems don't allow high-bit-on characters in file
    // names.  Thus, assume progname is all legal characters.
    //
    progname = progname_get();
    if (!progname_width)
    {
	wstring_ty	*ws;

	ws = wstr_from_c(progname);
	progname_width = wstr_column_width(ws);
	wstr_free(ws);
    }

    //
    // the message is for a human, so
    // use the human's locale
    //
    language_human();

    //
    // Emit the message a line at a time, wrapping as we go.  The
    // first line starts with the program name, subsequent lines are
    // indented by a tab.
    //
    first_line = 1;
    while (*s)
    {
	const wchar_t	*ep;
	int		ocol;
	const wchar_t	*break_space;
	int		break_space_col;
	const wchar_t	*break_punct;
	int		break_punct_col;

	//
	// Work out how many characters fit on the line.
	//
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
	    char	    dummy[MB_LEN_MAX];
	    int		    cw;
	    wchar_t	    c;

	    //
	    // Keep printing characters.  Use a dummy
	    // character for unprintable sequences (which
	    // should not happen).
	    //
	    c = *ep;
	    if (!iswprint(c))
		c = '?';
	    nbytes = wctomb(dummy, c);

	    cw = wcwidth(c);
	    if (nbytes <= 0)
	    {
		//
		// This should not happen!  All
		// unprintable characters should have
		// been turned into C escapes inside the
		// common/wstr.c file when converting from C
		// string to wide strings.
		//
		// Replace invalid wide characters with
		// a C escape.
		//
		cw = 4;
		nbytes = 4;

		//
		// The wctomb state will be ``error'',
		// so reset it and brave the worst.  No
		// need to reset the wctomb state, it is
		// not broken.
		//
		wctomb(NULL, 0);
	    }

	    //
	    // Keep track of good places to break the line,
	    // but try to avoid runs of white space.  There
	    // is a pathological case where the line is
	    // entirely composed of white space, but it does
	    // not happen often.
	    //
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

	    //
	    // if we have run out of room, break here
	    //
	    if (ocol + cw > page_width)
		break;
	    ocol += cw;
	    ++ep;
	}

	//
	// see if there is a better place to break the line
	//
	// Break the line at space characters, otherwise break
	// at punctuator characters.  If it is possible to break
	// on either a space or a punctuator, choose the space.
	//
	// However, if the space is in the left half of the
	// line, things look very unbalanced, so break on a
	// punctuator in that case.
	//
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

	//
	// print the line
	//
	if (first_line)
	{
	    strlcpy(tmp, progname, sizeof(tmp));
	    strlcat(tmp, ": ", sizeof(tmp));
	}
	else
	    strlcpy(tmp, "\t", sizeof(tmp));
	tp = tmp + strlen(tmp);

	//
	// Turn the input into a multi bytes chacacters.
	//
	wctomb(NULL, 0);
	while (s < ep)
	{
	    wchar_t	    c;

	    //
	    // Keep printing characters.  Use a dummy
	    // character for unprintable sequences (which
	    // should not happen).
	    //
	    c = *s++;
	    if (!iswprint(c))
		c = '?';
	    nbytes = wctomb(tp, c);

	    if (nbytes <= 0)
	    {
		//
		// This should not happen!  All
		// unprintable characters should have
		// been turned into C escapes inside the
		// wstring.c file when converting from C
		// string to wide strings.
		//
		// Replace invalid wide characters with
		// a C escape.
		//
		nbytes = 4;
		tp[0] = '\\';
		tp[1] = '0' + ((c >> 6) & 7);
		tp[2] = '0' + ((c >> 3) & 7);
		tp[3] = '0' + (c & 7);

		//
		// The wctomb state will be ``error'',
		// so reset it and brave the worst.  No
		// need to reset the wctomb state, it is
		// not broken.
		//
		wctomb(NULL, 0);
	    }
	    tp += nbytes;
	}

	//
	// Add a newline and end any outstanding shift state and
	// add a NUL character.
	//
	nbytes = wctomb(tp, (wchar_t)'\n');
	if (nbytes > 0)
	    tp += nbytes;
	nbytes = wctomb(tp, (wchar_t)0);
	if (nbytes > 0)
	    tp += nbytes;

	//
	// Emit the line to stderr.  It is important to do this
	// a whole line at a time, otherwise performance is
	// terrible - stderr by default is character buffered.
	//
	fputs(tmp, stderr);
	if (ferror(stderr))
	    break;

	//
	// skip leading spaces for subsequent lines
	//
	while (*s == ' ')
	    ++s;
	first_line = 0;
    }

    //
    // done with humans
    //
    language_C();

    //
    // make sure nothing went wrong
    //
    if (fflush(stderr) || ferror(stderr))
	nfatal("(stderr)");
}


void
sub_context_ty::error_intl(const char *s)
{
    if (!os_testing_mode())
	language_check_translations();
    wstring_ty *message = subst_intl_wide(s);
    wrap(message->wstr_text);
    wstr_free(message);
}


void
sub_context_ty::fatal_intl(const char *s)
{
    if (!os_testing_mode())
	language_check_translations();

    //
    // Make sure that there isn't an infinite loop,
    // if there is a problem with a substitution
    // in an error message.
    //
    static const char *double_jeopardy;
    if (double_jeopardy)
    {
	//
	// this error message can't be internationalized
	//
	fatal_raw
	(
            "a fatal_intl error (\"%s\") happened while attempting "
		"to report an earlier fatal_intl error (\"%s\").  "
		"This is probably a bug.",
	    s,
	    double_jeopardy
	);
    }
    double_jeopardy = s;

    wstring_ty *message = subst_intl_wide(s);
    wrap(message->wstr_text);
    double_jeopardy = 0;
    quit(1);
    // NOTREACHED
}


void
sub_context_ty::verbose_intl(const char *s)
{
    if (option_verbose_get())
    {
	if (!os_testing_mode())
	    language_check_translations();
	wstring_ty *message = subst_intl_wide(s);
	wrap(message->wstr_text);
	wstr_free(message);
    }
    clear();
}


void
sub_context_ty::errno_setx(int x)
{
    errno_sequester = x;
}


int
sub_context_ty::errno_sequester_get()
    const
{
    return errno_sequester;
}
