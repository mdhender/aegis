//
// aegis - project change supervisor
// Copyright (C) 2008-2012 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/output/filter/set_width.h>
#include <libaegis/output/filter/wrap_make.h>

#include <aemakegen/target/debian.h>


bool
target_debian::redundant_license_file(const nstring &fn)
{
    //
    // Debian policy is to avoid redundant license files.
    //
    nstring_list components;
    components.split(fn, "/");
    if (components.size() < 2)
        return false;
    size_t j = components.size() - 2;
    nstring d1 = components[j];
    if (!d1.gmatch("man[1-8]"))
        return false;
    nstring section = d1.substr(3, 1);
    nstring d2 = components[j + 1];
    return
        (
            d2.gmatch("*[-_.]license." + section)
        ||
            d2.gmatch("*[-_.]copyright." + section)
        );
}


void
target_debian::gen_rules(void)
{
    nstring source_name = get_project_name();
    nstring bin_name =
        (
            source_name.starts_with("lib")
        ?
            source_name.substr(3, source_name.size() - 3)
        :
            source_name
        );
    nstring lib_root_name =
        (source_name.starts_with("lib") ? source_name : "lib" + source_name);
    nstring runtime_name = lib_root_name + data.get_version_info_major();
    nstring dbg_name = runtime_name + "-dbg";
    nstring dev_name = lib_root_name + "-dev";
    nstring doc_name = source_name + "-doc";

    // write debian/rules
    os_become_orig();
    op = output_file::open("debian/rules");
    os_become_undo();
    op = output_filter_set_width::create(op, 80);
    op = output_filter_wrap_make::create(op);
    op->fputs
    (
        "#!/usr/bin/make -f\n"
        "\n"
        "# Uncomment this to turn on verbose mode.\n"
        "#export DH_VERBOSE=1\n"
        "\n"
        "CFLAGS = -Wall -g\n"
        "ifneq (,$(findstring noopt,$(DEB_BUILD_OPTIONS)))\n"
        "CFLAGS += -O0\n"
        "else\n"
        "CFLAGS += -O2\n"
        "endif\n"
        "\n"
    );

    {
        nstring_list lhs;
        lhs.push_back("config.status");
        nstring_list rhs;
        rhs.push_back("configure");
        nstring_list body;
        body.push_back("dh_testdir");
        nstring command =
            "sh configure"
            " --prefix=/usr"
            " --localstatedir=/var"
            " --sysconfdir=/etc"
            ;
        if (data.seen_install_mandir())
            command += " --mandir=/usr/share/man";
        command +=
            " CFLAGS=\"$(CFLAGS)\" "
            " LDFLAGS=\"-Wl,-z,defs\""
            ;
        body.push_back(command);
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

    //
    // Refer to Debian Policy Manual section 4.9 (Main building script:
    // debian/rules) for details.
    //
    // According to Lintian, the following form is recommended:
    //
    //     build: build-arch build-indep
    //     build-arch: build-stamp
    //     build-indep: build-stamp
    //     build-stamp:
    //         blah blah
    //
    {
        nstring_list lhs;
        lhs.push_back("build");
        nstring_list rhs;
        rhs.push_back("build-arch");
        rhs.push_back("build-indep");
        nstring_list body;
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }
    location_comment(__FILE__, __LINE__);
    print_rule("build-arch", "build-stamp");
    location_comment(__FILE__, __LINE__);
    print_rule("build-indep", "build-stamp");

    print_comment("Build and test the tarball.");
    {
        nstring_list lhs;
        lhs.push_back("build-stamp");
        nstring_list rhs;
        rhs.push_back("config.status");
        nstring_list body;
        body.push_back("dh_testdir");
        body.push_back("$(MAKE)");
        if (data.seen_test_files())
            body.push_back("$(MAKE) check");
        body.push_back("touch $@");
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

    print_comment
    (
        "dpkg-buildpackage (step 3) invokes 'fakeroot debian/rules "
        "clean', and after that (step 5) does the actual build."
    );
    {
        nstring_list lhs;
        lhs.push_back("clean");
        nstring_list rhs;
        nstring_list body;
        body.push_back("dh_testdir");
        body.push_back("dh_testroot");
        body.push_back("rm -f build-stamp");
        body.push_back("test ! -f Makefile || $(MAKE) distclean");
        body.push_back("dh_clean");
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

    print_comment
    (
        "Install the built tarball into the temporary install tree.  It "
        "depends on the 'build' target, so the tarball is also built by this "
        "rule."
    );
    {
        nstring_list lhs;
        lhs.push_back("install");
        nstring_list rhs;
        rhs.push_back("build");
        nstring_list body;
        body.push_back("dh_testdir");
        body.push_back("dh_testroot");
        body.push_back("dh_clean -k");
        body.push_back("dh_installdirs -A --list-missing");
        body.push_back("mkdir -p $(CURDIR)/debian/tmp/usr/lib");
        body.push_back("mkdir -p $(CURDIR)/debian/tmp/usr/share");
        body.push_back("$(MAKE) DESTDIR=$(CURDIR)/debian/tmp install");
        if (data.use_libtool() && !data.use_lib_la_files())
        {
            // The debian folk take exception to the "almost always useless"
            // *.la files install by libtool.
            body.push_back("-rm $(CURDIR)/debian/tmp/usr/lib/*.la");
        }

        //
        // Debian don't like you installing redundant license files.
        //
        // Note: you must remove these files here, and you will also
        // have to do it in aemakegen/target/debian/control.cc as well.
        //
        if (data.seen_install_mandir())
        {
            binary_package_files.sort();
            for (size_t j = 0; j < binary_package_files.size(); ++j)
            {
                nstring fn = binary_package_files[j];
                fn = expand_make_macro(fn);
                if (redundant_license_file(fn))
                {
                    body.push_back("-rm -f $(CURDIR)/debian/tmp/" + fn);
                }
            }
            developer_package_files.sort();
            for (size_t j = 0; j < developer_package_files.size(); ++j)
            {
                nstring fn = developer_package_files[j];
                fn = expand_make_macro(fn);
                if (redundant_license_file(fn))
                {
                    body.push_back("-rm -f $(CURDIR)/debian/tmp/" + fn);
                }
            }
        }

        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

    //
    // The dpkg-buildpackage command calls
    // 5. debian/rules build
    //    followed by
    //    fakeroot debian/rules binary
    //
    print_comment("Build the binary package files here.");
    {
        nstring_list lhs;
        lhs.push_back("binary"); // dpkg-buildpackage -b (or default)
        lhs.push_back("binary-arch"); // dpkg-buildpackage -B
        lhs.push_back("binary-indep"); // dpkg-buildpackage -A
        nstring_list rhs;
        rhs.push_back("install");
        nstring_list body;
        body.push_back("dh_testdir");
        body.push_back("dh_testroot");
        body.push_back("dh_installchangelogs");
        body.push_back("dh_installdocs -A");
        // body.push_back("dh_installexamples");
        body.push_back("dh_install --fail-missing --sourcedir=debian/tmp");
        // body.push_back("dh_installmenu");
        // body.push_back("dh_installdebconf");
        // body.push_back("dh_installlogrotate");
        // body.push_back("dh_installemacsen");
        // body.push_back("dh_installpam");
        // body.push_back("dh_installmime");
        // body.push_back("dh_python");
        // body.push_back("dh_installinit");
        // body.push_back("dh_installcron");
        // body.push_back("dh_installinfo");
        if (data.seen_install_mandir())
            body.push_back("dh_installman -A");
        // body.push_back("dh_link -A");
        if (developer_package_files.empty())
            body.push_back("dh_strip");
        else
            body.push_back("dh_strip --dbg-package=" + dbg_name);
        body.push_back("dh_compress -A");
        body.push_back("dh_fixperms");
        // body.push_back("dh_perl");
        if (!runtime_package_files.empty())
        {
            body.push_back("dh_makeshlibs");
        }
        body.push_back("dh_shlibdeps");
        body.push_back("dh_installdeb");
        body.push_back("dh_gencontrol");
        body.push_back("dh_md5sums");
        body.push_back("dh_builddeb");

        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

    {
        nstring_list lhs;
        lhs.push_back(".PHONY");
        nstring_list rhs;
        rhs.push_back("binary");
        rhs.push_back("binary-arch");
        rhs.push_back("binary-indep");
        rhs.push_back("build");
        rhs.push_back("build-arch");
        rhs.push_back("build-indep");
        rhs.push_back("clean");
        rhs.push_back("install");
        nstring_list body;
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }
    print_comment("vi" "m: set ts=8 sw=8 noet :");
    op.reset();

    // make sure it is executable
    os_become_orig();
    os_chmod("debian/rules", 0755);
    os_become_undo();
}


// vim: set ts=8 sw=4 et :
