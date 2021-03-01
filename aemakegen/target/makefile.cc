//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or
//      modify it under the terms of the GNU General Public License,
//      version 3, as published by the Free Software Foundation.
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
    have_groff(false),
    have_soelim(false),
    objext("o"),
    libext("a"),
    seen_datadir(false),
    seen_libdir(false),
    seen_yacc(false),
    install_script(false)
{
}


void
target_makefile::preprocess(const nstring &filename)
{
    if (filename.starts_with("datadir/"))
        seen_datadir = true;
    if (filename.starts_with("libdir/"))
        seen_libdir = true;
    if (filename.ends_with(".y"))
        seen_yacc = true;
    if (filename.starts_with("script/") || filename.starts_with("scripts/"))
        install_script = true;
    if
    (
        memcmp(filename.c_str(), "configure", 9) == 0
    ||
        memmem(filename.c_str(), filename.size(), "/configure", 10)
    )
    {
        process_configure_dot_ac(filename);
    }

    if
    (
        filename.downcase().ends_with(".cc")
    ||
        filename.downcase().ends_with(".cxx")
    ||
        filename.ends_with(".C")
    )
    {
        seen_c_plus_plus = true;
    }
    else if (filename.ends_with(".c"))
    {
        seen_c = true;
    }
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
    if (install_script)
        printf("INSTALL_SCRIPT = @INSTALL_SCRIPT@\n");
    printf("\n");
    printf("#\n");
    printf("# the name of the compiler to use\n");
    printf("#\n");
    if (seen_c_plus_plus)
        printf("CXX = @CXX@\n");
    if (seen_c)
        printf("CC = @CC@\n");
    printf("\n");
    printf("#\n");
    printf("# The compiler flags to use\n");
    printf("#\n");
    if (seen_c_plus_plus)
        printf("CXXFLAGS = @CXXFLAGS@\n");
    if (seen_c)
        printf("CFLAGS = @CFLAGS@\n");
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

    if (seen_datadir)
    {
        printf("\n");
        printf("#\n");
        printf("# where to put the non-executable package data\n");
        printf("#\n");
        printf("# On a network, this would be shared between all\n");
        printf("# machines on the network.  It can be read-only.\n");
        printf("#\n");
        printf("# The $(DESTDIR) is for Debian packaging.\n");
        printf("# The $(RPM_BUILD_ROOT) is for RPM packaging.\n");
        printf("#\n");
        printf("DATADIR = $(DESTDIR)$(RPM_BUILD_ROOT)@datadir@\n");
        printf("DATAROOTDIR = $(DESTDIR)$(RPM_BUILD_ROOT)@datarootdir@\n");
    }

    if (seen_libdir)
    {
        printf("\n");
        printf("#\n");
        printf("# where to put the supplementary package executables\n");
        printf("#\n");
        printf("# On a network, this would be shared between all\n");
        printf("# machines of identical architecture.  It can be\n");
        printf("# read-only.\n");
        printf("#\n");
        printf("# The $(DESTDIR) is for Debian packaging.\n");
        printf("# The $(RPM_BUILD_ROOT) is for RPM packaging.\n");
        printf("#\n");
        printf("LIBDIR = $(DESTDIR)$(RPM_BUILD_ROOT)@libdir@\n");
    }

    if (seen_yacc)
    {
        printf("\n");
        printf("#\n");
        printf("# Which yacc to use\n");
        printf("#\n");
        printf("YACC = @YACC@\n");
    }

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
    if (objext != "o")
    {
        printf("\n");
        printf("#\n");
        printf("# object file name extension (typically \"o\" or \"obj\")\n");
        printf("#\n");
        printf("OBJEXT = @OBJEXT@\n");
    }
    if (libext != "a")
    {
        printf("\n");
        printf("#\n");
        printf("# library file name extension (typically \"a\" or \"lib\")\n");
        printf("#\n");
        printf("LIBEXT = @LIBEXT@\n");
    }
    if (!exeext.empty())
    {
        printf("\n");
        printf("#\n");
        printf("# command file name extension (typically \"\" or \".exe\")\n");
        printf("#\n");
        printf("EXEEXT = @EXEEXT@\n");
    }
    if (have_groff)
    {
        printf("\n");
        printf("#\n");
        printf("# Set GROFF to the name of the roff command on your\n");
        printf("# system, usually \"groff\" or \"troff\" or \"nroff\"\n");
        printf("#\n");
        printf("GROFF = @GROFF@\n");
    }
    if (have_soelim)
    {
        printf("\n");
        printf("#\n");
        printf("# Set SOELIM to the name of the roff proprocessor command\n");
        printf("# on your system, usually \"soelim\" or \"gsoelim\"\n");
        printf("#\n");
        printf("SOELIM = @SOELIM@\n");
    }

    printf("\n");
    printf("# -------------------------------------------------------------\n");
    printf("# You should not need to change anything below this line.\n");
    printf("\n");
    printf("#\n");
    printf("# The default target\n");
    printf("#\n");
    printf("the-default-target: all\n");

    clean_files.push_back("core");
    clean_files.push_back("y.tab.c");
    clean_files.push_back("y.tab.h");
    clean_files.push_back("y.output");
    clean_files.push_back(".bin");
    clean_files.push_back(".bindir");
    clean_files.push_back("lib/lib." + libext);

    dist_clean_files.push_back("Makefile");
    dist_clean_files.push_back("config.cache");
    dist_clean_files.push_back("config.log");
    dist_clean_files.push_back("config.status");
    dist_clean_files.push_back("lib/config.h");
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
    assert(path.ends_with("." + objext));
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
	if (0 == memcmp(line, "AC_CHECK_PROGS(SOELIM,", 21))
            have_soelim = true;
	if (0 == memcmp(line, "AC_OBJEXT", 9))
            objext = "$(OBJEXT)";
	if (0 == memcmp(line, "AC_LIBEXT", 9))
            libext = "$(LIBEXT)";
	if (0 == memcmp(line, "AC_EXEEXT", 9))
            exeext = "$(EXEEXT)";
    }
}


void
target_makefile::process(const nstring &a_fn, bool is_a_script)
{
    nstring fn = a_fn;
    if (fn.ends_with(".in"))
    {
        nstring fn2 = nstring(fn.c_str(), fn.size() - 3);
        printf("\n");
        printf("%s: %s ./config.status\n\t", fn2.c_str(), fn.c_str());
        if (fn2.ends_with(".h"))
        {
            printf("CONFIG_FILES= CONFIG_HEADERS=$@:%s ", fn.c_str());
        }
        else
        {
            printf("CONFIG_FILES=$@:%s CONFIG_HEADERS= ", fn.c_str());
        }
        printf("$(SH) ./config.status\n");
        fn = fn2;
        dist_clean_files.push_back_unique(fn);
    }

    if (fn.starts_with("datadir/"))
    {
        nstring src = fn;
        nstring stem = src.substring(8, src.size() - 8);
        nstring dst = "$(DATADIR)/" + stem;

	nstring src_dir = os_dirname_relative(src);
	nstring dst_dir = os_dirname_relative(dst);
	recursive_mkdir(src_dir, dst_dir, "datadir");

        printf("\n");
	printf("%s: %s %s/.datadir\n", dst.c_str(), src.c_str(),
            src_dir.c_str());
        printf("\t$(INSTALL_DATA) %s $@\n", src.c_str());

        install_datadir.push_back(dst);
        return;
    }

    if (fn.starts_with("libdir/"))
    {
        nstring src = fn;
        nstring stem = src.substring(7, src.size() - 7);
        nstring dst = "$(LIBDIR)/" + stem;

	nstring src_dir = os_dirname_relative(src);
	nstring dst_dir = os_dirname_relative(dst);
	recursive_mkdir(src_dir, dst_dir, "libdir");

        printf("\n");
	printf("%s: %s %s/.libdir\n", dst.c_str(), src.c_str(),
            src_dir.c_str());
        printf("\t$(INSTALL_DATA) %s $@\n", src.c_str());

        install_libdir.push_back(dst);
        return;
    }

    if (fn.gmatch("*/*.y"))
    {
	nstring base = os_basename(fn, ".y");
	nstring stem = fn.substring(0, fn.size() - 2);
	clean_files.push_back(stem + ".yacc.c");
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
	clean_files.push_back(stem + ".lex.c");
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
	nstring stem = fn.trim_extension();
	remember_object_file(stem + "." + objext);

	nstring root = os_basename(stem);
        if (root == "main")
	    remember_program(fn.first_dirname());
	nstring dep = c_include_dependencies(fn);

	printf("\n");
	printf("%s.%s: %s %s\n", stem.c_str(), objext.c_str(), fn.c_str(),
            dep.c_str());
	printf("\t$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I. -c %s\n", fn.c_str());
        if (root != stem)
        {
            printf("\tmv %s.%s %s.%s\n", root.c_str(), objext.c_str(),
                stem.c_str(), objext.c_str());
        }
    }
    else if (fn.ends_with(".c"))
    {
	nstring stem = fn.trim_extension();
	remember_object_file(stem + "." + objext);

	if (fn.ends_with("/main.c"))
	    remember_program(fn.first_dirname());

	nstring root = os_basename(stem);
	nstring dep = c_include_dependencies(fn);

	printf("\n");
	printf("%s.%s: %s %s\n", stem.c_str(), objext.c_str(), fn.c_str(),
		dep.c_str());
	printf("\t$(CC) $(CPPFLAGS) $(CFLAGS) -I. -c %s\n", fn.c_str());
        if (root != stem)
        {
            printf("\tmv %s.%s %s.%s\n", root.c_str(), objext.c_str(),
                stem.c_str(), objext.c_str());
        }
    }
    else if (fn.gmatch("test/*/*.sh"))
    {
	nstring base = os_basename(fn, ".sh");
	test_files.push_back(base);

	printf("\n");
	printf("%s: %s all\n", base.c_str(), fn.c_str());
	printf("\tPATH=`pwd`/bin:$$PATH $(SH) %s\n", fn.c_str());
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
        if (have_soelim)
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
	nstring base = os_basename(fn, ".man");
#if 0
	nstring dep = roff_include_dependencies(fn);
#else
	nstring dep;
#endif

        if (have_groff)
        {
            printf("\n");
            nstring tgt = "etc/" + base + ".ps";
            printf("%s: %s %s\n", tgt.c_str(), fn.c_str(), dep.c_str());
            printf("\t$(GROFF) -s -I. -t -man %s > $@\n", fn.c_str());

            all_doc.push_back(tgt);
        }
    }
    else if (fn.starts_with("script/") || fn.starts_with("scripts/"))
    {
        nstring name = fn.basename();
        nstring ext = name.get_extension();
        if (ext == "tcl" || ext == "sh" || ext == "pl" || ext == "py")
            name = name.trim_extension();

        nstring bin_name = "bin/" + name + exeext;
        printf("\n");
        printf("%s: %s .bin\n", bin_name.c_str(), fn.c_str());
        printf("\tcp %s $@\n", fn.c_str());
        printf("\tchmod a+rx $@\n");

        all_bin.push_back_unique(bin_name);
        clean_files.push_back_unique(bin_name);

        nstring install_name = "$(RPM_BUILD_ROOT)$(bindir)/$(PROGRAM_PREFIX)"
            + name + "$(PROGRAM_SUFFIX)" + exeext;
        printf("\n");
        printf("%s: %s .bindir\n", install_name.c_str(), bin_name.c_str());
        printf("\t$(INSTALL_SCRIPT) %s $@\n", bin_name.c_str());

        if (!name.starts_with("test_"))
            install_bin.push_back_unique(install_name);
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
    temp.sort();
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
        lib_obj_list->sort();
        printf("lib_obj = %s\n", lib_obj_list->unsplit().c_str());
        printf("\n");
        printf("lib/lib.%s: $(lib_obj)\n", libext.c_str());
        printf("\trm -f $@\n");
        printf("\t$(AR) qc $@ $(lib_obj)\n");
        printf("\t$(RANLIB) $@\n");
    }

    programs.sort();
    for (size_t j = 0; j < programs.size(); ++j)
    {
	nstring prog = programs[j];
	nstring_list *obj_file_list = object_list.query(prog);
	assert(obj_file_list);
	printf("\n");
	printf("%s_obj = %s\n", prog.c_str(), obj_file_list->unsplit().c_str());

	printf("\n");
	printf("bin/%s: $(%s_obj) .bin", (prog + exeext).c_str(), prog.c_str());
        if (lib_obj_list)
            printf(" lib/lib.%s", libext.c_str());
        printf("\n");
        if (seen_c_plus_plus)
            printf("\t$(CXX)");
        else
            printf("\t$(CC)");
        printf(" $(LDFLAGS) -o $@ $(%s_obj)", prog.c_str());
        if (lib_obj_list)
            printf(" lib/lib.%s", libext.c_str());
        printf(" $(LIBS)\n");

	all_bin.push_back("bin/" + prog + exeext);

	printf("\n");
	printf("$(bindir)/%s: bin/%s .bindir\n", (prog + exeext).c_str(),
            (prog + exeext).c_str());
	printf("\t$(INSTALL_PROGRAM) bin/%s $@\n", (prog + exeext).c_str());

	if (!prog.starts_with("test_"))
	    install_bin.push_back("$(bindir)/" + prog);
    }

    printf("\n");
    printf("all:");
    if (!all_bin.empty())
        printf(" all-bin");
    if (!all_doc.empty())
        printf(" all-doc");
    printf("\n");
    if (!all_bin.empty())
    {
        all_bin.sort();
        printf("all-bin: %s\n", all_bin.unsplit().c_str());
    }
    if (!all_doc.empty())
    {
        all_doc.sort();
        printf("all-doc: %s\n", all_doc.unsplit().c_str());
    }

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
    test_files.sort();
    printf("sure: %s\n", test_files.unsplit().c_str());
    printf("\t@echo Passed All Tests\n");

    printf("\n");
    printf("clean-obj:\n");
    clean_files.sort();
    for (size_t k = 0; k < clean_files.size(); ++k)
    {
	printf("\trm -f %s\n", clean_files[k].quote_shell().c_str());
    }

    printf("\n");
    printf("clean: clean-obj\n");
    for (size_t k = 0; k < all_bin.size(); ++k)
    {
	printf("\trm -f %s\n", all_bin[k].quote_shell().c_str());
    }
    for (size_t k = 0; k < all_doc.size(); ++k)
    {
	printf("\trm -f %s\n", all_doc[k].quote_shell().c_str());
    }

    dist_clean_files.sort();
    printf("\n");
    printf("distclean: clean\n");
    for (size_t k = 0; k < dist_clean_files.size(); ++k)
    {
	printf("\trm -f %s\n", dist_clean_files[k].quote_shell().c_str());
    }

    printf("\n");
    install_bin.sort();
    printf("install-bin: %s\n", install_bin.unsplit().c_str());

    if (!man_files.empty())
    {
        printf("\n");
        man_files.sort();
        printf("install-man: %s\n", man_files.unsplit().c_str());
    }

    if (!install_datadir.empty())
    {
        install_datadir.sort();
        printf("install-datadir: %s\n", install_datadir.unsplit().c_str());
    }
    if (!install_libdir.empty())
    {
        install_libdir.sort();
        printf("install-libdir: %s\n", install_libdir.unsplit().c_str());
    }

    printf("\n");
    printf("install: install-bin");
    if (!man_files.empty())
        printf(" install-man");
    if (!install_datadir.empty())
        printf(" install-datadir");
    if (!install_libdir.empty())
        printf(" install-libdir");
    printf("\n");
}
