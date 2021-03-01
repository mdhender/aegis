//
// aegis - project change supervisor
// Copyright (C) 2010-2012 Peter Miller
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

#include <common/ac/assert.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <common/sizeof.h>
#include <libaegis/output/filter/wrap_simple.h>
#include <libaegis/output/filter/wrap_make.h>

#include <aemakegen/target/rpm-spec.h>
#include <aemakegen/util.h>


target_rpm_spec::~target_rpm_spec()
{
}


target_rpm_spec::target_rpm_spec(change_identifier &a_cid) :
    target(a_cid)
{
}


target_rpm_spec::pointer
target_rpm_spec::create(change_identifier &a_cid)
{
    return pointer(new target_rpm_spec(a_cid));
}


void
target_rpm_spec::process3_begin(void)
{
    // FIXME: look up whether or not RPM .spec files understand "\\\n" sequence

    warning_this_file_is_generated();

    nstring summary =
        get_cp()->pconf_attributes_find("aemakegen:rpm-spec:summary");
    if (summary.empty())
        summary = get_project_name();
    op->fputs
    (
        "Summary: " + summary + "\n"
        "Name: " + get_project_name() + "\n"
        "Version: " + get_cp()->version_get() + "\n"
        "Release: 1\n"
    );

    nstring license =
        get_cp()->pconf_attributes_find("aemakegen:rpm-spec:license");
    if (license.empty())
        license = "GPL";
    op->fputs("License: " + license + "\n");

    nstring group = get_cp()->pconf_attributes_find("aemakegen:rpm-spec:group");
    if (group.empty())
        group = "Development/Tools";
    op->fputs("Group: " + group + "\n");

    nstring homepage =
        get_cp()->pconf_attributes_find("aemakegen:debian:homepage");
    if (homepage.empty())
        homepage = "http://" + get_project_name() + ".sourceforge.net/";
    if (!homepage.ends_with("/"))
        homepage += "/";
    op->fputs
    (
        "Source: " + homepage + "%{name}-%{version}.tar.gz\n"
        "URL: " + homepage + "\n"
        "BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-"
            "%(%{__id_u} -n)\n"
    );
}


static bool
devel_section(const nstring &filename)
{
    if (filename.empty())
        return false;
    unsigned char c = filename[filename.size() - 1];
    switch (c)
    {
    case '2':
    case '3':
    case '4':
        return true;
    }
    return false;
}


static void
insert_directories(nstring_list &files)
{
    nstring_list dirs;
    for (size_t j = 0; j < files.size(); ++j)
    {
        nstring path = files[j];
        for (;;)
        {
            nstring dir = path.dirname();
            if (dir == "." || dir == "/")
                break;
            if (!strchr(dir.c_str(), '/'))
                break;
            dirs.push_back_unique("%dir " + dir);
            path = dir;
        }
    }
    dirs.sort();
    dirs.push_back(files);
    files = dirs;
}


static nstring
translate_make_macro(const nstring &text)
{
    struct table_t
    {
        const char *from;
        const char *to;
    };
    static const table_t table[] =
    {
        { "$(bindir)",      "%{_bindir}"      },
        { "$(datadir)",     "%{_datadir}"     },
        { "$(datarootdir)", "%{_datarootdir}" },
        { "$(exec_prefix)", "%{_prefix}"      },
        { "$(EXEEXT)",      ""                },
        { "$(includedir)",  "%{_includedir}"  },
        { "$(libdir)",      "%{_libdir}"      },
        { "$(localedir)",   "%{_localedir}"   },
        { "$(mandir)",      "%{_mandir}"      },
        { "$(NLSDIR)",      "%{_localedir}"   },
        { "$(prefix)",      "%{_prefix}"      },
        { "$(sysconfdir)",  "%{_sysconfdir}"  },
    };

    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        const char *cp = strstr(text.c_str(), tp->from);
        if (cp)
        {
            size_t s1 = cp - text.c_str();
            size_t s2 = strlen(tp->from);
            size_t s3 = text.size() - (s1 + s2);

            // There are two choices here.
#if 1
            // This choice does not permit recursive substitutions
            // and is thus slightly faster than the next choice.
            return
                (
                    text.substr(0, s1)
                +
                    tp->to
                +
                    translate_make_macro(text.substr(s1 + s2, s3))
                );
#else
            // This choice permits recursive substitutions (potential
            // for infinite recursion, though not this specific case).
            return
                (
                    text.substr(0, s1)
                +
                    translate_make_macro(tp->to + text.substr(s1 + s2, s3))
                );
#endif
        }
    }

    const char *cp = strchr(text.c_str(), '$');
    if (cp)
    {
        const char *cp2 = strchr(cp, '/');
        if (!cp2)
            cp2 = cp + strlen(cp);
        error_raw
        (
            "make macro %s not understood",
            nstring(cp, cp2 - cp).quote_c().c_str()
        );
        this_is_a_bug();
        // NOTREACHED
    }

    return text;
}


void
wrap_and_print(const output::pointer &fp, const nstring &text)
{
    output::pointer op = output_filter_wrap_simple::create(fp);
    op->fputs(text);
}


void
target_rpm_spec::process3_end(void)
{
    nstring_list build_depends_list;
    nstring build_depends =
        get_cp()->pconf_attributes_find("aemakegen:rpm-spec:build-requires");
    if (build_depends.empty())
        build_depends =
            get_cp()->pconf_attributes_find("aemakegen:debian:build-depends");
    if (!build_depends.empty())
        build_depends_list.split(build_depends, ",", true);
    if (data.use_libtool())
        build_depends_list.push_back_unique("libtool");
    if (data.have_groff())
        build_depends_list.push_back_unique("groff");
    if (data.need_yacc())
        build_depends_list.push_back_unique("bison");
    if (data.need_lex())
        build_depends_list.push_back_unique("flex");
    build_depends_list.push_back_unique("diffutils");
    build_depends_list.push_back_unique("sharutils");
    if (data.need_ghostscript())
        build_depends_list.push_back_unique("ghostscript");
    build_depends_list.sort();
    {
        int column = 0;
        for (size_t j = 0; j < build_depends_list.size(); ++j)
        {
            nstring pkg = build_depends_list[j];
            if (pkg.empty())
                continue;
            if (column && column + 2 + pkg.size() > 80)
            {
                op->fputc('\n');
                column = 0;
            }
            if (!column)
            {
                op->fputs("BuildPrereq:");
                column = 12;
            }
            else
            {
                op->fputc(',');
                ++column;
            }
            op->fputc(' ');
            ++column;
            op->fputs(pkg);
            column += pkg.size();
        }
        if (column)
            op->fputc('\n');
    }

    nstring_list files_libs;
    nstring_list files_devel;
    if (data.use_libtool())
    {
        nstring libname = "%{_libdir}/" + get_library_libname();
        files_libs.push_back_unique(libname + ".so.*");
        files_devel.push_back_unique(libname + ".so");
        files_devel.push_back_unique(libname + ".a");
        if (data.use_lib_la_files())
        {
            // The debian folk take exception to the "almost
            // always useless" *.la files installed by libtool.
            files_devel.push_back_unique(libname + ".la");
        }

        files_devel.push_back_unique(data.get_install_include());

        // and the devel man pages
        const nstring_list &mans = data.get_install_mandir();
        for (size_t j = 0; j < mans.size(); ++j)
        {
            nstring fn = mans[j];
            if (devel_section(fn))
                files_devel.push_back_unique(fn);
        }
    }

    // the "*.pc" and "*.la" files go in devel
    {
        const nstring_list &files = data.get_install_libdir();
        for (size_t j = 0; j < files.size(); ++j)
        {
            nstring fn = files[j];
            if (fn.ends_with(".pc"))
            {
                files_devel.push_back_unique(fn);
            }
            else if (fn.ends_with(".la"))
            {
                if (data.use_lib_la_files())
                {
                    // The debian folk take exception to the "almost
                    // always useless" *.la files installed by libtool.
                    files_devel.push_back_unique(fn);
                }
            }
        }
    }

    nstring description =
        get_cp()->pconf_attributes_find
        (
            "aemakegen:rpm-spec:description:" + get_project_name()
        );
    if (description.empty())
    {
        description =
            get_cp()->pconf_attributes_find("aemakegen:rpm-spec:description");
    }
    if (description.empty())
    {
        description =
            get_cp()->pconf_attributes_find
            (
                "aemakegen:debian:description:" + get_project_name()
            );
    }
    if (description.empty())
        description = get_project_name();

    nstring description_libs =
        get_cp()->pconf_attributes_find
        (
            "aemakegen:rpm-spec:description:" + get_project_name() + "-libs"
        );
    if (description_libs.empty())
    {
        description_libs =
            "This package contains the shared libraries for "
            "applications that " + description;
    }

    op->fputs("\n");
    op->fputs("%description\n");
    if (!files_libs.empty())
    {
        wrap_and_print(op, description_libs);
        op->fputs
        (
            "\n"
            "%package bin\n"
            "Summary: " + get_project_name() + " programs\n"
            "\n"
            "%description bin\n"
        );
    }
    wrap_and_print(op, description);

    nstring group = get_cp()->pconf_attributes_find("aemakegen:rpm-spec:group");
    if (group.empty())
        group = "Development/Tools";

    if (!files_libs.empty())
    {
        op->fputs
        (
            "\n"
            "%package libs" + data.get_version_info_major() + "\n"
            "Summary: " + get_project_name() + " libraries\n"
            "Group: " + group + "\n"
            "\n"
            "%description libs" + data.get_version_info_major() + "\n"
        );
        wrap_and_print(op, description_libs);
    }

    if (!files_devel.empty())
    {
        op->fputs
        (
            "\n"
            "%package devel\n"
            "Summary: " + get_project_name() + " development files\n"
            "Group: " + group + "\n"
            "Requires: " + get_project_name() + "-libs%{?_isa} = "
            "%{?epoch:%{epoch}:}%{version}-%{release}\n"
            "\n"
            "%description devel\n"
        );

        nstring description_devel =
            get_cp()->pconf_attributes_find
            (
               "aemakegen:rpm-spec:description:" + get_project_name() + "-devel"
            );
        if (description_devel.empty())
        {
            description_devel =
                "This package contains static libraries and header files "
                "for applications that " + description;
        }
        wrap_and_print(op, description_devel);

        op->fputs
        (
            "\n"
            "%post\n"
            "/sbin/ldconfig\n"
            "\n"
            "%postun\n"
            "/sbin/ldconfig\n"
        );
    }

    op->fputs
    (
        "\n"
        "\n"
        "%prep\n"
        "%setup -q\n"
    );
    {
        output::pointer op2 = output_filter_wrap_make::create(op);
        op2->fputs
        (
            "\n"
            "\n"
            "%build\n"
            "%configure"
                " --sysconfdir=/etc"
                " --prefix=%{_prefix}"
        );
        if (data.seen_install_mandir())
            op2->fputs(" --mandir=%{_mandir}");
        if (data.seen_install_i18n())
            op2->fputs(" --with-nlsdir=%{_datadir}/locale");
        op2->fputs("\nmake\n");
    }
    op->fputs
    (
        "\n"
        "\n"
        "%install\n"
        "rm -rf $RPM_BUILD_ROOT\n"
        "make DESTDIR=$RPM_BUILD_ROOT install\n"
    );
    if (data.use_libtool() && !data.use_lib_la_files())
    {
        // The debian folk take exception to the "almost
        // always useless" *.la files installed by libtool.
        op->fputs("rm -f $RPM_BUILD_ROOT/usr/lib/*.la\n");
    }
    op->fputs
    (
        "\n"
        "\n"
        "%check\n"
    );
    if (data.seen_test_files())
        op->fputs("make sure\n");
    else
        op->fputs("true\n");
    op->fputs
    (
        "\n"
        "\n"
        "%clean\n"
        "rm -rf $RPM_BUILD_ROOT\n"
        "\n"
        "\n"
    );
    if (files_libs.empty())
        op->fputs("%files\n");
    else
        op->fputs("%files bin\n");
    op->fputs
    (
        "%defattr (-,root,root,-)\n"
        "%doc LICENSE BUILDING README\n"
    );

    if (data.seen_install_doc())
    {
        nstring_list files = data.get_install_doc();
        files.sort();
        for (size_t j = 0; j < files.size(); ++j)
        {
            op->fputs("%doc ");
            op->fputs(translate_make_macro(files[j]));
            op->fputc('\n');
        }
    }

    {
        nstring_list files_bin = data.get_install_bin();
        files_bin.push_back(data.get_install_datadir());
        files_bin.push_back(data.get_install_i18n());

        // not the "*.pc" or "*.la" files, they go in devel
        {
            const nstring_list &files = data.get_install_libdir();
            for (size_t j = 0; j < files.size(); ++j)
            {
                nstring fn = files[j];
                if (!fn.ends_with(".pc") && !fn.ends_with(".la"))
                    files_bin.push_back_unique(fn);
            }
        }

        // and the non-devel man pages
        const nstring_list &mans = data.get_install_mandir();
        for (size_t j = 0; j < mans.size(); ++j)
        {
            nstring fn = mans[j];
            if (!devel_section(fn))
                files_bin.push_back_unique(fn);
        }

        files_bin.sort();
        for (size_t j = 0; j < files_bin.size(); ++j)
        {
            op->fputs(translate_make_macro(files_bin[j]));
            op->fputc('\n');
        }
    }

    if (!files_libs.empty())
    {
        op->fputs
        (
            "\n"
            "\n"
            "%files libs\n"
            "%defattr (-,root,root,-)\n"
        );
        files_libs.sort();
        insert_directories(files_libs);
        for (size_t j = 0; j < files_libs.size(); ++j)
        {
            op->fputs(translate_make_macro(files_libs[j]));
            op->fputc('\n');
        }
    }
    if (!files_devel.empty())
    {
        op->fputs
        (
            "\n"
            "\n"
            "%files devel\n"
            "%defattr (-,root,root,-)\n"
        );
        files_devel.sort();
        // insert_directories(files_devel);
        for (size_t j = 0; j < files_devel.size(); ++j)
        {
            op->fputs(translate_make_macro(files_devel[j]));
            op->fputc('\n');
        }
    }
}


// vim: set ts=8 sw=4 et :
