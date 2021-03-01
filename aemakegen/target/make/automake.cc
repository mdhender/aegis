//
// aegis - project change supervisor
// Copyright (C) 2007-2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
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
#include <common/ac/string.h>
#include <libaegis/output/filter/wrap_make.h>

#include <common/trace.h>

#include <aemakegen/target/make/automake.h>
#include <aemakegen/util.h>


target_make_automake::~target_make_automake()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


target_make_automake::target_make_automake(change_identifier &a_cid) :
    target_make(a_cid)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    op = output_filter_wrap_make::create(op);
}


target::pointer
target_make_automake::create(change_identifier &a_cid)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new target_make_automake(a_cid));
}


void
target_make_automake::process3_begin(void)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    warning_this_file_is_generated();

    print_comment
    (
        "\n"
        "Tell automake to put the object file for foo/bar.cc in "
        "directory foo/\n"
        "\n"
    );
    print_assignment("AUTOMAKE_OPTIONS", "subdir-objects");
}


void
target_make_automake::process_item_uudecode(const nstring &fn)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring fn2 = nstring(fn.c_str(), fn.size() - 4);

    // this isn't automatic in automake (?)
    nstring_list lhs;
    lhs.push_back(fn2);
    nstring_list rhs;
    rhs.push_back(fn);
    nstring_list body;
    body.push_back("uudecode -o " + fn2 + " " + fn);
    location_comment(__FILE__, __LINE__);
    print_rule(lhs, rhs, body);

    // now process the result of the rule.
    // it could be just about anything.
    processing.run_process(fn2);
}


void
target_make_automake::process_item_scripts(const nstring &fn)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    //
    // The logic here *must* match the logic in
    // aemakegen/target/debian/process.cc
    //
    assert(filename_implies_is_a_script(fn));
    if (fn.starts_with("test/"))
        return;
    nstring prog = fn.basename().trim_extension();
    nstring bin_prog = "bin/" + prog;

    nstring_list lhs;
    lhs.push_back(bin_prog);
    nstring_list rhs;
    rhs.push_back(fn);
    nstring_list body;
    body.push_back("@mkdir -p " + bin_prog.dirname());
    body.push_back("cp " + fn + " $@");
    body.push_back("chmod a+rx $@");
    location_comment(__FILE__, __LINE__);
    print_rule(lhs, rhs, body);
}


void
target_make_automake::process_item_aegis_lib_doc(const nstring &fn)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
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


void
target_make_automake::process3_end(void)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (data.have_yacc() || data.need_yacc())
    {
        print_comment("\nWhich yacc flags to use.\n\n");
        location_comment(__FILE__, __LINE__);
        print_assignment("AM_YFLAGS", "-d");
    }

    if (data.seen_install_mandir())
    {
        print_comment("manual pages");
        location_comment(__FILE__, __LINE__);
        print_assignment_sorted("man_MANS", data.get_man_sources());
    }

    if (data.seen_all_bin())
    {
        nstring_list programs = data.get_all_bin();
        programs.sort();
        nstring_list bin_programs;
        nstring_list noinst_programs;
        nstring_list check_programs;
        nstring_list bin_scripts;
        nstring_list noinst_scripts;
        nstring_list check_scripts;
        for (size_t j = 0; j < programs.size(); ++j)
        {
            nstring bin_program = programs[j];
            assert(bin_program.starts_with("bin/"));
            nstring program = bin_program.substr(4, bin_program.size() - 4);
            nstring progdir = data.progdir_from_progname(program);
            bool is_a_script = data.get_object_files_by_dir(progdir).empty();
            if (is_a_script)
            {
                if (data.is_explicit_noinst(program))
                    noinst_scripts.push_back(bin_program);
                else
                    bin_scripts.push_back(bin_program);
                if (program.starts_with("test"))
                    check_scripts.push_back(bin_program);
            }
            else
            {
                if (data.is_explicit_noinst(program))
                    noinst_programs.push_back(bin_program);
                else
                    bin_programs.push_back(bin_program);
                if (program.starts_with("test"))
                    check_programs.push_back(bin_program);
            }
        }

        if (!bin_programs.empty())
        {
            print_comment("executables to be installed");
            print_assignment_sorted("bin_PROGRAMS", bin_programs);
        }
        if (!noinst_programs.empty())
        {
            print_comment("executables not to be installed");
            print_assignment_sorted("noinst_PROGRAMS", noinst_programs);
        }
        if (!check_programs.empty())
        {
            print_comment("executables needed to run tests");
            print_assignment_sorted("check_PROGRAMS", check_programs);
        }

        if (!bin_scripts.empty())
        {
            print_comment("scripts to be installed");
            print_assignment_sorted("bin_SCRIPTS", bin_scripts);
        }
        if (!noinst_scripts.empty())
        {
            print_comment("scripts not to be installed");
            print_assignment_sorted("noinst_SCRIPTS", noinst_scripts);
        }
        if (!check_scripts.empty())
        {
            print_comment("scripts needed to run tests");
            print_assignment_sorted("check_SCRIPTS", check_scripts);
        }
    }

    if (data.seen_pkgconfig_source())
    {
        op->fputc('\n');
        print_assignment("pkgconfigdir", "$(libdir)/pkgconfig");

        print_comment("Data files to be installed in $(pkgconfigdir)");
        print_assignment_sorted("pkgconfig_DATA", data.get_pkgconfig_sources());
    }

    // Emit the definitions for the library or libraries.
    {
        nstring_list lib_ltlibraries;
        nstring_list noinst_libraries;
        nstring_list lib_dirs = data.get_list_of_library_directories();
        lib_dirs.sort();
        for (size_t j = 0; j < lib_dirs.size(); ++j)
        {
            nstring library_dirname = lib_dirs[j];

            nstring library_libname = library_dirname;
            if (library_libname == "lib")
                library_libname = get_project_name();
            if (!library_libname.starts_with("lib"))
                library_libname = "lib" + library_libname;

            nstring path =
                library_dirname + "/" + library_libname + "." + data.libext();

            bool shared =
                (
                    data.use_libtool()
                &&
                    library_libname == get_library_libname()
                );

            print_comment("The " + path + " library.");

            print_assignment_sorted
            (
                path.identifier() + "_includes",
                data.get_include_files_by_dir(library_dirname)
            );
            print_assignment_sorted
            (
                path.identifier() + "_SOURCES",
                data.get_source_files_by_dir(library_dirname)
            );

            if (shared)
            {
                print_assignment
                (
                    path.identifier() + "_LDFLAGS",
                    "-version-info " + data.get_version_info()
                );
                lib_ltlibraries.push_back(path);
            }
            else
                noinst_libraries.push_back(path);
        }

        if (!lib_ltlibraries.empty())
        {
            print_comment("Shared libraries, to be installed.");
            print_assignment_sorted("lib_LTLIBRARIES", lib_ltlibraries);
        }

        if (!noinst_libraries.empty())
        {
            print_comment("Static libraries, not to be installed.");
            print_assignment_sorted("noinst_LIBRARIES", noinst_libraries);
        }
    }

    if (data.seen_datadir() || data.seen_datarootdir())
    {
        nstring_list files = data.get_install_datadir();
        nstring_list pkgdatadir;
        nstring_list datadir;
        nstring_list datarootdir;
        nstring projname = get_project_name() + "/";
        for (size_t j = 0; j < files.size(); ++j)
        {
            nstring file = files[j];
            if (file.starts_with("$(datarootdir)/"))
            {
                file = file.trim_first_directory();
                datarootdir.push_back(file);
            }
            else if (file.starts_with("$(datadir)/"))
            {
                file = file.trim_first_directory();
                if (file.starts_with(projname))
                {
                    file = file.trim_first_directory();
                    pkgdatadir.push_back(file);
                }
                else
                {
                    datadir.push_back(file);
                }
            }
            else if (file.starts_with("$(sysconfdir)/"))
            {
                // FIXME: this is wrong, but I don't know how to force
                // Automake to change the name of a file when it is
                // installed.
                datarootdir.push_back(file);
            }
            else
            {
                print_comment
                (
                    "Don't know what to do with the " +
                    file.quote_c() + " file."
                );
            }
        }
        if (!pkgdatadir.empty())
        {
            print_comment
            (
                "Data files to be installed in $(datadir)/" + projname
            );
            location_comment(__FILE__, __LINE__);
            print_assignment_sorted("dist_pkgdatadir_DATA", pkgdatadir);
        }
        if (!datadir.empty())
        {
            print_comment("Data files to be installed in $(datadir)");
            location_comment(__FILE__, __LINE__);
            print_assignment_sorted("dist_datadir_DATA", pkgdatadir);
        }
        if (!datarootdir.empty())
        {
            print_comment("Data files to be installed in $(datarootdir)");
            location_comment(__FILE__, __LINE__);
            print_assignment_sorted("dist_datarootdir_DATA", datarootdir);
        }
    }

    {
        nstring_list drd;
        drd.push_back(data.get_am_data_data());
        drd.push_back(data.get_install_i18n());
        if (!drd.empty())
        {
            print_comment("Data to be installed below $(datarootdir)/");
            location_comment(__FILE__, __LINE__);
            print_assignment_sorted("data_DATA", drd);
        }
    }

    if (!data.get_install_include_sources().empty())
    {
        print_comment("header files to be installed");
        print_assignment_sorted
        (
            (
                data.get_library_directory() == "lib"
            ?
                "include_HEADERS"
            :
                "nobase_include_HEADERS"
            ),
            data.get_install_include_sources()
        );
    }

    if (data.seen_dist_clean_files())
    {
        print_comment("Files to be removed by the \"distclean\" make target.");
        print_assignment_sorted("DISTCLEANFILES", data.get_dist_clean_files());
    }

    if (data.seen_programs())
    {
        bool package_library = !data.get_library_name().empty();
        nstring_list programs = data.get_programs();
        programs.sort();
        for (size_t j = 0; j < programs.size(); ++j)
        {
            nstring prog = programs[j];
            nstring iprog = prog.identifier();
            print_comment("The " + prog + " program.");

            print_assignment_sorted
            (
                "bin_" + iprog + "_includes",
                data.get_include_files_by_dir(prog)
            );
            print_assignment_sorted
            (
                "bin_" + iprog + "_SOURCES",
                data.get_source_files_by_dir(prog)
            );

            {
                nstring_list ldadd = data.get_library_list_by_program(prog);
                if (ldadd.empty() && package_library)
                {
                    ldadd.push_back
                    (
                        data.get_library_directory() + "/" +
                        get_library_libname() + "." + data.libext()
                    );
                }
                print_assignment_sorted("bin_" + iprog + "_LDADD", ldadd);
            }
        }
    }

    if (data.seen_test_files())
    {
        //
        // FIXME: this needs to be smarter about the test_command field
        // of the aegis.conf project configuration file.
        //
        print_comment("How to run the test scripts.");
        nstring_list rhs;
        if (data.seen_etc_test_sh())
            rhs.push_back("$(SHELL) etc/test.sh");
        else
            rhs.push_back("PATH=`pwd`/bin:$$PATH $(SHELL)");
        print_assignment("TESTS_ENVIRONMENT", rhs);

        print_comment("The test scripts to be run.");
        print_assignment_sorted("TESTS", data.get_test_sources());
    }

    // These files don't appear in any other list of files,
    // but are to be included in tarballs.
    if (data.seen_extra_dist())
    {
        print_comment("Additional source files to be included in the tarball.");
        print_assignment_sorted("EXTRA_DIST", data.get_extra_dist());
    }

    if (data.use_libtool())
    {
        //
        // Libtool makes executables that have to be writable.  Sheesh.  This
        // makes baseline testing and regression testing impossible, when
        // you are using Aegis.  So we have to execute thm once <i>at build
        // time</i> to get all the crap resolved into an executable that
        // <i>doesn't</i> need to be writable.
        //
        // (It doesn't matter if the program doesn't understand the -V option,
        // it still has the effect of making the program the way we want it.)
        //
        op->fprintf("\n");
        op->fprintf("all-am: goose-libtool-writable-crap\n");
        op->fprintf("goose-libtool-writable-crap: $(PROGRAMS)\n");
        nstring_list programs = data.get_programs();
        programs.sort();
        for (size_t j = 0; j < programs.size(); ++j)
        {
            nstring prog = programs[j];
            op->fprintf("\t-bin/%s -V\n", prog.c_str());
        }
    }

    //
    // BUILT_SOURCES is how automake handles catch-22 include dependencies.
    //
    if (data.seen_built_sources())
    {
        print_comment("Catch-22 when building dependencies.");
        location_comment(__FILE__, __LINE__);
        print_assignment_sorted("BUILT_SOURCES", data.get_built_sources());
    }

    print_comment("vim: set ts=8 sw=8 noet :");
}


// vim: set ts=8 sw=4 et :
