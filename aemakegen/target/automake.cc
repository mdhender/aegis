//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <common/error.h> // for assert

#include <aemakegen/target/automake.h>


target_automake::~target_automake()
{
}


target_automake::target_automake() :
    use_libtool(false),
    use_dot_man(false)
{
}


void
target_automake::begin()
{
    printf("# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("#\n");
    printf("#    W   W    A    RRRR   N   N   III  N   N  III  N   N   GGG\n");
    printf("#    W   W   A A   R   R  NN  N    I   NN  N   I   NN  N  G   G\n");
    printf("#    W   W  A   A  RRRR   N N N    I   N N N   I   N N N  G\n");
    printf("#    W W W  AAAAA  R R    N  NN    I   N  NN   I   N  NN  G  GG\n");
    printf("#    W W W  A   A  R  R   N   N    I   N   N   I   N   N  G   G\n");
    printf("#     W W   A   A  R   R  N   N   III  N   N  III  N   N   GGG\n");
    printf("#\n");
    printf("# Warning: DO NOT send patches which fix this file.\n");
    printf("# This file is GENERATED from the Aegis repository file\n");
    printf("# manifest.  If you find a bug in this file, it could\n");
    printf("# well be an Aegis bug.\n");
    printf("#\n");
    printf("# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

    printf("\n");
    printf("#\n");
    printf("# Tell automake to put the object file for foo/bar.cc in\n");
    printf("# directory foo/\n");
    printf("#\n");
    printf("AUTOMAKE_OPTIONS = subdir-objects\n");
}


void
target_automake::remember_source_file(const nstring &path)
{
    nstring prog = path.first_dirname();
    if (prog != "lib")
        prog = "bin/" + prog;
    nstring_list *source_file_list = source_list.query(prog);
    if (!source_file_list)
    {
	source_file_list = new nstring_list();
	source_list.assign(prog, source_file_list);
    }
    source_file_list->push_back_unique(path);
}


void
target_automake::remember_include_file(const nstring &path)
{
    nstring include_dir = path.first_dirname();
    nstring_list *include_file_list = include_list.query(include_dir);
    if (!include_file_list)
    {
	include_file_list = new nstring_list();
	include_list.assign(include_dir, include_file_list);
    }
    include_file_list->push_back_unique(path);
}


void
target_automake::remember_program(const nstring &name)
{
    if (name.starts_with("test_"))
        check_programs.push_back_unique("bin/" + name);
    else
        bin_programs.push_back_unique("bin/" + name);
}


void
target_automake::process(const nstring &a_fn, bool is_a_script)
{
    nstring fn = a_fn;
    extra_dist.push_back(fn);
    if (fn.ends_with(".in"))
    {
	fn = fn.substring(0, fn.size() - 3);
        dist_clean_files.push_back_unique(fn);
    }
    nstring fn_lc = fn.downcase();

    if (fn.starts_with("datadir/"))
    {
        pkgdatadir.push_back(fn);
        return;
    }
    if (fn.starts_with("libdir/"))
    {
        pkglibdir.push_back(fn);
        return;
    }

    if (is_a_script)
    {
        nstring prog = fn.basename();
        if (trim_script_suffix())
            prog = prog.trim_extension();
        nstring bin_prog = "bin/" + prog;
        bool copy_and_chmod = (fn != prog);
        if (prog.starts_with("test_"))
            check_scripts.push_back(bin_prog);
        else if (fn_lc.starts_with("script/") || fn_lc.starts_with("scripts/"))
            bin_scripts.push_back(bin_prog);
        else if (!fn.starts_with("test/"))
            noinst_scripts.push_back(bin_prog);
        else
            copy_and_chmod = false;

        if (copy_and_chmod)
        {
            printf("\n");
            printf("%s: %s\n", bin_prog.c_str(), fn.c_str());
            printf("\tcp %s $@\n", fn.c_str());
            printf("\tchmod a+rx $@\n");
        }
        return;
    }

    if (fn_lc.ends_with(".y"))
    {
        remember_source_file(fn);
    }
    else if (fn_lc.ends_with(".l"))
    {
        remember_source_file(fn);
    }
    else if
    (
        fn_lc.ends_with(".c")
    ||
        fn_lc.ends_with(".cc")
    ||
        fn_lc.ends_with(".cpp")
    ||
        fn_lc.ends_with(".cxx")
    )
    {
        remember_source_file(fn);

	nstring stem = fn.trim_extension();
	nstring base = stem.basename();
        if (base == "main")
            remember_program(fn.first_dirname());
    }
    else if
    (
        fn_lc.ends_with(".h")
    ||
        fn_lc.ends_with(".hh")
    ||
        fn_lc.ends_with(".hpp")
    ||
        fn_lc.ends_with(".hxx")
    )
    {
        remember_source_file(fn);
        remember_include_file(fn);
        noinst_header.push_back_unique(fn);
    }
    else if (fn.gmatch("test/*/*.sh"))
    {
	test_files.push_back_unique(fn);
    }
    else if (fn.gmatch("*.[1-9]") || (use_dot_man && fn.ends_with(".man")))
    {
        man_mans.push_back_unique(fn);
    }
}


static void
print_file_list(const nstring &name, const nstring_list &value,
    const nstring &comment = "")
{
    if (value.empty())
        return;
    printf("\n");
    if (!comment.empty())
        printf("# %s\n", comment.c_str());
    printf("%s =", name.identifier().c_str());
    for (size_t j = 0; j < value.size(); ++j)
        printf(" \\\n\t%s", value[j].c_str());
    printf("\n");
}


void
target_automake::end()
{
    print_file_list
    (
        "bin_PROGRAMS",
        bin_programs,
        "executables to be installed"
    );
    print_file_list
    (
        "noinst_PROGRAMS",
        noinst_programs,
        "executables not to be installed"
    );
    print_file_list
    (
        "check_PROGRAMS",
        check_programs,
        "executables needed to run tests"
    );
    print_file_list("man_MANS", man_mans, "manual pages");
    print_file_list("EXTRA_DIST", extra_dist);
    print_file_list
    (
        "bin_SCRIPTS",
        bin_scripts,
        "scripts to be installed"
    );
    print_file_list
    (
        "check_SCRIPTS",
        check_scripts,
        "scripts needed to run tests"
    );
    print_file_list
    (
        "noinst_SCRIPTS",
        noinst_scripts,
        "scripts not to be installed"
    );
    print_file_list
    (
        "dist_pkgdatadir_DATA",
        pkgdatadir,
        "Data files to be installed in $(DATADIR)"
    );
    print_file_list
    (
        "dist_pkglibdir_DATA",
        pkglibdir,
        "Data files to be installed in $(LIBDIR)"
    );

    nstring_list lib_list;
    if (source_list.query("lib"))
    {
        if (use_libtool)
        {
            lib_list.push_back("lib/lib.la");
            print_file_list("noinst_LTLIBRARIES", lib_list);
        }
        else
        {
            lib_list.push_back("lib/lib.a");
            print_file_list("noinst_LIBRARIES", lib_list);
        }
    }

    print_file_list("noinst_HEADER", noinst_header, "project header files");

    print_file_list("DISTCLEANFILES", dist_clean_files);

    if (!lib_list.empty())
    {
        nstring_list *file_list = include_list.query("lib");
        if (file_list)
            print_file_list(lib_list[0] + "_includes", *file_list);
        file_list = source_list.query("lib");
        assert(file_list);
        print_file_list(lib_list[0] + "_SOURCES", *file_list);
    }

    for (size_t j = 0; j < bin_programs.size(); ++j)
    {
        nstring prog = bin_programs[j];
        nstring_list *file_list = include_list.query(prog);
        if (file_list)
            print_file_list(prog + "_includes", *file_list);
        file_list = source_list.query(prog);
        assert(file_list);
        if (file_list)
            print_file_list(prog + "_SOURCES", *file_list);
        print_file_list(prog + "_LDADD", lib_list);
    }

    for (size_t j = 0; j < noinst_programs.size(); ++j)
    {
        printf("\n");
        nstring prog = noinst_programs[j];
        nstring_list *file_list = include_list.query(prog);
        if (file_list)
            print_file_list(prog + "_includes", *file_list);
        file_list = source_list.query(prog);
        assert(file_list);
        if (file_list)
            print_file_list(prog + "_SOURCES", *file_list);
        print_file_list(prog + "_LDADD", lib_list);
    }

    if (!test_files.empty())
    {
        nstring_list dummy;
        dummy.push_back("PATH=`pwd`/bin:$$PATH");
        dummy.push_back("$(SHELL)");
        print_file_list("TESTS_ENVIRONMENT", dummy);
        print_file_list("TESTS", test_files);
    }
}
