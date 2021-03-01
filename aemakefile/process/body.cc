//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: implementation of the process_body class
//

#pragma implementation "process_body"

#include <ac/errno.h>
#include <ac/stdio.h>
#include <ac/string.h>

#include <error.h>
#include <process/body.h>


nstring_list process_body::search_path;


static nstring
dirname(const nstring &path)
{
    const char *cp = path.c_str();
    const char *ep = strrchr(cp, '/');
    if (!ep)
	return ".";
    return nstring(cp, ep - cp);
}


static nstring
trim_dir(const nstring &path)
{
    const char *cp = strchr(path.c_str(), '/');
    if (!cp)
	return "";
    return nstring(cp + 1);
}


static nstring
top_dir(const nstring &path)
{
    const char *cp = path.c_str();
    const char *ep = strchr(cp, '/');
    if (!ep)
	return ".";
    return nstring(cp, ep - cp);
}


static nstring
basename(const nstring &path)
{
    const char *cp = strrchr(path.c_str(), '/');
    if (!cp)
	return path;
    return nstring(cp + 1);
}


static nstring
basename(const nstring &path, const nstring &ext)
{
    const char *cp = strrchr(path.c_str(), '/');
    if (cp)
	++cp;
    else
	cp = path.c_str();

    const char *ep = path.c_str() + path.size();
    if (path.ends_with(ext.c_str()))
	ep -= ext.size();

    return nstring(cp, ep - cp);
}


process_body::~process_body()
{
}


process_body::process_body(printer &arg) :
    process(arg)
{
}


void
process_body::read_dependency_file(const nstring &file, nstring_list &dep)
{
    nstring filename(file + ",Md");
    FILE *fp = fopen(filename.c_str(), "r");
    if (!fp)
    {
	if (errno != ENOENT)
	    nfatal("open %s", filename.c_str());
	for (size_t j = 0; ; ++j)
	{
	    if (j >= search_path.size())
		return;
	    nstring fn2(search_path[j] + "/" + filename);
	    fp = fopen(fn2.c_str(), "r");
	    if (fp)
		break;
	    if (errno != ENOENT)
		nfatal("open %s", fn2.c_str());
	}
    }

    for (;;)
    {
	char buffer[2000];
	char *bp = buffer;
	int c;
	for (;;)
	{
	    c = getc(fp);
	    if (c == EOF || c == '\n')
		break;
	    if (bp < buffer + sizeof(buffer))
		*bp++ = c;
	}
	if (bp == buffer && c == EOF)
	    break;
	nstring line(buffer, bp - buffer);
	dep.push_back(line.replace("[arch]/", ""));
    }
    fclose(fp);
}


void
process_body::per_file(const nstring &filename)
{
    nstring file(filename);

    if (file.ends_with(".in"))
	file = nstring(file.c_str(), file.size() - 3);

    if (file.ends_with(".y"))
    {
	nstring stem(file.c_str(), file.size() - 2);
	nstring root = basename(stem);
	nstring dir(dirname(file));
	nstring yy = trim_dir(stem).identifier();

	nstring numconf("no");
	if (file == "common/gettime.y")
	    numconf = "8 shift/reduce";
	else if (file == "libaegis/aer/report.y")
	    numconf = "2 reduce/reduce";
	else if (file == "aefind/cmdline.y")
	    numconf = "5 shift/reduce, 23 reduce/reduce";
	else if (file == "common/gettime.y")
	    numconf = "8 shift/reduce";

	print << "\n";
	print << stem << ".gen.cc " << stem << ".gen.h: " << file << "\n";
	print << "\t@echo Expect " << numconf << " conflicts.\n";
	print << "\t$(YACC) -d " << file << "\n";
	print << "\tsed -e 's/[yY][yY]/" << yy << "_/g' -e '/<stdio.h>/d' "
	    "-e '/<stdlib.h>/d' -e '/<stddef.h>/d' y.tab.c > "
	    << stem << ".gen.cc\n";
	print << "\tsed -e 's/[yY][yY]/" << yy << "_/g' -e 's/Y_TAB_H/" << yy
	    << "_TAB_H/g' y.tab.h > " << stem << ".gen.h\n";
	print << "\trm -f y.tab.c y.tab.h\n";

#if 0
	//
        // We don't need this bit.  The cookbook adds these to the list
        // of files given to us to process, which also ensures that the
        // dependency (,Md) file has been created.
	//
	nstring_list dep;
	read_dependency_file(stem + ".gen.cc", dep);

	print << "\n";
	print << stem << ".gen.$(OBJEXT): " << stem << ".gen.cc " << dep
	    << "\n";
	print << "\t$(CXX) -I" << dir << " -Ilibaegis -Icommon $(CXXFLAGS) -c "
	    << stem << ".gen.cc\n";
	print << "\tmv " << basename(stem) << ".gen.$(OBJEXT) $@\n";
#endif
    }
    else if (file.ends_with(".cc"))
    {
	nstring stem(file.c_str(), file.size() - 3);
	nstring root(basename(stem));
	nstring dir(top_dir(file));

	nstring_list dep;
	read_dependency_file(file, dep);

	print << "\n";
	print << stem << ".$(OBJEXT): " << file << " " << dep << "\n";
	print << "\t$(CXX) -I" << dir << " -Ilibaegis -Icommon $(CXXFLAGS) -c "
	    << file << "\n";
	print << "\tmv " << root << ".$(OBJEXT) $@\n";
    }
    else if (file.gmatch("lib/icon2/*.uue"))
    {
        nstring rest(file.c_str() + 10, file.size() - 14);
	nstring tmp = "lib/icon/" + rest;
        print << "\n";
        print << tmp << ": " << file << " bin/test_base64\n";
        print << "\tbin/test_base64 -uu -i -nh " << file << " $@\n";
        print << "\n";
        print << "$(RPM_BUILD_ROOT)$(datadir)/icon/" << rest << ": " << tmp
	      << " " << dirname(tmp) << "/.mkdir.datadir\n";
        print << "\t$(INSTALL_DATA) " << tmp << " $@\n";
    }
    else if (file.gmatch("lib/icon/*.uue"))
    {
        nstring rest(file.c_str() + 9, file.size() - 13);
        nstring dir(dirname(file));
        print << "\n";
        print << "lib/icon/" << rest << ": " << file << " bin/test_base64\n";
        print << "\tbin/test_base64 -uu -i -nh " << file << " $@\n";
        print << "\n";
        print << "$(RPM_BUILD_ROOT)$(datadir)/icon/" << rest << ": lib/icon/"
	      << rest << " " << dir << "/.mkdir.datadir\n";
        print << "\t$(INSTALL_DATA) " << "lib/icon/" << rest << " $@\n";
    }
    else if (file.gmatch("lib/*.uue"))
    {
	// do nothing
    }
    else if (file == "lib/cshrc" || file == "lib/profile")
    {
	// should only be setting ${sharedstatedir}
	// so be inconsistent and install them in share
	nstring rest(basename(file));
	nstring dir(dirname(file));
	print << "\n";
	print << "$(RPM_BUILD_ROOT)$(datadir)/" << rest << ": " << file << " "
	    << dir << "/.mkdir.datadir\n";
	print << "\t$(INSTALL_SCRIPT) " << file << " $@\n";

	if (file == "lib/profile")
	{
	    print << "\n";
	    print << "$(RPM_BUILD_ROOT)$(sysconfdir)/profile.d/aegis.sh: "
		"$(RPM_BUILD_ROOT)$(datadir)/" << rest << "\n";
	    print << "\t-@mkdir -p $(RPM_BUILD_ROOT)$(sysconfdir)/profile.d\n";
	    print << "\t-ln -s $(datadir)/" << rest << " $@\n";
	}
	else
	{
	    print << "\n";
	    print << "$(RPM_BUILD_ROOT)$(sysconfdir)/profile.d/aegis.csh: "
		"$(RPM_BUILD_ROOT)$(datadir)/" << rest << "\n";
	    print << "\t-@mkdir -p $(RPM_BUILD_ROOT)$(sysconfdir)/profile.d\n";
	    print << "\t-ln -s $(datadir)/" << rest << " $@\n";
	}
    }
    else if (file.gmatch("lib/*.sh"))
    {
	nstring rest(trim_dir(file));
	nstring dir(dirname(file));
	print << "\n";
	print << "$(RPM_BUILD_ROOT)$(datadir)/" << rest << ": " << file << " "
	    << dir << "/.mkdir.datadir\n";
	print << "\t$(INSTALL_SCRIPT) " << file << " $@\n";
    }
    else if (file.gmatch("lib/*/libaegis.po"))
    {
	// obsolete
    }
    else if (file.gmatch("lib/*.po"))
    {
	//
	// Assume that we are using the GNU Gettext program.  All others
	// will fail, because they do not have the -o option.
	//
	nstring stem(file.c_str() + 4, file.size() - 7);
	nstring dir(dirname(file));
	print << "\n";
	print << "lib/" << stem << ".mo: etc/msgfmt.sh " << file << "\n";
	print << "\t$(SH) etc/msgfmt.sh --msgfmt=$(MSGFMT) --msgcat=$(MSGCAT) "
	    "--output=$@ " << file << "\n";
	print << "\n";
	print << "$(RPM_BUILD_ROOT)$(NLSDIR)/" << stem << ".mo: lib/" << stem
	    << ".mo " << dir << "/.mkdir.libdir\n";
	print << "\t$(INSTALL_DATA) lib/" << stem << ".mo $@\n";
    }
    else if (file.gmatch("lib/*/man[1-9]/*.[1-9]"))
    {
	nstring dir(dirname(file));
	nstring base(basename(file));
	nstring stem(trim_dir(file));
	nstring part(trim_dir(stem));
	nstring ugly(file.field('/', 2));

	nstring_list dep;
	read_dependency_file(file, dep);

	print << "\n";
	print << "$(RPM_BUILD_ROOT)$(datadir)/" << stem << ": " << file << " "
	    << dir << "/.mkdir.datadir " << dep << "\n";
	print << "\t$(SOELIM) -I" << dir << " -Ietc " << file
	    << " | sed '/^\\.lf/d' > $${TMPDIR-/tmp}/aegis.tmp\n";
	print << "\t$(INSTALL_DATA) $${TMPDIR-/tmp}/aegis.tmp $@\n";
	print << "\t@rm -f $${TMPDIR-/tmp}/aegis.tmp\n";

	if (file.field('/', 1) == "en")
	{
	    print << "\n";
	    print << "$(RPM_BUILD_ROOT)$(mandir)/" << part << ": " << file
		<< " " << dep << " ." << ugly << "dir\n";
	    print << "\t$(SOELIM) -I" << dir << " -Ietc " << file
		<< " | sed '/^\\.lf/d' > $${TMPDIR-/tmp}/aegis.tmp\n";
	    print << "\t$(INSTALL_DATA) $${TMPDIR-/tmp}/aegis.tmp $@\n";
	    print << "\t@rm -f $${TMPDIR-/tmp}/aegis.tmp\n";
	}
    }
    else if (file.gmatch("lib/*/man?/*"))
    {
	// do nothing
    }
    else if (file.gmatch("lib/*/*/*.so"))
    {
	// do nothing
    }
    else if (file.gmatch("lib/*/*/*.bib"))
    {
	// do nothing
    }
    else if (file.gmatch("lib/*/*/main.*"))
    {
	const char *cp = strrchr(file.c_str(), '.');
	nstring macros(cp ? cp + 1 : "");
	nstring stem(trim_dir(file));
	nstring dir(dirname(file));
	nstring dirdir(dirname(dir));

	nstring_list dep;
	read_dependency_file(file, dep);

	if (macros == "roff")
	    macros = "";
	else if (macros == "mm")
	    macros = "$(MM)";
	else if (macros == "ms")
	    macros = "$(MS)";
	else
	    macros = "-" + macros;
	nstring stem2(dirname(stem));
	nstring stem3(dirname(stem2));

	print << "\n";
	print << "lib/" << stem2 << ".ps: " << file << " " << dep << "\n";
	print << "\t$(SOELIM) -I" << dir << " -Ietc -I" << dirdir << "/man1 -I"
	    << dirdir << "/man5 -I" << dirdir << "/readme " << file
	    << " | $(GROFF) -R -t -p " << macros << " -mpic -mpspic > $@\n";

	print << "\n";
	print << "$(RPM_BUILD_ROOT)$(datadir)/" << stem2 << ".ps: lib/" << stem2
	    << ".ps lib/" << stem3 << "/.mkdir.datadir\n";
	print << "\t$(INSTALL_DATA) lib/" << stem2 << ".ps $@\n";

	print << "\n";
	print << "lib/" << stem2 << ".dvi: " << file << " " << dep << "\n";
	print << "\t$(SOELIM) -I" << dir << " -Ietc -I" << dirdir << "/man1 -I"
	    << dirdir << "/man5 -I" << dirdir << "/readme " << file
	    << " | $(GROFF) -Tdvi -R -t -p " << macros << " -mpic > $@\n";

	print << "\n";
	print << "$(RPM_BUILD_ROOT)$(datadir)/" << stem2 << ".dvi: lib/"
	    << stem2 << ".dvi lib/" << stem3 << "/.mkdir.datadir\n";
	print << "\t$(INSTALL_DATA) lib/" << stem2 << ".dvi $@\n";

	print << "\n";
	print << "lib/" << stem2 << ".txt: " << file << " " << dep << "\n";
	print << "\t-$(SOELIM) -I" << dir << " -Ietc -I" << dirdir << "/man1 -I"
	    << dirdir << "/man5 -I" << dirdir << "/readme " << file
	    << " | $(GROFF) -Tascii -R -t -p " << macros << " -mpic > $@\n";

	print << "\n";
	print << "$(RPM_BUILD_ROOT)$(datadir)/" << stem2 << ".txt: lib/"
	    << stem2 << ".txt lib/" << stem3 << "/.mkdir.datadir\n";
	print << "\t$(INSTALL_DATA) lib/" << stem2 << ".txt $@\n";
    }
    else if (file.starts_with("lib/"))
    {
	nstring stem(file.c_str() + 4, file.size() - 4);
	nstring dir(dirname(file));
	print << "\n";
	print << "$(RPM_BUILD_ROOT)$(datadir)/" << stem << ": " << file << " "
	    << dir << "/.mkdir.datadir\n";
	print << "\t$(INSTALL_DATA) " << file << " $@\n";
    }
    else if (file.ends_with(".def"))
    {
	nstring stem(file.c_str(), file.size() - 4);
	nstring root(basename(file));
	nstring dir(dirname(file));

	nstring_list dep;
	read_dependency_file(file, dep);

	print << "\n";
	print << stem << ".cc " << stem << ".h: " << file
	    << " bin/fmtgen$(EXEEXT) " << dep << "\n";
	print << "\tbin/fmtgen$(EXEEXT) -I" << dir << " " << file << " "
	    << stem << ".cc " << stem << ".h\n";

#if 0
	//
        // We don't need this bit.  The cookbook adds these to the list
        // of files given to us to process, which also ensures that the
        // dependency (,Md) file has been created.
	//
	dep.clear();
	read_dependency_file(stem + ".cc", dep);

	print << "\n";
	print << stem << ".$(OBJEXT): " << stem << ".cc " << dep << "\n";
	print << "\t$(CXX) -I" << dir << " -Ilibaegis -Icommon $(CXXFLAGS) -c "
	    << stem << ".cc\n";
	print << "\tmv " << basename(stem) << ".$(OBJEXT) $@\n";
#endif
    }
    else if (file.gmatch("test/*/*.sh"))
    {
	nstring stem(file.c_str(), file.size() - 3);
	print << "\n";
	print << stem << ".ES: " << file << " all-bin etc/test.sh\n";
	print << "\tCXX=\"$(CXX)\" $(SH) etc/test.sh -shell $(SH) -run "
	    << file << " " << stem << ".ES\n";
    }
    else if (file.gmatch("script/*.tcl"))
    {
	nstring root(basename(file, ".tcl"));
	print << "\n";
	print << "bin/" << root << ": " << file << "\n";
	print << "\tcp " << file << " bin/" << root << "\n";
	print << "\tchmod a+rx bin/" << root << "\n";
    }
}


void
process_body::directory(const char *arg)
{
    search_path.push_back(arg);
}
