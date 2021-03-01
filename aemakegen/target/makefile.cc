//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2007 Peter Miller;
//
//      This program is free software; you can redistribute it and/or
//      modify it under the terms of the GNU General Public License,
//      version 2, as published by the Free Software Foundation.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public
//      License along with this program; if not, write to the Free
//      Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//      MA 02111, USA.
//

#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <common/progname.h>
#include <libaegis/os.h>

#include <aemakegen/target/makefile.h>


target_makefile::~target_makefile()
{
}


target_makefile::target_makefile() :
    seen_c(false),
    seen_c_plus_plus(false),
    have_groff(false)
{
}


void
target_makefile::begin()
{
    printf("#\n");
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
    printf("# IT IS NOT the original source file.  This file is\n");
    printf("# GENERATED.  If you find a bug in this file, please\n");
    printf("# send me a patch for the the problem source file.\n");
    printf("#\n");
    printf("# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("#\n");
    printf("# @configure_input@\n");
    printf("#\n");
    printf("# The configure script generates 2 files:\n");
    printf("#\t1. This Makefile\n");
    printf("#\t2. lib/config.h\n");
    printf("# If you change this Makefile, you may also need to\n");
    printf("# change these files.  To see what is configured by the\n");
    printf("# configure script, search for @ in the Makefile.in\n");
    printf("# file.\n");
    printf("#\n");
    printf("# If you wish to reconfigure the installation\n");
    printf("# directories it is RECOMMENDED that you re-run the\n");
    printf("# configure script.\n");
    printf("#\n");
    printf("# Use ``./configure --help'' for a list of options.\n");
    printf("#\n");
    printf("\n");
    printf("#\n");
    printf("# directory containing the source\n");
    printf("#\n");
    printf("srcdir = @srcdir@\n");
    printf("VPATH = @srcdir@\n");
    printf("\n");
    printf("#\n");
    printf("# the name of the install program to use\n");
    printf("#\n");
    printf("INSTALL = @INSTALL@\n");
    printf("INSTALL_PROGRAM = @INSTALL_PROGRAM@\n");
    printf("INSTALL_DATA = @INSTALL_DATA@\n");
    printf("\n");
    printf("#\n");
    printf("# the name of the compiler to use\n");
    printf("#\n");
    printf("CXX = @CXX@\n");
    printf("CC = @CC@\n");
    printf("\n");
    printf("#\n");
    printf("# The compiler flags to use\n");
    printf("#\n");
    printf("CXXFLAGS = @CXXFLAGS@ -Wall -Wextra\n");
    printf("CFLAGS = @CFLAGS@ -Wall -Wextra\n");
    printf("\n");
    printf("#\n");
    printf("# The preprocessor flags to use\n");
    printf("#\n");
    printf("CPPFLAGS = @CPPFLAGS@\n");
    printf("\n");
    printf("#\n");
    printf("# The linker flags to use\n");
    printf("#\n");
    printf("LDFLAGS = @LDFLAGS@\n");
    printf("\n");
    printf("#\n");
    printf("# prefix for installation path\n");
    printf("#\n");
    printf("prefix = @prefix@\n");
    printf("exec_prefix = @exec_prefix@\n");
    printf("\n");
    printf("#\n");
    printf("# where to put the executables\n");
    printf("#\n");
    printf("# On a network, this would only be shared between machines\n");
    printf("# of identical cpu-hw-os flavour.  It can be read-only.\n");
    printf("#\n");
    printf("# The $(DESTDIR) is for Debian packaging.\n");
    printf("# The $(RPM_BUILD_ROOT) is for RPM packaging.\n");
    printf("#\n");
    printf("bindir = $(DESTDIR)$(RPM_BUILD_ROOT)@bindir@\n");
    printf("\n");
    printf("#\n");
    printf("# where to put the manuals\n");
    printf("#\n");
    printf("# On a network, this would be shared between all machines\n");
    printf("# on the network.  It can be read-only.\n");
    printf("#\n");
    printf("# The $(DESTDIR) is for Debian packaging.\n");
    printf("# The $(RPM_BUILD_ROOT) is for RPM packaging.\n");
    printf("#\n");
    printf("mandir = $(DESTDIR)$(RPM_BUILD_ROOT)@mandir@\n");
    printf("\n");
    printf("#\n");
    printf("# Which yacc to use\n");
    printf("#\n");
    printf("YACC = @YACC@\n");
    printf("\n");
    printf("#\n");
    printf("# extra libraries required for your system\n");
    printf("#\n");
    printf("LIBS = @LIBS@\n");
    printf("\n");
    printf("#\n");
    printf("# shell to use to run tests and commands\n");
    printf("#\n");
    printf("SH = @SH@\n");
    printf("\n");
    printf("#\n");
    printf("# program used to place index within libraries\n");
    printf("#\n");
    printf("RANLIB = @RANLIB@\n");
    printf("AR = ar\n");
    printf("\n");
    printf("#\n");
    printf("# object file name extension (typically \"o\" or \"obj\")\n");
    printf("#\n");
    printf("OBJEXT = @OBJEXT@\n");
    printf("\n");
    printf("#\n");
    printf("# library file name extension (typically \"a\" or \"lib\")\n");
    printf("#\n");
    printf("LIBEXT = @LIBEXT@\n");
    printf("\n");
    printf("# -------------------------------------------------------------\n");
    printf("# You should not need to change anything below this line.\n");
    printf("\n");
    printf("#\n");
    printf("# The default target\n");
    printf("#\n");
    printf("all: bin-all\n");

    clean_files.push_back("core");
    clean_files.push_back("y.tab.c");
    clean_files.push_back("y.tab.h");
    clean_files.push_back("y.output");
    clean_files.push_back(".bin");
    clean_files.push_back(".bindir");
    clean_files.push_back("lib/lib.$(LIBEXT)");
}


void
target_makefile::recursive_mkdir(const nstring &a_src_dir,
    const nstring &a_dst_dir, const nstring &flavor)
{
    nstring src_dir(a_src_dir);
    nstring dst_dir(a_dst_dir);
    for (;;)
    {
	nstring dotdot1 = os_dirname_relative(src_dir);
	nstring dotdot2 = os_dirname_relative(dst_dir);
	nstring key = src_dir + "/." + flavor;
	int *p = dir_table.query(key);
	if (!p)
	{
	    dir_table.assign(key, 1);
	    printf("\n");
	    if (dotdot1 != "." && dotdot2 != ".")
	    {
		printf("%s/.%s: %s/.%s\n", src_dir.c_str(), flavor.c_str(),
			dotdot1.c_str(), flavor.c_str());
	    }
	    else
	    {
		printf("%s/.%s:\n", src_dir.c_str(), flavor.c_str());
	    }
	    printf("\t-$(INSTALL) -m 0755 -d %s\n", dst_dir.c_str());
	    printf("\t@-test -d %s && touch $@\n", dst_dir.c_str());
	    printf("\t@sleep 1\n");
	    clean_files.push_back(key);
	}
	src_dir = dotdot1;
	dst_dir = dotdot2;
	if (src_dir == "." || dst_dir == ".")
	    break;
    }
}


void
target_makefile::remember_program(const nstring &name)
{
    programs.push_back_unique(name);
}


void
target_makefile::remember_object_file(const nstring &path)
{
    assert(path.ends_with(".$(OBJEXT)"));
    nstring objdir = path.first_dirname();
    nstring_list *obj_file_list = object_list.query(objdir);
    if (!obj_file_list)
    {
	obj_file_list = new nstring_list();
	object_list.assign(objdir, obj_file_list);
    }
    obj_file_list->push_back(path);
    clean_files.push_back(path);
}


void
target_makefile::process_configure_dot_ac(const nstring &fn)
{
    nstring path = resolve(fn);
    FILE *fp = fopen(path.c_str(), "r");
    if (!fp)
	return;
    for (;;)
    {
	char line[2000];
	if (!fgets(line, sizeof(line), fp))
	    break;
	if (0 == memcmp(line, "AC_CHECK_PROGS(GROFF,", 21))
            have_groff = true;
    }
}


void
target_makefile::process(const nstring &a_fn, bool is_a_script)
{
    if
    (
        memcmp(a_fn.c_str(), "configure", 9) == 0
    ||
        strstr(a_fn.c_str(), "/configure")
    )
    {
        process_configure_dot_ac(a_fn);
    }

    nstring fn = a_fn;
    if (fn.ends_with(".in"))
	fn = fn.substring(0, fn.size() - 3);
    if (fn.gmatch("*/*.y"))
    {
	nstring base = os_basename(fn, ".y");
	nstring stem = fn.substring(0, fn.size() - 2);
	clean_files.push_back(stem + ".yacc.cc");
	clean_files.push_back(stem + ".yacc.h");
	nstring yy = base.identifier();

	printf("\n");
        if (seen_c_plus_plus)
        {
            printf("%s.yacc.cc %s.yacc.h: %s\n", stem.c_str(), stem.c_str(),
                    fn.c_str());
            printf("\t$(YACC) -d %s\n", fn.c_str());
            printf("\tsed -e 's/[yY][yY]/%s_/g' y.tab.c > %s.yacc.cc\n",
                    yy.c_str(), stem.c_str());
            printf("\tsed -e 's/[yY][yY]/%s_/g' y.tab.h > %s.yacc.h\n",
                    yy.c_str(), stem.c_str());
            printf("\trm y.tab.c y.tab.h\n");

            process(stem + ".yacc.cc", is_a_script);
        }
        else
        {
            printf("%s.yacc.c %s.yacc.h: %s\n", stem.c_str(), stem.c_str(),
                    fn.c_str());
            printf("\t$(YACC) -d %s\n", fn.c_str());
            printf("\tsed -e 's/[yY][yY]/%s_/g' y.tab.c > %s.yacc.c\n",
                    yy.c_str(), stem.c_str());
            printf("\tsed -e 's/[yY][yY]/%s_/g' y.tab.h > %s.yacc.h\n",
                    yy.c_str(), stem.c_str());
            printf("\trm y.tab.c y.tab.h\n");

            process(stem + ".yacc.c", is_a_script);
        }
    }
    else if (fn.gmatch("*/*.l"))
    {
	nstring stem = fn.substring(0, fn.size() - 2);
	clean_files.push_back(stem + ".lex.cc");
	nstring yy = os_basename(stem).identifier();

	printf("\n");
        if (seen_c_plus_plus)
        {
            printf("%s.lex.cc: %s\n", stem.c_str(), fn.c_str());
            printf("\t$(LEX) $(LEXFLAGS) %s\n", fn.c_str());
            printf("\tsed -e 's|[yY][yY]|%s_|g' lex.yy.c > %s.lex.cc\n",
                    yy.c_str(), stem.c_str());
            printf("\trm lex.yy.c\n");

            process(stem + ".lex.cc", is_a_script);
        }
        else
        {
            printf("%s.lex.c: %s\n", stem.c_str(), fn.c_str());
            printf("\t$(LEX) $(LEXFLAGS) %s\n", fn.c_str());
            printf("\tsed -e 's|[yY][yY]|%s_|g' lex.yy.c > %s.lex.c\n",
                    yy.c_str(), stem.c_str());
            printf("\trm lex.yy.c\n");

            process(stem + ".lex.c", is_a_script);
        }
    }
    else if
    (
        fn.downcase().ends_with(".cc")
    ||
        fn.downcase().ends_with(".cxx")
    ||
        fn.ends_with(".C")
    )
    {
        seen_c_plus_plus = true;

	nstring stem = fn.trim_extension();
	remember_object_file(stem + ".$(OBJEXT)");

	nstring root = os_basename(stem);
        if (root == "main")
	    remember_program(fn.first_dirname());
	nstring dep = c_include_dependencies(fn);

	printf("\n");
	printf("%s.$(OBJEXT): %s %s\n", stem.c_str(), fn.c_str(), dep.c_str());
	printf("\t$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I. -c %s\n", fn.c_str());
        if (root != stem)
        {
            printf("\tmv %s.$(OBJEXT) %s.$(OBJEXT)\n", root.c_str(),
                stem.c_str());
        }
    }
    else if (fn.ends_with(".c"))
    {
        seen_c = true;

	nstring stem = fn.trim_extension();
	remember_object_file(stem + ".$(OBJEXT)");

	if (fn.ends_with("/main.c"))
	    remember_program(fn.first_dirname());

	nstring root = os_basename(stem);
	nstring dep = c_include_dependencies(fn);

	printf("\n");
	printf("%s.$(OBJEXT): %s %s\n", stem.c_str(), fn.c_str(),
		dep.c_str());
	printf("\t$(CC) $(CPPFLAGS) $(CFLAGS) -I. -c %s\n", fn.c_str());
        if (root != stem)
        {
            printf("\tmv %s.$(OBJEXT) %s.$(OBJEXT)\n", root.c_str(),
                stem.c_str());
        }
    }
    else if (fn.gmatch("test/*/*.sh"))
    {
	nstring base = os_basename(fn, ".sh");
	test_files.push_back(base);

	printf("\n");
	printf("%s: %s all\n", base.c_str(), fn.c_str());
	printf("\t$(SH) %s\n", fn.c_str());
    }
    else if (fn.gmatch("man/man[0-9]/*.[0-9]"))
    {
	nstring stem = fn.substring(4, fn.size() - 4);
	nstring file2 = "$(mandir)/" + stem;
	man_files.push_back(file2);

	nstring src = os_dirname_relative(fn);
	nstring dst = os_dirname_relative(file2);
	recursive_mkdir(src, dst, "mandir");

	nstring dir = os_dirname_relative(fn);
#if 0
	nstring dep = roff_include_dependencies(fn);
#else
	nstring dep;
#endif

	printf("\n");
	printf("%s: %s %s %s/.mandir\n", file2.c_str(), fn.c_str(), dep.c_str(),
                dir.c_str());
        if (have_groff)
        {
            printf("\t$(SOELIM) -I. -I%s %s > tmp\n", dir.c_str(), fn.c_str());
            printf("\t$(INSTALL_DATA) tmp $@\n");
            printf("\t@rm -f tmp\n");
        }
        else
        {
            printf("\t$(INSTALL_DATA) %s $@\n", fn.c_str());
        }
    }
    else if (fn.gmatch("etc/*.man"))
    {
	nstring stem = fn.substring(4, fn.size() - 8);
	clean_files.push_back(stem + ".ps");
	clean_files.push_back(stem + ".pdf");
	clean_files.push_back(stem + ".dvi");
	clean_files.push_back(stem + ".txt");

	nstring base = os_basename(fn, ".man");
#if 0
	nstring dep = roff_include_dependencies(fn);
#else
	nstring dep;
#endif

        if (have_groff)
        {
            printf("\n");
            printf("etc/%s.ps: %s %s\n", base.c_str(), fn.c_str(),
                dep.c_str());
            printf("\t$(GROFF) -s -I. -t -man %s > $@\n", fn.c_str());
        }
    }
}


nstring
target_makefile::c_include_dependencies(const nstring &fn)
{
    nstring_list result;
    result.push_back(fn);
    for (size_t j = 0; j < result.size(); ++j)
	c_include_dependencies(result, result[j]);
    nstring_list temp;
    for (size_t k = 1; k < result.size(); ++k)
    {
	nstring file = result[k];
	nstring path = resolve(file);
	if (exists(path))
	    temp.push_back(file.quote_shell());
    }
    return temp.unsplit();
}


static inline bool
safe_isspace(int c)
{
    return isspace((unsigned char)c);
}


void
target_makefile::c_include_dependencies(nstring_list &results,
    const nstring &fn)
{
    nstring path = resolve(fn);
    FILE *fp = fopen(path.c_str(), "r");
    if (!fp)
	return;
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
}


void
target_makefile::c_directive(const char *s, nstring_list &results)
{
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
    while (isspace(*s))
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
target_makefile::end()
{
    nstring_list *lib_obj_list = object_list.query("lib");
    if (lib_obj_list)
    {
        printf("\n");
        printf("lib_obj = %s\n", lib_obj_list->unsplit().c_str());
        printf("\n");
        printf("lib/lib.$(LIBEXT): $(lib_obj)\n");
        printf("\trm -f $@\n");
        printf("\t$(AR) qc $@ $(lib_obj)\n");
        printf("\t$(RANLIB) $@\n");
    }

    for (size_t j = 0; j < programs.size(); ++j)
    {
	nstring prog = programs[j];
	nstring_list *obj_file_list = object_list.query(prog);
	assert(obj_file_list);
	printf("\n");
	printf("%s_obj = %s\n", prog.c_str(), obj_file_list->unsplit().c_str());

	printf("\n");
	printf("bin/%s: $(%s_obj) .bin", prog.c_str(), prog.c_str());
        if (lib_obj_list)
            printf(" lib/lib.$(LIBEXT)");
        printf("\n");
        if (seen_c_plus_plus)
            printf("\t$(CXX)");
        else
            printf("\t$(CC)");
        printf(" $(LDFLAGS) -o $@ $(%s_obj)", prog.c_str());
        if (lib_obj_list)
            printf(" lib/lib.$(LIBEXT)");
        printf(" $(LIBS)\n");

	all.push_back("bin/" + prog);

	printf("\n");
	printf("$(bindir)/%s: bin/%s .bindir\n", prog.c_str(), prog.c_str());
	printf("\t$(INSTALL_PROGRAM) bin/%s $@\n", prog.c_str());

	if (prog != progname_get())
	    install_bin.push_back("$(bindir)/" + prog);
    }

    printf("\n");
    printf("bin-all: %s\n", all.unsplit().c_str());

    printf("\n");
    printf(".bin:\n");
    printf("\t-mkdir bin\n");
    printf("\t-chmod 0755 bin\n");
    printf("\t@-test -d bin && touch $@\n");
    printf("\t@sleep 1\n");

    printf("\n");
    printf(".bindir:\n");
    printf("\t-$(INSTALL) -m 0755 -d $(bindir)\n");
    printf("\t@-test -d $(bindir) && touch $@\n");
    printf("\t@sleep 1\n");

    printf("\n");
    printf("sure: %s\n", test_files.unsplit().c_str());
    printf("\t@echo Passed All Tests\n");

    printf("\n");
    printf("clean-obj:\n");
    for (size_t k = 0; k < clean_files.size(); ++k)
    {
	printf("\trm -f %s\n", clean_files[k].quote_shell().c_str());
    }

    printf("\n");
    printf("clean: clean-obj\n");
    for (size_t k = 0; k < all.size(); ++k)
    {
	printf("\trm -f %s\n", all[k].quote_shell().c_str());
    }

    printf("\n");
    printf("distclean: clean\n");
    printf("\trm -f Makefile lib/config.h\n");
    printf("\trm -f config.status config.cache config.log\n");

    printf("\n");
    printf("install-bin: %s\n", install_bin.unsplit().c_str());

    if (!man_files.empty())
    {
        printf("\n");
        if (have_groff)
        {
            printf("install-man: install-man-${HAVE_GROFF}\n");
            printf("\n");
            printf("install-man-yes: %s\n", man_files.unsplit().c_str());
            printf("\n");
            printf("install-man-no:\n");
        }
        else
        {
            printf("install-man: %s\n", man_files.unsplit().c_str());
        }
    }

    printf("\n");
    printf("install: install-bin");
    if (!man_files.empty())
        printf(" install-man");
    printf("\n");
}
