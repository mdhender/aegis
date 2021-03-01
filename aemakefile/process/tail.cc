//
// aegis - project change supervisor
// Copyright (C) 2004-2006, 2008, 2012 Peter Miller
// Copyright (C) 2007, 2008 Walter Franzini
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>

#include <aemakefile/process/tail.h>


process_tail::~process_tail()
{
}


process_tail::process_tail(printer &arg) :
    process(arg)
{
    dir_st.set_reaper();

    clean_files.push_back("core");
    clean_files.push_back("bin/fmtgen$(EXEEXT)");
    clean_files.push_back("libaegis/libaegis.$(LIBEXT)");
    clean_files.push_back("common/common.$(LIBEXT)");
    clean_files.push_back(".bin");
    clean_files.push_back(".bindir");
    clean_files.push_back(".po_files");
    clean_files.push_back(".man1dir");
    clean_files.push_back(".man3dir");
    clean_files.push_back(".man5dir");
    clean_files.push_back(".comdir");

    common_files = new nstring_list;
    dir_st.assign("common", common_files);

    libaegis_files = new nstring_list;
    dir_st.assign("libaegis", libaegis_files);

    datadir_files.push_back(
        "$(DESTDIR)$(sysconfdir)/profile.d/aegis.sh");
    datadir_files.push_back(
        "$(DESTDIR)$(sysconfdir)/profile.d/aegis.csh");
}


static nstring
dirname(const nstring &s)
{
    const char *cp = s.c_str();
    const char *ep = strrchr(cp, '/');
    if (!ep)
        return ".";
    return nstring(cp, ep - cp);
}



//
// Create directories and any necessary parent directories.
//
void
process_tail::recursive_mkdir(const nstring &src_dir_arg,
    const nstring &dst_dir_arg, const nstring &dir_suffix)
{
    nstring src_dir(src_dir_arg);
    nstring dst_dir(dst_dir_arg);
    static int defined;
    static symtab<int> dir_exists;
    for (;;)
    {
        nstring dirvar = "mkdir." + src_dir + "." + dir_suffix;
        nstring dotdot = dirname(src_dir);

        if (!dir_exists.query(dirvar))
        {
            dir_exists.assign(dirvar, &defined);
            print << "\n";
            if (dotdot != ".")
            {
                print << src_dir << "/.mkdir." << dir_suffix << ": "
                    << dotdot << "/.mkdir." << dir_suffix << "\n";
            }
            else
            {
                print << src_dir << "/.mkdir." << dir_suffix << ":\n";
            }
            print << "\t-$(INSTALL) -m 0755 -d " << dst_dir << "\n";
            print << "\t-@touch $@\n";
            print << "\t@sleep 1\n";
            clean_files.push_back(src_dir + "/.mkdir." + dir_suffix);
        }
        src_dir = dotdot;
        if (src_dir == ".")
            break;
        dst_dir = dirname(dst_dir);
    }
}


void
process_tail::per_file(const nstring &filename)
{
    nstring file(filename);
    if (!file.starts_with("script/") && file.ends_with(".in"))
    {
        file = nstring(file.c_str(), file.size() - 3);
        clean_files.push_back_unique(file);
    }

    else if (file.starts_with("script/"))
    {
        if (file.ends_with(".in"))
        {
            nstring name = nstring(file.c_str(), file.size() - 3);
            clean_files.push_back_unique(name);
        }
        nstring name = file.trim_extension().basename();
        if (name != "aegis.synpic" && name != "ae-symlinks")
        {
            commands_bin.push_back("bin/" + name + "$(EXEEXT)");
            clean_files.push_back("bin/" + name + "$(EXEEXT)");
            if (!name.starts_with("test_"))
            {
                nstring install_name = "$(DESTDIR)$(bindir)/"
                    "$(PROGRAM_PREFIX)" + name + "$(PROGRAM_SUFFIX)$(EXEEXT)";
                commands_install.push_back(install_name);
            }
        }
    }
    else if (file.ends_with("/main.cc"))
    {
        nstring name(file.field('/', 0));
        commands.push_back(name);
        commands_bin.push_back("bin/" + name + "$(EXEEXT)");

        if
        (
            name != "aefp"
        &&
            name != "fmtgen"
        &&
            name != "aemakefile"
        &&
            !name.starts_with("test_")
        )
        {
            commands_install.push_back(
                "$(DESTDIR)$(bindir)/$(PROGRAM_PREFIX)" + name +
                "$(PROGRAM_SUFFIX)$(EXEEXT)");
        }
    }

    if (file.ends_with(".cc"))
    {
        nstring dir(file.field('/', 0));
        nstring_list *dir_p = dir_st.query(dir);
        if (!dir_p)
        {
            dir_p = new nstring_list;
            dir_st.assign(dir, dir_p);
        }
        nstring stem(file.c_str(), file.size() - 3);
        nstring obj(stem + ".$(OBJEXT)");
        dir_p->push_back(obj);
        clean_files.push_back(obj);
    }
    else if (file.ends_with(".def"))
    {
        nstring dir(file.field('/', 0));
        nstring_list *dir_p = dir_st.query(dir);
        if (!dir_p)
        {
            dir_p = new nstring_list;
            dir_st.assign(dir, dir_p);
        }
        nstring stem(file.c_str(), file.size() - 4);
        nstring obj(stem + ".$(OBJEXT)");
        dir_p->push_back(obj);
        clean_files.push_back(obj);
        clean_files.push_back(stem + ".cc");
        clean_files.push_back(stem + ".h");
    }
    else if (file.ends_with(".y"))
    {
        nstring dir(file.field('/', 0));
        nstring_list *dir_p = dir_st.query(dir);
        if (!dir_p)
        {
            dir_p = new nstring_list;
            dir_st.assign(dir, dir_p);
        }
        nstring stem(file.c_str(), file.size() - 2);
        nstring obj(stem + ".yacc.$(OBJEXT)");
        dir_p->push_back(obj);
        clean_files.push_back(stem + ".yacc.cc");
        clean_files.push_back(stem + ".yacc.h");
        clean_files.push_back(obj);
    }
    else if (file.starts_with("lib/") && file.ends_with("/libaegis.po"))
    {
        // obsolete
    }
    else if (file.starts_with("lib/") && file.ends_with(".po"))
    {
        nstring stem = nstring(file.c_str() + 4, file.size() - 7);
        nstring src("lib/" + stem + ".mo");
        po_files.push_back(src);
        clean_files.push_back(src);
        nstring dst("$(DESTDIR)$(NLSDIR)/" + stem + ".mo");
        install_po_files.push_back(dst);
        recursive_mkdir(dirname(src), dirname(dst), "libdir");
    }
    else if (file.gmatch("lib/*.so"))
    {
        // documentation include file
    }
    else if (file.gmatch("lib/*.bib"))
    {
        // documentation include file
    }
    else if (file.gmatch("lib/icon2/*.uue"))
    {
        nstring stem(file.c_str() + 10, file.size() - 14);
        nstring tmp = "lib/icon/" + stem;
        nstring dst = "$(DESTDIR)$(datadir)/icon/" + stem;
        datadir_files.push_back(dst);
        recursive_mkdir(dirname(tmp), dirname(dst), "datadir");
        clean_files.push_back(tmp);
    }
    else if (file.gmatch("lib/icon/*.uue"))
    {
        nstring stem(file.c_str() + 9, file.size() - 13);
        nstring dst = "$(DESTDIR)$(datadir)/icon/" + stem;
        datadir_files.push_back(dst);
        recursive_mkdir(dirname(file), dirname(dst), "datadir");
        clean_files.push_back("lib/icon/" + stem);
    }
    else if (file.gmatch("lib/*.uue"))
    {
        // do nothing
    }
    else if (file.gmatch("lib/*/man[1-9]/*.[1-9]"))
    {
        nstring stem = nstring(file.c_str() + 4, file.size() - 4);
        install_doc_files.push_back("$(DESTDIR)$(datadir)/" + stem);
        nstring src(file);
        nstring dst("$(DESTDIR)$(datadir)/" + stem);
        recursive_mkdir(dirname(src), dirname(dst), "datadir");
        if (file.gmatch("lib/en/*"))
        {
            nstring part = nstring(file.c_str() + 7, file.size() - 7);
            man_files.push_back("$(DESTDIR)$(mandir)/" + part);
        }
    }
    else if (file.gmatch("lib/*/*/main.*"))
    {
        nstring stem = file.field('/', 1) + "/" + file.field('/', 2);
        ps_doc_files.push_back("lib/" + stem + ".ps");
        dvi_doc_files.push_back("lib/" + stem + ".dvi");
        txt_doc_files.push_back("lib/" + stem + ".txt");
        clean_files.push_back("lib/" + stem + ".ps");
        clean_files.push_back("lib/" + stem + ".dvi");
        clean_files.push_back("lib/" + stem + ".txt");
        install_doc_files.push_back("$(DESTDIR)$(datadir)/" + stem
            + ".ps");
        install_doc_files.push_back("$(DESTDIR)$(datadir)/" + stem
            + ".txt");
        nstring src("lib/" + stem + ".ps");
        nstring dst("$(DESTDIR)$(datadir)/" + stem + ".ps");
        recursive_mkdir(dirname(src), dirname(dst), "datadir");
    }
    else if (file.starts_with("lib/"))
    {
        nstring rest(file.c_str() + 4, file.size() - 4);
        nstring dst("$(DESTDIR)$(datadir)/" + rest);
        datadir_files.push_back(dst);
        recursive_mkdir(dirname(file), dirname(dst), "datadir");
    }
    else if (file.gmatch("test/*/*.sh"))
    {
        nstring stem(file.c_str(), file.size() - 3);
        test_files.push_back(stem + ".ES");
        clean_files.push_back(stem + ".ES");
    }
}


void
process_tail::postlude()
{
    commands_bin.sort();
    print << "\n";
    print << "all-bin: " << commands_bin << "\n";

    for (size_t j = 0; j < commands.size(); ++j)
    {
        nstring name(commands[j]);
        nstring_list *dir_p = dir_st.query(name);
        if (!dir_p)
            continue;
        print << "\n";
        print << name << "_files = " << *dir_p << "\n";
        print << "\n";

        if
        (
            name == "aemakefile"
        ||
            name == "aemeasure"
        ||
            name == "fmtgen"
        )
        {
            print << "bin/" << name << "$(EXEEXT): $(" << name
                << "_files) common/common.$(LIBEXT) .bin\n"
                << "\t@sleep 1\n"
                << "\t$(CXX) $(LDFLAGS) -o $@ $(" << name
                    << "_files) common/common.$(LIBEXT) $(LIBS)\n"
                << "\t@sleep 1\n";
        }
        else
        {
            print << "bin/" << name << "$(EXEEXT): $(" << name
                << "_files) libaegis/libaegis.$(LIBEXT) .bin\n";
            print << "\t@sleep 1\n";
            print << "\t$(CXX) $(LDFLAGS) -o $@ $(" << name
                << "_files) libaegis/libaegis.$(LIBEXT) $(LIBS)\n";
            if (name == "aegis" || name == "aeimport" || name == "aelock")
                print << "\t-chown root $@ && chmod 4755 $@\n";
            print << "\t@sleep 1\n";
        }

        print << "\n";
        print << "$(DESTDIR)$(bindir)/$(PROGRAM_PREFIX)" << name
            << "$(PROGRAM_SUFFIX)$(EXEEXT): bin/" << name
            << "$(EXEEXT) .bindir\n";
        print << "\t$(INSTALL_PROGRAM) bin/" << name << "$(EXEEXT) $@\n";
        if (name == "aegis" || name == "aeimport" || name == "aelock")
            print << "\t-chown root $@ && chmod 4755 $@\n";
    }

    print << "\nCommonFiles = " << *common_files << "\n";
    print << "\nlibaegis_obj = " << *libaegis_files << " " << *common_files
        << "\n";
    print << "\nLibFiles = " << libdir_files << "\n";
    print << "\nDataFiles = " << datadir_files << "\n";
    print << "\ninstall-man-yes: " << man_files << "\n";

    print << "\n"
        "uninstall-man:\n"
        "\trm -f " << man_files << "\n";

    print << "\ninstall-man-no:\n";
    print << "\npo_files_yes: " << po_files << "\n";
    print << "\npo_files_no:\n";
    print << "\ninstall-po-yes: " << install_po_files << "\n";

    print << "\n"
        "uninstall-po:\n"
        "\trm -f " << install_po_files << "\n";

    print << "\ninstall-po-no:\n";
    print << "\ndvi-doc-files: " << dvi_doc_files << "\n";
    print << "\ndoc_files_yes: " << ps_doc_files << " " << txt_doc_files
        << "\n";
    print << "\ndoc_files_no:\n";
    print << "\ninstall-doc-yes: " << install_doc_files << "\n";

    print << "\n"
        "uninstall-doc:\n"
        "\trm -f " << install_doc_files << "\n";

    print << "\ninstall-doc-no:\n";
    print << "\nTestFiles = " << test_files << "\n";

    //
    // clean up the area
    //  (make sure command lines do not get too long)
    //
    print << "\nclean-obj:\n";
    clean_files.sort();
    for (size_t m = 0; m < clean_files.size(); ++m)
        print << "\trm -f " << clean_files[m] << "\n";

    print << "\nclean: clean-obj\n";
    print << "\trm -f " << commands_bin << "\n";

    commands_install.sort();
    print << "\ninstall-bin: " << commands_install << "\n";
    print << "\n"
        "uninstall-bin:\n"
        "\trm -f " << commands_install << "\n";

    print << "\n"
        ".bindir:\n"
        "\t-$(INSTALL) -m 0755 -d $(DESTDIR)$(bindir)\n"
        "\t-@touch $@\n"
        "\t@sleep 1\n"
        "\n"
        ".man1dir:\n"
        "\t-$(INSTALL) -m 0755 -d $(DESTDIR)$(mandir)/man1\n"
        "\t-@touch $@\n"
        "\t@sleep 1\n"
        "\n"
        ".man3dir:\n"
        "\t-$(INSTALL) -m 0755 -d $(DESTDIR)$(mandir)/man3\n"
        "\t-@touch $@\n"
        "\t@sleep 1\n"
        "\n"
        ".man5dir:\n"
        "\t-$(INSTALL) -m 0755 -d $(DESTDIR)$(mandir)/man5\n"
        "\t-@touch $@\n"
        "\t@sleep 1\n"
        "\n"
        ".comdir:\n"
        "\t-$(INSTALL) -m 0755 -d $(DESTDIR)$(comdir)\n"
        "\t-chown $(AEGIS_UID) $(DESTDIR)$(comdir) && "
            "chgrp $(AEGIS_GID) $(DESTDIR)$(comdir)\n"
        "\t$(SH) etc/compat.2.3\n"
        "\t-@touch $@\n"
        "\t@sleep 1\n"
        "\n"
        ".po_files: po_files_$(HAVE_MSGFMT)\n"
        "\t@touch $@\n"
        "\n"
        ".doc_files: doc_files_$(HAVE_GROFF)\n"
        "\t@touch $@\n"
        "\n"
        "distclean: clean\n"
        "\trm -f config.status config.log config.cache\n"
        "\trm -f Makefile common/config.h etc/howto.conf\n"
        "\trm -f lib/cshrc lib/profile etc/libdir.so\n"
        "\n"
        ".bin:\n"
        "\t-mkdir bin\n"
        "\t-@touch $@\n"
        "\n"
        "common/common.$(LIBEXT): $(CommonFiles)\n"
        "\trm -f $@\n"
        "\t$(AR) qc $@ $(CommonFiles)\n"
        "\t$(RANLIB) $@\n"
        "\n"
        "libaegis/libaegis.$(LIBEXT): $(libaegis_obj)\n"
        "\trm -f $@\n"
        "\t$(AR) qc $@ $(libaegis_obj)\n"
        "\t$(RANLIB) $@\n"
        "\n"
        "sure: $(TestFiles) etc/test.sh\n"
        "\t@$(SH) etc/test.sh -summary $(TestFiles)\n"
        "\n"
        "#\n"
        "# This target is used when preparing for the second\n"
        "# pass of testing, when aegis is set-uid-root.\n"
        "#\n"
        "install-libdir: lib/.mkdir.datadir lib/.mkdir.libdir .comdir "
            "install-po\n"
        "\t-chown root bin/aegis$(EXEEXT) "
            "&& chmod 4755 bin/aegis$(EXEEXT)\n"
        "\t-chown root bin/aeimport$(EXEEXT) "
            "&& chmod 4755 bin/aeimport$(EXEEXT)\n"
        "\n"
        "install-lib: $(LibFiles) $(DataFiles) .comdir\n"
        "\n"
        "uninstall-lib:\n"
        "\trm -f $(LibFiles) $(DataFiles)\n"
        "\n"
        "install-po: install-po-$(HAVE_MSGFMT)\n"
        "\n"
        "install-man: install-man-$(HAVE_GROFF)\n"
        "\n"
        "install-doc: install-doc-$(HAVE_GROFF)\n"
        "\n"
        "install: install-bin install-lib install-po install-man install-doc\n"
        "\n"
        "uninstall: uninstall-bin uninstall-lib uninstall-po uninstall-man "
            "uninstall-doc\n";
}


// vim: set ts=8 sw=4 et :
