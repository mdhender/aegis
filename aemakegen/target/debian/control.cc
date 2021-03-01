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

#include <common/ac/assert.h>
#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <common/fstrcmp.h>
#include <common/now.h>
#include <common/sizeof.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/output/filter/dotblankline.h>
#include <libaegis/output/filter/prefix.h>
#include <libaegis/output/wrap.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>

#include <aemakegen/target/debian.h>


static nstring
first_line_only(const nstring &s)
{
    unsigned max = 47;
    const char *start = s.c_str();
    const char *end = start + s.size();
    if (s.size() > max)
        end = start + max;
    const char *cp = start;
    while (cp < end)
    {
        unsigned char c = *cp;
        // tabs are OK, newlines are not
        if (c == '\t' || isprint(c))
            ++cp;
        else
            break;
    }
    return nstring(start, cp - start);
}


static void
wrap_and_print(const output::pointer &op, const nstring &caption,
    const nstring_list &items)
{
    if (items.empty())
        return;
    op->fputs(caption);
    size_t column = caption.size();
    op->fputc(':');
    ++column;
    for (size_t j = 0; j < items.size(); ++j)
    {
        if (j)
        {
            op->fputc(',');
            ++column;
        }
        nstring s = items[j];
        if (column + 1 + s.size() >= 80)
        {
            op->fputc('\n');
            column = 0;
        }
        op->fputc(' ');
        ++column;
        op->fputs(s);
        column += s.size();
    }
    op->fputc('\n');
}


static void
check_against_list(const nstring &name, const char **list, size_t list_size)
{
    for (const char **p = list; p < list + list_size; ++p)
    {
        if (name == *p)
            return;
    }
    nstring best;
    double best_weight = 0.6;
    for (const char **f = list; f < list + list_size; ++f)
    {
        nstring candidate(*f);
        double weight = fstrcmp(name.c_str(), candidate.c_str());
        if (best_weight < weight)
        {
            best_weight = weight;
            best = candidate;
        }
    }
    if (best.empty())
    {
        sub_context_ty sc;
        sc.var_set_string("Name", name);
        fatal_intl(&sc, i18n("the name \"$name\" is undefined"));
        // NOTREACHED
    }
    else
    {
        sub_context_ty sc;
        sc.var_set_string("Name", name);
        sc.var_set_string("Guess", best);
        fatal_intl(&sc, i18n("no \"$name\", guessing \"$guess\""));
        // NOTREACHED
    }
}


static void
check_section(const nstring &name)
{
    // Debian Policy Manual, version 2.8.2.0, section 2.4
    nstring_list parts;
    parts.split(name, "/");
    while (parts.size() < 2)
        parts.push_front("main");

    struct table_t
    {
        const char *name;
        const char *version;
    };

    static const table_t table[] =
    {
        { "admin", "3.5.6" },
        // { "base", "3.5.6" }, // obsolete
        { "cli-mono", "3.8.2" },
        { "comm", "3.5.6" },
        // { "contrib", "3.5.6" }, // obsolete
        { "database", "3.8.2" },
        { "debug", "3.8.2" },
        { "devel", "3.5.6" },
        { "doc", "3.5.6" },
        { "editors", "3.5.6" },
        { "electronics", "3.5.6" },
        { "embedded", "3.6.1" },
        { "fonts", "3.8.2" },
        { "games", "3.5.6" },
        { "gnome", "3.6.1" },
        { "gnu-r", "3.8.2" },
        { "gnustep", "3.8.2" },
        { "graphics", "3.5.6" },
        { "hamradio", "3.5.6" },
        { "haskell", "3.8.2" },
        { "httpd", "3.8.2" },
        { "interpreters", "3.5.6" },
        { "java", "3.8.2" },
        { "kde", "3.6.1" },
        { "kernel", "3.8.2" },
        { "libdevel", "3.6.1" },
        { "libs", "3.5.6" },
        { "lisp", "3.8.2" },
        { "localization", "3.8.2" },
        { "mail", "3.5.6" },
        { "math", "3.5.6" },
        { "misc", "3.5.6" },
        { "net", "3.5.6" },
        { "news", "3.5.6" },
        // { "non-free", "3.5.6" }, // obsolete
        // { "non-US", "3.5.6" }, // obsolete
        { "ocaml", "3.8.2" },
        { "oldlibs", "3.5.6" },
        { "otherosfs", "3.5.6" },
        { "perl", "3.6.1" },
        { "php", "3.8.2" },
        { "python", "3.6.1" },
        { "ruby", "3.8.2" },
        { "science", "3.5.6" },
        { "shells", "3.5.6" },
        { "sound", "3.5.6" },
        { "tex", "3.5.6" },
        { "text", "3.5.6" },
        { "utils", "3.5.6" },
        { "vcs", "3.8.2" },
        { "video", "3.8.2" },
        { "web", "3.5.6" },
        { "x11", "3.5.6" },
        { "xfce", "3.8.2" },
        { "zope", "3.8.2" },
    };

    const table_t *best = 0;
    double best_weight = 0.6;
    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        if (0 == strcmp(name.c_str(), tp->name))
        {
            return;
        }
        double weight = fstrcmp(name.c_str(), tp->name);
        if (weight > best_weight)
        {
            best = tp;
            best_weight = weight;
        }
    }

    if (!best)
    {
        sub_context_ty sc;
        sc.var_set_string("Name", name);
        fatal_intl(&sc, i18n("the name \"$name\" is undefined"));
        // NOTREACHED
    }
    else
    {
        sub_context_ty sc;
        sc.var_set_string("Name", name);
        sc.var_set_charstar("Guess", best->name);
        fatal_intl(&sc, i18n("no \"$name\", guessing \"$guess\""));
        // NOTREACHED
    }
}


static nstring
doc_base_section_from_debian_section(const nstring &name)
{
    struct table_t
    {
        const char *deb_name;
        const char *doc_base_name;
    };

    static const table_t table[] =
    {
        { "admin", "System/Administration" },
        { "cli-mono", "Programming" },
        { "comm", "Network" },
        { "database", "Data Management" },
        { "debug", "Programming", },
        { "devel", "Programming" },
        { "doc", "Debian" },
        { "editors", "Editors" },
        { "electronics", "Science/Electronics" },
        { "embedded", "Programming" },
        { "fonts", "Office" },
        { "games", "Games" },
        { "gnome", "Graphics" },
        { "gnu-r", "Programming" },
        { "gnustep", "Programming" },
        { "graphics", "Graphics" },
        { "hamradio", "Amateur Radio" },
        { "haskell", "Programming" },
        { "httpd", "Network" },
        { "interpreters", "Emulators" },
        { "java", "Programming/Java" },
        { "kde", "Graphics" },
        { "kernel", "System" },
        { "libdevel", "Programming" },
        { "libs", "Programming" },
        { "lisp", "Programming/Lisp" },
        { "localization", "Programming" },
        { "mail", "Network/Communication" },
        { "math", "Science/Mathematics" },
        { "misc", "Programming" },
        { "net", "Network" },
        { "news", "Network" },
        { "ocaml", "Programming/Ocaml" },
        { "oldlibs", "Programming" },
        { "otherosfs", "Programming" },
        { "perl", "Programming/Perl" },
        { "php", "Programming/Php" },
        { "python", "Programming/Python" },
        { "ruby", "Programming/Ruby" },
        { "science", "Science" },
        { "shells", "Shells" },
        { "sound", "Sound" },
        { "tex", "Typesetting" },
        { "text", "Text" },
        { "utils", "Programming" },
        { "vcs", "Programming" },
        { "video", "Video" },
        { "web", "Network" },
        { "x11", "Graphics" },
        { "xfce", "Graphics" },
        { "zope", "Network" },
    };

    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        if (name == tp->deb_name)
            return tp->doc_base_name;
    }
    return "Programming";
}


static void
check_priority(const nstring &name)
{
    // Debian Policy Manual 2.5
    static const char *ok[] =
    {
        "required", "important", "standard", "optional", "extra"
    };
    check_against_list(name, ok, SIZEOF(ok));
}


void
target_debian::maybe_field(const output::pointer &fp, const nstring &field_name,
    const nstring &root_name)
{
    nstring name = "aemakegen:debian:" + field_name;
    if (!root_name.empty())
        name += ":" + root_name;
    nstring value = get_cp()->pconf_attributes_find(name);
    if (!value.empty())
    {
        fp->fprintf("%s: %s\n", field_name.capitalize().c_str(), value.c_str());
    }
}


static nstring_list
filter_man15678(const nstring_list &src)
{
    nstring_list result;
    for (size_t j = 0; j < src.size(); ++j)
    {
        nstring fn = src[j];
        nstring section = fn.field('/', 1);
        if (section == "man1" || (section >= "man5" && section <= "man8"))
            result.push_back(fn);
    }
    return result;
}


static nstring_list
filter_man234(const nstring_list &src)
{
    nstring_list result;
    for (size_t j = 0; j < src.size(); ++j)
    {
        nstring fn = src[j];
        nstring section = fn.field('/', 1);
        if (section >= "man2" && section <= "man4")
            result.push_back(fn);
    }
    return result;
}


static nstring_list
filter_runtime(const nstring_list &src)
{
    nstring_list result;
    for (size_t j = 0; j < src.size(); ++j)
    {
        nstring fn = src[j];
        if (fn.ends_with(".la") || fn.ends_with(".a"))
        {
            nstring stem = fn.trim_extension();
            result.push_back_unique(stem + ".so.*");
        }
    }
    return result;
}


static nstring_list
filter_bin(const nstring_list &src)
{
    nstring_list result;
    for (size_t j = 0; j < src.size(); ++j)
    {
        nstring fn = src[j];
        if (!fn.ends_with(".la") && !fn.ends_with(".a") && !fn.ends_with(".pc"))
        {
            result.push_back(fn);
        }
    }
    return result;
}


nstring_list
target_debian::filter_dev(const nstring_list &src)
    const
{
    nstring_list result;
    for (size_t j = 0; j < src.size(); ++j)
    {
        nstring fn = src[j];
        if (fn.ends_with(".la") || fn.ends_with(".a"))
        {
            nstring stem = fn.trim_extension();
            result.push_back_unique(stem + ".a");
            result.push_back_unique(stem + ".so");
            if (data.use_lib_la_files())
            {
                // The debian folk take exception to the "almost
                // always useless" *.la files installed by libtool.
                result.push_back_unique(stem + ".la");
            }
        }
        else if (fn.ends_with(".pc"))
        {
            result.push_back_unique(fn);
        }
    }
    return result;
}


nstring
target_debian::expand_make_macro(const nstring &text)
{
    struct table_t
    {
        const char *from;
        const char *to;
    };
    static const table_t table[] =
    {
        { "$(bindir)",      "usr/bin"          },
        { "$(datadir)",     "usr/share"        },
        { "$(datarootdir)", "usr/share"        },
        { "$(exec_prefix)", "usr"              },
        { "$(EXEEXT)",      ""                 },
        { "$(includedir)",  "usr/include"      },
        { "$(libdir)",      "usr/lib"          },
        { "$(localedir)",   "usr/share/locale" },
        { "$(mandir)",      "usr/share/man"    },
        { "$(NLSDIR)",      "usr/share/locale" },
        { "$(prefix)",      "usr"              },
        { "$(sysconfdir)",  "etc"              },
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
                    expand_make_macro(text.substr(s1 + s2, s3))
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
target_debian::fill_instance_variables(void)
{
    source_package_name = get_project_name();

    // binary_package_files are for executable programs
    // and libdir and datadir and i18n required to support them
    binary_package_name =
        (
            source_package_name.starts_with("lib")
        ?
            source_package_name.substr(3, source_package_name.size() - 3)
        :
            source_package_name
        );
    binary_package_files.push_back(data.get_install_bin());
    if (data.use_libtool())
    {
        binary_package_files.push_back
        (
            filter_man15678(data.get_install_mandir())
        );
    }
    else
        binary_package_files.push_back(data.get_install_mandir());
    binary_package_files.push_back(filter_bin(data.get_install_libdir()));
    binary_package_files.push_back(data.get_install_datadir());
    binary_package_files.push_back(data.get_install_i18n());

    // doc files are for the "*-doc" package
    documentation_package_name = source_package_name + "-doc";
    documentation_package_files.push_back(data.get_install_doc());

    nstring lib_root_name = "lib" + binary_package_name;

    // runtime files are for shared libraries
    runtime_package_name = lib_root_name + data.get_version_info_major();
    runtime_package_files.push_back(filter_runtime(data.get_install_libdir()));

    debug_package_name = runtime_package_name + "-dbg";

    // developer files are for static libraries and include files
    developer_package_name = lib_root_name + "-dev";
    developer_package_files.push_back(data.get_install_include());
    if (data.use_libtool())
    {
        developer_package_files.push_back
        (
            filter_man234(data.get_install_mandir())
        );
        developer_package_files.push_back
        (
            filter_dev(data.get_install_libdir())
        );
    }
}


void
target_debian::gen_control(void)
{
    nstring source_description =
        nstring(project_brief_description_get(get_pp()->trunk_get()));
    source_description = first_line_only(source_description).trim();

    nstring source_extended_description =
        get_cp()->pconf_attributes_find
        (
            "aemakegen:debian:extended-description:" + source_package_name
        );
    if (source_extended_description.empty())
    {
        source_extended_description =
            get_cp()->pconf_attributes_find
            (
                "aemakegen:debian:extended-description"
            );
    }

    nstring runtime_extended_description =
        get_cp()->pconf_attributes_find
        (
            "aemakegen:debian:description:" + runtime_package_name
        );
    if (runtime_extended_description.empty())
    {
        runtime_extended_description =
            get_cp()->pconf_attributes_find
            (
                "aemakegen:debian:description:lib" + binary_package_name
            );
    }
    if (runtime_extended_description.empty())
        runtime_extended_description = source_extended_description;

    nstring devel_extended_description =
        get_cp()->pconf_attributes_find
        (
            "aemakegen:debian:extended-description:" + developer_package_name
        );
    if (devel_extended_description.empty())
        devel_extended_description = source_extended_description;

    // write debian/control
    os_become_orig();
    output::pointer fp = output_file::open("debian/control");
    os_become_undo();

    // ----------  source package  -----------------------------------------

    fp->fprintf("Source: %s\n", source_package_name.c_str());

    nstring section =
        get_cp()->pconf_attributes_find("aemakegen:debian:section");
    if (section.empty())
        section = "utils";
    check_section(section);
    fp->fprintf("Section: %s\n", section.c_str());

    nstring priority =
        get_cp()->pconf_attributes_find("aemakegen:debian:priority");
    if (priority.empty())
        priority = "optional";
    check_priority(priority);
    fp->fprintf("Priority: %s\n", priority.c_str());

    nstring maintainer =
        get_cp()->pconf_attributes_find("aemakegen:debian:maintainer");
    if (maintainer.empty())
        get_up()->get_email_address();
    fp->fprintf("Maintainer: %s\n", maintainer.c_str());

    nstring homepage =
        get_cp()->pconf_attributes_find("aemakegen:debian:homepage");
    if (!homepage.empty())
    {
        fp->fprintf("Homepage: %s\n", homepage.c_str());
    }
    if
    (
        get_cp()->pconf_attributes_get_boolean
        (
            "aemakegen:debian:dm-upload-allowed"
        )
    )
    {
        fp->fprintf("DM-Upload-Allowed: yes\n");
    }

    //
    // The Build-Depends: list may need a few more things, and we sort it.
    //
    nstring_list build_depends_list;
    nstring build_depends =
        get_cp()->pconf_attributes_find("aemakegen:debian:build-depends");
    if (!build_depends.empty())
    {
        build_depends_list.split(build_depends, ",", true);
        build_depends_list.remove(""); // common infelicity
    }
    build_depends_list.push_back_unique("debhelper (>= 5)");
    if (data.use_libtool())
        build_depends_list.push_back_unique("libtool");
    if (data.need_groff())
        build_depends_list.push_back_unique("groff");
    if (data.need_yacc())
        build_depends_list.push_back_unique("bison");
    if (data.need_lex())
        build_depends_list.push_back_unique("flex");
    if (data.need_ghostscript())
        build_depends_list.push_back_unique("ghostscript");
    build_depends_list.sort();
    wrap_and_print(fp, "Build-Depends", build_depends_list);

    if (!runtime_package_files.empty())
    {
        check_section("libs");
        check_section("debug");
    }
    if (!developer_package_files.empty())
        check_section("libdevel");

    // Lintian would be a whole lot more useful if it actually checked
    // packages against the claimed standards version, instead of
    // blindly demanding that the standards version is always up-to-date.
    fp->fputs("Standards-Version: 3.9.3\n");

    // ----------  binary package  -----------------------------------------

    //
    // This is the binary package, it contains the executables.
    //
    if (!binary_package_files.empty())
    {
        binary_package_files.sort();
        fp->fprintf("\n");
        fp->fprintf("Package: %s\n", binary_package_name.c_str());
        fp->fprintf("Architecture: any\n");

        nstring description =
            get_cp()->pconf_attributes_find
            (
                "aemakegen:debian:description:" + binary_package_name
            );
        if (description.empty())
        {
            description = source_description;
            if (binary_package_name != source_package_name)
                description += " - programs";
        }
        fp->fprintf("Description: %s\n", description.c_str());
        // wrap the extended description, if one has been provided
        // in the project-specific attributes.
        {
            nstring aname =
                "aemakegen:debian:extended-description:" + binary_package_name;
            nstring binary_extended_description =
                get_cp()->pconf_attributes_find(aname);
            if (binary_extended_description.empty())
                binary_extended_description = source_extended_description;
            if (!binary_extended_description.empty())
            {
                output::pointer op2 =
                    output_wrap_open
                    (
                        output_filter_dot_blank_lines::create
                        (
                            output_filter_prefix::create(fp, " ")
                        ),
                        79
                    );
                op2->fputs(binary_extended_description);
                op2->end_of_line();
            }
        }

        nstring_list depends_list;
        nstring depends =
            get_cp()->pconf_attributes_find("aemakegen:debian:depends");
        if (!depends.empty())
            depends_list.split(depends, ",", true);
        depends_list.push_back("${shlibs:Depends}");
        depends_list.push_back("${misc:Depends}");
        if (!runtime_package_files.empty())
            depends_list.push_back(runtime_package_name);
        depends_list.remove("");
        depends_list.sort();
        wrap_and_print(fp, "Depends", depends_list);

        maybe_field(fp, "breaks", binary_package_name);
        maybe_field(fp, "conflicts", binary_package_name);
        maybe_field(fp, "provides", binary_package_name);
        maybe_field(fp, "recommends", binary_package_name);
        maybe_field(fp, "replaces", binary_package_name);
        maybe_field(fp, "suggests", binary_package_name);
    }

    // ----------  documentation package  ----------------------------------

    if (!documentation_package_files.empty())
    {
        //
        // The doc binary package for the user documentation,
        // other than man pages.
        //
        // Package: <name>-doc
        //
        documentation_package_files.sort();
        fp->fprintf("\n");
        fp->fprintf("Package: %s\n", documentation_package_name.c_str());
        fp->fprintf("Architecture: all\n");
        fp->fprintf("Section: doc\n");

        nstring description =
            get_cp()->pconf_attributes_find
            (
                "aemakegen:debian:description:" + documentation_package_name
            );
        if (description.empty())
        {
            description = source_description;
            if
            (
                runtime_package_name
            !=
                source_package_name + data.get_version_info_major()
            )
                description += " - documentation";
        }
        fp->fprintf("Description: %s\n", description.c_str());

        // Wrap the extended description, if one has been provided
        // in the project-specific attributes.
        nstring aname =
            "aemakegen:debian:extended-description:" +
            documentation_package_name;
        nstring doc_extended_description =
            get_cp()->pconf_attributes_find(aname);
        if (doc_extended_description.empty())
            doc_extended_description = devel_extended_description;
        if (!doc_extended_description.empty())
        {
            output::pointer op2 =
                output_wrap_open
                (
                    output_filter_dot_blank_lines::create
                    (
                        output_filter_prefix::create(fp, " ")
                    ),
                    79
                );
            op2->fputs(doc_extended_description);
            op2->end_of_line();
        }

        {
            nstring depname =
                "aemakegen:debian:depends:" + documentation_package_name;
            nstring depends = get_cp()->pconf_attributes_find(depname);

            nstring_list depends_list;
            depends_list.split(depends, ",", true);
            depends_list.remove(""); // common infelicity
            depends_list.push_back("${misc:Depends}"); // for debhelper
            depends_list.sort();
            wrap_and_print(fp, "Depends", depends_list);
        }

        maybe_field(fp, "breaks", documentation_package_name);
        maybe_field(fp, "conflicts", documentation_package_name);
        maybe_field(fp, "provides", documentation_package_name);
        maybe_field(fp, "recommends", documentation_package_name);
        maybe_field(fp, "replaces", documentation_package_name);
        maybe_field(fp, "suggests", documentation_package_name);

        //
        // Generate the debian/<name>-doc.doc-base file.
        //
        // We will do it as a single document, it simplifies everything
        // except doc-base.  I wonder if anyone actually uses this
        // facility?
        //
        // According to dh_installdocs(1),
        //
        //    debian/<package>.doc-base.*
        //        "If your package needs to register more than one
        //        document, you need multiple doc-base files, and can
        //        name them like this."
        //
        // Which is a whole lot of work for not much gain.
        //
        nstring fn2 = "debian/" + documentation_package_name + ".doc-base";
        os_become_orig();
        output::pointer op2 = output_file::open(fn2);
        os_become_undo();
        nstring uc_source_package_name = source_package_name.capitalize();
        op2->fputs("Document: ");
        op2->fputs(source_package_name);
        op2->fputs("\nTitle: ");
        op2->fputs(uc_source_package_name);
        op2->fputs(" Manuals\nAuthor: ");
        op2->fputs(get_cp()->pconf_copyright_owner_get());
        op2->fputs("\nAbstract: ");
        op2->fputs(uc_source_package_name);
        op2->fputs(" Manuals\nSection: ");
        op2->fputs(doc_base_section_from_debian_section(section));
        op2->fputs("\n\nFormat: PDF\nFiles:");
        for (size_t j = 0; j < documentation_package_files.size(); ++j)
        {
            op2->fputs(" /");
            op2->fputs(expand_make_macro(documentation_package_files[j]));
            // we need the wildcard, because they may or may not be
            // compressed by the time we see this.
            op2->fputs("*\n");
        }
    }

    if (data.use_libtool())
    {
        // The libtool program is only used when a shared library is
        // going to be installed.  Use this to decide whether or not to
        // add the binary packages for shared libraries.
        if (!runtime_package_files.empty())
        {
            // ----------  runtime package  --------------------------------
            //
            // The runtime binary package for the shared library.  Using a
            // shared library (as opposed to a static library) reduces the
            // size of executable of all clients, but increases the amount
            // of work to be done by a program before main() is called.
            //
            // Package: lib<name>.<version>
            //
            runtime_package_files.sort();
            fp->fprintf("\n");
            fp->fprintf("Package: %s\n", runtime_package_name.c_str());
            if (section != "libs")
            {
                // lintian(1) does not like redundant Section lines
                fp->fprintf("Section: libs\n");
            }
            fp->fprintf("Architecture: any\n");

            nstring description =
                get_cp()->pconf_attributes_find
                (
                    "aemakegen:debian:description:" + runtime_package_name
                );
            if (description.empty())
            {
                description = source_description;
                if
                (
                    runtime_package_name
                !=
                    source_package_name + data.get_version_info_major()
                )
                    description += " - runtime library";
            }
            fp->fprintf("Description: %s\n", description.c_str());
            // wrap the extended description, if one has been provided
            // in the project-specific attributes.
            if (!runtime_extended_description.empty())
            {
                output::pointer op2 =
                    output_wrap_open
                    (
                        output_filter_dot_blank_lines::create
                        (
                            output_filter_prefix::create(fp, " ")
                        ),
                        79
                    );
                op2->fputs(runtime_extended_description);
                op2->end_of_line();
            }

            nstring_list depends_list;
            nstring depends =
                get_cp()->pconf_attributes_find("aemakegen:debian:depends");
            if (!depends.empty())
                depends_list.split(depends, ",", true);
            depends_list.push_back("${shlibs:Depends}");
            depends_list.push_back("${misc:Depends}");
            depends_list.sort();
            wrap_and_print(fp, "Depends", depends_list);

            // ----------  debug package  ----------------------------------
            //
            // The debug binary package, used to hold the debug symbls
            // for the runtime package.  This simplifies start-up
            // linking for client programs, and reduces the time before
            // main() is called.
            //
            // package: lib<name>.<version>-dbg
            //
            fp->fprintf("\n");
            fp->fprintf("Package: %s\n", debug_package_name.c_str());
            if (section != "debug")
            {
                // lintian(1) does n't like redundant Section lines
                fp->fprintf("Section: debug\n");
            }
            if (priority != "extra")
            {
                // lintian(1) does n't like redundant Priority lines
                fp->fprintf("Priority: extra\n");
            }
            fp->fprintf("Architecture: any\n");

            nstring aname =
                "aemakegen:debian:description:" + debug_package_name;
            description = get_cp()->pconf_attributes_find(aname);
            if (description.empty())
                description = source_description;
            description += " - debugging symbols";
            fp->fprintf("Description: %s\n", description.c_str());
            aname =
                "aemakegen:debian:extended-description:" + debug_package_name;
            nstring debug_extended_description =
                get_cp()->pconf_attributes_find(aname);
            if (debug_extended_description.empty())
                debug_extended_description = runtime_extended_description;
            if (debug_extended_description.empty())
                debug_extended_description = source_extended_description;
            if (!debug_extended_description.empty())
            {
                output::pointer op2 =
                    output_wrap_open
                    (
                        output_filter_dot_blank_lines::create
                        (
                            output_filter_prefix::create(fp, " ")
                        ),
                        79
                    );
                op2->fputs(debug_extended_description);
                op2->end_of_line();
            }

            depends_list.push_back
            (
                runtime_package_name + " (= ${binary:Version})"
            );
            wrap_and_print(fp, "Depends", depends_list);

            maybe_field(fp, "breaks", runtime_package_name);
            maybe_field(fp, "conflicts", runtime_package_name);
            maybe_field(fp, "provides", runtime_package_name);
            maybe_field(fp, "recommends", runtime_package_name);
            maybe_field(fp, "replaces", runtime_package_name);
            maybe_field(fp, "suggests", runtime_package_name);
        }
        if (!developer_package_files.empty())
        {
            // ----------  developer package  ------------------------------
            //
            // The dev package, for linking programs to the library, and
            // header files required to build the library.
            //
            // Package: lib<name>-dev
            //
            developer_package_files.sort();
            fp->fprintf("\n");
            fp->fprintf("Package: %s\n", developer_package_name.c_str());
            if (section != "libdevel")
            {
                // lintian(1) does not like redundant Section lines
                fp->fprintf("Section: libdevel\n");
            }
            fp->fprintf("Architecture: any\n");

            nstring aname =
                "aemakegen:debian:description:" + developer_package_name;
            nstring description = get_cp()->pconf_attributes_find(aname);
            if (description.empty())
                description = source_description;
            description += " - development files";
            fp->fprintf("Description: %s\n", description.c_str());
            if (!devel_extended_description.empty())
            {
                output::pointer op2 =
                    output_wrap_open
                    (
                        output_filter_dot_blank_lines::create
                        (
                            output_filter_prefix::create(fp, " ")
                        ),
                        79
                    );
                op2->fputs(devel_extended_description);
                op2->end_of_line();
            }

            nstring_list depends_list;
            nstring depends =
                get_cp()->pconf_attributes_find("aemakegen:debian:depends");
            if (!depends.empty())
                depends_list.split(depends, ",", true);
            depends_list.push_back("${shlibs:Depends}");
            depends_list.push_back("${misc:Depends}");
            depends_list.push_back
            (
                runtime_package_name + " (= ${binary:Version})"
            );
            depends_list.sort();
            wrap_and_print(fp, "Depends", depends_list);

            maybe_field(fp, "breaks", developer_package_name);
            maybe_field(fp, "conflicts", developer_package_name);
            maybe_field(fp, "provides", developer_package_name);
            maybe_field(fp, "recommends", developer_package_name);
            maybe_field(fp, "replaces", developer_package_name);
            maybe_field(fp, "suggests", developer_package_name);
        }
    }
    fp.reset();

    // ---------- binary package  ------------------------------------------
    //
    // The binary package (executables, man pages)
    //
    if (!binary_package_files.empty())
    {
        nstring_list tmp;
        for (size_t j = 0; j < binary_package_files.size(); ++j)
        {
            nstring fn = binary_package_files[j];
            if (!redundant_license_file(fn))
            {
                //
                // Debian don't like you installing redundant license files.
                //
                // Note: you must remove these files here, and you will also
                // have to do it in aemakegen/target/debian/rules.cc as well.
                //
                tmp.push_back(expand_make_macro(fn));
            }
        }
        tmp.sort();

        nstring fn = "debian/" + binary_package_name + ".install";
        os_become_orig();
        fp = output_file::open(fn);
        for (size_t j = 0; j < tmp.size(); ++j)
        {
            fp->fputs(tmp[j]);
            fp->fputc('\n');
        }
        fp.reset();
        os_become_undo();
    }

    // ---------- documentation package  -----------------------------------
    //
    // The documentation package.
    //
    if (!documentation_package_files.empty())
    {
        nstring_list tmp;
        for (size_t j = 0; j < documentation_package_files.size(); ++j)
        {
            nstring fn = documentation_package_files[j];
            if (!redundant_license_file(fn))
                tmp.push_back(expand_make_macro(fn));
        }
        tmp.sort();

        nstring fn = "debian/" + documentation_package_name + ".install";
        os_become_orig();
        fp = output_file::open(fn);
        for (size_t j = 0; j < tmp.size(); ++j)
        {
            fp->fputs(tmp[j]);
            fp->fputc('\n');
        }
        fp.reset();
        os_become_undo();
    }

    if (data.use_libtool())
    {
        // ---------- runtime package  -------------------------------------
        //
        // The runtime package, for shared libraries
        //
        if (!runtime_package_files.empty())
        {
            nstring_list tmp;
            for (size_t j = 0; j < runtime_package_files.size(); ++j)
            {
                nstring fn = runtime_package_files[j];
                tmp.push_back(expand_make_macro(fn));
            }
            tmp.sort();

            nstring fn = "debian/" + runtime_package_name + ".install";
            os_become_orig();
            fp = output_file::open(fn);
            for (size_t j = 0; j < tmp.size(); ++j)
            {
                fp->fputs(tmp[j]);
                fp->fputc('\n');
            }
            fp.reset();
            os_become_undo();
        }

        // ---------- developer package  -----------------------------------
        //
        // The developer package, for .a files, and include files,
        // and section 2 or 3 man pages.
        //
        if (!developer_package_files.empty())
        {
            nstring_list tmp;
            for (size_t j = 0; j < developer_package_files.size(); ++j)
            {
                nstring fn = developer_package_files[j];
                assert(!fn.ends_with(".la") || data.use_lib_la_files());
                if (!redundant_license_file(fn))
                    tmp.push_back(expand_make_macro(fn));
            }
            tmp.sort();

            nstring fn = "debian/" + developer_package_name + ".install";
            os_become_orig();
            fp = output_file::open(fn);
            for (size_t j = 0; j < tmp.size(); ++j)
            {
                fp->fputs(tmp[j]);
                fp->fputc('\n');
            }
            fp.reset();
            os_become_undo();
        }
    }
}


// vim: set ts=8 sw=4 et :
