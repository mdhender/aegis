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

#ifndef AEMAKEGEN_PROCESS_DATA_H
#define AEMAKEGEN_PROCESS_DATA_H

#include <map>

#include <common/nstring.h>
#include <common/nstring/list.h>

#include <libaegis/change/identifier.h>

/**
  * The process_data class is used to represent the data common to all
  * flavours and targets for generation tasks.
  */
class process_data
{
public:
    /**
      * The destructor.  It is intentionally not virtual.
      * Thou shalt not derive from this class.
      */
    ~process_data();

    /**
      * The default constructor.
      */
    process_data();

    /**
      * The notify_preprocess_complete method is called to mark the
      * end of preprocessing, and the beginning of output.  After this
      * point, no instance variables shouldchange.
      */
    void notify_preprocess_complete(void);

    /**
      * The set_use_libtool method is used to request that
      * libtool is to be used to build shared libraries.
      */
    void set_use_libtool(void);

    /**
      * The use_libtool method is used to determine whether or not the
      * output shall use libtool.
      */
    bool use_libtool(void) const { return use_libtool_flag; }

    /**
      * The set_use_lib_la_files method is used to request that
      * libtool .la file be installed.  The default is false.
      */
    void set_use_lib_la_files(void);

    /**
      * The use_lib_la_files is used to determine whether or not to
      * install libtool .la files for shared libraries.
      */
    bool use_lib_la_files(void) const { return use_lib_la_files_flag; }

    /**
      * The set_version_info method is used to establish the shared
      * files's version info string.
      */
    void set_version_info(const nstring &text);

    /**
      * The get_version_info method is used to obtain a suitable
      * argument to the libtool -version-info option, based on the
      * project version.
      * Only meaningful if #use_libtool returns true.
      */
    nstring get_version_info(void) const;

    /**
      * The get_version_info_major method is used to obtain the major
      * version of the version-info.
      */
    nstring get_version_info_major(void) const;

    /**
      * The set_library_directory is used to determine the name of
      * the library (shared or static) to be used for code common to
      * executables built for the project.
      *
      * @param cid
      *     The change set identification of the change of interest.
      * @param filenames
      *     The list of source files to be considered.
      */
    void set_library_directory(change_identifier &cid,
        const nstring_list &filenames);

    /**
      * The get_library_directory method may be used tp obtain the name
      * of the top-level directory containing common files to be linked
      * with each of the executables.
      */
    nstring get_library_directory(void) const { return library_directory; }

    /**
      * The get_library_name method may be used to obtain the name
      * of the library containing common files to be linked
      * with each of the executables (basename, not including extension).
      *
      * Or the emopty string, if the project doesn't have a library.
      */
    nstring get_library_name(void) const { return library_name; }

    /**
      * The remember_explicit_noinst method is used to add another
      * program name that is not to be installed.
      *
      * @param progname
      *     The name of the program to be omitted from the files that
      *     are installed by the "install:" target.
      */
    void remember_explicit_noinst(const nstring &progname);

    /**
      * The is_explicit_noinst method may be used to determine whether
      * or not the named program is not to be installed.
      */
    bool is_explicit_noinst(const nstring &progname) const;

    /**
      * The set_use_x11 method is used to control whether or not
      * X11 is to be used to build and link.
      */
    void set_use_x11(void);

    /**
      * The use_x11 method is used to determine whether or not X11 is to
      * be used.
      */
    bool use_x11(void) const { return use_x11_flag; }

    /**
      * The uses_pkgconfig method is used to determine whether or not
      * there is a pkg-config data file installed by the project.
      * This is only meaningful if #use_libtool returns true.
      */
    bool uses_pkgconfig(void) const;

    /**
      * use_i18n methof is used to remember whether or not the project
      * installs internationalization and localization files.
      */
    bool use_i18n(void) const { return use_i18n_flag; }

    /**
      * The set_use_i18n method is used to remember that the
      * project installs i18n files.
      */
    void set_use_i18n(void) { use_i18n_flag = true; }

    /**
      * The set_seen_c method is used to notify that probably C source
      * files have been observed.
      */
    void set_seen_c(void) { seen_c_flag = true; }

    /**
      * The seen_c method is used to determine whether or not probable C
      * source files have been observed.
      */
    bool seen_c(void) const { return seen_c_flag; }

    /**
      * The set_seen_c_plus_plus method is used to notify that probably
      * C++ source files have been observed.
      */
    void set_seen_c_plus_plus(void) { seen_c_plus_plus_flag = true; }

    /**
      * The seen_c_plus_plus method is used to determine whether or not
      * probable C source files have been observed.
      */
    bool seen_c_plus_plus(void) const { return seen_c_plus_plus_flag; }

    /**
      * The set_need_yacc method is used to notify that probable yacc
      * source files have been observed in the file manifest.
      */
    void set_need_yacc(void) { need_yacc_flag = true; }

    /**
      * The need_yacc method is used to determine whether or not
      * probable yacc source files have been observed in the file manifest.
      */
    bool need_yacc(void) const { return need_yacc_flag; }

    /**
      * The set_have_yacc method is used to notify that the configure.ac
      * file contains a reference to AC_PROG_YACC, necessary for
      * building *.y source files.
      */
    void set_have_yacc(void) { have_yacc_flag = true; }

    /**
      * The have_yacc method is used to determine whether or not
      * the configure.ac file contains a reference to AC_PROG_YACC,
      * necessary for building *.y source files.
      */
    bool have_yacc(void) const { return have_yacc_flag; }

    /**
      * The set_need_lex method is used to notify that probable lex
      * source files have been observed in the file manifest.
      */
    void set_need_lex(void) { need_lex_flag = true; }

    /**
      * The need_lex method is used to determine whether or not
      * probable lex source files have been observed in the file manifest.
      */
    bool need_lex(void) const { return need_lex_flag; }

    /**
      * The set_have_lex method is used to notify that the configure.ac
      * file contains a reference to AC_PROG_LEX, necessary for
      * building *.l source files.
      */
    void set_have_lex(void) { have_lex_flag = true; }

    /**
      * The have_lex method is used to determine whether or not
      * the configure.ac file contains a reference to AC_PROG_lex,
      * necessary for building *.l source files.
      */
    bool have_lex(void) const { return have_lex_flag; }

    /**
      * The remember_clean_misc_file method is used to append
      * another misc file to the list of files to be removed by the
      * "clean-misc:" make target.
      */
    void remember_clean_misc_file(const nstring &filename);

    /**
      * The get_clean_misc_files method is used to get read-only access
      * to the list of misc files to be cleaned.
      */
    const nstring_list &get_clean_misc_files(void) const
        { return clean_misc_files; }

    /**
      * The remember_clean_obj_file method is used to add another file
      * to be removed by the "clean-obj:" makefile target.
      */
    void remember_clean_obj_file(const nstring &filename);

    /**
      * The get_clean_obj_files method is used to get read-only access
      * to the list of object files to be cleaned.
      */
    const nstring_list &get_clean_obj_files(void) const
        { return clean_obj_files; }

    /**
      * The remember_dist_clean_file method is used to add another file
      * to be removed by the "distclean-files:" makefile target.
      */
    void remember_dist_clean_file(const nstring &filename);

    /**
      * The get_dist_clean_files method is used to get read-only
      * access to the list of support files to be cleaned by the
      * "distclean-files:" make target.
      */
    const nstring_list &get_dist_clean_files(void) const
        { return dist_clean_files; }

    /**
      * The seen_dist_clean_files method may be used to determine
      * whether or not any dist_clean_files have been seen.
      */
    bool seen_dist_clean_files(void) const { return !dist_clean_files.empty(); }

    /**
      * The remember_dist_clean_dir method is used to add another
      * directory to be removed by the "distclean-dirs:" makefile
      * target.
      */
    void remember_dist_clean_dir(const nstring &path);

    /**
      * The get_dist_clean_dirs method is used to get read-only
      * access to the list of support files to be cleaned by the
      * "distclean-dirs:" make target.
      */
    const nstring_list &get_dist_clean_dirs(void) const
        { return dist_clean_dirs; }

    /**
      * The exeext method is used to obtain the file extension of
      * executable files.
      */
    const nstring &exeext(void) const { return executable_file_extension; }

    /**
      * The set_exeext method is used to support the autoconf $(EXEEXT)
      * substitution.  The default is the empty string.
      */
    void set_exeext(void);

    /**
      * The libext method is used to obtain the file extension of
      * library files.
      */
    nstring libext(void) const;

    /**
      * The set_libext method is used to support the autoconf $(LIBEXT)
      * substitution.  The default is "a".
      */
    void set_libext(void);

    /**
      * The set_libext_libtool method is used to support the libtool
      * "la" extension.
      */
    void set_libext_libtool(void);

    /**
      * The objext method is used to obtain the file extension of
      * object files.
      */
    const nstring objext(void) const { return object_file_extension; }

    /**
      * The set_objext method is used to support the autoconf $(OBJEXT)
      * substitution.  The default is "o".
      */
    void set_objext(void);

    /**
      * The remember_source_file method is used to add another source
      * file to the lists of source files being maintained, one for each
      * top-level directory.
      *
      * By "source_file" we mean a file that needs to be compiled, not
      * a script file.  Or indirectly needs to be compiled, like yacc
      * "*.y" files.
      *
      * @param filename
      *     The name of the source file to be remembered.
      */
    void remember_source_file(const nstring &filename);

    /**
      * The get_source_files_by_dir method is used to obtain read-only
      * access to the list of source files for a given top level
      * directory.
      *
      * @param top_level_dir
      *     The top level directory that the source files fall under.
      * @returns
      *     A const reference to a list of source file names.
      *     The empty strings means there were no source files, or no
      *     such top level directory.
      */
    const nstring_list &get_source_files_by_dir(const nstring &top_level_dir)
        const;

    /**
      * The remember_include_file method is used to add another include
      * file to the lists of include files being maintained, one for each
      * top-level directory.
      *
      * @param filename
      *     The name of the include file to be remembered.
      */
    void remember_include_files_by_dir(const nstring &filename);

    /**
      * The get_include_files_by_dir method is used to obtain read-only
      * access to the list of include files for a given top level
      * directory.
      *
      * @param top_level_dir
      *     The top level directory that the include files fall under.
      * @returns
      *     A const reference to a list of include file names.
      *     The empty strings means there were no include files, or no
      *     such top level directory.
      */
    const nstring_list &get_include_files_by_dir(const nstring &top_level_dir)
        const;

    /**
      * The remember_object_file method is used to add another object
      * file to the lists of object files being maintained, one for each
      * top-level directory.
      *
      * @param filename
      *     The name of the object file to be remembered.
      */
    void remember_object_file(const nstring &filename);

    /**
      * The get_object_files_by_dir method is used to obtain read-only
      * access to the list of object files for a given top level
      * directory.
      *
      * @param top_level_dir
      *     The top level directory that the object files fall under.
      * @returns
      *     A const reference to a list of object file names.
      *     The empty strings means there were no object files, or no
      *     such top level directory.
      */
    const nstring_list &get_object_files_by_dir(const nstring &top_level_dir)
        const;

    /**
      * The library_plus_library method is used to append the contents
      * of one list of object filoes to another.
      *
      * @param to_name
      *     The directory to receive the additonal object files.
      * @param from_name
      *     The directory from which to obtain the additonal object files.
      */
    void library_plus_library(const nstring &to_name, const nstring &from_name);

    /**
      * The get_library_list_by_program method is used to obtain
      * read-only access to the list of library files for a given
      * program (top level directory).
      *
      * @param progname
      *     The name of the program (top level directory) that may have
      *     extra libraries.
      * @returns
      *     A const reference to a list of library file names.
      *     The empty strings means there were no library files, or no
      *     such top level directory.
      */
    const nstring_list &get_library_list_by_program(const nstring &progname)
        const;

    /**
      * The program_needs_library method is used to add another library
      * directory to the list of library directories needed by a
      * program.
      *
      * @param prog_name
      *     The name of the program (a top level directory).
      * @param lib_name
      *     The name of the library to append (a top level directory,
      *     does not include the .a suffix, etc)
      */
    void program_needs_library(const nstring &prog_name,
        const nstring &lib_name);

    /**
      * The remember_progdir method is used to add another program to
      * the list of programs to be linked into an executable.
      *
      * @param progdir
      *     The directory containing the program's source tree.
      *     May contain slashes, e.g. "test/open".
      */
    void remember_progdir(const nstring &progdir);

    /**
      * The get_programs method is used to obtain read-only access to
      * the list of programs to be linked.  It derives the program name
      * from the program directory name.
      */
    nstring_list get_programs(void) const;

    /**
      * The progdir_from_progname method is used to figure out the
      * program source directory from the program name.
      *
      * @param progname
      *     The name of the program to find
      * @returns
      *     the directory name (could contain slashes). or the empty
      *     string if it doesn't exist.
      */
    nstring progdir_from_progname(const nstring &progname) const;

    /**
      * The get_progdirs method is used to obtain read-only access to
      * the list of program directories to be linked.
      */
    const nstring_list &get_progdirs(void) const { return progdirs; }

    /**
      * The seen_programs method may be used to determine whether or
      * not any programs have been see (that is, programs that require
      * linking, not scripts).
      */
    bool seen_programs(void) const;

    /**
      * The need_install_script_macro method variable is used to
      * determine whether or not to emit the "INSTALL_SCRIPT =
      * @INSTALL_SCRIPT@" line in the Makefile.in file.
      */
    bool need_install_script_macro(void) const { return install_script_macro; }

    /**
      * The set_install_script_macro method is used to notify that a
      * "INSTALL_SCRIPT = @INSTALL_SCRIPT@" in the Makefile.in file.
      */
    void set_install_script_macro(void) { install_script_macro = true; }

    /**
      * The remember_all_bin method is used to add another file that
      * is to be build, and makes part of the "all-bin" make target.
      *
      * @param filename
      *     The name of the binary file.
      */
    void remember_all_bin(const nstring &filename);

    /**
      * The get_all_bin method is used to obtain read-only access to
      * the list of programs to be alled, as prerequisites of the
      * "all-bin" make target.
      */
    const nstring_list &get_all_bin(void) const { return all_bin; }

    bool seen_all_bin(void) const { return !all_bin.empty(); }

    /**
      * The remember_install_bin method is used to add another file that
      * is to be build, and makes part of the "install-bin" make target,
      * into the $(bindir) directory.
      *
      * @param filename
      *     The name of the binary file.
      */
    void remember_install_bin(const nstring &filename);

    /**
      * The get_install_bin method is used to obtain read-only access to
      * the list of programs to be installed, as prerequisites of the
      * "install-bin" make target.
      */
    const nstring_list &get_install_bin(void) const { return install_bin; }

    /**
      * The seen_etc_test_sh method is used to determine whether or not
      * the project contains a "etc/test.sh" source file.
      * This will be used to run tests, if it is present.
      */
    bool seen_etc_test_sh(void) const { return etc_test_sh_flag; }

    /**
      * The set_etc_test_sh method is used to notify that the project
      * has a "etc/test.sh" source file.
      * This will be used to run tests, if it is present.
      */
    void set_etc_test_sh(void) { etc_test_sh_flag = true; }

    /**
      * The remember_test_source method is used to add another source
      * file to the list of test source files.  This is used by the
      * Makefile.am TESTS definition.
      */
    void remember_test_source(const nstring &filename);

    /**
      * The get_test_sources method may be used to obtain read-only
      * access to the list of test source files.
      */
    const nstring_list &get_test_sources(void) const { return test_sources; }

    /**
      * The remember_test_file method is used to add another test file
      * to the list of test files.  This is used by the "make sure"
      * rule.
      */
    void remember_test_file(const nstring &filename);

    /**
      * The get_test_files method may be used to obtain read-only access
      * to the list of test files.
      */
    const nstring_list &get_test_files(void) const { return test_files; }

    /**
      * The seen_test_files method may be used to determine
      * whether or not there are any test files to run.
      */
    bool seen_test_files(void) const { return !test_files.empty(); }

    /**
      * The have_groff method is used to determine whether or not the
      * groff(1) command was mentioned in the configure.ac file.
      */
    bool have_groff(void) const { return have_groff_flag; }

    /**
      * The set_have_groff method is used to notify that the groff(1)
      * command is mentioned in the configure.ac file.
      */
    void set_have_groff(void);

    /**
      * The need_groff method is used to determine whether or not the
      * groff(1) command required to build the sources.
      */
    bool need_groff(void) const { return need_groff_flag; }

    /**
      * The set_need_groff method is used to notify that the groff(1)
      * command is required to build one or more files in the source
      * file manifest.
      */
    void set_need_groff(void);

    /**
      * The have_soelim method is used to determine whether or not
      * there is a "AC_CHECK_PROGS(SOELIM, gsoelim soelim)" line in the
      * "configure.ac" file.
      */
    bool have_soelim(void) const { return have_soelim_flag; }

    /**
      * The set_have_soelim method is used to notify that there
      * is a "AC_CHECK_PROGS(SOELIM, gsoelim soelim)" line in the
      * "configure.ac" file.
      */
    void set_have_soelim(void);

    /**
      * The need_soelim method is used to determine whether or not
      * there are any source files that require soelim(1) to build.
      */
    bool need_soelim(void) const { return need_soelim_flag; }

    /**
      * The set_need_soelim method is used to notify that there are
      * source files that require soelim(1) to build.
      */
    void set_need_soelim(void);

    /**
      * The seen_datadir method is used to determine whether or not
      * content is to be installed into $(datadir)/<progname>/
      */
    bool seen_datadir(void) const { return seen_datadir_flag; }

    /**
      * The set_seen_datadir method is used to determine notify that
      * content is to be installed into $(datadir)/<progname>/
      */
    void set_seen_datadir(void) { seen_datadir_flag = true; }

    /**
      * The remember_install_datadir method is used to add another file
      * to the set of files to be installed under $(datadir)/<progname>/
      *
      * @param filename
      *     The name of the fiel to be installed.
      */
    void remember_install_datadir(const nstring &filename);

    /**
      * The get_install_datadir method may be used to obtain
      * read-only access to the list of files to be installed
      * below $(datadir)/<progname>/
      */
    const nstring_list &get_install_datadir(void) const
        { return install_datadir; }

    /**
      * The seen_datarootdir method is used to determine whether or not
      * content is to be installed into $(datarootdir)/
      */
    bool seen_datarootdir(void) const { return seen_datarootdir_flag; }

    /**
      * The set_seen_datarootdir method is used to notify that
      * content is to be installed into $(datarootdir)/
      */
    void set_seen_datarootdir(void) { seen_datarootdir_flag = true; }

    /**
      * The seen_libdir method is used to determine whether or not
      * content is to be installed into $(libdir)/<progname>/
      */
    bool seen_libdir(void) const { return seen_libdir_flag; }

    /**
      * The set_seen_libdir method is used to notify that
      * content is to be installed into $(libdir)/<progname>/
      */
    void set_seen_libdir(void) { seen_libdir_flag = true; }

    /**
      * The remember_pkgconfig_source method is used to add
      * a file to the set of source files for pkgconfig/ .pc files.
      */
    void remember_pkgconfig_source(const nstring &filename);

    /**
      * The seen_pkgconfig_source method may be used to determine
      * whether or not and source files have been seen that are to be
      * installed below $(libdir)/pkgconfig/
      */
    bool seen_pkgconfig_source(void) const
        { return !pkgconfig_sources.empty(); }

    /**
      * The get_get_pkgconfig_sources method may be used to obtain read-only
      * access to the list of source to be installed below $(libdir)/pkgconfig/
      */
    const nstring_list &get_pkgconfig_sources(void) const
        { return pkgconfig_sources; }

    /**
      * The remember_install_libdir method is used to add another file
      * to the set of files to be installed below $(libdir)
      *
      * @param filename
      *     The name of the file to be installed.
      */
    void remember_install_libdir(const nstring &filename);

    /**
      * The get_install_libdir method may be used to obtain read-only
      * access to the list of files to be installed below $(libdir)/
      */
    const nstring_list &get_install_libdir(void) const
        { return install_libdir; }

    /**
      * The seen_sysconfdir method is used to determine whether or not
      * content is to be installed into $(sysconfdir)/  (i.e. /etc)
      */
    bool seen_sysconfdir(void) const { return seen_sysconfdir_flag; }

    /**
      * The set_seen_sysconfdir method is used to notify that
      * content is to be installed into $(sysconfdir)/  (i.e. /etc)
      */
    void set_seen_sysconfdir(void) { seen_sysconfdir_flag = true; }

    /**
      * The remember_man_sources method is used to add another file to
      * the set of source files for man pages.
      */
    void remember_man_sources(const nstring &filename);

    /**
      * The get_man_sources method may be used to obtain read-only
      * access to the set of source files for man pages.
      */
    const nstring_list &get_man_sources(void) const { return man_sources; }

    /**
      * The remember_install_mandir method is used to add another file
      * to the set of files to be installed below $(mandir)/
      *
      * @param filename
      *     The name of the file to be installed.
      */
    void remember_install_mandir(const nstring &filename);

    /**
      * The seen_install_mandir method may be used to determine
      * whether or not there are any files to install below $(mandir)/
      */
    bool seen_install_mandir(void) const { return !install_mandir.empty(); }

    /**
      * The get_install_mandir method may be used to obtain read-only
      * access to the set of files to be installed under the $(mandir)/
      * directory.
      */
    const nstring_list &get_install_mandir(void) const
        { return install_mandir; }

    /**
      * The remember_all_doc method is used to add another file to the
      * set of files to be built by the "all-doc" make target.
      *
      * @param filename
      *     The name of the file to be installed.
      */
    void remember_all_doc(const nstring &filename);

    /**
      * The seen_all_doc method may be used to determine whether or not
      * there are any documentation files to be built.
      */
    bool seen_all_doc(void) const { return !all_doc.empty(); }

    /**
      * The get_all_doc method may be used to obtain read-only access to
      * the set of documentation files to be built.
      */
    const nstring_list &get_all_doc(void) const { return all_doc; }

    /**
      * The remember_install_doc method is used to add another file
      * to the set of files to be installed by the "install-doc" make
      * target.
      *
      * @param filename
      *     The name of the file to be installed.
      */
    void remember_install_doc(const nstring &filename);

    /**
      * The seen_install_doc method may be used to determine whether
      * or not there are any documentation files to be installed.
      */
    bool seen_install_doc(void) const { return !install_doc.empty(); }

    /**
      * The get_install_doc method may be used to obtain read-only access to
      * the set of documentation files to be installed.
      */
    const nstring_list &get_install_doc(void) const { return install_doc; }

    /**
      * The set_program_prefix method is used to set the program
      * prefix to "$(PROGRAM_PREFIX)".  This is set by the ./configure
      * --program-prefix option.
      */
    void set_program_prefix(void);

    /**
      * The get_program_prefix method is used to obtain the value of the
      * program prefix to be added to the names of install programs.
      */
    nstring get_program_prefix(void) const { return program_prefix; }

    /**
      * The set_program_suffix method is used to set the program
      * suffix to "$(PROGRAM_SUFFIX)".  This is set by the ./configure
      * --program-suffix option.
      */
    void set_program_suffix(void);

    /**
      * The get_program_suffix method is used to obtain the value of the
      * program suffix to be added to the names of install programs.
      */
    nstring get_program_suffix(void) const { return program_suffix; }

    /**
      * The remember_all_i18n method is used to add another file to the
      * set of files to be built by the "all-i18n" make target.
      *
      * @param filename
      *     The name of the file to be installed.
      */
    void remember_all_i18n(const nstring &filename);

    /**
      * The seen_all_i18n method may be used to determine whether or not
      * there are any i18n files to be built.
      */
    bool seen_all_i18n(void) const { return !all_i18n.empty(); }

    /**
      * The get_all_i18n method may be used to obtain read-only access to
      * the set of i18n files to be built.
      */
    const nstring_list &get_all_i18n(void) const { return all_i18n; }

    /**
      * The remember_install_i18n method is used to add another file
      * to the set of files to be install by the "install-i18n" make
      * target.
      *
      * @param filename
      *     The name of the file to be installed.
      */
    void remember_install_i18n(const nstring &filename);

    /**
      * The seen_install_i18n method may be used to determine whether or
      * not there are any i18n files to be installed.
      */
    bool seen_install_i18n(void) const { return !install_i18n.empty(); }

    /**
      * The get_install_i18n method may be used to obtain read-only
      * access to the set of i18n files to be installed.
      */
    const nstring_list &get_install_i18n(void) const { return install_i18n; }

    /**
      * The set_have_nlsdir method is used to notify that the
      * configure.ac file contain a AC_SUBST(NLSDIR) line.
      */
    void set_have_nlsdir(void);

    /**
      * The have_nlsdir method is used to determine whether or not the
      * configure.ac file contains a AC_SUBST(NLSDIR) line.
      */
    bool have_nlsdir(void) const { return have_nlsdir_flag; }

    /**
      * The set_etc_msgfmt_sh method is used to notify that an
      * "etc/msgfmt.sh" source file has been seen.  If present, this
      * will be used in preference to raw msgfmt(1) command.
      */
    void set_etc_msgfmt_sh(void);

    /**
      * The seen_etc_msgfmt_sh method is used to determine whether or
      * not an "etc/fmtgen.sh" file has been seen.
      */
    bool seen_etc_msgfmt_sh(void) const { return etc_msgfmt_sh_flag; }

    /**
      * The remember_install_include_source method is used to add
      * another file to the set of source files that will eventually be
      * installed under $(includedir), see #remember_install_include,
      * but these are the source files.
      *
      * @param filename
      *     The name of the source file to be remembered.
      */
    void remember_install_include_source(const nstring &filename);

    /**
      * The get_install_include_sources method may be used to obtain
      * read-only access to the set of include files to be installed,
      * the project source filename, NOT the installed path.
      */
    const nstring_list &get_install_include_sources(void) const
        { return install_include_sources; }

    /**
      * The seen_install_include_sources method may be used to determine
      * whether or not there are any include files to be installed.
      */
    bool seen_install_include_sources(void) const
        { return !install_include_sources.empty(); }

    /**
      * The remember_install_include method is used to add another file
      * to the set of files to be install by the "install-include" make
      * target.
      *
      * @param filename
      *     The name of the file to be installed.
      */
    void remember_install_include(const nstring &filename);

    /**
      * The seen_install_include method may be used to determine whether or
      * not there are any include files to be installed.
      */
    bool seen_install_include(void) const { return !install_include.empty(); }

    /**
      * The get_install_include method may be used to obtain
      * read-only access to the list of include files to be installed
      * under the $(includedir) system directory.
      */
    const nstring_list &get_install_include(void) const
        { return install_include; }

    void set_groff_macro(void);

    bool seen_groff_macro(void) const { return groff_macro_flag; }

    /**
      * The get_list_of_library_directories is used to obtain a
      * list of directories that have object files in them, bit are
      * not mentioned in the 'programs' list.  It is possible that
      * not all of them are libraries, but it's a pretty good first
      * approcimation.
      */
    nstring_list get_list_of_library_directories(void) const;

    /**
      * The remember_built_sources method is used to add another file
      * to the set of files that consTitute catch-22 cases when
      * automatically deriving include dependencies.
      *
      * @param filename
      *     The name of the file to be installed.
      */
    void remember_built_sources(const nstring &filename);

    /**
      * The seen_built_sources method may be used to determine whether or
      * not there are any include catch-22 file.
      */
    bool seen_built_sources(void) const { return !built_sources.empty(); }

    /**
      * The get_built_sources method may be used to obtain read-only
      * access to the set of catch-22 include files.
      */
    const nstring_list &get_built_sources(void) const
        { return built_sources; }

    /**
      * The remember_extra_dist method is used to add another file
      * to the set of files that consTitute catch-22 cases when
      * automatically deriving include dependencies.
      *
      * @param filename
      *     The name of the file to be installed.
      */
    void remember_extra_dist(const nstring &filename);

    /**
      * The seen_extra_dist method may be used to determine whether or
      * not there are any include catch-22 file.
      */
    bool seen_extra_dist(void) const { return !extra_dist.empty(); }

    /**
      * The get_extra_dist method may be used to obtain read-only
      * access to the set of catch-22 include files.
      */
    const nstring_list &get_extra_dist(void) const
        { return extra_dist; }

    /**
      * The set_install_data_macro method is used to notify that the
      * Makefile.in file will need to define the INSTAL_DATA make macro.
      */
    void set_install_data_macro(void);

    /**
      * The need_install_data_macro method is used to determine whether
      * or not the INSTALL_DATA make macro needs to be defined.
      */
    bool need_install_data_macro(void) const { return install_data_macro; }

    /**
      * The get_install_directories method is used to obtain a list
      * of the list of <b>directories</b> mentioned to the set of
      * <i>all</i> remember_install_<i>something</i> methods.
      */
    nstring_list get_install_directories(void) const;

    /**
      * The set_have_ar method is used to notify that an a
      * "AC_CHECK_PROGS(AR, ar)" line was observed in the "configure.ac"
      * file.
      */
    void set_have_ar(void);

    /**
      * The have_ar method is used to determine whether or not a
      * "AC_CHECK_PROGS(AR, ar)" line was observed in the "configure.ac"
      * file.
      */
    bool have_ar(void) const { return have_ar_flag; }

    /**
      * The set_have_ranlib method is used to notify that an a
      * "AC_PROG_RANLIB" line was observed in the "configure.ac" file.
      */
    void set_have_ranlib(void);

    /**
      * The have_ranlib method is used to determine whether or not a
      * "AC_PROG_RANLIB" line was observed in the "configure.ac" file.
      */
    bool have_ranlib(void) const { return have_ranlib_flag; }

    /**
      * The set_need_ghostscript method is used to remember that the
      * "ghostscript" package (for ps2pdf) is required.
      */
    void set_need_ghostscript(void);

    /**
      * The need_ghostscript method is used to determine whether or not
      * the "ghostscript" package (for ps2pdf) is required.
      */
    bool need_ghostscript(void) const { return need_ghostscript_flag; }

    /**
      * The seen_am_data_data method is used to determine whether or not
      * any files have been seen that are to be installed as data by
      * automake.
      */
    bool seen_am_data_data(void) const { return !am_data_data.empty(); }

    /**
      * The get_am_data_data method is used to obtain read-only access
      * to the set of files to be installed as data by automake.
      */
    const nstring_list &get_am_data_data(void) const { return am_data_data; }

    /**
      * The remember_am_data_data instance variable is used to add
      * another file the set of files to be installed as data by
      * automake.
      */
    void remember_am_data_data(const nstring &filename);

private:
    /**
      * The project_name instance variable is used to remember the name
      * of the project.  We need this for library .a file naming, in
      * some cases.  It is set by the #set_library_directory method.
      */
    nstring project_name;

    /**
      * The use_libtool_flag is used to remember whether or not libtool
      * is to be used to build shared libraries.
      */
    bool use_libtool_flag;

    /**
      * The use_lib_la_files_flag is used to remember whether or not to
      * install *.la files.  Only meaningful if #use_libtool_flag is true.
      *
      * The debian folk take exception to the "almost always useless"
      * *.la files install by libtool.  So this defaults to false.
      */
    bool use_lib_la_files_flag;

    /**
      * The version_info instance variable is used to remember the
      * shared-library version-info string.
      */
    nstring version_info;

    /**
      * The library_directory instance variable is used to remember
      * the directory containing a library to be linked with the other
      * executables.  Empty if none.
      */
    nstring library_directory;

    /**
      * The library_name instance variable is used to remember
      * the name of a library to be linked with the other executables
      * (basename, not including extension).  Empty if none.
      */
    nstring library_name;

    /**
      * The explicit_noinst instance variable is used to remember
      * the names of programs that are not to be installed.
      */
    nstring_list explicit_noinst;

    /**
      * The use_x11_flag instance variable is used to remember
      * whether or not the output should use the X11 macros.
      */
    bool use_x11_flag;

    /**
      * The use_i18n_flag is used to remember whether or not the project
      * installs internationalization and localization files.
      */
    bool use_i18n_flag;

    /**
      * The seen_c_flag instance variable is used to remember that the
      * project appears to contain C source files.
      */
    bool seen_c_flag;

    /**
      * The seen_c_plus_plus_flag instance variable is used to remember
      * that the project appears to contain C++ source files.
      */
    bool seen_c_plus_plus_flag;

    /**
      * The seen_yacc_flag instance variable is used to remember
      * that the project appears to contain yacc source files.
      */
    bool need_yacc_flag;

    /**
      * The have_yacc_flag instance variable is used to remember
      * that the configure.ac file contains a definition for @YACC@
      */
    bool have_yacc_flag;

    /**
      * The seen_lex_flag instance variable is used to remember
      * that the project appears to contain lex source files.
      */
    bool need_lex_flag;

    /**
      * The have_lex_flag instance variable is used to remember
      * that the configure.ac file contains a definition for @LEX@
      */
    bool have_lex_flag;

    /**
      * The clean_misc_files instance variable is used to remember files
      * that are to be removed by the "clean-misc:" target, but do not
      * it into any of the other clean categories.
      */
    nstring_list clean_misc_files;

    /**
      * The clean_obj_files instance variable is used to remember files
      * that are to be removed by the "clean-obj:" target.
      */
    nstring_list clean_obj_files;

    /**
      * The dist_clean_files instance variable is used to remember files
      * that are to be removed by the "distclean-files:" target.  These
      * files are the ones created withing the build dir to optimize the
      * install.
      */
    nstring_list dist_clean_files;

    /**
      * The dist_clean_dirs instance variable is used to remember the
      * directories that are to be removed by the "distclean-dirs:"
      * target.  These directories are the ones created within the build
      * directory, usually by libtool but not always.
      */
    nstring_list dist_clean_dirs;

    /**
      * The executable_file_extension instance variable is used to
      * remember the file extension of executable files.  On Unix, the
      * isn't one.
      */
    nstring executable_file_extension;

    /**
      * The library_file_extension instance variable is used to
      * remember the file extension of library files.
      * Defaults to "a".
      */
    nstring library_file_extension;

    /**
      * The object_file_extension instance variable is used to
      * remember the file extension of object files.
      * Defaults to "o".
      */
    nstring object_file_extension;

    typedef std::map<nstring, nstring_list> source_list_t;

    /**
      * The source_list instance variable is used to remember the list
      * of source files associated with each top-level directory.
      */
    source_list_t source_list;

    typedef std::map<nstring, nstring_list> include_list_t;

    /**
      * The include_list instance variable is used to remember the list
      * of include files associated with each top-level directory.
      */
    include_list_t include_list;

    typedef std::map<nstring, nstring_list> object_list_t;

    /**
      * The object_list instance variable is used to remember the list
      * of object files associated with each top-level directory.
      */
    object_list_t object_list;

    typedef std::map<nstring, nstring_list> library_list_t;

    /**
      * The library_list instance variable is used to remember the list
      * of library files associated with each program.
      */
    library_list_t library_list;

    /**
      * The progdirs instance variable is used to rememeber the list of
      * directorirs that contain program sources and objects that need
      * to be linked from object files to build an executable (i.e. not
      * a script).
      */
    nstring_list progdirs;

    /**
      * The install_script_macro instance variable is used to remember
      * whether or not a script has been seen.  This is used to decide
      * whether or not to emit the "INSTALL_SCRIPT = @INSTALL_SCRIPT@"
      * line in the Makefile.in file.
      */
    bool install_script_macro;

    /**
      * The all_bin instance variable is used to remember the list of
      * executable files to be constructed by the "all-bin" target.
      */
    nstring_list all_bin;

    /**
      * The install_bin instance variable is used to remember the list of
      * executable files to be installed by the "install-bin" target.
      */
    nstring_list install_bin;

    /**
      * The etc_test_sh_flag is used to remember whether or not the
      * "etc/test.sh" source file has been seen.  This is used to write
      * test rules.
      * This will be used to run tests, if it is present.
      */
    bool etc_test_sh_flag;

    /**
      * The test_sources instance variable is used to remember the test
      * source files, for use in the Automake TESTS definition.
      */
    nstring_list test_sources;

    /**
      * The test_files instance variable is used to remember the test
      * output files, for use in the "make sure" rule.
      */
    nstring_list test_files;

    /**
      * The have_groff_flag instance variable is used to remember
      * whether or not we have seen GROFF mentioned in the configure.ac
      * file.  If this is the case, we emit more rules which use groff.
      */
    bool have_groff_flag;

    /**
      * The need_groff_flag instance variable is used to remember
      * whether or not we have seen source files that require groff to
      * build them.
      */
    bool need_groff_flag;

    /**
      * The have_soelim_flag instance variable is used to remember
      * whether or not there is a "AC_CHECK_PROGS(SOELIM, gsoelim
      * soelim)" line in the "configure.ac" file.
      */
    bool have_soelim_flag;

    /**
      * The need_soelim_flag instance variable is used to remember
      * whether or not ther are source files that required soelim(1) to
      * build.
      */
    bool need_soelim_flag;

    /**
      * The seen_datadir_flag instance variable is used to remember
      * whether or not we have seen content to be installed from
      * datadir/ into ${prefix}/share/<progname>/
      */
    bool seen_datadir_flag;

    /**
      * The install_datadir instance variable is used to remember
      * the name of files to be installed below $(datadir)/<progname>/
      *
      * This also includes files to be installed below $(datarootdir)
      */
    nstring_list install_datadir;

    /**
      * The seen_datarootdir_flag instance variable is used to remember whether
      * or not we have seen content to be installed from libdir/ into
      * ${prefix}/share/ and is usually for other applications' use.
      */
    bool seen_datarootdir_flag;

    /**
      * The seen_libdir_flag instance variable is used to remember
      * whether or not we have seen content to be installed from libdir/
      * into ${prefix}/lib/<progname>/
      */
    bool seen_libdir_flag;

    /**
      * The pkgconfig_sources instance variable is used to remember the
      * names of the source files to be installed below $(libdir)/pkgconfig/
      */
    nstring_list pkgconfig_sources;

    /**
      * The install_libdir instance variable is used to remember the
      * names of the files to be installed under $(libdir)/
      */
    nstring_list install_libdir;

    /**
      * The seen_sysconfdir_flag instance variable is used to
      * remember whether or not we have seen content to be
      * installed from libdir/ into /etc/
      */
    bool seen_sysconfdir_flag;

    /**
      * The man_sources instance variable is used to remember the set
      * of sourcfe files of man pages.
      */
    nstring_list man_sources;

    /**
      * The install_mandir instance variable is used to remember the set
      * of files to be installed into $(mandir)/
      */
    nstring_list install_mandir;

    /**
      * The all_doc instance variable is used to remember the list of
      * documentation files to be constructed by the "all-doc" target.
      */
    nstring_list all_doc;

    /**
      * The install_doc instance variable is used to remember the list
      * of documentation files to be installed by the "install-doc"
      * target.
      */
    nstring_list install_doc;

    /**
      * The program_prefix instance variable is used to remember
      * the prefix string to add to program names when installing.
      * Defaults to the empty string.
      */
    nstring program_prefix;

    /**
      * The program_suffix instance variable is used to remember
      * the suffix string to add to program names when installing.
      * Defaults to the empty string.
      */
    nstring program_suffix;

    /**
      * The all_i18n instance variable is used to remember set set
      * of i18n files to be built (*.po => *.mo).
      */
    nstring_list all_i18n;

    /**
      * The install_i18n instance variable is used to remember set set
      * of files to be installed under $(NLSDIR)/
      */
    nstring_list install_i18n;

    /**
      * The have_nlsdir_flag instance is used to remember whether or not
      * a line of the form AC_SUBST(NLSDIR) appears in the configure.ac
      * file.
      */
    bool have_nlsdir_flag;

    /**
      * The etc_msgfmt_sh_flag instance variable is used to remember whether
      * or not an "etc/msgfmt.sh" source file has been seen.  If present, this
      * will be used in preference to raw msgfmt(1) command.
      */
    bool etc_msgfmt_sh_flag;

    /**
      * The install_include_sources instance variable is used to rememebr
      * the list of include source files.
      */
    nstring_list install_include_sources;

    /**
      * The install_include instance variable is used to rememebr
      * the list of include files to be installed into $(includedir).
      * The filename all include "$(includedir)" as the first component.
      */
    nstring_list install_include;

    /**
      * The groff_macro_flag is used to remember whether or not
      * groff -ms or groff -mm flags have been seen during preprocessing.
      */
    bool groff_macro_flag;

    /**
      * The built_source instance variable i sused to remember the set of
      * catch-22 non-source files that need to be built early in the build.
      */
    nstring_list built_sources;

    /**
      * The extra_dist instance variable is used to remember the set
      * of source files that are also to be added to the tarball, even
      * though they appear on no other source file list.
      */
    nstring_list extra_dist;

    /**
      * The install_data_macro instance variable is used to remember
      * whether or not the INSTALL_DATA macro needs to be defined.
      */
    bool install_data_macro;

    typedef std::map<nstring, int> install_directories_t;

    /**
      * The install_directories instance variable is used to remember
      * the set of install directories as mentioned by other
      * remember_install_<i>something</i> methods.
      */
    install_directories_t install_directories;

    /**
      * The remember_install_directory_for method
      * is used to remember the directory to be installed.
      *
      * @param install_file
      *     The path of the file to be installed.  It will rip off the
      *     final component to construct the directory name.
      */
    void remember_install_directory_for(const nstring &install_file);

    /**
      * The have_ar_flag is used to remember whether or not there is a
      * "AC_CHECK_PROGS(AR, ar)" line in the configure.ac source file.
      */
    bool have_ar_flag;

    /**
      * The have_ranlib_flag is used to remember whether or not there is a
      * "AC_PROG_RANLIB" line in the configure.ac source file.
      */
    bool have_ranlib_flag;

    /**
      * The need_ghostscript_flag instance variable is used to remember
      * whether or not the build depends on the ghostscript package.
      */
    bool need_ghostscript_flag;

    /**
      * The am_data_data instance variable is used to remmeber the set
      * of files to be installed as data by automake.
      */
    nstring_list am_data_data;

    /**
      * The read_only instance variable is used to remember whether or
      * not the preprocessing stage has finished, at which point no more
      * changes to any instance variable should be occuring.
      */
    bool read_only; // MUST BE LAST

    /**
      * The check_read_write method is used by all methods that
      * change this instance's state, to be sure that modification is
      * appropriate.  This is a debugging technique, not a public method.
      */
    void check_read_write(const char *method_name);

    /**
      * The copy constructor.  Do not use.
      */
    process_data(const process_data &);

    /**
      * The assignment operator.  Do not use.
      */
    process_data &operator=(const process_data &);
};

// vim: set ts=8 sw=4 et :
#endif // AEMAKEGEN_PROCESS_DATA_H
