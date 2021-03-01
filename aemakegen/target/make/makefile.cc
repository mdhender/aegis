//
// aegis - project change supervisor
// Copyright (C) 2006-2012 Peter Miller
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
#include <common/ac/string.h>

#include <common/error.h>
#include <common/progname.h>
#include <common/trace.h>
#include <libaegis/output/filter/wrap_make.h>

#include <aemakegen/debug.h>
#include <aemakegen/target/make/makefile.h>
#include <aemakegen/util.h>


target_make_makefile::~target_make_makefile()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


target_make_makefile::target_make_makefile(change_identifier &a_cid) :
    target_make(a_cid)
{
    trace(("%s {\n", __PRETTY_FUNCTION__));
    data.remember_dist_clean_file("Makefile");
    data.remember_dist_clean_file("config.cache");
    data.remember_dist_clean_file("config.log");
    data.remember_dist_clean_file("config.status");
    data.remember_dist_clean_dir("bin");
    trace(("}\n"));
}


target::pointer
target_make_makefile::create(change_identifier &a_cid)
{
    return pointer(new target_make_makefile(a_cid));
}


bool
target_make_makefile::need_ar(void)
    const
{
    if (data.use_libtool())
        return false;
    nstring_list dirs = data.get_list_of_library_directories();
    return !dirs.empty();
}


void
target_make_makefile::process3_begin(void)
{
    trace(("%s {\n", __PRETTY_FUNCTION__));
    op = output_filter_wrap_make::create(op);
    warning_this_file_is_generated();
    print_comment
    (
        "\n"
        "@configure_input@\n"
        "\n"
        "The configure script generates 2 files:\n"
        "1. This Makefile\n"
        "2. " + data.get_library_directory() + "/config.h\n"
        "If you change this Makefile, you may also need to change these files. "
        "To see what is configured by the configure script, search for @ in "
        "the Makefile.in file.\n"
        "\n"
        "If you wish to reconfigure the installations directories it is "
        "RECOMMENDED that you re-run the configure script.\n"
        "\n"
        "Use \"./configure --help\" for a list of options.\n"
        "\n"
    );

    print_comment("\ndirectory containing the source\n\n");
    location_comment(__FILE__, __LINE__);
    print_assignment("srcdir", "@srcdir@");
    print_assignment("VPATH", "@srcdir@");

    print_comment("\nthe name of the install program to use\n\n");
    location_comment(__FILE__, __LINE__);
    print_assignment("INSTALL", "@INSTALL@");
    print_assignment("INSTALL_PROGRAM", "@INSTALL_PROGRAM@");
    if (data.need_install_data_macro())
        print_assignment("INSTALL_DATA", "@INSTALL_DATA@");
    print_assignment("INSTALL_DIR", "@INSTALL@ -m 0755 -d");
    if (data.need_install_script_macro())
        print_assignment("INSTALL_SCRIPT", "@INSTALL_SCRIPT@");

    if (data.seen_c_plus_plus())
    {
        print_comment("\nThe name of the C++ compiler to use.\n\n");
        location_comment(__FILE__, __LINE__);
        print_assignment("CXX", "@CXX@");
    }
    if (data.seen_c())
    {
        print_comment("\nThe name of the C compiler to use.\n\n");
        location_comment(__FILE__, __LINE__);
        print_assignment("CC", "@CC@");
    }

    if (data.seen_c_plus_plus())
    {
        print_comment("\nThe C++ compiler flags to use.\n\n");

        nstring name = "CXXFLAGS";
        nstring_list value;
        value.push_back("@CXXFLAGS@");
        if (data.use_x11())
            value.push_back("@X_CFLAGS@");
        location_comment(__FILE__, __LINE__);
        print_assignment(name, value);
    }
    if (data.seen_c())
    {
        print_comment("\nThe C compiler flags to use.\n\n");

        nstring name = "CFLAGS";
        nstring_list value;
        value.push_back("@CFLAGS@");
        if (data.use_x11())
            value.push_back("@X_CFLAGS@");
        location_comment(__FILE__, __LINE__);
        print_assignment(name, value);
    }

    if (data.seen_c_plus_plus())
    {
        if (data.seen_c())
            print_comment("\nThe C and C++ preprocessor flags to use.\n\n");
        else
            print_comment("\nThe C++ preprocessor flags to use.\n\n");
        location_comment(__FILE__, __LINE__);
        print_assignment("CPPFLAGS", "@CPPFLAGS@");
    }
    else if (data.seen_c())
    {
        print_comment("\nThe C preprocessor flags to use.\n\n");
        location_comment(__FILE__, __LINE__);
        print_assignment("CPPFLAGS", "@CPPFLAGS@");
    }

    if (data.seen_c() || data.seen_c_plus_plus() || data.use_x11())
    {
        print_comment("\nThe linker flags to use\n\n");
        location_comment(__FILE__, __LINE__);
        print_assignment("LDFLAGS", "@LDFLAGS@");
    }

    print_comment("\nprefix for installation path\n\n");
    location_comment(__FILE__, __LINE__);
    print_assignment("prefix", "@prefix@");
    print_assignment("exec_prefix", "@exec_prefix@");

    if (data.get_install_bin().size())
    {
        print_comment
        (
            "\n"
            "Where to put the executables.\n"
            "\n"
            "On a network, this would only be shared between machines of "
            "identical cpu-hw-os flavour.  It can be read-only.\n"
            "\n"
            "The $(DESTDIR) is for packaging.\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("bindir", "$(DESTDIR)@bindir@");
    }

    if (data.seen_install_mandir())
    {
        print_comment
        (
            "\n"
            "Where to put the manuals.\n"
            "\n"
            "On a network, this would be shared between all machines on the "
            "network.  It can be read-only.\n"
            "\n"
            "The $(DESTDIR) is for packaging.\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("mandir", "$(DESTDIR)@mandir@");
    }

    if (data.seen_datadir())
    {
        print_comment
        (
            "\n"
            "Where to put the non-executable package data.\n"
            "\n"
            "On a network, this would be shared between all machines on "
            "the network.  It can be read-only.\n"
            "\n"
            "The $(DESTDIR) is for packaging.\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("datadir", "$(DESTDIR)@datadir@");
    }

    if (data.seen_datarootdir())
    {
        nstring text =
            "\n"
            "Where to put the non-executable data for consumption by other "
            "packages, usually to inform other packages of this package's "
            "existence."
            ;
#if 0
        if (data.uses_pkgconfig())
            text += "  See also the $(pkgconfigdir) defintion, below.";
#endif
        text +=
            "\n\n"
            "On a network, this would be shared between all machines on the "
            "network.  It can be read-only.\n"
            "\n"
            "The $(DESTDIR) is for packaging.\n"
            "\n"
            ;
        print_comment(text);
    }
    else
    {
        //
        // In order to stop ./configure complaining about
        //
        //     config.status: WARNING: Makefile.in seems to ignore the
        //     --datarootdir setting
        //
        // It is necessary to define this *always*, not just when you
        // use the @datadir@, to get the message to go away.  Sheesh.
        //
        print_comment("define this to silence ./configure warning\n");
    }
    location_comment(__FILE__, __LINE__);
    print_assignment("datarootdir", "$(DESTDIR)@datarootdir@");

    if (data.seen_libdir() || data.uses_pkgconfig())
    {
        print_comment
        (
            "\n"
            "Where to put the supplementary package executables.\n"
            "\n"
            "On a network, this would be shared between all machines of "
            "identical architecture.  It can be read-only.\n"
            "\n"
            "The $(DESTDIR) is for packaging.\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("libdir", "$(DESTDIR)@libdir@");
    }
#if 0
    if (data.uses_pkgconfig())
    {
        print_comment("\nWhere to put the pkg-config(1) data files.\n\n");
        location_comment(__FILE__, __LINE__);
        print_assignment("pkgconfigdir", "$(libdir)/pkgconfig");
    }
#endif

    if (data.use_i18n())
    {
        print_comment
        (
            "\n"
            "${NLSDIR} is for locale files\n"
            "\n"
            "This is where the binary error message translation files (.mo) "
            "are to be installed.\n"
            "\n"
            "The $(DESTDIR) is for packaging.\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("NLSDIR", "$(DESTDIR)@NLSDIR@");

        print_comment
        (
            "\n"
            "The program to be used to translate source error "
            "message translation .po files into binary error message "
            "translation .mo files.  It is usually called \"msgfmt\" or "
            "\"gmsgfmt\".\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("MSGFMT", "@MSGFMT@");

        print_comment
        (
            "\n"
            "The program to be used to join, combine and merge source "
            "error message translation .po files.  It is usually called "
            "\"msgcat\" or \"gmsgcat\".\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("MSGCAT", "@MSGCAT@");
    }

    if (data.seen_install_include())
    {
        print_comment
        (
            "\n"
            "Where to put the include files for the library.\n"
            "\n"
            "On a network, this would be shared between all machines of "
            "identical architecture.  It can be read-only.\n"
            "\n"
            "The $(DESTDIR) is for packaging.\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("includedir", "$(DESTDIR)@includedir@");
    }

    if (data.seen_sysconfdir())
    {
        print_comment
        (
            "\n"
            "Where to put system configuration files.\n"
            "\n"
            "On a network, this would be unique to each machine.  "
            "It can be read-only.\n"
            "\n"
            "The $(DESTDIR) is for packaging.\n"
            "\n"
        );
        nstring name = "sysconfdir";
        nstring_list value;
        value.push_back("$(DESTDIR)@sysconfdir@");
        location_comment(__FILE__, __LINE__);
        print_assignment(name, value);
    }

    if (data.need_yacc())
    {
        print_comment("\nWhich yacc to use.\n\n");
        location_comment(__FILE__, __LINE__);
        print_assignment("YACC", "@YACC@");

        print_comment("\nThe yacc flags to use.\n\n");
        location_comment(__FILE__, __LINE__);
        print_empty_assignment("YFLAGS");
    }
    if (data.need_lex())
    {
        print_comment("\nWhich lex(1) to use.\n\n");
        location_comment(__FILE__, __LINE__);
        print_assignment("LEX", "@LEX@");

        print_comment("\nThe lex flags to use.\n\n");
        location_comment(__FILE__, __LINE__);
        print_empty_assignment("LEXFLAGS");
    }

    if (data.use_libtool())
    {
        print_comment("\nThe name of the GNU Libtool command.\n\n");
        location_comment(__FILE__, __LINE__);
        print_assignment("LIBTOOL", "@LIBTOOL@");
    }

    if (data.seen_c() || data.seen_c_plus_plus() || data.use_x11())
    {
        print_comment("\nextra libraries required for your system\n\n");

        nstring name = "LIBS";
        nstring_list value;
        value.push_back("@LIBS@");
        if (data.use_x11())
        {
            value.push_front("@X_PRE_LIBS@");
            value.push_back("@X_LIBS@");
            value.push_back("@X_EXTRA_LIBS@");
            value.push_back("-lX11");
        }
        location_comment(__FILE__, __LINE__);
        print_assignment(name, value);
    }

    print_comment("\nshell to use to run tests and commands\n\n");
    location_comment(__FILE__, __LINE__);
    print_assignment("SH", "@SH@");

    if (need_ar())
    {
        if (!data.have_ar())
        {
            fatal_raw
            (
                "The configure.ac file fails to invoke "
                "\"AC_CHECK_PROGS(AR, ar)\" and yet the project source "
                "files would appear to require it."
            );
        }
        print_comment
        (
            "\n"
            "The program to use to construct static libraries.\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("AR", "@AR@");

        if (!data.have_ranlib())
        {
            fatal_raw
            (
                "The configure.ac file fails to invoke \"AC_PROG_RANLIB\" and "
                "yet the project source files would appear to require it."
            );
        }
        print_comment
        (
            "\nThe program to use to place symbol indexes within static "
            "libraries.\n\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("RANLIB", "@RANLIB@");
    }

    if (!data.use_libtool() && data.objext() != "o")
    {
        print_comment
        (
            "\n"
            "object file name extension (typically \"o\" or \"obj\")\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("OBJEXT", "@OBJEXT@");
    }
    if (!data.use_libtool() && data.libext() != "a")
    {
        print_comment
        (
            "\n"
            "library file name extension (typically \"a\" or \"lib\")\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("LIBEXT", "@LIBEXT@");
    }
    if (!data.exeext().empty())
    {
        print_comment
        (
            "\n"
            "command file name extension (typically \"\" or \".exe\")\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("EXEEXT", "@EXEEXT@");
    }
    if (data.need_groff())
    {
        if (!data.have_groff())
        {
            fatal_raw
            (
                "The configure.ac file fails to invoke "
                "\"AC_CHECK_PROGS(GROFF, groff roff)\" and yet the "
                "project source files would appear to require it."
            );
        }
        print_comment
        (
            "\n"
            "Set GROFF to the name of the roff command on your "
            "system, usually \"groff\" or \"troff\" or \"nroff\".\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("GROFF", "@GROFF@");
    }
    if (data.need_soelim())
    {
        if (!data.have_soelim())
        {
            fatal_raw
            (
                "The configure.ac file fails to invoke "
                "\"AC_CHECK_PROGS(SOELIM, gsoelim soelim)\" and yet the "
                "project source files would appear to require it."
            );
        }
        print_comment
        (
            "\n"
            "Set SOELIM to the name of the roff proprocessor command "
            "on your system, usually \"soelim\" or \"gsoelim\".\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("SOELIM", "@SOELIM@");
    }
    if (data.seen_groff_macro())
    {
        {
            nstring lhs = "MM";
            nstring_list rhs;
            rhs.push_back("@GROFF_MM_MACROS@");
            location_comment(__FILE__, __LINE__);
            print_assignment(lhs, rhs);
        }
        {
            nstring lhs = "MS";
            nstring_list rhs;
            rhs.push_back("@GROFF_MS_MACROS@");
            location_comment(__FILE__, __LINE__);
            print_assignment(lhs, rhs);
        }
    }

    if (!data.get_program_prefix().empty())
    {
        print_comment
        (
            "\n"
            "Prepend this string to the beginning of installed program names.  "
            "This is set by the ./configure --program-prefix option.\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("PROGRAM_PREFIX", "@PROGRAM_PREFIX@");
    }
    if (!data.get_program_suffix().empty())
    {
        print_comment
        (
            "\n"
            "Append this string to the end of installed program names.  "
            "This is set by the ./configure --program-suffix option.\n"
            "\n"
        );
        location_comment(__FILE__, __LINE__);
        print_assignment("PROGRAM_SUFFIX", "@PROGRAM_SUFFIX@");
    }

    print_comment
    (
        "---------------------------------------------------------\n"
        "You should not need to change anything below this line.\n"
    );

    print_comment("\nThe default target\n\n");
    location_comment(__FILE__, __LINE__);
    print_rule("the-default-target", "all");

    trace(("}\n"));
}


void
target_make_makefile::process_item_aegis_lib_doc(const nstring &fn)
{
    nstring macros = fn.get_extension();
    if (macros == "roff")
        macros = "";
    else if (macros == "mm")
        macros = "-m$(MM)";
    else if (macros == "ms")
        macros = "-m$(MS)";
    else
    {
        if (!macros.starts_with("m"))
            macros = "m" + macros;
        macros = "-" + macros;
    }

    nstring src = fn.dirname() + ".pdf";

    nstring_list parts;
    parts.split(fn, "/");
    nstring dst = "$(datarootdir)/doc/" + get_project_name() + "/";
    if (parts[1] != "en")
        dst += parts[1] + "/";
    dst += parts[2] + ".pdf";

    {
        nstring ps = fn.dirname() + ".ps";

        nstring_list lhs;
        lhs.push_back(src);
        nstring_list rhs;
        rhs.push_back(fn);
        rhs.push_back_unique(roff_include_dependencies(fn));
        nstring_list body;
        body.push_back
        (
            "$(GROFF) -Tps -I. -R -t -p " + macros + " -mpic -mpspic " + fn +
            " > " + ps
        );
        body.push_back("ps2pdf " + ps + " $@");
        body.push_back("rm " + ps);
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

    {
        nstring_list lhs;
        lhs.push_back(dst);
        nstring_list rhs;
        rhs.push_back(src);
        rhs.push_back(make_pseudo_dir_for(dst));
        nstring_list body;
        assert(data.need_install_data_macro());
        body.push_back("$(INSTALL_DATA) " + src + " $@");
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }
}


void
target_make_makefile::process_item_aegis_lib_else(const nstring &fn)
{
    nstring stem = fn.trim_first_directory();
    nstring dst = "$(datadir)/" + get_project_name() + "/" + stem;

    location_comment(__FILE__, __LINE__);
    nstring_list lhs;
    lhs.push_back(dst);
    nstring_list rhs;
    rhs.push_back(fn);
    rhs.push_back(make_pseudo_dir_for(dst));
    nstring_list body;
    assert(data.need_install_data_macro());
    body.push_back("$(INSTALL_DATA) " + fn + " $@");
    print_rule(lhs, rhs, body);
}


void
target_make_makefile::process_item_aegis_lib_icon2(const nstring &fn)
{
    nstring rest = fn.trim_first_directory();

    nstring dst = "$(datadir)/" + get_project_name() + "/" + rest;

    nstring_list lhs;
    lhs.push_back(dst);
    nstring_list rhs;
    rhs.push_back(fn);
    rhs.push_back(make_pseudo_dir_for(dst));
    nstring_list body;
    assert(data.need_install_data_macro());
    body.push_back("$(INSTALL_DATA) " + fn + " $@");
    location_comment(__FILE__, __LINE__);
    print_rule(lhs, rhs, body);
}


void
target_make_makefile::process_item_aegis_lib_sh(const nstring &fn)
{
    nstring rest(fn.trim_first_directory());
    nstring dir(fn.dirname());
    nstring dst ="$(datadir)/" + get_project_name() + "/" + rest;

    nstring_list lhs;
    lhs.push_back(dst);
    nstring_list rhs;
    rhs.push_back(fn);
    rhs.push_back(make_pseudo_dir_for(dst));
    nstring_list body;
    body.push_back("$(INSTALL_SCRIPT) " + fn + " $@");
    location_comment(__FILE__, __LINE__);
    print_rule(lhs, rhs, body);
}


void
target_make_makefile::process_item_aegis_test_base64(const nstring &fn)
{
    nstring fn2 = fn.trim_extension();

    //
    // This differs from the #process_item_uudecode method, because
    // it uses the code Aegis already has, rather than adding another
    // package build dependency.
    //
    {
        nstring_list lhs;
        lhs.push_back(fn2);
        nstring_list rhs;
        rhs.push_back(fn);
        rhs.push_back("bin/test_base64");
        nstring_list body;
        body.push_back("bin/test_base64 -uu -i -nh " + fn + " $@");
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

    // also process the output of the rule
    processing.run_process(fn2);
}


void
target_make_makefile::process_item_c(const nstring &fn)
{
    trace(("%s {\n", __PRETTY_FUNCTION__));
    trace(("fn = %s\n", fn.quote_c().c_str()));
    nstring stem = fn.trim_extension();
    nstring dot_o_file = stem + "." + data.objext();

    nstring base = stem.basename();

    nstring_list dep = c_include_dependencies(fn);

    nstring_list lhs;
    lhs.push_back(dot_o_file);
    nstring_list rhs;
    rhs.push_back(fn);
    rhs.push_back_unique(dep);
    nstring_list body;
    if (data.use_libtool())
    {
        nstring dot_lo_file = stem + ".lo";
        lhs.push_back(dot_lo_file);

        nstring command =
            "$(LIBTOOL) --mode=compile --tag=CC "
            "$(CC) $(CPPFLAGS) $(CFLAGS) -I. "
            "-c " + fn + " "
            "-o " + dot_lo_file
            ;
        body.push_back(command);
    }
    else
    {
        body.push_back("$(CC) $(CPPFLAGS) $(CFLAGS) -I. -c " + fn);
        if (base != stem)
        {
            nstring command =
                "mv " + base + "." + data.objext() + " $@";
            body.push_back(command);
        }
    }
    location_comment(__FILE__, __LINE__);
    print_rule(lhs, rhs, body);
    trace(("}\n"));
}


void
target_make_makefile::process_item_cxx(const nstring &fn)
{
    nstring stem = fn.trim_extension();
    nstring dot_o_file = stem + "." + data.objext();

    nstring_list dep = c_include_dependencies(fn);

    nstring_list lhs;
    lhs.push_back(dot_o_file);
    nstring_list rhs;
    rhs.push_back(fn);
    rhs.push_back_unique(dep);
    nstring_list body;
    if (data.use_libtool())
    {
        nstring dot_lo_file = stem + ".lo";
        lhs.push_back(dot_lo_file);

        nstring command =
            "$(LIBTOOL) --mode=compile --tag=CXX "
            "$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I. "
            "-c " + fn + " "
            "-o " + dot_lo_file;
        body.push_back(command);
    }
    else
    {
        body.push_back("$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I. -c " + fn);
        nstring base = stem.basename();
        if (base != stem)
        {
            nstring command = "mv " + base + "." + data.objext() + " $@";
            body.push_back(command);
        }
    }
    location_comment(__FILE__, __LINE__);
    print_rule(lhs, rhs, body);
}


void
target_make_makefile::process_item_datadir(const nstring &fn)
{
    nstring src = fn;
    nstring stem = src.substr(8, src.size() - 8);
    nstring dst = "$(datadir)/" + get_project_name() + "/" + stem;

    nstring_list lhs;
    lhs.push_back(dst);
    nstring_list rhs;
    rhs.push_back(src);
    rhs.push_back(make_pseudo_dir_for(dst));
    nstring_list body;
    assert(data.need_install_data_macro());
    body.push_back("$(INSTALL_DATA) " + src + " $@");
    location_comment(__FILE__, __LINE__);
    print_rule(lhs, rhs, body);
}


void
target_make_makefile::process_item_datarootdir(const nstring &fn)
{
    nstring src = fn;
    nstring stem = src.trim_first_directory();
    nstring dst = "$(datarootdir)/" + stem;

    nstring_list lhs;
    lhs.push_back(dst);
    nstring_list rhs;
    rhs.push_back(src);
    rhs.push_back(make_pseudo_dir_for(dst));
    nstring_list body;
    assert(data.need_install_data_macro());
    body.push_back("$(INSTALL_DATA) " + src + " $@");
    location_comment(__FILE__, __LINE__);
    print_rule(lhs, rhs, body);
}


void
target_make_makefile::process_item_etc_man(const nstring &fn)
{
    nstring ps = fn.trim_extension() + ".ps";
    nstring pdf = fn.trim_extension() + ".pdf";

    {
        nstring_list dep = roff_include_dependencies(fn);

        nstring_list lhs;
        lhs.push_back(pdf);
        nstring_list rhs;
        rhs.push_back(fn);
        rhs.push_back_unique(dep);
        nstring_list body;
        body.push_back("$(GROFF) -Tps -s -I. -t -man " + fn + " > " + ps);
        body.push_back("ps2pdf " + ps + " $@");
        body.push_back("rm " + ps);
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

#if USE_DEBIAN
    // this is what Debian wants
    nstring kind = "datarootdir";
    nstring dst =
        "$(datarootdir)/doc/" + get_project_name() + "/" +
        pdf.trim_first_directory();
#else
    // this is what I'd prefer
    nstring kind = "datadir";
    nstring dst =
        "$(datadir)/" + get_project_name() + "/" + pdf.trim_first_directory();
#endif

    nstring_list lhs;
    lhs.push_back(dst);
    nstring_list rhs;
    rhs.push_back(pdf);
    rhs.push_back(make_pseudo_dir_for(dst));
    nstring_list body;
    assert(data.need_install_data_macro());
    body.push_back("$(INSTALL_DATA) " + pdf + " $@");
    location_comment(__FILE__, __LINE__);
    print_rule(lhs, rhs, body);
}


void
target_make_makefile::process_item_etc_profile(const nstring &fn)
{
    nstring rest(fn.trim_first_directory());
    nstring dir(fn.dirname());

    {
        nstring dst = "$(datadir)/" + get_project_name() + "/" + rest;

        nstring_list lhs;
        lhs.push_back(dst);
        nstring_list rhs;
        rhs.push_back(fn);
        rhs.push_back(make_pseudo_dir_for(dst));
        nstring_list body;
        body.push_back("$(INSTALL_SCRIPT) " + fn + " $@");
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

    {
        nstring dst = "$(sysconfdir)/profile.d/" + get_project_name() + ".";
        nstring ext = fn.get_extension();
        if (ext.empty())
            dst += (strstr(fn.c_str(), "csh") ? "csh" : "sh");
        else
            dst += ext;

        nstring_list lhs;
        lhs.push_back(dst);
        nstring_list rhs;
        rhs.push_back(fn);
        rhs.push_back(make_pseudo_dir_for(dst));
        nstring_list body;
        assert(data.need_install_data_macro());
        body.push_back("$(INSTALL_SCRIPT) " + fn + " $@");
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }
}


void
target_make_makefile::process_item_i18n(const nstring &filename)
{
    if (filename.ends_with("/common.po"))
    {
        // Do nothing directly,
        // it will be used indirectly by other rules.
        return;
    }

    nstring mo = filename.trim_extension() + ".mo";

    {
        nstring_list lhs;
        lhs.push_back(mo);
        nstring_list rhs;
        rhs.push_back(filename);
        nstring_list body;
        nstring command;
        if (data.seen_etc_msgfmt_sh())
        {
            command =
                "$(SH) etc/msgfmt.sh --msgfmt=$(MSGFMT) --msgcat=$(MSGCAT) "
                "--output=$@";
            rhs.push_back("etc/msgfmt.sh");
        }
        else
            command = "$(MSGFMT) -o $@";
        command += " " + filename;
        nstring po2 = filename.dirname() + "/common.po";
        if (file_is_in_manifest(po2))
        {
            command += " " + po2;
            rhs.push_back_unique(po2);
        }
        body.push_back(command);
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

    // install the .mo files
    if (data.use_i18n())
    {
        //
        // We have to calculate what the installed destination files
        // look like.  It is very similar to the .mo file we just built,
        // but has to arrive in the correct place, using just the right
        // abount of the .mo file's path.
        //
        // something like "/usr/share/locale/ru/LC_MESSAGES/aegis.mo"
        //          NLSDIR ^^^^^^^^^^^^^^^^^
        //
        nstring_list src_parts;
        src_parts.split(mo, "/");
        while (src_parts.size() > 3)
            src_parts.pop_front();
        nstring dst = "$(NLSDIR)/" + src_parts.unsplit("/");

        nstring_list lhs;
        lhs.push_back(dst);
        nstring_list rhs;
        rhs.push_back(mo);
        rhs.push_back(make_pseudo_dir_for(dst));
        nstring_list body;
        assert(data.need_install_data_macro());
        body.push_back("$(INSTALL_DATA) " + mo + " $@");
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }
}


void
target_make_makefile::process_item_include(const nstring &fn)
{
    if (data.get_install_include_sources().member(fn))
    {
        nstring dst = "$(includedir)/" + fn;
        if (data.get_install_include_sources().size() == 1)
            dst = "$(includedir)/" + fn.trim_first_directory();
        else if (data.get_library_directory() == "lib")
        {
            dst =
                (
                    "$(includedir)/"
                +
                    get_project_name()
                +
                    "/"
                +
                    fn.trim_first_directory()
                );
        }

        nstring_list lhs;
        lhs.push_back(dst);
        nstring_list rhs;
        rhs.push_back(fn);
        rhs.push_back(make_pseudo_dir_for(dst));
        nstring_list body;
        assert(data.need_install_data_macro());
        body.push_back("$(INSTALL_DATA) " + fn + " $@");
        print_rule(lhs, rhs, body);
    }
}


void
target_make_makefile::process_item_lex(const nstring &fn)
{
    nstring stem = fn.trim_extension();

    nstring oc = stem + ".lex.c";
    if (data.seen_c_plus_plus())
        oc += "c";

    // rule to create a C or C++ file from a .l file
    // using lex(1) or gnu's flex(1)
    {
        nstring yy = fn.trim_first_directory().trim_extension().identifier();

        nstring_list lhs;
        lhs.push_back(oc);
        nstring_list rhs;
        rhs.push_back(fn);
        nstring_list body;
        body.push_back("$(LEX) $(LEXFLAGS) " + fn);
        body.push_back("sed -e 's|[yY][yY]|" + yy + "_|g' lex.yy.c > " + oc);
        body.push_back("rm lex.yy.c");
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

    processing.run_process(oc);
}


void
target_make_makefile::process_item_libdir(const nstring &fn)
{
    nstring src = fn;
    nstring stem = fn.trim_first_directory();
    nstring dst = "$(libdir)/" + stem;

    nstring_list lhs;
    lhs.push_back(dst);
    nstring_list rhs;
    rhs.push_back(src);
    rhs.push_back(make_pseudo_dir_for(dst));
    nstring_list body;
    assert(data.need_install_data_macro());
    body.push_back("$(INSTALL_DATA) " + src + " $@");
    location_comment(__FILE__, __LINE__);
    print_rule(lhs, rhs, body);
}


void
target_make_makefile::process_item_man_page(const nstring &fn)
{
    trace(("target_make_makefile::process_item_man_page(fn = %s)\n{\n",
        fn.quote_c().c_str()));
    nstring_list dep = roff_include_dependencies(fn);

    //
    // Build a local cat1 directory.  It is just for show,
    // we will not be installing anything from it.
    //
    {
        nstring dst;
        const char *p = strstr(fn.c_str(), "/man");
        if (p)
            dst = fn.substr(0, p - fn.c_str()) + "/cat" + nstring(p + 4);
        else
        {
            nstring src = extract_man_page_details(fn);
            assert(src.starts_with("man"));
            dst = "cat" + src.substr(3, src.size() - 3);
        }

        nstring_list lhs;
        lhs.push_back(dst);
        nstring_list rhs;
        rhs.push_back(fn);
        rhs.push_back_unique(dep);
        nstring_list body;
        body.push_back("@mkdir -p " + dst.dirname());
        body.push_back
        (
            "GROFF_NO_SGR=-c "
            "$(GROFF) -I. -Tascii -t -man " + fn + " > $@"
        );
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

    //
    // Do not install the man pages of programs that are not going
    // to be installed.
    //
    {
        nstring name = fn.basename();
        if
        (
            name.ends_with(".1")
        ||
            name.ends_with(".8")
        ||
            name.ends_with(".man")
        )
        {
            name = name.trim_extension();
            trace(("name = %s\n", name.quote_c().c_str()));
            if (!is_installable(name))
            {
                trace(("not going to be installed\n"));
                trace(("}\n"));
                return;
            }
        }
    }

    //
    // Build the path of the file to be installed.
    //
    nstring stem = extract_man_page_details(fn);
    trace(("stem = %s;\n", stem.quote_c().c_str()));
    nstring file2 = "$(mandir)/" + stem;
    trace(("file2 = %s;\n", file2.quote_c().c_str()));

    nstring src = fn.dirname();
    nstring dst = file2.dirname();

    nstring_list lhs;
    lhs.push_back(file2);
    nstring_list rhs;
    rhs.push_back(fn);
    rhs.push_back_unique(dep);
    rhs.push_back(make_pseudo_dir_for(file2));
    nstring_list body;
    bool clean_up_tmp = false;
    if (contains_dot_so_directive(fn))
    {
        nstring command = "$(SOELIM) -I. " + fn;
        if (contains_dot_xx_directive(fn))
        {
            // Get rid of .XX indexing lines (and hope they didn't use a
            // backslash line continuation).
            command += " | sed '/^[.]XX ./d'";
        }
        command += " > " + fn + ".tmp";
        body.push_back(command);
        assert(data.need_install_data_macro());
        body.push_back("$(INSTALL_DATA) " + fn + ".tmp $@");
        clean_up_tmp = true;
    }
    else
    {
        if (contains_dot_xx_directive(fn))
        {
            body.push_back("sed '/^[.]XX ./d' < " + fn + " > " + fn + ".tmp");
            assert(data.need_install_data_macro());
            body.push_back("$(INSTALL_DATA) " + fn + ".tmp $@");
            clean_up_tmp = true;
        }
        else
        {
            assert(data.need_install_data_macro());
            body.push_back("$(INSTALL_DATA) " + fn + " $@");
        }
    }
    if (clean_up_tmp)
        body.push_back("@rm -f " + fn + ".tmp");
    location_comment(__FILE__, __LINE__);
    print_rule(lhs, rhs, body);
    trace(("}\n"));
}


void
target_make_makefile::process_item_pkgconfig(const nstring &fn)
{
    nstring src = fn;
    nstring dst = "$(libdir)/pkgconfig/" + fn.basename();

    nstring_list lhs;
    lhs.push_back(dst);
    nstring_list rhs;
    rhs.push_back(src);
    rhs.push_back(make_pseudo_dir_for(dst));
    nstring_list body;
    assert(data.need_install_data_macro());
    body.push_back("$(INSTALL_DATA) " + src + " $@");
    location_comment(__FILE__, __LINE__);
    print_rule(lhs, rhs, body);
}


void
target_make_makefile::process_item_scripts(const nstring &fn)
{
    nstring name = fn.basename();
    nstring ext = name.get_extension();
    if (extension_implies_script(name))
        name = name.trim_extension();

    nstring bin_name = "bin/" + name + data.exeext();

    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back(bin_name);
        nstring_list rhs;
        rhs.push_back(fn);
        rhs.push_back(".bin");
        nstring_list body;
        body.push_back("cp " + fn + " $@");
        body.push_back("chmod a+rx $@");
        print_rule(lhs, rhs, body);
    }

    if (is_installable(name))
    {
        nstring install_name =
            (
                "$(bindir)/"
            +
                data.get_program_prefix()
            +
                name
            +
                data.get_program_suffix()
            +
                data.exeext()
            );

        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back(install_name);
        nstring_list rhs;
        rhs.push_back(bin_name);
        rhs.push_back(make_pseudo_dir_for(install_name));
        nstring_list body;
        body.push_back("$(INSTALL_SCRIPT) " + bin_name + " $@");
        print_rule(lhs, rhs, body);
    }
}


void
target_make_makefile::process_item_test_sh(const nstring &fn)
{
    nstring stem = fn.trim_extension();
    nstring base = stem.basename();

    if (data.seen_etc_test_sh())
    {
        // This code branch is for Aegis itself.
        // (or any project of sufficiently similar shape)
        // ES = Exit Status
        nstring es = base + ".ES";

        nstring_list lhs;
        lhs.push_back(es);
        nstring_list rhs;
        rhs.push_back(fn);
        rhs.push_back("all");
        rhs.push_back("etc/test.sh");
        nstring_list body;
        nstring cmd =
            "$(SH) etc/test.sh "
            "-shell $(SH) "
            "-run " + fn + " " +
            es;
        if (data.seen_c_plus_plus())
            cmd = "CXX=\"$(CXX)\" " + cmd;
        else if (data.seen_c())
            cmd = "CC=\"$(CC)\" " + cmd;
        body.push_back(cmd);
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }
    else
    {
        nstring_list lhs;
        lhs.push_back(base);
        nstring_list rhs;
        rhs.push_back(fn);
        rhs.push_back("all");
        nstring_list body;
        body.push_back("PATH=`pwd`/bin:$$PATH $(SH) " + fn);
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }
}


void
target_make_makefile::process_item_uudecode(const nstring &fn)
{
    nstring fn2 = fn.trim_extension();

    nstring_list lhs;
    lhs.push_back(fn2);
    nstring_list rhs;
    rhs.push_back(fn);
    nstring_list body;
    body.push_back("uudecode -o $@ " + fn);
    location_comment(__FILE__, __LINE__);
    print_rule(lhs, rhs, body);

    // now process the result of the rule.
    // it could be just about anything.
    processing.run_process(fn2);
}


void
target_make_makefile::process_item_yacc(const nstring &fn)
{
    nstring stem = fn.trim_extension();
    nstring yy = fn.trim_first_directory().trim_extension().identifier();

    nstring oc = stem + ".yacc.c";
    if (data.seen_c_plus_plus())
        oc += "c";
    nstring oh = stem + ".yacc.h";
    nstring ol = stem + ".output";

    // rules to make C or C++ file from a *.y file,
    // using yacc(1) or gnu's bison(1)
    {
        nstring_list lhs;
        lhs.push_back(oc);
        lhs.push_back(oh);
        nstring_list rhs;
        rhs.push_back(fn);
        nstring_list body;
        body.push_back("$(YACC) -d $(YFLAGS) " + fn);
        body.push_back
        (
            "sed -e 's/[yY][yY]/" + yy + "_/g' "
                "-e '/<stdio.h>/d' "
                "-e '/<stdlib.h>/d' "
                "-e '/<stddef.h>/d' "
                "y.tab.c > " + oc
        );
        nstring YY = fn.trim_extension().identifier();
        body.push_back
        (
            "sed -e 's/[yY][yY]/" + yy + "_/g' "
                "-e 's/Y_TAB_H/" + YY + "_YACC_H/g' "
                "y.tab.h > " + oh
        );
        body.push_back
        (
            "test -f y.output && mv y.output " + ol + " || true"
        );
        body.push_back("rm -f y.tab.c y.tab.h y.output || true");
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

    processing.run_process(oc);
    processing.run_process(oh);
}


void
target_make_makefile::process3_end(void)
{
    trace(("%s {\n", __PRETTY_FUNCTION__));
    nstring_list phony;
    phony.push_back("the-default-target");

    // Print the object file list for each of the libraries, and the
    // rules to build each of the libraries.
    {
        nstring_list dirs = data.get_list_of_library_directories();
        dirs.sort();
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

            // If the library is *the* library, it may be a shared library,
            // otherwise always assume it is no-inst and static.
            print_comment("\nThe " + filename + " library.\n\n");
            bool shared =
                (
                    data.use_libtool()
                &&
                    library_libname == get_library_libname()
                );
            emit_library_rules(library_dirname, library_libname, shared);
        }
    }
    bool package_library = !data.get_library_name().empty();

    // directories required to install stuff
    {
        nstring_list dirs = data.get_install_directories();
        for (size_t j = 0; j < dirs.size(); ++j)
        {
            nstring dir = dirs[j];
            nstring pseudo = make_pseudo_dir(dir);
            nstring dir_dotdot = dir.dirname();
            nstring pseudo_dotdot = make_pseudo_dir(dir_dotdot);

            nstring_list lhs;
            lhs.push_back(pseudo);
            nstring_list rhs;
            if (dir_dotdot != "." && dir_dotdot != "/")
                rhs.push_back(pseudo_dotdot);
            nstring_list body;
            body.push_back("-$(INSTALL_DIR) " + dir);
            body.push_back("@-test -d " + dir + " && touch $@");
            body.push_back("@sleep 1");
            location_comment(__FILE__, __LINE__);
            print_rule(lhs, rhs, body);
        }
    }

    // Print the rules to build each program.
    {
        nstring_list progdirs = data.get_progdirs();
        progdirs.sort();
        for (size_t j = 0; j < progdirs.size(); ++j)
        {
            nstring progdir = progdirs[j];
            nstring prog = progname_from_dir_of(progdir + "/main.c");
            print_comment("\nThe " + prog + " program.\n\n");

            const nstring_list &objref = data.get_object_files_by_dir(progdir);
            if (objref.empty())
            {
                if (debug)
                {
                    print_comment
                    (
                        "corresponding object file list for " + prog +
                        " is missing"
                    );
                }
                assert(!"corresponding object file list is missing");
                continue;
            }

            // Print a make assignment containing all of the object files.
            // We sort the list for aesthetics, and also for predictable
            // results for automatic testing.
            location_comment(__FILE__, __LINE__);
            nstring prog_var_name = prog.identifier() + "_obj";
            print_assignment_sorted(prog_var_name, objref);

            // Print rule to link the program.
            {
                nstring_list lhs;
                lhs.push_back("bin/" + prog + data.exeext());
                nstring_list rhs;
                rhs.push_back("$(" + prog_var_name + ")");
                rhs.push_back(".bin");

                //
                // There are times when you want to be more selective
                // about whether or not to use libraries.  Perhapse
                // we need to have a file attribute on main.c so the
                // user can select which libraries?  Plus, making the
                // distinction between what the shared library needs,
                // and what utility programs in the same project need.
                //
                const nstring_list &libs =
                    data.get_library_list_by_program(prog);
                if (!libs.empty())
                    rhs.push_back(libs);
                else if (package_library)
                {
                    rhs.push_back
                    (
                        data.get_library_directory() + "/" +
                        get_library_libname() + "." + data.libext()
                    );
                }

                nstring_list body;
                nstring command;
                if (data.use_libtool())
                {
                    command = "$(LIBTOOL) --mode=link --tag=";
                    command += (data.seen_c_plus_plus() ? "CXX" : "CC");
                    command += " ";
                }
                command +=
                    (
                        data.seen_c_plus_plus()
                    ?
                        "$(CXX) $(CPPFLAGS) $(CXXFLAGS)"
                    :
                        "$(CC) $(CPPFLAGS) $(CFLAGS)"
                    );
                command += " -o $@";
                command += " $(" + prog_var_name + ")";
                if (!libs.empty())
                {
                    command += " " + libs.unsplit();
                }
                else if (package_library)
                {
                    command += " " + data.get_library_directory() + "/" +
                        get_library_libname() + "." + data.libext();
                }

                command += " $(LDFLAGS) $(LIBS)";
                body.push_back(command);
                location_comment(__FILE__, __LINE__);
                print_rule(lhs, rhs, body);
            }

            if (is_installable(prog))
            {
                location_comment(__FILE__, __LINE__);
                nstring install_name =
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

                nstring_list lhs;
                lhs.push_back(install_name);
                nstring_list rhs;
                rhs.push_back("bin/" + prog + data.exeext());
                rhs.push_back(make_pseudo_dir_for(install_name));
                nstring_list body;
                nstring command;
                if (data.use_libtool())
                    command = "$(LIBTOOL) --mode=install ";
                command +=
                    "$(INSTALL_PROGRAM) bin/" + prog + data.exeext() + " $@";
                body.push_back(command);
                print_rule(lhs, rhs, body);
            }
        }
    }

    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("all");
        nstring_list rhs;
        if (!data.get_all_bin().empty())
            rhs.push_back("all-bin");
        if (data.seen_all_doc())
            rhs.push_back("all-doc");
        if (data.seen_all_i18n())
            rhs.push_back("all-i18n");
        nstring_list body;
        print_rule(lhs, rhs, body);
        phony.push_back("all");
    }

    if (!data.get_all_bin().empty())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("all-bin");
        nstring_list rhs = data.get_all_bin();
        rhs.sort();
        nstring_list body;
        print_rule(lhs, rhs, body);
        phony.push_back("all-bin");
    }
    if (data.seen_all_doc())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("all-doc");
        nstring_list rhs = data.get_all_doc();
        rhs.sort();
        nstring_list body;
        print_rule(lhs, rhs, body);
        phony.push_back("all-doc");
    }
    if (data.seen_all_i18n())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("all-i18n");
        nstring_list rhs = data.get_all_i18n();
        rhs.sort();
        nstring_list body;
        print_rule(lhs, rhs, body);
        phony.push_back("all-i18n");
    }

    if (!data.get_all_bin().empty())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back(".bin");
        nstring_list rhs;
        nstring_list body;
        body.push_back("-mkdir bin");
        body.push_back("-chmod 0755 bin");
        body.push_back("@-test -d bin && touch $@");
        body.push_back("@sleep 1");
        print_rule(lhs, rhs, body);
    }

    // ---------- test summary  --------------------------------------------
    //
    // "make check" is what automake generates.
    // not nearly as cute as "make sure"
    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("check");
        nstring_list rhs;
        rhs.push_back("sure");
        nstring_list body;
        print_rule(lhs, rhs, body);
        phony.push_back("check");
    }

    print_assignment_sorted("test_files", data.get_test_files());

    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("sure");
        nstring_list rhs;
        if (!data.get_test_files().empty())
        {
            rhs.push_back("$(test_files)");
            if (data.seen_etc_test_sh())
                rhs.push_back("etc/test.sh");
        }
        nstring_list body;
        if (data.seen_etc_test_sh())
            body.push_back("@$(SH) etc/test.sh -summary $(test_files)");
        else
            body.push_back("@echo Passed All Tests");
        print_rule(lhs, rhs, body);
        phony.push_back("sure");

        if (!data.seen_etc_test_sh())
            phony.push_back(data.get_test_files());
    }

    // ---------- clean  ---------------------------------------------------

    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("clean");
        nstring_list rhs;
        if (!data.get_all_bin().empty())
            rhs.push_back("clean-bin");
        if (data.seen_all_doc())
            rhs.push_back("clean-doc");
        if (data.seen_all_i18n())
            rhs.push_back("clean-i18n");
        if (!data.get_clean_misc_files().empty())
            rhs.push_back("clean-misc");
        if (!data.get_clean_obj_files().empty())
            rhs.push_back("clean-obj");
        nstring_list body;
        print_rule(lhs, rhs, body);
        phony.push_back("clean");
    }
    if (!data.get_all_bin().empty())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list rhs;
        nstring_list files = data.get_all_bin();
        files.sort();
        print_clean_file_rule("clean-bin", rhs, files);
        phony.push_back("clean-bin");
    }
    if (data.seen_all_doc())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list rhs;
        nstring_list files = data.get_all_doc();
        files.sort();
        print_clean_file_rule("clean-doc", rhs, files);
        phony.push_back("clean-doc");
    }
    if (data.seen_all_i18n())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list rhs;
        nstring_list files = data.get_all_i18n();
        files.sort();
        print_clean_file_rule("clean-i18n", rhs, files);
        phony.push_back("clean-i18n");
    }
    if (!data.get_clean_misc_files().empty())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list rhs;
        nstring_list files = data.get_clean_misc_files();
        files.sort();
        print_clean_file_rule("clean-misc", rhs, files);
    }
    if (!data.get_clean_obj_files().empty())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list rhs;
        nstring_list files = data.get_clean_obj_files();
        files.sort();
        print_clean_file_rule("clean-obj", rhs, files);
    }

    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("distclean");
        nstring_list rhs;
        rhs.push_back("clean");
        if (!data.get_dist_clean_files().empty())
            rhs.push_back("distclean-files");
        if (!data.get_dist_clean_dirs().empty())
            rhs.push_back("distclean-directories");
        nstring_list body;
        print_rule(lhs, rhs, body);
        phony.push_back("distclean");
    }
    if (!data.get_dist_clean_files().empty())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list rhs;
        nstring_list files = data.get_dist_clean_files();
        files.sort();
        print_clean_file_rule("distclean-files", rhs, files);
        phony.push_back("distclean-files");
    }
    if (!data.get_dist_clean_dirs().empty())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list rhs;
        nstring_list cdirs = data.get_dist_clean_dirs();
        cdirs.sort();
        print_clean_dirs_rule("distclean-directories", rhs, cdirs);
        phony.push_back("distclean-directories");
    }

    // ----------  install  ------------------------------------------------

    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("install");
        nstring_list rhs;
        if (!data.get_install_bin().empty())
            rhs.push_back("install-bin");
        if (!data.get_install_datadir().empty())
            rhs.push_back("install-datadir");
        if (data.seen_install_doc())
            rhs.push_back("install-doc");
        if (data.seen_install_i18n())
            rhs.push_back("install-i18n");
        if (data.seen_install_include())
            rhs.push_back("install-include");
        if (!data.get_install_libdir().empty())
            rhs.push_back("install-libdir");
        if (data.seen_install_mandir())
            rhs.push_back("install-man");
        nstring_list body;
        print_rule(lhs, rhs, body);
        phony.push_back("install");
    }
    if (!data.get_install_bin().empty())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("install-bin");
        nstring_list rhs = data.get_install_bin();
        nstring_list body;
        print_rule(lhs, rhs, body);
        phony.push_back("install-bin");
    }
    if (!data.get_install_datadir().empty())
    {
        // this includes datarootdir files
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("install-datadir");
        nstring_list rhs = data.get_install_datadir();
        nstring_list body;
        print_rule(lhs, rhs, body);
        phony.push_back("install-datadir");
    }
    if (data.seen_install_doc())
    {
        nstring_list lhs;
        lhs.push_back("install-doc");
        nstring_list rhs = data.get_install_doc();
        nstring_list body;
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }
    if (data.seen_install_i18n())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("install-i18n");
        nstring_list rhs = data.get_install_i18n();
        nstring_list body;
        print_rule(lhs, rhs, body);
        phony.push_back("install-i18n");
    }
    if (data.seen_install_include())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("install-include");
        nstring_list rhs = data.get_install_include();
        nstring_list body;
        print_rule(lhs, rhs, body);
        phony.push_back("install-include");
    }
    if (!data.get_install_libdir().empty())
    {
        if (data.use_libtool())
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
        }
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("install-libdir");
        nstring_list rhs = data.get_install_libdir();
        nstring_list body;
        print_rule(lhs, rhs, body);
        phony.push_back("install-libdir");
    }
    if (data.seen_install_mandir())
    {
        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back("install-man");
        nstring_list rhs = data.get_install_mandir();
        nstring_list body;
        print_rule(lhs, rhs, body);
        phony.push_back("install-mandir");
    }

    // ---------- uninstall  -----------------------------------------------
    //
    // Uninstall all the files the "install" target installed.
    //
    {
        location_comment(__FILE__, __LINE__);
        nstring lhs = "uninstall";
        nstring_list rhs;
        nstring_list ufiles;
        ufiles.push_back(data.get_install_bin());
        ufiles.push_back(data.get_install_datadir());
        ufiles.push_back(data.get_install_doc());
        ufiles.push_back(data.get_install_i18n());
        ufiles.push_back(data.get_install_include());
        ufiles.push_back(data.get_install_libdir());
        ufiles.push_back(data.get_install_mandir());
        for (size_t j = 0; j < data.get_install_libdir().size(); ++j)
        {
            nstring fn = data.get_install_libdir()[j];
            if (fn.ends_with(".la"))
            {
                fn = fn.trim_extension();
                ufiles.push_back(fn + ".a");
                ufiles.push_back(fn + ".so*");
            }
        }
        print_clean_file_rule(lhs, rhs, ufiles);
    }

    {
        nstring_list lhs;
        lhs.push_back(".PHONY");
        nstring_list body;
        print_rule(lhs, phony, body);
    }

    print_comment("vim: set ts=8 sw=8 noet :");
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
