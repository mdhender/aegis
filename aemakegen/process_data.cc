//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
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
#include <common/ac/stdlib.h>

#include <common/error.h>
#include <common/nstring/list.h>
#include <common/trace.h>
#include <libaegis/os.h>
#include <libaegis/project.h>

#include <aemakegen/process_data.h>
#include <aemakegen/util.h>


process_data::~process_data()
{
}


process_data::process_data() :
    use_libtool_flag(false),
    use_lib_la_files_flag(false),
    use_x11_flag(false),
    use_i18n_flag(false),
    seen_c_flag(false),
    seen_c_plus_plus_flag(false),
    need_yacc_flag(false),
    have_yacc_flag(false),
    need_lex_flag(false),
    have_lex_flag(false),
    library_file_extension("a"),
    object_file_extension("o"),
    install_script_macro(false),
    etc_test_sh_flag(false),
    have_groff_flag(false),
    need_groff_flag(false),
    have_soelim_flag(false),
    need_soelim_flag(false),
    seen_datadir_flag(false),
    seen_datarootdir_flag(false),
    seen_libdir_flag(false),
    seen_sysconfdir_flag(false),
    have_nlsdir_flag(false),
    etc_msgfmt_sh_flag(false),
    groff_macro_flag(false),
    install_data_macro(false),
    have_ar_flag(false),
    have_ranlib_flag(false),
    need_ghostscript_flag(false),
    read_only(false) // must be last
{
}


void
process_data::notify_preprocess_complete(void)
{
    read_only = true;
}


#if HAVE_BACKTRACE

#include <common/ac/execinfo.h>

static void
emit_backtrace(void)
{
    enum { BT_MAX_DEPTH = 20 };
    void **bt_info = new void * [BT_MAX_DEPTH];
    if (bt_info)
    {
        int bt_depth = backtrace(bt_info, BT_MAX_DEPTH);
        char **symbol = backtrace_symbols(bt_info, bt_depth);
        for (int j = 0; j < bt_depth; ++j)
        {
            error_raw("%d: %s", j, symbol[j]);
        }
        delete [] bt_info;
    }
}

#endif


void
process_data::check_read_write(const char *method_name)
{
    if (read_only)
    {
#if HAVE_BACKTRACE
        emit_backtrace();
#endif

        // This will be fatal_raw shortly.
        error_raw
        (
            "read-only mode in effect, may no longer call %s",
            method_name
        );
    }
}


void
process_data::set_use_libtool(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    use_libtool_flag = true;

    set_libext_libtool();

    // This is so the install rule can use $(INSTALL_DATA)
    set_install_data_macro();
}


void
process_data::set_use_lib_la_files(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    use_lib_la_files_flag = true;
}


void
process_data::set_use_x11(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    use_x11_flag = true;
}


void
process_data::set_version_info(const nstring &text)
{
    check_read_write(__PRETTY_FUNCTION__);
    nstring_list nl1;
    nl1.split(text, ":");
    nstring_list nl2;
    while (nl2.size() < 3)
    {
        if (nl1.size() > nl2.size())
        {
            long n = nl1[nl2.size()].to_long();
            nl2.push_back(nstring::format("%ld", n));
        }
        else
            nl2.push_back("0");
    }
    version_info = nl2.unsplit(":");
}


nstring
process_data::get_version_info(void)
    const
{
    return version_info;
}


nstring
process_data::get_version_info_major(void)
    const
{
    //
    // This is weirder than you think.  It actually wants $1-$3
    // (revision - age) not just $1 (revision), because this is the name
    // that libtool actually links to.  Sigh.
    //
    nstring_list parts;
    parts.split(version_info, ":");
    switch (parts.size())
    {
    case 0:
        // Should never happen, but cope.
        return "0";

    case 1:
    case 2:
        // Should never happen, but cope.
        return parts[0];

    case 3:
        // This is the normal case.
        break;

    default:
        assert(!"this should not happen, because we already sanitized it");
        break;
    }

    return nstring::format("%ld", parts[0].to_long() - parts[2].to_long());
}


void
process_data::set_library_directory(change_identifier &cid,
    const nstring_list &filenames)
{
    check_read_write(__PRETTY_FUNCTION__);
    project_name = nstring(cid.get_pp()->trunk_get()->name_get());

    //
    // scan the list of file names, looking for a "lib*" directory with
    // C++ or C files in it.  Use the first one as the library directory.
    //
    library_directory =
        cid.get_cp()->pconf_attributes_find("aemakegen:library-directory");
    if (!library_directory.empty())
    {
        library_name = library_directory;
        if (library_directory == "lib")
            library_name = project_name;
        if (!library_name.starts_with("lib"))
            library_name = "lib" + library_name;
    }
    else
    {
        nstring_list libdirs;
        for (size_t j = 0; j < filenames.size(); ++j)
        {
            nstring filename = filenames[j];
            if (filename.starts_with("lib") && is_a_source_file(filename))
            {
                nstring dir = filename.first_dirname();
                // libdir is supposed to contain files to be installed
                // into $(libdir), not sources.
                if (dir != "." && dir != "libdir")
                    libdirs.push_back_unique(dir);
            }
        }
        libdirs.sort();
        if (!libdirs.empty())
        {
            library_directory = libdirs[0];
            library_name = library_directory;
            if (library_directory == "lib")
                library_name = project_name;
            if (!library_name.starts_with("lib"))
                library_name = "lib" + library_name;
        }
    }
}


void
process_data::remember_explicit_noinst(const nstring &progname)
{
    check_read_write(__PRETTY_FUNCTION__);
    explicit_noinst.push_back_unique(progname);
}


bool
process_data::is_explicit_noinst(const nstring &progname)
    const
{
    return explicit_noinst.member(progname);
}


bool
process_data::uses_pkgconfig(void)
    const
{
    return (use_libtool_flag && seen_pkgconfig_source());
}


void
process_data::remember_clean_misc_file(const nstring &filename)
{
    clean_misc_files.push_back_unique(filename);
}


void
process_data::remember_clean_obj_file(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    clean_obj_files.push_back_unique(filename);
}


void
process_data::remember_dist_clean_file(const nstring &filename)
{
    dist_clean_files.push_back_unique(filename);
}


void
process_data::remember_dist_clean_dir(const nstring &path)
{
    dist_clean_dirs.push_back_unique(path);
}


void
process_data::set_exeext(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    executable_file_extension = "$(EXEEXT)";
}


nstring
process_data::libext(void)
    const
{
    if (use_libtool_flag)
        return "la";
    return library_file_extension;
}


void
process_data::set_libext(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    library_file_extension = "$(LIBEXT)";
}


void
process_data::set_libext_libtool(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    library_file_extension = "la";
}


void
process_data::set_objext(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    object_file_extension = "$(OBJEXT)";
}


void
process_data::remember_source_file(const nstring &path)
{
    trace(("%s {\n", __PRETTY_FUNCTION__));
    trace(("path = %s\n", path.quote_c().c_str()));
    check_read_write(__PRETTY_FUNCTION__);
    nstring srcdir = progname_from_dir_of(path);
    trace(("srcdir = %s\n", srcdir.quote_c().c_str()));
    source_list_t::iterator it = source_list.find(srcdir);
    if (it == source_list.end())
    {
        nstring_list tmp;
        tmp.push_back(path);
        source_list.insert(source_list_t::value_type(srcdir, tmp));
    }
    else
    {
        it->second.push_back(path);
    }
    trace(("}\n"));
}


const nstring_list &
process_data::get_source_files_by_dir(const nstring &dir)
    const
{
    source_list_t::const_iterator it = source_list.find(dir);
    if (it == source_list.end())
    {
        static nstring_list fail;
        return fail;
    }
    return it->second;
}


void
process_data::remember_include_files_by_dir(const nstring &path)
{
    trace(("%s {\n", __PRETTY_FUNCTION__));
    trace(("path = %s\n", path.quote_c().c_str()));
    check_read_write(__PRETTY_FUNCTION__);
    assert(is_an_include_file(path));
    nstring objdir = progname_from_dir_of(path);
    trace(("objdir = %s\n", objdir.quote_c().c_str()));
    include_list_t::iterator it = include_list.find(objdir);
    if (it == include_list.end())
    {
        nstring_list tmp;
        tmp.push_back(path);
        include_list.insert(include_list_t::value_type(objdir, tmp));
    }
    else
    {
        it->second.push_back(path);
    }
    trace(("}\n"));
}


const nstring_list &
process_data::get_include_files_by_dir(const nstring &dir)
    const
{
    include_list_t::const_iterator it = include_list.find(dir);
    if (it == include_list.end())
    {
        static nstring_list fail;
        return fail;
    }
    return it->second;
}


void
process_data::remember_object_file(const nstring &path)
{
    trace(("%s {\n", __PRETTY_FUNCTION__));
    trace(("path = %s\n", path.quote_c().c_str()));
    check_read_write(__PRETTY_FUNCTION__);
    assert(path.ends_with("." + objext()) || path.ends_with(".o")
        || path.ends_with(".lo"));

    //
    // See if the path is below any of the progdirs.
    // Otherwise, just use the first dir.
    //
    nstring objdir = path.first_dirname();
    for (size_t j = 0; j < progdirs.size(); ++j)
    {
        nstring dir = progdirs[j];
        nstring relative = os_below_dir(dir, path);
        trace(("relative = %s\n", relative.quote_c().c_str()));
        if (!relative.empty())
        {
            assert(relative != ".");
            objdir = dir;
            break;
        }
    }
    trace(("objdir = %s\n", objdir.quote_c().c_str()));

    object_list_t::iterator it = object_list.find(objdir);
    if (it == object_list.end())
    {
        nstring_list tmp;
        tmp.push_back(path);
        object_list.insert(object_list_t::value_type(objdir, tmp));
    }
    else
    {
        it->second.push_back(path);
    }
    remember_clean_obj_file(path);
    trace(("}\n"));
}


const nstring_list &
process_data::get_object_files_by_dir(const nstring &dir)
    const
{
    object_list_t::const_iterator it = object_list.find(dir);
    if (it == object_list.end())
    {
        static nstring_list fail;
        return fail;
    }
    return it->second;
}


void
process_data::library_plus_library(const nstring &to_name,
    const nstring &from_name)
{
    check_read_write(__PRETTY_FUNCTION__);
    object_list_t::iterator to_obj_list = object_list.find(to_name);
    if (to_obj_list == object_list.end())
        return;
    object_list_t::const_iterator from_obj_list = object_list.find(from_name);
    if (from_obj_list == object_list.end())
        return;
    to_obj_list->second.push_back(from_obj_list->second);
}


void
process_data::program_needs_library(const nstring &prog_name,
    const nstring &lib_name)
{
    check_read_write(__PRETTY_FUNCTION__);
    library_list_t::iterator lnames = library_list.find(prog_name);
    if (lnames == library_list.end())
    {
        nstring_list tmp;
        library_list.insert(library_list_t::value_type(prog_name, tmp));
        lnames = library_list.find(prog_name);
        assert(lnames != library_list.end());
    }

    // If you want to say "this program has no library dependencies"
    // pass the empty string for the library name.
    if (lib_name.empty())
        return;

    nstring library_dirname = lib_name;
    nstring library_libname = library_dirname;
    if (library_libname == "lib")
        library_libname = project_name;
    if (!library_libname.starts_with("lib"))
        library_libname = "lib" + library_libname;
    nstring path = library_dirname + "/" + library_libname + "." + libext();
    lnames->second.push_back(path);
}


const nstring_list &
process_data::get_library_list_by_program(const nstring &prog_name)
    const
{
    library_list_t::const_iterator it = library_list.find(prog_name);
    if (it == library_list.end())
    {
        static nstring_list fail;
        return fail;
    }
    return it->second;
}


void
process_data::remember_progdir(const nstring &progdir)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("progdir = %s\n", progdir.quote_c().c_str()));
    check_read_write(__PRETTY_FUNCTION__);
    progdirs.push_back_unique(progdir);
}


void
process_data::remember_all_bin(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    all_bin.push_back_unique(filename);
}


void
process_data::remember_install_directory_for(const nstring &filename)
{
    assert(filename[0] == '$');
    nstring dir = filename.dirname();
    for (;;)
    {
        // When we get to the top of the directory tree, we are done.
        if (dir == "." || dir == "/")
            break;

        // if the directory is already known, nothing more needs to be done.
        if (install_directories.find(dir) != install_directories.end())
            break;

        // Add the directory to the set of known directories.
        install_directories.insert(install_directories_t::value_type(dir, 1));
        remember_clean_misc_file(make_pseudo_dir(dir));

        // Move one level up the directory tree.
        dir = dir.dirname();
    }
}


nstring_list
process_data::get_install_directories(void)
    const
{
    nstring_list result;
    for
    (
        install_directories_t::const_iterator it = install_directories.begin();
        it != install_directories.end();
        ++it
    )
    {
        result.push_back(it->first);
    }
    return result;
}


void
process_data::remember_install_bin(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    install_bin.push_back_unique(filename);
    remember_install_directory_for(filename);
}


void
process_data::remember_test_source(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    test_sources.push_back_unique(filename);
}


void
process_data::remember_test_file(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    test_files.push_back_unique(filename);
}


void
process_data::remember_install_datadir(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    install_datadir.push_back_unique(filename);
    remember_install_directory_for(filename);
    set_install_data_macro();
}


void
process_data::remember_pkgconfig_source(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    pkgconfig_sources.push_back_unique(filename);
}


void
process_data::remember_install_libdir(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    install_libdir.push_back_unique(filename);
    remember_install_directory_for(filename);
    set_install_data_macro();
    set_seen_libdir();
}


void
process_data::remember_man_sources(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    man_sources.push_back_unique(filename);
}


void
process_data::remember_install_mandir(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    install_mandir.push_back_unique(filename);
    remember_install_directory_for(filename);
    set_install_data_macro();
}


void
process_data::remember_all_doc(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    all_doc.push_back_unique(filename);
}


void
process_data::remember_install_doc(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    install_doc.push_back_unique(filename);
    remember_install_directory_for(filename);
    set_install_data_macro();
}


void
process_data::set_program_prefix(void)
{
    program_prefix = "$(PROGRAM_PREFIX)";
}


void
process_data::set_program_suffix(void)
{
    program_suffix = "$(PROGRAM_SUFFIX)";
}


void
process_data::remember_all_i18n(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    all_i18n.push_back_unique(filename);
}


void
process_data::remember_install_i18n(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    install_i18n.push_back_unique(filename);
    remember_install_directory_for(filename);
    set_install_data_macro();
}


void
process_data::set_etc_msgfmt_sh(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    etc_msgfmt_sh_flag = true;
}


void
process_data::remember_install_include_source(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    assert(filename[0] != '$');
    assert(filename[0] != '/');
    install_include_sources.push_back_unique(filename);
}


void
process_data::remember_install_include(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    assert(filename[0] == '$');
    install_include.push_back_unique(filename);
    remember_install_directory_for(filename);
    set_install_data_macro();
}


void
process_data::set_groff_macro(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    groff_macro_flag = true;
}


bool
process_data::seen_programs(void)
    const
{
    return !progdirs.empty();
}


nstring
process_data::progdir_from_progname(const nstring &progname)
    const
{
    for (size_t j = 0; j < progdirs.size(); ++j)
    {
        nstring dir = progdirs[j];
        nstring pn2 = progname_from_dir_of(dir + "/main.c");
        if (progname == pn2)
            return dir;
    }
    return nstring();
}


nstring_list
process_data::get_programs(void)
    const
{
    nstring_list result;
    for (size_t j = 0; j < progdirs.size(); ++j)
    {
        nstring dir = progdirs[j];
        nstring progname = progname_from_dir_of(dir + "/main.c");
        result.push_back(progname);
    }
    result.sort();
    return result;
}


nstring_list
process_data::get_list_of_library_directories(void)
    const
{
    // Use a map for efficiency.  Some of my projects have both many
    // directories and many programs, and the O(n**2) shows up very
    // quickly, so change to O(n*log(n))
    typedef std::map<nstring, int> progs_t;
    progs_t progs;
    for (size_t j = 0; j < progdirs.size(); ++j)
    {
        progs[progdirs[j]] = 1;
    }

    //
    // Build a list of directories that contain object files that
    // are not listed in the 'programs' list.  They may not all be
    // libraries, but its a reasonably good guess.
    //
    nstring_list result;
    for
    (
        object_list_t::const_iterator it = object_list.begin();
        it != object_list.end();
        ++it
    )
    {
        if (progs.find(it->first) == progs.end())
        {
            result.push_back(it->first);
        }
    }
    return result;
}


void
process_data::remember_extra_dist(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    extra_dist.push_back_unique(filename);
}


void
process_data::remember_built_sources(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    built_sources.push_back_unique(filename);
}


void
process_data::set_install_data_macro(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    install_data_macro = true;
}


void
process_data::set_have_ar(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    have_ar_flag = true;
}


void
process_data::set_have_groff(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    have_groff_flag = true;
}


void
process_data::set_need_groff(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    need_groff_flag = true;
    if (!have_groff_flag)
    {
        fatal_raw
        (
            "The \"configure.ac\" file fails to invoke "
            "\"AC_CHECK_PROGS(GROFF, groff roff)\" and yet the project source "
            "files would appear to require it."
        );
    }
}


void
process_data::set_have_soelim(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    have_soelim_flag = true;
}


void
process_data::set_need_soelim(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    need_soelim_flag = true;
    if (!have_soelim_flag)
    {
        fatal_raw
        (
            "The \"configure.ac\" file fails to invoke "
            "\"AC_CHECK_PROGS(SOELIM, gsoelim soelim)\" and yet the project "
            "source files would appear to require it."
        );
    }
}


void
process_data::set_have_ranlib(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    have_ranlib_flag = true;
}


void
process_data::set_need_ghostscript(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    need_ghostscript_flag = true;
}


void
process_data::remember_am_data_data(const nstring &filename)
{
    check_read_write(__PRETTY_FUNCTION__);
    am_data_data.push_back_unique(filename);
}


void
process_data::set_have_nlsdir(void)
{
    check_read_write(__PRETTY_FUNCTION__);
    have_nlsdir_flag = true;
}


// vim: set ts=8 sw=4 et :
