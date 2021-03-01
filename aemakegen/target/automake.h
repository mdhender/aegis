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

#ifndef AEMAKEGEN_TARGET_AUTOMAKE_H
#define AEMAKEGEN_TARGET_AUTOMAKE_H

#include <common/symtab/template.h>

#include <aemakegen/target.h>

/**
  * The target_automake class is used to represent the code generation
  * needed to build a Makefile.am automake input file from the Aegis
  * file manifest.
  */
class target_automake:
    public target
{
public:
    /**
      * The destructor.
      */
    virtual ~target_automake();

    /**
      * The default constructor.
      */
    target_automake();

protected:
    // See base class for documentation.
    void begin();

    // See base class for documentation.
    void process(const nstring &filename, bool is_a_script);

    // See base class for documentation.
    void end();

private:
    /**
      * The source_list instance variable is used to remember the source
      * files for a given directory (program or library), indexed by
      * top-level directory name.
      */
    symtab<nstring_list> source_list;

    /**
      * The remember_source_file method is used to append the given
      * source file to the appropriate list in the source_list instance
      * variable.
      */
    void remember_source_file(const nstring &filename);

    /**
      * The include_list instance variable is used to remember the
      * include files for a given directory (program or library),
      * indexed by top-level directory name.
      */
    symtab<nstring_list> include_list;

    /**
      * The remember_include_file method is used to append the given
      * include file to the appropriate list in the include_list
      * instance variable.
      */
    void remember_include_file(const nstring &filename);

    /**
      * The dist_clean_files instance variable is used to remember the
      * files which need to be remove by a dist-clean.
      */
    nstring_list dist_clean_files;

    /**
      * The bin_programs instance variable is used to remember the list
      * of programs to be build and installed.  The name mirrors the
      * bin_PROGRAMS automake variable.
      */
    nstring_list bin_programs;

    /**
      * The check_programs instance variable is used to remember the
      * list of programs to be build in order to run tests.  The name
      * mirrors the check_PROGRAMS automake variable.
      */
    nstring_list check_programs;

    /**
      * The noinst_programs instance variable is used to remember
      * the list of programs which are to be build but are not to be
      * installed.  The name mirrors the noinst_PROGRAMS automake variable.
      */
    nstring_list noinst_programs;

    /**
      * The bin_scripts instance variable is used to remember the list
      * of scripts to be installed.  The name mirrors the bin_SCRIPTS
      * automake variable.
      */
    nstring_list bin_scripts;

    /**
      * The check_scripts instance variable is used to remember the
      * list of scripts needed to run the tests.  The name mirrors the
      * check_SCRIPTS automake variable.
      */
    nstring_list check_scripts;

    /**
      * The noinst_scripts instance variable is used to remember the
      * list of scripts not to be installed.  The name mirrors the
      * noinst_SCRIPTS automake variable.
      */
    nstring_list noinst_scripts;

    /**
      * The remember_program method is used to place a program name into
      * one of the bin_programs or noinst_programs lists.  Typically
      * programs which perform specific tests are built but not
      * installed.
      *
      * @param name
      *     The name of the program.
      */
    void remember_program(const nstring &name);

    /**
      * The noinst_header instance variable is used to remember the
      * list of header files not to be installed.  The name mirrors the
      * noinst_HEADER automake variable.
      */
    nstring_list noinst_header;

    /**
      * The test_files instance variable is used to remember the list of
      * Aegis test files.
      */
    nstring_list test_files;

    /**
      * The man_mans instance variable is used to remember the list
      * of manual page source files, it is named after the man_MANS
      * automake variable.  Usually there is one manual page for each
      * program.
      */
    nstring_list man_mans;

    /**
      * The extra_dist instance variable is used to remember the list
      * of primary source files not mentined in any of the other lists.
      * The name mirrors the EXTRA_DIST automake variable.
      */
    nstring_list extra_dist;

    /**
      * The use_libtool instance variable is used to remember whether
      * or not the libtool facilities are to be used for libraries.
      * Defaults to false.
      */
    bool use_libtool;

    /**
      * The use_dot_man instance variable is used to remember whether
      * or not to install "*.man" ffiles as "*.1" files.e Defaults to
      * false.
      */
    bool use_dot_man;

    /**
      * The pkgdatadir instance variable is used to remember the files
      * to be installed into $(DATADIR)/$(PACKAGE).  The name mirrors
      * the dist_pkgdatadir_DATA makefile variable.
      */
    nstring_list pkgdatadir;

    /**
      * The pkglibdir instance variable is used to remember the files
      * to be installed into $(LIBDIR)/$(PACKAGE).  The name mirrors
      * the dist_pkglibdir_DATA makefile variable.
      */
    nstring_list pkglibdir;

    /**
      * The copy constructor.  Do not use.
      */
    target_automake(const target_automake &);

    /**
      * The assignment operator.  Do not use.
      */
    target_automake &operator=(const target_automake &);
};

#endif // AEMAKEGEN_TARGET_AUTOMAKE_H
