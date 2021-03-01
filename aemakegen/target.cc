//
// aegis - project change supervisor
// Copyright (C) 2006-2012 Peter Miller;
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 3, as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/unistd.h>

#include <common/trace.h>
#include <libaegis/attribute.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/output/filter/prefix.h>
#include <libaegis/output/filter/set_width.h>
#include <libaegis/output/filter/wrap_simple.h>
#include <libaegis/output/stdout.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>

#include <aemakegen/debug.h>
#include <aemakegen/flavour.h>
#include <aemakegen/target.h>
#include <aemakegen/process_item/extra_dist.h>
#include <aemakegen/util.h>


nstring_list target::scripts;


target::~target()
{
}


void
target::set_flavour(const nstring &name)
{
    flavour::pointer fp = flavour::factory(name, *this);
    fp->set_process();
    processing.register_default(process_item_extra_dist::create(*this));
}


static void
check_state(change::pointer cp)
{
    cstate_ty *csp = cp->cstate_get();
    assert(csp);
    string_ty *d = 0;
    switch (csp->state)
    {
    case cstate_state_being_integrated:
        d = change_integration_directory_get(cp, 0);
        break;

    case cstate_state_awaiting_review:
    case cstate_state_awaiting_integration:
    case cstate_state_being_reviewed:
    case cstate_state_being_developed:
        d = change_development_directory_get(cp, 0);
        break;

    default:
        change_fatal(cp, 0, i18n("bad cd, no dir"));
        // NOTREACHED
    }
    chdir(d->str_text);
}


target::target(change_identifier &a_cid) :
    cid(a_cid),
    ostate(ostate_reset)
{
    check_state(cid.get_cp());

    // FIXME: output_file::create for the --output=filename option.
    op = output_stdout::create();
    op = output_filter_set_width::create(op, 80);

    //
    // Initialise the view path used to locate actual files in the project
    // baseline, and not in the change set's development directory.
    //
    nstring_list dirs;
    cid.get_cp()->search_path_get(dirs, true);
    for (size_t k = 0; k < dirs.size(); ++k)
        view_path.push_back(dirs[k] + "/");

    //
    // See if the project is known to install a shared library.  We use the
    // "aemakegen:libtool=true" project attribute as a proxy for this.
    //
    if (cid.get_cp()->pconf_attributes_get_boolean("aemakegen:libtool"))
    {
        data.set_use_libtool();
    }
    if (data.use_libtool())
    {
        //
        // The debian folk take exception to the "almost
        // always useless" *.la files install by libtool.
        // So this defaults to false.
        //
        if (cid.get_cp()->pconf_attributes_get_boolean("aemakegen:libtool-la"))
            data.set_use_lib_la_files();

        //
        // We also need to get the version information to use with the
        // shared file.
        //
        nstring text =
            cid.get_cp()->pconf_attributes_find("aemakegen:version-info");
        data.set_version_info(text);
    }
}


void
target::script(const nstring &arg)
{
    scripts.push_back(arg);
}


nstring
target::resolve(const nstring &relpath)
{
    if (relpath[0] == '/')
        return relpath;
    for (size_t j = 0; j < view_path.size(); ++j)
    {
        nstring path = view_path[j] + relpath;
        if (exists(path))
            return path;
    }
    return relpath;
}


bool
target::exists(const nstring &path)
{
    return (access(path.c_str(), F_OK) == 0);
}


void
target::process(const nstring_list &filenames)
{
    manifest = filenames;

    // Processing the list of file names has several phases:
    //
    // 1. pre-preprocess
    //    no text is emitted at this stage
    //    1a. set library directory
    //    1b. configure.ac
    //    1c. noinst file attribute
    //    1d. master include file, and closure
    process1(manifest);

    // 2. preprocess -- this stage figures out file lists
    //    no text is emitted at this stage
    //    2a. preprocess_begin()
    //    2b. preprocess per file
    //    2c. preprocess_end()
    process2(manifest);

    // 3. process -- this stage emits text
    //    *this stage is a virtual method*
    //    3a. begin()
    //    3b. process per file
    //    3c. end()
    process3(manifest);
}


static bool
program_name_implies_noinst(const nstring &base)
{
    assert(base == base.basename());
    return
        (
            base.starts_with("test_")
        ||
            base.starts_with("test-")
        ||
            base.starts_with("noinst_")
        ||
            base.starts_with("noinst-")
        );
}


void
target::process1(const nstring_list &filenames)
{
    //
    // Before we preprocess anything, we locate the project's library
    // directory, if the project has one.
    //
    data.set_library_directory(cid, filenames);

    //
    // 1. Set the list of files that are *not* to be installed.
    // 2. Grab information from the configure.ac file.
    //
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        nstring filename = filenames[j];

        //
        // Progess the configure.ac file, to see if there is any useful
        // information to be gleaned from it.
        //
        nstring base = filename.basename();
        if
        (
            base == "configure.ac"
        ||
            base == "configure.ac.in"
        ||
            base == "configure.in"
        )
        {
            preprocess_configure_dot_ac(filename);
        }

        // We need to find all the program directories.
        if (is_a_source_file(filename) && filename_implies_progname(filename))
        {
            data.remember_progdir(filename.dirname());
        }

        //
        // Remember the names of the files that are not to be installed,
        // taken from the "aemakegen:noinst" file attribute.
        //
        if (file_attr_noinst(filename))
        {
            if (filename_implies_is_a_script(filename))
            {
                nstring prog = filename.basename();
                if (filename.ends_with(".in"))
                    prog = prog.trim_extension();
                if (extension_implies_script(prog))
                    prog = prog.trim_extension();
                data.remember_explicit_noinst(prog);
            }
            else
            {
                nstring prog = progname_from_dir_of(filename);
                data.remember_explicit_noinst(prog);
            }
        }
        else
        {
            //
            // Some file names imply that the program is not to be installed.
            //
            if (filename_implies_is_a_script(filename))
            {
                nstring prog = filename.basename();
                if (filename.ends_with(".in"))
                    prog = prog.trim_extension();
                if (extension_implies_script(prog))
                    prog = prog.trim_extension();
                if (program_name_implies_noinst(prog))
                    data.remember_explicit_noinst(prog);
            }
            else
            {
                nstring progname = progname_from_dir_of(filename);
                if (program_name_implies_noinst(progname))
                    data.remember_explicit_noinst(progname);
            }
        }
    }

    //
    // The master include file, and the files it includes, are all installable.
    // The master include file's name is "lib<project>/lib<project>.h"
    //
    nstring master_include_file =
        data.get_library_directory() + "/" + data.get_library_name() + ".h";
    if (filenames.member(master_include_file))
    {
        preprocess_include_headers(master_include_file);
    }
    else
    {
        master_include_file =
            data.get_library_directory() + "/" + get_project_name() + ".h";
        if (filenames.member(master_include_file))
        {
            preprocess_include_headers(master_include_file);
        }
    }
}


void
target::process2(const nstring_list &filenames)
{
    //
    // Preprocess per file:
    //
    // Let the derived classes look at the files.
    //
    process2_begin();
    processing.run_preprocess(filenames);
    process2_end();

    // done with preprocessing stage
    data.notify_preprocess_complete();
}


void
target::process2_begin(void)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process2_end(void)
{
    // remember to clean up after programs
    {
        nstring_list programs = data.get_programs();
        for (size_t j = 0; j < programs.size(); ++j)
        {
            nstring prog = programs[j];

            {
                nstring filename = "bin/" + prog + data.exeext();
                data.remember_all_bin(filename);
            }

            if (is_installable(prog))
            {
                nstring filename =
                    (
                        "$(bindir)/"
                    +
                        data.get_program_prefix()
                    +
                        prog
                    +
                        data.get_program_suffix()
                    +
                        data.exeext()
                    );
                data.remember_install_bin(filename);
            }
        }
    }

    // remember to clean up after libraries
    {
        nstring_list dirs = data.get_list_of_library_directories();
        for (size_t j = 0; j < dirs.size(); ++j)
        {
            nstring library_dirname = dirs[j];

            nstring library_libname = library_dirname;
            if (library_libname == "lib")
                library_libname = get_project_name();
            if (!library_libname.starts_with("lib"))
                library_libname = "lib" + library_libname;

            nstring filename =
                library_dirname + "/" + library_libname + "." + data.libext();
            data.remember_clean_obj_file(filename);

            // otherwise always assume it is no-inst and static.
            bool shared =
                (
                    data.use_libtool()
                &&
                    library_libname == get_library_libname()
                );
            if (shared)
            {
                data.remember_clean_obj_file(filename.trim_extension() + ".a");
                data.remember_dist_clean_dir(filename.dirname() + "/.libs");

                nstring fn = "$(libdir)/" + filename.basename();
                data.remember_install_libdir(fn);
            }
        }
    }

    // The distclean target should remove the config.h file, as well.
    {
        nstring libdir = data.get_library_directory();
        if (!libdir.empty())
        {
            data.remember_dist_clean_file(libdir + "/config.h");
        }
    }
}


void
target::process3(const nstring_list &filenames)
{
    process3_begin();
    processing.run_process(filenames);
    process3_end();
}


void
target::process3_begin(void)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process3_end(void)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


bool
target::filename_implies_is_a_script(const nstring &fn)
{
    if (fn.gmatch(scripts))
        return true;
    return
        (
            (fn.starts_with("script/") || fn.starts_with("scripts/"))
        &&
            (extension_implies_script(fn) || is_hash_bang(fn))
        );
}


bool
target::trim_script_suffix(void)
    const
{
    // FIXME: this may need to be controlled from a command line option,
    // one day.
    return true;
}


nstring
target::get_project_name(void)
    const
{
    return nstring(cid.get_pp()->trunk_get()->name_get());
}


nstring
target::get_project_version(void)
    const
{
    return cid.get_cp()->version_get();
}


static bool
ac_lang_cplusplus(const nstring &line)
{
    return
        (
            line.starts_with("AC_LANG_CPLUSPLUS")
        ||
            line.starts_with("AC_LANG(C++)")
        ||
            line.starts_with("AC_LANG([C++])")
        );
}


static bool
ac_lang_c(const nstring &line)
{
    return
        (
            line.starts_with("AC_LANG_C")
        ||
            line.starts_with("AC_LANG(C)")
        ||
            line.starts_with("AC_LANG([C])")
        );
}


void
target::preprocess_configure_dot_ac(const nstring &fn)
{
    nstring path = resolve(fn);
    FILE *fp = fopen(path.c_str(), "r");
    if (!fp)
    {
        nstring fn2 = fn + ".in";
        path = resolve(fn2);
        fp = fopen(path.c_str(), "r");
    }
    if (!fp)
        return;
    for (;;)
    {
        char cline[2000];
        if (!fgets(cline, sizeof(cline), fp))
            break;
        nstring line(cline);

        //
        // Check whether or not the project installs internationalisation and
        // localisation files.
        //
        if
        (
            line.starts_with("AC_CHECK_PROGS(MSGFMT,")
        ||
            line.starts_with("AC_CHECK_PROGS(MSGCAT,")
        )
        {
            data.set_use_i18n();
        }
        if (line.starts_with("AC_SUBST(NLSDIR)"))
        {
            data.set_have_nlsdir();
            data.set_use_i18n();
        }

        //
        // Check whether the project needs X11 to build.
        //
        if (line.starts_with("AC_PATH_XTRA"))
            data.set_use_x11();

        if (line.starts_with("AC_PROG_LEX"))
            data.set_have_lex();
        if (line.starts_with("AC_PROG_YACC"))
            data.set_have_yacc();
        if (line.starts_with("AC_PROG_RANLIB"))
            data.set_have_ranlib();
        if (line.starts_with("AC_CHECK_PROGS(AR,"))
            data.set_have_ar();
        if (line.starts_with("AC_CHECK_PROGS(GROFF,"))
            data.set_have_groff();
        if (line.starts_with("AC_CHECK_PROGS(SOELIM,"))
            data.set_have_soelim();
        if (line.starts_with("AC_OBJEXT"))
            data.set_objext();
        if (line.starts_with("AC_LIBEXT"))
            data.set_libext();
        if (line.starts_with("AC_EXEEXT"))
            data.set_exeext();

        if (ac_lang_cplusplus(line))
            data.set_seen_c_plus_plus();
        else if (ac_lang_c(line))
            data.set_seen_c();

        if (line.starts_with("dnl! PROGRAM_PREFIX"))
            data.set_program_prefix();
        if (line.starts_with("dnl! PROGRAM_SUFFIX"))
            data.set_program_suffix();
    }
    fclose(fp);
}


bool
target::is_installable(const nstring &name)
{
    return !data.is_explicit_noinst(name);
}


bool
target::file_attr_noinst(const nstring &filename)
{
    fstate_src_ty *src = cid.get_cp()->file_find(filename, view_path_extreme);
    return
        (
            src
        &&
            src->attribute
        &&
            attributes_list_find_boolean(src->attribute, "aemakegen:noinst")
        );
}


bool
target::is_hash_bang(const nstring &filename)
{
    nstring path = resolve(filename);
    FILE *fp = fopen(path.c_str(), "r");
    if (!fp)
    {
        nstring fn2 = filename + ".in";
        path = resolve(fn2);
        fp = fopen(path.c_str(), "r");
    }
    if (!fp)
        return false;
    int c1 = getc(fp);
    int c2 = getc(fp);
    fclose(fp);
    return (c1 == '#' && c2 == '!');
}


bool
target::contains_dot_so_directive(const nstring &filename)
{
    nstring path = resolve(filename);
    FILE *fp = fopen(path.c_str(), "r");
    if (!fp)
        return false;
    char state = '\n';
    for (;;)
    {
        int c = getc(fp);
        switch (c)
        {
        case EOF:
            fclose(fp);
            return false;

        case '\n':
            state = '\n';
            break;

        case '.':
            state = (state == '\n' ? '.' : 'X');
            break;

        case 's':
            state = (state == '.' ? 's' : 'X');
            break;

        case 'o':
            state = (state == 's' ? 'o' : 'X');
            break;

        case ' ':
            if (state == '.')
                break;
            if (state == 'o')
            {
                fclose(fp);
                return true;
            }
            state = 'X';
            break;

        default:
            state = 'X';
            break;
        }
    }
}


bool
target::contains_dot_xx_directive(const nstring &filename)
{
    nstring path = resolve(filename);
    FILE *fp = fopen(path.c_str(), "r");
    if (!fp)
        return false;
    for (;;)
    {
        char line[100];
        if (!fgets(line, sizeof(line), fp))
        {
            fclose(fp);
            return false;
        }
        if (strlen(line) > 6 && 0 == memcmp(line, ".XX ", 4))
        {
            fclose(fp);
            return true;
        }
    }
}


nstring
target::get_library_libname(void)
    const
{
    nstring result = data.get_library_name();
    if (result == "lib")
        result = get_project_name();
    if (!result.starts_with("lib"))
        result = "lib" + result;
    return result;
}


void
target::warning_this_file_is_generated(void)
{
    warning_this_file_is_generated(op);
    ostate = ostate_comment;
}


void
target::warning_this_file_is_generated(const output::pointer &fp)
    const
{
    trace(("%s {\n", __PRETTY_FUNCTION__));
    output::pointer op2 = output_filter_prefix::create(fp, "# ");
    op2->fputs
    (
        "\n"
        "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
        "\n"
        "   W   W    A    RRRR   N   N   III  N   N  III  N   N   GGG\n"
        "   W   W   A A   R   R  NN  N    I   NN  N   I   NN  N  G   G\n"
        "   W   W  A   A  RRRR   N N N    I   N N N   I   N N N  G\n"
        "   W W W  AAAAA  R R    N  NN    I   N  NN   I   N  NN  G  GG\n"
        "   W W W  A   A  R  R   N   N    I   N   N   I   N   N  G   G\n"
        "    W W   A   A  R   R  N   N   III  N   N  III  N   N   GGG\n"
        "\n"
    );
    {
        output::pointer op3 = output_filter_wrap_simple::create(op2);
        op3->fputs
        (
            "Warning: DO NOT send patches which fix this file.  IT IS NOT "
            "the original source file.  This file is GENERATED from the "
            "Aegis repository file manifest.  If you find a bug in this "
            "file, it could well be an Aegis bug.\n"
        );
    }
    op2->fputs
    (
        "\n"
        "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
        "\n"
    );
    trace(("}\n"));
}


void
target::register_process(const process_item::pointer &pip)
{
    processing.register_process_item(pip);
}


void
target::register_process_front(const process_item::pointer &pip)
{
    processing.register_process_item_front(pip);
}


void
target::location_comment(const char *file, int line)
{
    if (!debug)
        return;
    ostate_become(ostate_comment);
    op->fprintf
    (
        "#line %d %s\n",
        line,
        nstring(file).quote_c().c_str()
    );
}


#define OPAIR(a, b) ((a) * 8 + (b))

void
target::ostate_become(ostate_t new_ostate)
{
    switch (OPAIR(ostate, new_ostate))
    {
    case OPAIR(ostate_reset, ostate_comment):
    case OPAIR(ostate_reset, ostate_assignment):
    case OPAIR(ostate_reset, ostate_rule):
    case OPAIR(ostate_comment, ostate_assignment):
    case OPAIR(ostate_comment, ostate_rule):
    case OPAIR(ostate_assignment, ostate_assignment):
        break;

    case OPAIR(ostate_comment, ostate_comment):
    case OPAIR(ostate_assignment, ostate_comment):
    case OPAIR(ostate_assignment, ostate_rule):
    case OPAIR(ostate_rule, ostate_comment):
    case OPAIR(ostate_rule, ostate_assignment):
    case OPAIR(ostate_rule, ostate_rule):
        op->fputc('\n');
        break;
    }
    ostate = new_ostate;
}


void
target::print_comment(const nstring &text)
{
    ostate_become(ostate_comment);
    output::pointer op2 = output_filter_prefix::create(op, "# ");
    output::pointer op3 = output_filter_wrap_simple::create(op2);
    op3->fputs(text);
}


void
target::print_empty_assignment(const nstring &name)
{
    ostate_become(ostate_assignment);
    op->fputs(name);
    op->fputs(" =\n");
}


void
target::print_assignment(const nstring &name, const nstring &value)
{
    nstring_list values;
    values.push_back(value);
    print_assignment(name, values);
}


void
target::print_assignment(const nstring &name,
    const nstring_list &value)
{
    if (value.empty())
        return;
    ostate_become(ostate_assignment);
    op->fputs(name);
    op->fputs(" =");
    for (size_t j = 0; j < value.size(); ++j)
    {
        op->fputc(' ');
        op->fputs(value[j]);
    }
    op->fputc('\n');
}


void
target::print_assignment_sorted(const nstring &name, const nstring_list &value)
{
    nstring_list value2 = value;
    value2.sort();
    print_assignment(name, value2);
}


void
target::print_rule(const nstring_list &lhs, const nstring_list &rhs,
    const nstring_list &body)
{
    assert(!lhs.empty() && "no target given");
    if (rhs.empty() && body.empty())
        return;
    ostate_become(ostate_rule);
    nstring_list lhs2 = lhs;
    lhs2.sort();
    for (size_t j = 0; j < lhs2.size(); ++j)
    {
        if (j)
            op->fputc(' ');
        op->fputs(lhs2[j]);
    }
    op->fputc(':');
    nstring_list rhs2 = rhs;
    rhs2.sort();
    for (size_t j = 0; j < rhs2.size(); ++j)
    {
        op->fputc(' ');
        op->fputs(rhs2[j]);
    }
    op->fputc('\n');
    for (size_t j = 0; j < body.size(); ++j)
    {
        op->fputs("\t");
        op->fputs(body[j]);
        op->fputc('\n');
    }
}


void
target::print_rule(const nstring &lhs, const nstring &rhs)
{
    nstring_list lhs2;
    lhs2.push_back(lhs);
    nstring_list rhs2;
    rhs2.push_back(rhs);
    nstring_list body;
    print_rule(lhs2, rhs2, body);
}


void
target::print_clean_file_rule(const nstring &lhs,
    const nstring_list &rhs, const nstring_list &files)
{
    nstring_list files2 = files;
    files2.sort();

    ostate_become(ostate_rule);
    op->fputs(lhs);
    op->fputc(':');
    for (size_t j = 0; j < rhs.size(); ++j)
    {
        op->fputc(' ');
        op->fputs(rhs[j]);
    }
    op->fputc('\n');

    if (!files2.empty())
    {
        output::pointer op2 = output_filter_prefix::create(op, "\trm -f ");
        output::pointer op3 = output_filter_wrap_simple::create(op2);
        for (size_t j = 0; j < files2.size(); ++j)
        {
            if (j)
                op3->fputc(' ');
            op3->fputs(files2[j]);
        }
        op3->fputc('\n');
    }
}


void
target::print_clean_dirs_rule(const nstring &lhs,
    const nstring_list &rhs, const nstring_list &dirs1)
{
    nstring_list dirs = dirs1;
    dirs.sort();

    ostate_become(ostate_rule);
    op->fputs(lhs);
    op->fputc(':');
    nstring_list rhs2 = rhs;
    rhs2.sort();
    for (size_t j = 0; j < rhs2.size(); ++j)
    {
        op->fputc(' ');
        op->fputs(rhs2[j]);
    }
    op->fputc('\n');

    if (!dirs.empty())
    {
        output::pointer op2 = output_filter_prefix::create(op, "\trm -rf ");
        output::pointer op3 = output_filter_wrap_simple::create(op2);
        for (size_t j = 0; j < dirs.size(); ++j)
        {
            if (j)
                op3->fputc(' ');
            op3->fputs(dirs[j]);
        }
        op3->fputc('\n');
    }
}


bool
target::is_an_include_candidate(const nstring &fn)
    const
{
    return
        (
            manifest.member(fn)
        ||
            data.get_built_sources().member(fn)
        ||
            fn.ends_with(".fmtgen.h")
        ||
            fn.ends_with(".yacc.h")
        ||
            manifest.member(fn + ".in")
        );
}


nstring_list
target::c_include_dependencies(const nstring &ifn)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring_list result;
    result.push_back(ifn);
    for (size_t j = 0; j < result.size(); ++j)
        c_include_dependencies(result, result[j]);

    // we have to be sure the files actually exist
    // and we may need some heuristics to do it
    nstring_list temp;
    for (size_t j = 0; j < result.size(); ++j)
    {
        nstring ofn = result[j];
        if (is_an_include_candidate(ofn))
            temp.push_back(ofn);
    }
    temp.sort();
    return temp;
}


static inline bool
safe_isspace(int c)
{
    return isspace((unsigned char)c);
}


void
target::c_include_dependencies(nstring_list &results,
    const nstring &fn)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring path = resolve(fn);
    FILE *fp = fopen(path.c_str(), "r");
    if (!fp)
    {
        // heuristic to cope with "etc/libdir.h.in"
        // otherwise not being found
        path = resolve(fn + ".in");
        fp = fopen(path.c_str(), "r");
        if (!fp)
            return;
    }
    for (;;)
    {
        char line[2000];
        if (!fgets(line, sizeof(line), fp))
            break;
        char *cp = line;
        while (safe_isspace(*cp))
            ++cp;
        if (*cp == '#')
            c_directive(cp, results);
    }
    fclose(fp);
}


void
target::c_directive(const char *s, nstring_list &results)
{
    trace(("%s\n", __PRETTY_FUNCTION__));

    //
    // see if it is a #include directive
    //
    assert(*s == '#');
    ++s;
    while (safe_isspace(*s))
        ++s;
    if (memcmp(s, "include", 7))
        return;
    s += 7;
    while (safe_isspace(*s))
        ++s;

    //
    // figure which type
    // ignore broken directives
    //
    int right = 0;
    switch (*s++)
    {
    default:
        return;

    case '"':
        right = '"';
        break;

    case '<':
        right = '>';
        break;
    }

    //
    // find the end of the filename
    // ignore broken directives
    // ignore anything on the end of the line
    //
    const char *filename = s;
    while (*s != right)
    {
        if (!*s || *s == '\n')
            return;
        ++s;
    }

    //
    // extract the path
    // ignore broken directives
    //
    if (s == filename)
        return;
    nstring path(filename, s - filename);

    //
    // remember this path for later scanning
    //
    results.push_back_unique(path);
}


void
target::roff_include_dependencies(nstring_list &results,
    const nstring &fn)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring path = resolve(fn);
    FILE *fp = fopen(path.c_str(), "r");
    if (!fp)
    {
        // heuristic to cope with "etc/libdir.so.in"
        // otherwise not being found
        path = resolve(fn + ".in");
        fp = fopen(path.c_str(), "r");
        if (!fp)
            return;
    }
    for (;;)
    {
        char line[2000];
        if (!fgets(line, sizeof(line), fp))
            break;
        nstring_list words;
        words.split(nstring(line));
        if (words.size() >= 2 && words[0] == ".so")
            results.push_back_unique(words[1]);
    }
    fclose(fp);
}


nstring_list
target::roff_include_dependencies(const nstring &fn)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring_list result;
    result.push_back(fn);
    for (size_t j = 0; j < result.size(); ++j)
        roff_include_dependencies(result, result[j]);

    // we have to be sure the files actually exist
    // and we may need some heuristics to do it
    nstring_list temp;
    for (size_t j = 0; j < result.size(); ++j)
    {
        nstring ofn = result[j];
        if (is_an_include_candidate(ofn))
            temp.push_back(ofn);
    }
    temp.sort();
    return temp;
}


void
target::emit_static_library_rules(const nstring &name)
{
    nstring the_library_directory = name;

    nstring the_library_libname = name;
    if (the_library_libname == "lib")
        the_library_libname = get_project_name();
    if (!the_library_libname.starts_with("lib"))
        the_library_libname = "lib" + the_library_libname;

    emit_library_rules(the_library_directory, the_library_libname, false);
}


void
target::emit_library_rules(const nstring &the_library_directory,
    const nstring &the_library_libname, bool is_shared)
{
    const nstring_list &objref =
        data.get_object_files_by_dir(the_library_directory);
    if (objref.empty())
        return;
    nstring src =
        the_library_directory + "/" + the_library_libname + "." + data.libext();
    nstring dst = "$(libdir)/" + the_library_libname + "." + data.libext();
    nstring dst_la = "$(libdir)/" + the_library_libname + ".la";

    // Print a make assignment containing all of the object files.
    // We sort the list for aesthetics, and also for predictable
    // results for automatic testing.
    location_comment(__FILE__, __LINE__);
    print_assignment_sorted(the_library_directory + "_obj", objref);

    if (is_shared)
    {
        nstring objs = "$(" + the_library_directory + "_obj)";
        nstring src_dir = src.dirname();
        nstring dst_dir = dst.dirname();

        //
        // Link the shared library
        //
        {
            nstring_list lhs;
            lhs.push_back(src);
            nstring_list rhs;
            rhs.push_back(objs);
            nstring_list body;
            body.push_back("rm -f $@");
            nstring command = "$(LIBTOOL) --mode=link ";
            if (data.seen_c_plus_plus())
                command += "--tag=CXX $(CXX) $(CPPFLAGS) $(CXXFLAGS)";
            else
                command += "--tag=CC $(CC) $(CPPFLAGS) $(CFLAGS)";
            command += " $(LDFLAGS) -o $@ " + objs + " $(LIBS)";

            //
            // The Debian Policy Manual doesn't like -rpath, and they
            // have some good reasons, but the package will not build
            // without it.
            //
            command += " -rpath $(libdir)";
            command += " -version-info " + data.get_version_info();
            body.push_back(command);
            location_comment(__FILE__, __LINE__);
            print_rule(lhs, rhs, body);
        }

        //
        // Install the shared library.
        //
        {
            print_comment
            (
                "\n"
                "The install of the *.la file automatically causes "
                "\"$(LIBTOOL) --mode=install\" to install the *.a and *.so* "
                "files as well, which is why you don't see them explicitly "
                "mentioned here.\n"
                "\n"
            );
            nstring_list lhs;
            lhs.push_back(dst);
            nstring_list rhs;
            rhs.push_back(src);
            rhs.push_back(make_pseudo_dir_for(dst));
            nstring_list body;
            assert(data.need_install_data_macro());
            body.push_back
            (
                "$(LIBTOOL) --mode=install $(INSTALL_DATA) " + src + " $@"
            );
            location_comment(__FILE__, __LINE__);
            print_rule(lhs, rhs, body);
        }
    }
    else
    {
        nstring objs = "$(" + the_library_directory + "_obj)";

        //
        // Link the noinst library
        //
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back(src);
        nstring_list rhs;
        rhs.push_back(objs);
        nstring_list body;
        body.push_back("rm -f $@");
        body.push_back("$(AR) qc $@ " + objs);
        body.push_back("$(RANLIB) $@");
        print_rule(lhs, rhs, body);
    }
}


void
target::process_item_aegis_fmtgen(const nstring &fn)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring stem = fn.trim_extension();
    nstring oc = stem + ".fmtgen.cc";
    nstring oh = stem + ".fmtgen.h";
    processing.run_process(oc);
    processing.run_process(oh);
}


void
target::process_item_aegis_lib_doc(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_aegis_lib_else(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_aegis_lib_icon2(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_aegis_lib_junk(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_aegis_lib_sh(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_aegis_test_base64(const nstring &fn)
{
    // The default implementation resubmits the derived file,
    // without printing anything
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring fn2 = fn.trim_extension();
    processing.run_process(fn2);
}


void
target::process_item_configure_ac_in(const nstring &fn)
{
    // The default implementation resubmits the derived file,
    // without printing anything
    nstring fn2 = nstring(fn.c_str(), fn.size() - 3);

    // now process the result of the rule.
    processing.run_process(fn2);
}


void
target::process_item_autoconf(const nstring &fn)
{
    // The default implementation resubmits the derived file,
    // without printing anything
    nstring fn2 = nstring(fn.c_str(), fn.size() - 3);

    // now process the result of the rule.
    // it could be just about anything.
    processing.run_process(fn2);
}


void
target::process_item_c(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_cxx(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_datadir(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_datarootdir(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_etc_man(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_etc_msgfmt_sh(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_etc_profile(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_extra_dist(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_flexible(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_i18n(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_include(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_lex(const nstring &fn)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    // The default implementation resubmits the derived file,
    // but prints no output.
    nstring stem = fn.trim_extension();
    nstring oc = stem + ".lex.c";
    if (data.seen_c_plus_plus())
        oc += "c";
    processing.run_process(oc);
}


void
target::process_item_libdir(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_library(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_man_page(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_pkgconfig(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_scripts(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_test_sh(const nstring &)
{
    // The default implementation does nothing.
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target::process_item_uudecode(const nstring &fn)
{
    // The default implementation resubmits the derived file,
    // without printing anything
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring fn2 = fn.trim_extension();
    processing.run_process(fn2);
}


void
target::process_item_yacc(const nstring &fn)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    // The default implementation resubmits derived files,
    // but prints no output.
    nstring stem = fn.trim_extension();

    nstring oc = stem + ".yacc.c";
    if (data.seen_c_plus_plus())
        oc += "c";
    nstring oh = stem + ".yacc.h";

    processing.run_process(oc);
    processing.run_process(oh);
}


bool
target::file_is_in_manifest(const nstring &filename)
    const
{
    return manifest.member(filename);
}


// vim: set ts=8 sw=4 et :
