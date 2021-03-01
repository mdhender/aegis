//
//	aegis - project change supervisor
//	Copyright (C) 1991-2004 Peter Miller;
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
#include <str_list.h>
#include <sub.h>
#include <sub/addpathsuffi.h>
#include <sub/architecture.h>
#include <sub/base_relativ.h>
#include <sub/basename.h>
#include <sub/binary_direc.h>
#include <sub/capitalize.h>
#include <sub/change/delta.h>
#include <sub/change/develo_direc.h>
#include <sub/change/developer.h>
#include <sub/change/files.h>
#include <sub/change/integr_direc.h>
#include <sub/change/integrator.h>
#include <sub/change/number.h>
#include <sub/change/reviewer.h>
#include <sub/change/state.h>
#include <sub/change/version.h>
#include <sub/comment.h>
#include <sub/common_direc.h>
#include <sub/copyri_years.h>
#include <sub/data_directo.h>
#include <sub/date.h>
#include <sub/dirname.h>
#include <sub/dirname_rel.h>
#include <sub/dollar.h>
#include <sub/downcase.h>
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
#include <wstr_list.h>


#define RESUB_DOLLARS 2
#define RESUB_PERCENT 1
#define RESUB_BOTH (RESUB_DOLLARS | RESUB_PERCENT)

enum getc_type
{
    getc_type_control,
    getc_type_data
};


struct sub_context_ty
{
    struct sub_diversion_ty *diversion;
    struct sub_table_ty *sub_var_list;
    size_t	    sub_var_size;
    size_t	    sub_var_pos;
    struct change_ty *cp;
    struct project_ty *pp;
    const char	    *suberr;
    int		    errno_sequester;
    const char	    *file_name;
    int		    line_number;
};


typedef wstring_ty *(*fp)(sub_context_ty *, wstring_list_ty *);

struct sub_table_ty
{
    const char	    *name;
    fp		    func;
    int		    resubstitute;
    wstring_ty	    *value;
    int		    must_be_used:   1;
    int		    append_if_unused: 1;
    int		    override:	    1;
};


struct sub_diversion_ty
{
    long	    pos;
    wstring_ty	    *text;
    sub_diversion_ty *prev;
    int		    resubstitute;
};


struct collect_ty
{
    size_t	    pos;
    size_t	    size;
    wchar_t	    *buf;
};


static void
collect_constructor(collect_ty *cp)
{
    cp->buf = 0;
    cp->size = 0;
    cp->pos = 0;
}


static void
collect_destructor(collect_ty *cp)
{
    if (cp->buf)
	mem_free(cp->buf);
    cp->buf = 0;
    cp->size = 0;
    cp->pos = 0;
}


sub_context_ty *
sub_context_New(const char *file_name, int line_number)
{
    sub_context_ty  *p;
    int		    hold;

    hold = errno; // must preserve!
    p = (sub_context_ty *)mem_alloc(sizeof(sub_context_ty));
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
    errno = hold; // must preserve!
    return p;
}


void
sub_context_delete(sub_context_ty *p)
{
    size_t	    j;

    for (j = 0; j < p->sub_var_pos; ++j)
	wstr_free(p->sub_var_list[j].value);
    if (p->sub_var_list)
	mem_free(p->sub_var_list);
    mem_free(p);
}


void
sub_context_error_set(sub_context_ty *scp, const char *s)
{
    scp->suberr = s;
}


project_ty *
sub_context_project_get(sub_context_ty *scp)
{
    return scp->pp;
}


change_ty *
sub_context_change_get(sub_context_ty *scp)
{
    return scp->cp;
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
    wstring_ty	    *result;

    trace(("sub_errno()\n{\n"));
    if (arg->nitems != 1)
    {
	scp->suberr = i18n("requires zero arguments");
	result = 0;
    }
    else
    {
	if (scp->errno_sequester == 0)
	    scp->errno_sequester = errno;
	if (scp->errno_sequester == EPERM || scp->errno_sequester == EACCES)
	{
	    int		    uid;
	    struct passwd   *pw;
	    char	    uidn[20];
	    int		    gid;
	    struct group    *gr;
	    char	    gidn[20];
	    string_ty	    *s;

	    os_become_query(&uid, &gid, (int *)0);
	    pw = getpwuid(uid);
	    if (pw)
		snprintf(uidn, sizeof(uidn), "user \"%.8s\"", pw->pw_name);
	    else
		snprintf(uidn, sizeof(uidn), "uid %d", uid);

	    gr = getgrgid(gid);
	    if (gr)
		snprintf(gidn, sizeof(gidn), "group \"%.8s\"", gr->gr_name);
	    else
		snprintf(gidn, sizeof(gidn), "gid %d", gid);

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
    {"Change_Files", sub_change_files, },
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
    {"Dirname", sub_dirname, },
    {"Dirname_RELative", sub_dirname_relative, },
    {"DownCase", sub_downcase, },
    {"DOLlar", sub_dollar, },
    // Edit
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
    {"Read_File", sub_read_file, RESUB_DOLLARS, },
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


//
// NAME
//	sub_diversion
//
// SYNOPSIS
//	void sub_diversion(sub_context_ty *, wstring_ty *s, int resub);
//
// DESCRIPTION
//	The sub_diversion function is used to divert input
//	to a string, until that string is exhausted.
//	When the string is exhausted, input will resume
//	from the previous string.
//
// ARGUMENTS
//	s - string to take as input
//

static void
sub_diversion(sub_context_ty *scp, wstring_ty *s, int resubstitute)
{
    sub_diversion_ty *dp;

    trace(("sub_diversion(s = %8.8lX, resub = %d)\n{\n", (long)s,
        resubstitute));
    dp = (sub_diversion_ty *)mem_alloc(sizeof(sub_diversion_ty));
    dp->text = wstr_copy(s);
    dp->pos = 0;
    dp->resubstitute = resubstitute;
    dp->prev = scp->diversion;
    scp->diversion = dp;
    trace(("}\n"));
}


//
// NAME
//	sub_diversion_close
//
// SYNOPSIS
//	void sub_diversion_close(sub_context_ty *);
//
// DESCRIPTION
//	The sub_diversion_close function is used to
//	release a diversion when it has been exhausted.
//

static void
sub_diversion_close(sub_context_ty *scp)
{
    sub_diversion_ty *dp;

    trace(("sub_diversion_close()\n{\n"));
    assert(scp->diversion);
    dp = scp->diversion;
    scp->diversion = dp->prev;
    wstr_free(dp->text);
    mem_free(dp);
    trace(("}\n"));
}


//
// NAME
//	execute
//
// SYNOPSIS
//	void execute(wstring_list_ty *args);
//
// DESCRIPTION
//	The execute function is used to perform the substitution
//	described by the argument list.
//
// ARGUMENTS
//	args - the name and arguments of the substitution
//

static void
execute(sub_context_ty *scp, wstring_list_ty *arg)
{
    string_ty	    *cmd;
    wstring_ty	    *s;
    sub_table_ty    *hit[20];
    int		    nhits;
    sub_table_ty    *tp;
    size_t	    j;

    trace(("execute()\n{\n"));
    if (arg->nitems == 0)
    {
	sub_context_ty	*inner;

	inner = sub_context_new();
	sub_var_set_charstar(inner, "File_Name", scp->file_name);
	sub_var_set_long(inner, "Line_Number", scp->line_number);
	fatal_intl
	(
	    inner,
	    i18n("$filename: $linenumber: empty $${} substitution")
	);
	// NOTREACHED
	sub_context_delete(inner);
    }

    //
    // scan the variables
    //
    nhits = 0;
    cmd = wstr_to_str(arg->item[0]);
    for (j = 0; j < scp->sub_var_pos; ++j)
    {
	tp = &scp->sub_var_list[j];
	if (arglex_compare(tp->name, cmd->str_text))
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

    //
    // scan the functions
    //
    for (tp = table; tp < ENDOF(table); ++tp)
    {
	if (arglex_compare(tp->name, cmd->str_text))
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
	scp->suberr = i18n("unknown substitution name");
	s = 0;
	break;

    case 1:
	tp = hit[0];
	override:
	if (tp->value)
	{
	    //
	    // flag that the variable has been used
	    //
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

    //
    // deal with the result
    //
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
	const char	*the_error;

	assert(scp->suberr);
	s2 = wstring_list_to_wstring(arg, 0, arg->nitems, (char *)0);
	s3 = wstr_to_str(s2);
	wstr_free(s2);
	the_error = scp->suberr ? scp->suberr : "this is a bug";
	inner = sub_context_new();
	sub_var_set_charstar(inner, "File_Name", scp->file_name);
	sub_var_set_long(inner, "Line_Number", scp->line_number);
	sub_var_set_string(inner, "Name", s3);
	sub_var_set_charstar(inner, "MeSsaGe", gettext(the_error));
	fatal_intl
	(
	    inner,
	 i18n("$filename: $linenumber: substitution $${$name} failed: $message")
	);
	// NOTREACHED
	sub_context_delete(inner);
	str_free(s3);
    }
    trace(("}\n"));
}


//
// NAME
//	sub_getc_meta
//
// SYNOPSIS
//	wchar_t sub_getc_meta(sub_context_ty *, getc_type *);
//
// DESCRIPTION
//	The sub_getc_meta function is used to get a character from
//	the current input string.  When the current string is exhaused,
//	the previous string is resumed.
//
// RETURNS
//	int - the chacater, or NUL to indicate end of input
//

static wchar_t
sub_getc_meta(sub_context_ty *scp, getc_type *tp)
{
    wchar_t	    result;
    sub_diversion_ty *dp;

    trace(("sub_getc_meta()\n{\n"));
    dp = scp->diversion;
    if (dp && (dp->resubstitute & RESUB_BOTH))
	*tp = getc_type_control;
    else
	*tp = getc_type_data;
    if (!dp)
	result = 0;
    else if (dp->pos >= (int)dp->text->wstr_length)
	result = 0;
    else
	result = dp->text->wstr_text[dp->pos++];
#ifdef DEBUG
    if (iswprint(result) && result >= CHAR_MIN && result <= CHAR_MAX)
    {
        trace(("return '%c' %s;\n", (char)result,
	    (*tp == getc_type_control ? "control" : "data")));
    }
    else
    {
        trace(("return %4.4lX %s;\n", (long)result,
	    (*tp == getc_type_control ? "control" : "data")));
    }
#endif
    trace(("}\n"));
    return result;
}


//
// NAME
//	sub_getc_meta_undo
//
// SYNOPSIS
//	void sub_getc_meta_undo(sub_context_ty *, wchar_t c);
//
// DESCRIPTION
//	The sub_getc_meta_undo function is used to give back
//	a character output by sub_getc_meta.
//
// ARGUMENTS
//	c - character being given back
//
// CAVEAT
//	Only push back what was read.
//

static void
sub_getc_meta_undo(sub_context_ty *scp, wchar_t c)
{
    sub_diversion_ty *dp;

    trace(("sub_getc_meta_undo(%ld)\n{\n", (long)c));
#ifdef DEBUG
    if (iswprint(c) && c >= CHAR_MIN && c <= CHAR_MAX)
        trace(("c = '%c'\n", (char)c));
#endif
    dp = scp->diversion;
    assert(dp);
    if (!c)
    {
	assert(dp->pos == (int)dp->text->wstr_length);
    }
    else
    {
	assert(dp->pos >= 1);
	dp->pos--;
	assert(c == dp->text->wstr_text[dp->pos]);
    }
    trace(("}\n"));
}


//
// NAME
//	collect
//
// SYNOPSIS
//	void collect(collect_ty *, int c);
//
// DESCRIPTION
//	The collect function is used to accumulate a string
//	one character at a time.  No size limit.
//
// ARGUMENTS
//	c - the character being collected
//

static void
collect(collect_ty *cp, wchar_t c)
{
    if (cp->pos >= cp->size)
    {
	size_t		nbytes;

	cp->size += (1L << 10);
	nbytes = cp->size * sizeof(wchar_t);
	cp->buf = (wchar_t *)mem_change_size(cp->buf, nbytes);
    }
    cp->buf[cp->pos++] = c;
}


static void
collect_n(collect_ty *cp, wchar_t *s, size_t n)
{
    while (n > 0)
    {
	collect(cp, *s++);
	--n;
    }
}


//
// NAME
//	collect_end
//
// SYNOPSIS
//	wstring_ty *collect_end(collect_ty *);
//
// DESCRIPTION
//	The collect_end function is used to fetch the string
//	accumulated with the collect function.
//	The bufferer for the collect function is cleared.
//
// RETURNS
//	wstring_ty *; pointer to the string in dynamic memory.
//

static wstring_ty *
collect_end(collect_ty *cp)
{
    wstring_ty	    *result;

    result = wstr_n_from_wc(cp->buf, cp->pos);
    cp->pos = 0;
    return result;
}


static wchar_t sub_getc(sub_context_ty *, getc_type *); // forward


//
// NAME
//	dollar
//
// SYNOPSIS
//	wchar_t dollar(sub_context_ty *);
//
// DESCRIPTION
//	The dollar function is used to perform dollar ($) substitutions.
//	On entry, the $ is expected to have been consumed.
//
//	The substitution is usually achieved as a side-effect,
//	by using the sub_diversion function.
//
// RETURNS
//	wchar_t a character to deliver as output,
//		or NUL if none.
//

static wchar_t
dollar(sub_context_ty *scp)
{
    wstring_list_ty arg;
    int		    result;
    wchar_t	    c;
    wstring_ty	    *s;
    wchar_t	    quoted;
    collect_ty	    tmp;
    getc_type	    ct;

    trace(("dollar()\n{\n"));
    collect_constructor(&tmp);
    wstring_list_constructor(&arg);
    result = 0;
    c = sub_getc_meta(scp, &ct);
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
	for (;;)
	{
	    collect(&tmp, c);
	    c = sub_getc_meta(scp, &ct);
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
	wstring_list_destructor(&arg);
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
	for (;;)
	{
	    collect(&tmp, c);
	    c = sub_getc_meta(scp, &ct);
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
	wstring_list_destructor(&arg);
	break;

    case '{':
	c = sub_getc(scp, &ct);
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
		sub_context_ty	*inner;

		inner = sub_context_new();
		sub_var_set_charstar(inner, "File_Name", scp->file_name);
		sub_var_set_long(inner, "Line_Number", scp->line_number);
		fatal_intl
		(
		    inner,
		  i18n("$filename: $linenumber: unterminated $${} substitution")
		);
		// NOTREACHED
		sub_context_delete(inner);
		break;
	    }

	    //
	    // skip white space separating the arguments
	    //
	    if (iswspace(c))
	    {
		c = sub_getc(scp, &ct);
		continue;
	    }

	    //
	    // collect the argument
	    //	    any run of non-white-space characters
	    //
	    quoted = 0;
	    for (;;)
	    {
		if (!c)
		{
		    if (quoted)
		    {
			sub_context_ty	*inner;

			inner = sub_context_new();
			sub_var_set_charstar
			(
			    inner,
			    "File_Name",
			    scp->file_name
			);
			sub_var_set_long
			(
			    inner,
			    "Line_Number",
			    scp->line_number
			);
			fatal_intl
			(
			    inner,
			i18n("$filename: $linenumber: unterminated $${} quotes")
			);
			// NOTREACHED
			sub_context_delete(inner);
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
		    c = sub_getc(scp, &ct);
		    if (!c)
		    {
			sub_context_ty	*inner;

			inner = sub_context_new();
			sub_var_set_charstar
			(
			    inner,
			    "File_Name",
			    scp->file_name
			);
			sub_var_set_long
			(
			    inner,
			    "Line_Number",
			    scp->line_number
			);
			fatal_intl
			(
			    inner,
		   i18n("$filename: $linenumber: unterminated $${} \\ sequence")
			);
			// NOTREACHED
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
	wstring_list_destructor(&arg);
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
    trace(("}\n"));
    return result;
}


//
// NAME
//	sub_getc
//
// SYNOPSIS
//	wchar_t sub_getc(sub_context_ty *);
//
// DESCRIPTION
//	The sub_getc function is used to get a character from
//	the substitution stream.  This is used both for the final output
//	and for fetching arguments to dollar ($) substitutions.
//
// RETURNS
//	wchar_t a character from the stream,
//		or NUL to indicate end of input.
//

static wchar_t
sub_getc(sub_context_ty *scp, getc_type	*tp)
{
    wchar_t	    c;

    trace(("sub_getc()\n{\n"));
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
    {
        trace(("return '%c' %s;\n", (char)c,
	    (*tp == getc_type_control ? "control" : "data")));
    }
    else
    {
        trace(("return 0x%lX; /* %s */\n", (long)c,
	    (*tp == getc_type_control ? "control" : "data")));
    }
#endif
    trace(("}\n"));
    return c;
}


void
subst_intl_project(sub_context_ty *scp, project_ty *a)
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
subst_intl_change(sub_context_ty *scp, change_ty *a)
{
    assert(!scp->pp);
    assert(!scp->cp);
    scp->pp = a->pp;
    scp->cp = a;
}


static wstring_ty *
subst(sub_context_ty *scp, wstring_ty *s)
{
    collect_ty	    buf;
    wchar_t	    c;
    wstring_ty	    *result;
    sub_table_ty    *tp;
    sub_table_ty    *the_end;
    int		    error_count;
    getc_type	    ct;

    trace(("subst(s = %8.8lX)\n{\n", (long)s));
    collect_constructor(&buf);
    sub_diversion(scp, s, RESUB_BOTH);
    for (;;)
    {
	//
	// get the next character
	//
	c = sub_getc(scp, &ct);
	if (!c)
	    break;

	//
	// save the character
	//
	collect(&buf, c);
    }

    //
    // find any unused variables marked "append if unused"
    //
    the_end = scp->sub_var_list + scp->sub_var_pos;
    for (tp = scp->sub_var_list; tp < the_end; ++tp)
    {
	if (!tp->append_if_unused)
	    continue;
	if (!tp->must_be_used)
	    continue;
	assert(tp->value);

	//
	// flag that the variable has been used
	//
	tp->must_be_used = 0;
	if (!tp->value->wstr_length)
	    continue;

	//
	// append to the buffer, separated by a space
	//
	collect(&buf, (wchar_t)' ');
	collect_n(&buf, tp->value->wstr_text, tp->value->wstr_length);
    }

    //
    // find any unused variables
    // and complain about them
    //
    error_count = 0;
    for (tp = scp->sub_var_list; tp < the_end; ++tp)
    {
	sub_context_ty	*inner;
	string_ty	*tmp;

	if (!tp->must_be_used)
	    continue;

	//
	// Make sure the variables of this message are optional,
	// to avoid infinite loops if there is a mistake in the
	// translation string.
	//
	inner = sub_context_new();
	sub_var_set_charstar(inner, "File_Name", scp->file_name);
	sub_var_set_long(inner, "Line_Number", scp->line_number);
	tmp = wstr_to_str(s);
	sub_var_set_string(inner, "Message", tmp);
	sub_var_optional(inner, "Message");
	sub_var_set_format(inner, "Name", "$%s", tp->name);
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

	//
	// Make sure the variables of this message are optional,
	// to avoid infinite loops if there is a mistake in the
	// translation string.
	//
	inner = sub_context_new();
	sub_var_set_charstar(inner, "File_Name", scp->file_name);
	sub_var_set_long(inner, "Line_Number", scp->line_number);
	tmp = wstr_to_str(s);
	sub_var_set_string(inner, "Message", tmp);
	sub_var_optional(inner, "Message");
	sub_var_set_long(inner, "Number", error_count);
	sub_var_optional(inner, "Number");
	fatal_intl
	(
	    inner,
i18n("$filename: $linenumber: \
in substitution \"$message\" found unused variables")
	);
	// NOTREACHED
	sub_context_delete(inner);
	str_free(tmp);
    }

    //
    // clear the slate, ready for the next run
    //
    sub_var_clear(scp);
    result = collect_end(&buf);
    collect_destructor(&buf);
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static wstring_ty *
subst_intl_wide(sub_context_ty *scp, const char *msg)
{
    char	    *tmp;
    wstring_ty	    *s;
    wstring_ty	    *result;

    trace(("subst_intl_wide(msg = \"%s\")\n{\n", msg));
    language_human();
    tmp = gettext(msg);
    language_C();
#if 0
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
#endif // HAVE_GETTEXT
#endif // DEBUG
    s = wstr_from_c(tmp);
    result = subst(scp, s);
    wstr_free(s);
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


string_ty *
subst_intl(sub_context_ty *scp, const char *s)
{
    wstring_ty	    *result_wide;
    string_ty	    *result;

    trace(("subst_intl(s = \"%s\")\n{\n", s));
    result_wide = subst_intl_wide(scp, s);
    result = wstr_to_str(result_wide);
    wstr_free(result_wide);
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}


//
// NAME
//	substitute
//
// SYNOPSIS
//	string_ty *substitute(change_ty *cp, string_ty *s);
//
// DESCRIPTION
//	The substitute function is used to perform substitutions on
//	strings.  Usually command strings, but not always.
//
//	The format of substitutions, and the commonly available
//	substitutions, are described in aesub(5).
//
// ARGUMENTS
//	cp	- the aegis change involved with the command
//		  This may never be NULL.
//	s	- the string to be substituted.
//
// RETURNS
//	string_ty *; pointer to string in dynamic memory
//

string_ty *
substitute(sub_context_ty *scp, change_ty *acp, string_ty *s)
{
    wstring_ty	    *ws;
    wstring_ty	    *result_wide;
    string_ty	    *result;
    int		    need_to_delete;

    trace(("substitute(acp = %08lX, s = \"%s\")\n{\n", (long)acp, s->str_text));
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
    trace(("}\n"));
    return result;
}


string_ty *
substitute_p(sub_context_ty *scp, project_ty *app, string_ty *s)
{
    wstring_ty	    *ws;
    wstring_ty	    *result_wide;
    string_ty	    *result;
    int		    need_to_delete;

    trace(("substitute(scp = %08lX, s = \"%s\")\n{\n", (long)scp, s->str_text));
    if (!scp)
    {
	scp = sub_context_new();
	need_to_delete = 1;
    }
    else
	need_to_delete = 0;
    assert(app);
    subst_intl_project(scp, app);
    ws = str_to_wstr(s);
    result_wide = subst(scp, ws);
    wstr_free(ws);
    result = wstr_to_str(result_wide);
    wstr_free(result_wide);
    if (need_to_delete)
	sub_context_delete(scp);
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}


//
// NAME
//	sub_var_clear
//
// SYNOPSIS
//	void sub_var_clear(void);
//
// DESCRIPTION
//	The sub_var_clear function is used to clear all of
//	the substitution variables.  Not usually needed manually,
//	as this is done automatically at the end of every substitute().
//

void
sub_var_clear(sub_context_ty *scp)
{
    size_t	    j;

    for (j = 0; j < scp->sub_var_pos; ++j)
	wstr_free(scp->sub_var_list[j].value);
    scp->sub_var_pos = 0;
    scp->cp = 0;
    scp->pp = 0;
    scp->errno_sequester = 0;
}


//
// NAME
//	sub_var_set
//
// SYNOPSIS
//	void sub_var_set(char *name, char *fmt, ...);
//
// DESCRIPTION
//	The sub_var_set function is used to set the value of a
//	substitution variable.	These variables are command specific,
//	as opposed to the functions which are always present.
//	The user documentation does NOT make this distinction by
//	using the names "variable" and "function", they are always referred
//	to as "substitutions".
//
// ARGUMENTS
//	name	- the name of the variable
//	fmt,... - a format string and arguments to construct the value.
//		  Handed to str_vformat to make a (string_ty *) out of it.
//
// CAVEAT
//	remains in scope until the next invokation of sub_var_clear,
//	or until the end of the next invokation of substitute.
//

void
sub_var_set_format(sub_context_ty *scp, const char *name, const char *fmt, ...)
{
    va_list	    ap;
    string_ty	    *s;

    va_start(ap, fmt);
    s = str_vformat(fmt, ap);
    va_end(ap);
    sub_var_set_string(scp, name, s);
    str_free(s);
}


void
sub_var_set_string(sub_context_ty *scp, const char *name, string_ty *value)
{
    sub_table_ty    *svp;

    if (scp->sub_var_pos >= scp->sub_var_size)
    {
	size_t		nbytes;

	scp->sub_var_size += 10;
	nbytes = scp->sub_var_size * sizeof(sub_table_ty);
	scp->sub_var_list =
            (sub_table_ty *)mem_change_size(scp->sub_var_list, nbytes);
    }
    svp = &scp->sub_var_list[scp->sub_var_pos++];
    svp->name = name;
    svp->value = str_to_wstr(value);
    svp->must_be_used = 1;
    svp->append_if_unused = 0;
    svp->override = 0;
    svp->resubstitute = !svp->must_be_used;
}


void
sub_var_resubstitute(sub_context_ty *scp, const char *name)
{
    sub_table_ty    *the_end;
    sub_table_ty    *svp;

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
sub_var_override(sub_context_ty *scp, const char *name)
{
    sub_table_ty    *the_end;
    sub_table_ty    *svp;

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
sub_var_optional(sub_context_ty *scp, const char *name)
{
    sub_table_ty    *the_end;
    sub_table_ty    *svp;

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
sub_var_append_if_unused(sub_context_ty *scp, const char *name)
{
    sub_table_ty    *the_end;
    sub_table_ty    *svp;

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
	    strcpy(tmp, progname);
	    strcat(tmp, ": ");
	}
	else
	    strcpy(tmp, "\t");
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
error_intl(sub_context_ty *scp, const char *s)
{
    wstring_ty	    *message;
    int		    need_to_delete;

    if (!os_testing_mode())
	language_check_translations();
    if (!scp)
    {
	scp = sub_context_new();
	need_to_delete = 1;
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
fatal_intl(sub_context_ty *scp, const char *s)
{
    wstring_ty	    *message;
    static const char *double_jeopardy;
    int		    need_to_delete;

    if (!os_testing_mode())
	language_check_translations();
    if (!scp)
    {
	scp = sub_context_new();
	need_to_delete = 1;
    }
    else
	need_to_delete = 0;

    //
    // Make sure that there isn't an infinite loop,
    // if there is a problem with a substitution
    // in an error message.
    //
    if (double_jeopardy)
    {
	//
	// this error message can't be internationalized
	//
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
    // NOTREACHED

    if (need_to_delete)
	sub_context_delete(scp);
}


void
verbose_intl(sub_context_ty *scp, const char *s)
{
    if (option_verbose_get())
    {
	wstring_ty	    *message;
	int		    need_to_delete;

	if (!os_testing_mode())
	    language_check_translations();

	if (!scp)
	{
	    scp = sub_context_new();
	    need_to_delete = 1;
	}
	else
	    need_to_delete = 0;

	message = subst_intl_wide(scp, s);
	wrap(message->wstr_text);
	wstr_free(message);

	if (need_to_delete)
	    sub_context_delete(scp);
    }
    else if (scp)
	sub_var_clear(scp);
}


void
sub_errno_setx(sub_context_ty *scp, int x)
{
    scp->errno_sequester = x;
}
