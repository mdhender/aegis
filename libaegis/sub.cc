//
//      aegis - project change supervisor
//	Copyright (C) 1991-2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/errno.h>
#include <common/ac/stdio.h>
#include <common/ac/stddef.h>
#include <common/ac/stdlib.h>
#include <common/ac/libintl.h>
#include <common/ac/limits.h>
#include <common/ac/string.h>
#include <common/ac/wchar.h>
#include <common/ac/wctype.h>
#include <common/ac/grp.h>
#include <common/ac/pwd.h>

#include <common/arglex.h>
#include <common/error.h>
#include <common/language.h>
#include <common/mem.h>
#include <common/page.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/collect.h>
#include <libaegis/file.h>
#include <libaegis/gonzo.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/sub/functor/glue.h>
#include <libaegis/sub/functor/variable.h>
#include <libaegis/sub/addpathsuffi.h>
#include <libaegis/sub/architecture.h>
#include <libaegis/sub/base_relativ.h>
#include <libaegis/sub/basename.h>
#include <libaegis/sub/binary_direc.h>
#include <libaegis/sub/capitalize.h>
#include <libaegis/sub/change/active_direc.h>
#include <libaegis/sub/change/attribute.h>
#include <libaegis/sub/change/delta.h>
#include <libaegis/sub/change/develo_direc.h>
#include <libaegis/sub/change/develop_list.h>
#include <libaegis/sub/change/developer.h>
#include <libaegis/sub/change/files.h>
#include <libaegis/sub/change/integr_direc.h>
#include <libaegis/sub/change/integrator.h>
#include <libaegis/sub/change/number.h>
#include <libaegis/sub/change/reviewer.h>
#include <libaegis/sub/change/reviewr_list.h>
#include <libaegis/sub/change/state.h>
#include <libaegis/sub/change/version.h>
#include <libaegis/sub/comment.h>
#include <libaegis/sub/common_direc.h>
#include <libaegis/sub/copyri_owner.h>
#include <libaegis/sub/copyri_years.h>
#include <libaegis/sub/data_directo.h>
#include <libaegis/sub/date.h>
#include <libaegis/sub/diff.h>
#include <libaegis/sub/dirname.h>
#include <libaegis/sub/dirname_rel.h>
#include <libaegis/sub/dollar.h>
#include <libaegis/sub/downcase.h>
#include <libaegis/sub/email_addres.h>
#include <libaegis/sub/expr.h>
#include <libaegis/sub/getenv.h>
#include <libaegis/sub/histo_direc.h>
#include <libaegis/sub/history_path.h>
#include <libaegis/sub/identifier.h>
#include <libaegis/sub/left.h>
#include <libaegis/sub/length.h>
#include <libaegis/sub/librar_direc.h>
#include <libaegis/sub/namemax.h>
#include <libaegis/sub/path_reduce.h>
#include <libaegis/sub/perl.h>
#include <libaegis/sub/plural.h>
#include <libaegis/sub/plural_forms.h>
#include <libaegis/sub/project.h>
#include <libaegis/sub/project/adminis_list.h>
#include <libaegis/sub/project/baseline.h>
#include <libaegis/sub/project/develop_list.h>
#include <libaegis/sub/project/integra_list.h>
#include <libaegis/sub/project/reviewe_list.h>
#include <libaegis/sub/project/specific.h>
#include <libaegis/sub/quote.h>
#include <libaegis/sub/read_file.h>
#include <libaegis/sub/right.h>
#include <libaegis/sub/search_path.h>
#include <libaegis/sub/shell.h>
#include <libaegis/sub/source.h>
#include <libaegis/sub/split.h>
#include <libaegis/sub/substitute.h>
#include <libaegis/sub/substr.h>
#include <libaegis/sub/switch.h>
#include <libaegis/sub/trim_directo.h>
#include <libaegis/sub/trim_extensi.h>
#include <libaegis/sub/unsplit.h>
#include <libaegis/sub/upcase.h>
#include <libaegis/sub/user.h>
#include <libaegis/sub/zero_pad.h>


enum getc_type
{
    getc_type_control,
    getc_type_data
};


sub_context_ty::sub_context_ty(const char *arg1, int arg2) :
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


change::pointer
sub_context_ty::change_get()
{
    return cp;
}


//
// NAME
//      sub_errno - the errno substitution
//
// SYNOPSIS
//      wstring sub_errno(const wstring_list &arg);
//
// DESCRIPTION
//      The sub_errno function implements the errno substitution.  The
//      errno substitution is replaced by the value if th errno variable
//      provided by the system, as mapped through the strerror function.
//
//      Requires exactly zero arguments.
//
//      The sub_errno_setx() function may be used to remember errno,
//      and thus isolate the error from subsequest system calls.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

static wstring
sub_errno(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_errno()\n{\n"));
    if (arg.size() != 1)
    {
        scp->error_set(i18n("requires zero arguments"));
        trace(("return NULL;\n"));
        trace(("}\n"));
        return wstring();
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

        nstring s = nstring::format("%s [%s, %s]", strerror(err), uidn, gidn);
        wstring result(s);
        trace(("return %8.8lX;\n", (long)result.get_ref()));
        trace(("}\n"));
        return result;
    }

    wstring result(strerror(err));
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}


static sub_functor_list globals;


static void
init_globals()
{
    if (!globals.empty())
        return;
    globals.push_back(sub_functor_glue::create("$", sub_dollar));
    globals.push_back(sub_functor_glue::create("#", sub_comment));
    globals.push_back
    (
        sub_functor_glue::create
        (
            "Active_Directory",
            sub_change_active_directory
        )
    );
    globals.push_back
    (
        sub_functor_glue::create("Add_Path_Suffix", sub_add_path_suffix)
    );
    globals.push_back
    (
        sub_functor_glue::create("Administrator_List", sub_administrator_list)
    );
    globals.push_back
    (
        sub_functor_glue::create("ARCHitecture", sub_architecture)
    );
    globals.push_back(sub_functor_glue::create("BaseLine", sub_baseline));
    globals.push_back(sub_functor_glue::create("Basename", sub_basename));
    globals.push_back
    (
        sub_functor_glue::create("BAse_RElative", sub_base_relative)
    );
    globals.push_back
    (
        sub_functor_glue::create("BINary_DIRectory", sub_binary_directory)
    );
    globals.push_back(sub_functor_glue::create("CAPitalize", sub_capitalize));
    globals.push_back(sub_functor_glue::create("Change", sub_change_number));
    globals.push_back
    (
        sub_functor_glue::create("Change_Attribute", sub_change_attribute)
    );
    globals.push_back
    (
        sub_functor_glue::create
        (
            "Change_Developer_List",
            sub_change_developer_list
        )
    );
    globals.push_back
    (
        sub_functor_glue::create("Change_Files", sub_change_files)
    );
    globals.push_back
    (
        sub_functor_glue::create
        (
            "Change_Reviewer_List",
            sub_change_reviewer_list
        )
    );
    globals.push_back
    (
        sub_functor_glue::create("Copyright_Owner", sub_copyright_owner)
    );
    globals.push_back
    (
        sub_functor_glue::create("Copyright_Years", sub_copyright_years)
    );
    globals.push_back(sub_functor_glue::create("COMment", sub_comment));
    globals.push_back
    (
        sub_functor_glue::create("COMmon_DIRectory", sub_common_directory)
    );
    globals.push_back
    (
        sub_functor_glue::create("DATa_DIRectory", sub_data_directory)
    );
    globals.push_back(sub_functor_glue::create("DAte", sub_date));
    globals.push_back(sub_functor_glue::create("DELta", sub_delta));
    globals.push_back(sub_functor_glue::create("DEVeloper", sub_developer));
    globals.push_back
    (
        sub_functor_glue::create("DEVeloper_List", sub_developer_list)
    );
    // Default_Development_Directory
    globals.push_back
    (
        sub_functor_glue::create
        (
            "Development_Directory",
            sub_development_directory
        )
    );
    globals.push_back(sub_functor_glue::create("DIFference", sub_diff));
    globals.push_back(sub_functor_glue::create("Dirname", sub_dirname));
    globals.push_back
    (
        sub_functor_glue::create("Dirname_RELative", sub_dirname_relative)
    );
    globals.push_back(sub_functor_glue::create("DownCase", sub_downcase));
    globals.push_back(sub_functor_glue::create("DOLlar", sub_dollar));
    // Edit
    globals.push_back
    (
        sub_functor_glue::create("EMail_Address", sub_email_address)
    );
    globals.push_back(sub_functor_glue::create("ENVironment", sub_getenv));
    globals.push_back(sub_functor_glue::create("ERrno", sub_errno));
    globals.push_back(sub_functor_glue::create("EXpression", sub_expression));
    // FieLD_List
    // File_List
    // File_Name
    globals.push_back(sub_functor_glue::create("Get_Environment", sub_getenv));
    // Guess
    // History
    globals.push_back
    (
        sub_functor_glue::create("History_Directory", sub_history_directory)
    );
    globals.push_back
    (
        sub_functor_glue::create("History_Path", sub_history_path)
    );
    // Input
    globals.push_back(sub_functor_glue::create("IDentifier", sub_identifier));
    globals.push_back
    (
        sub_functor_glue::create
        (
            "INTegration_Directory",
            sub_integration_directory
        )
    );
    globals.push_back(sub_functor_glue::create("INTegrator", sub_integrator));
    globals.push_back
    (
        sub_functor_glue::create("INTegrator_List", sub_integrator_list)
    );
    globals.push_back(sub_functor_glue::create("LEFt", sub_left));
    globals.push_back(sub_functor_glue::create("LENgth", sub_length));
    globals.push_back(sub_functor_glue::create("LIBrary", sub_data_directory));
    globals.push_back
    (
        sub_functor_glue::create("LIBrary_DIRectory", sub_library_directory)
    );
    // MAgic
    // MeSsaGe
    // Most_Recent
    globals.push_back(sub_functor_glue::create("Name_Maximum", sub_namemax));
    // Number
    // Output
    // ORiginal
    globals.push_back(sub_functor_glue::create("PAth_REduce", sub_path_reduce));
    globals.push_back(sub_functor_glue::create("PERL", sub_perl));
    globals.push_back(sub_functor_glue::create("PLural", sub_plural));
    globals.push_back
    (
        sub_functor_glue::create("PLural_Forms", sub_plural_forms)
    );
    globals.push_back(sub_functor_glue::create("Project", sub_project));
    globals.push_back
    (
        sub_functor_glue::create("Project_Specific", sub_project_specific)
    );
    globals.push_back(sub_functor_glue::create("QUote", sub_quote));
    globals.push_back
    (
        sub_functor_glue::create("Read_File", sub_read_file, true)
    );
    globals.push_back
    (
        sub_functor_glue::create("Read_File_Simple", sub_read_file)
    );
    globals.push_back(sub_functor_glue::create("Reviewer", sub_reviewer));
    globals.push_back
    (
        sub_functor_glue::create("Reviewer_List", sub_reviewer_list)
    );
    globals.push_back(sub_functor_glue::create("RIght", sub_right));
    globals.push_back(sub_functor_glue::create("Search_Path", sub_search_path));
    globals.push_back
    (
        sub_functor_glue::create("Search_Path_Executable", sub_search_path)
    );
    globals.push_back(sub_functor_glue::create("SHell", sub_shell));
    globals.push_back(sub_functor_glue::create("Source", sub_source));
    globals.push_back(sub_functor_glue::create("SPLit", sub_split));
    globals.push_back(sub_functor_glue::create("STate", sub_state));
    globals.push_back(sub_functor_glue::create("SUBSTitute", sub_substitute));
    globals.push_back(sub_functor_glue::create("SUBSTRing", sub_substr));
    globals.push_back(sub_functor_glue::create("SWitch", sub_switch));
    globals.push_back
    (
        sub_functor_glue::create("Trim_DIRectory", sub_trim_directory)
    );
    globals.push_back
    (
        sub_functor_glue::create("Trim_EXTension", sub_trim_extension)
    );

    // uname is undocumented
    globals.push_back(sub_functor_glue::create("UName", sub_architecture));

    globals.push_back(sub_functor_glue::create("UNSplit", sub_unsplit));
    globals.push_back(sub_functor_glue::create("UpCase", sub_upcase));
    globals.push_back(sub_functor_glue::create("USer", sub_user));
    globals.push_back(sub_functor_glue::create("Version", sub_version));
    globals.push_back(sub_functor_glue::create("Zero_Pad", sub_zero_pad));
}


void
sub_context_ty::execute(const wstring_list &arg)
{
    trace(("execute()\n{\n"));
    init_globals();
    if (arg.empty())
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
    // scan the variables and functions
    //
    nstring cmd = arg[0].to_nstring();
    sub_functor_list hits;
    globals.match(cmd, hits);
    var_list.match(cmd, hits);

    //
    // figure what to do
    //
    switch (hits.size())
    {
    case 0:
        {
            nstring s3 = arg.unsplit(" ").to_nstring();
            const char *the_error = i18n("unknown substitution name");
            sub_context_ty inner(__FILE__, __LINE__);
            inner.var_set_charstar("File_Name", file_name);
            inner.var_set_long("Line_Number", line_number);
            inner.var_set_string("Name", s3);
            inner.var_set_charstar("MeSsaGe", gettext(the_error));
            inner.fatal_intl
            (
                i18n
                (
                    "$filename: $linenumber: substitution $${$name} "
                    "failed: $message"
                )
            );
            // NOTREACHED
        }

    case 1:
        break;

    default:
        {
            nstring s3 = arg.unsplit(" ").to_nstring();
            const char *the_error = i18n("ambiguous substitution name");
            sub_context_ty inner(__FILE__, __LINE__);
            inner.var_set_charstar("File_Name", file_name);
            inner.var_set_long("Line_Number", line_number);
            inner.var_set_string("Name", s3);
            inner.var_set_charstar("MeSsaGe", gettext(the_error));
            inner.fatal_intl
            (
                i18n
                (
                    "$filename: $linenumber: substitution $${$name} "
                    "failed: $message"
                )
            );
            // NOTREACHED
        }
    }

    //
    // Do it.
    //
    sub_functor::pointer tp = hits[0];
    wstring_list tmp;
    tmp.push_back(wstring(tp->name_get()));
    for (size_t j = 1; j < arg.size(); ++j)
        tmp.push_back(arg[j]);
    wstring s = tp->evaluate(this, tmp);

    //
    // deal with the result
    //
    if (!s.empty())
        diversion_stack.push_back(s, tp->resubstitute());
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
    wstring_list arg;
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
            wstring s = tmp.end();
            trace(("push arg\n"));
            arg.push_back(s);
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
            wstring s = tmp.end();
            trace(("push arg\n"));
            arg.push_back(s);
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
            //      any run of non-white-space characters
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
            wstring s = tmp.end();
            trace(("push arg\n"));
            arg.push_back(s);
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
sub_context_ty::subst_intl_change(change::pointer a)
{
    assert(!pp);
    assert(!cp);
    pp = a->pp;
    cp = a;
}


wstring
sub_context_ty::subst(const wstring &s)
{
    trace(("subst(s = %8.8lX)\n{\n", (long)s.get_ref()));
    collect buf;
    diversion_stack.push_back(s, true);
    for (;;)
    {
        //
        // get the next character
        //
        getc_type ct;
        wchar_t c = getch(ct);
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
    for (size_t j = 0; j < var_list.size(); ++j)
    {
        sub_functor::pointer tp = var_list[j];
        if (!tp->append_if_unused())
            continue;
        if (!tp->must_be_used())
            continue;

        //
        // append to the buffer, separated by a space
        //
        buf.append(L' ');
        wstring_list args;
        buf.push_back(tp->evaluate(this, args));
    }

    //
    // find any unused variables
    // and complain about them
    //
    int error_count = 0;
    for (size_t j = 0; j < var_list.size(); ++j)
    {
        sub_functor::pointer tp = var_list[j];
        if (!tp->must_be_used())
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
        inner.var_set_string("MeSsaGe", s.to_nstring());
        inner.var_optional("MeSsaGe");
        inner.var_set_string("Name", "$" + tp->name_get());
        inner.var_optional("Name");
        inner.error_intl
        (
            i18n
            (
                "$filename: $linenumber: in substitution \"$message\" "
                "variable \"$name\" unused"
            )
        );
        ++error_count;
    }
    if (error_count > 0)
    {
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
        inner.var_set_string("MeSsaGe", s.to_nstring());
        inner.var_optional("MeSsaGe");
        inner.var_set_long("Number", error_count);
        inner.var_optional("Number");
        inner.fatal_intl
        (
            i18n
            (
                "$filename: $linenumber: in substitution \"$message\" "
                "found unused variables"
            )
        );
        // NOTREACHED
    }

    //
    // clear the slate, ready for the next run
    //
    clear();
    wstring result = buf.end();
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}


wstring
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
    wstring s(tmp);
    wstring result = subst(s);
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}


string_ty *
sub_context_ty::subst_intl(const char *s)
{
    trace(("subst_intl(s = \"%s\")\n{\n", s));
    wstring result_wide = subst_intl_wide(s);
    string_ty *result = wstr_to_str(result_wide.get_ref());
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}


string_ty *
sub_context_ty::substitute(change::pointer acp, string_ty *s)
{
    trace(("substitute(acp = %08lX, s = \"%s\")\n{\n", (long)acp, s->str_text));
    assert(acp);
    subst_intl_change(acp);
    wstring wis(s->str_text);
    wstring result_wide = subst(wis);
    string_ty *result = wstr_to_str(result_wide.get_ref());
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
    wstring wis(s->str_text, s->str_length);
    wstring result_wide = subst(wis);
    string_ty *result = wstr_to_str(result_wide.get_ref());
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}


void
sub_context_ty::clear()
{
    var_list.clear();
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
    var_list.push_back(sub_functor_variable::create(name, value));
}


void
sub_context_ty::var_set_string(const char *name, string_ty *value)
{
    var_set_string(name, nstring(value));
}


void
sub_context_ty::var_resubstitute(const char *name)
{
    sub_functor::pointer tp = var_list.find(name);
    if (tp)
        tp->resubstitute_set();
    else
        this_is_a_bug();
}


void
sub_context_ty::var_override(const char *name)
{
    sub_functor::pointer tp = var_list.find(name);
    if (tp)
        tp->override_set();
    else
        this_is_a_bug();
}


void
sub_context_ty::var_optional(const char *name)
{
    sub_functor::pointer tp = var_list.find(name);
    if (tp)
        tp->optional_set();
    else
        this_is_a_bug();
}


void
sub_context_ty::var_append_if_unused(const char *name)
{
    sub_functor::pointer tp = var_list.find(name);
    if (tp)
        tp->append_if_unused_set();
    else
        this_is_a_bug();
}


//
// NAME
//      wrap - wrap s string over lines
//
// SYNOPSIS
//      void wrap(const wstring &);
//
// DESCRIPTION
//      The wrap function is used to print error messages onto stderr
//      wrapping ling lines.  Be very careful of multi-byte characters
//      in international character sets.
//
// CAVEATS
//      Line length is assumed to be 80 characters.
//

static void
wrap(const wchar_t *s)
{
    const char      *progname;
    int             page_width;
    int             first_line;
    int             nbytes;
    static int      progname_width;
    int             midway;

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
        wstring wis = wstr_from_c(progname);
        progname_width = wis.column_width();
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
        const wchar_t   *ep;
        int             ocol;
        const wchar_t   *break_space;
        int             break_space_col;
        const wchar_t   *break_punct;
        int             break_punct_col;

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
            char            dummy[MB_LEN_MAX];
            int             cw;
            wchar_t         c;

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
                // The wctomb state will be "error",
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
        char tmp[(MAX_PAGE_WIDTH + 2) * MB_LEN_MAX];
        char *tp = tmp;
        if (first_line)
        {
            tp = strendcpy(tp, progname, tmp + sizeof(tmp));
            tp = strendcpy(tp, ": ", tmp + sizeof(tmp));
        }
        else
            tp = strendcpy(tp, "\t", tmp + sizeof(tmp));

        //
        // Turn the input into a multi bytes chacacters.
        //
        wctomb(NULL, 0);
        while (s < ep)
        {
            wchar_t         c;

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
                // The wctomb state will be "error",
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
    wstring message = subst_intl_wide(s);
    wrap(message.c_str());
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

    wstring message = subst_intl_wide(s);
    wrap(message.c_str());
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
        wstring message = subst_intl_wide(s);
        wrap(message.c_str());
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
