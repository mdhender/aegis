//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2007 Peter Miller;
//
//      This program is free software; you can redistribute it and/or
//      modify it under the terms of the GNU General Public License,
//      version 2, as published by the Free Software Foundation.
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

#ifndef MAKEGEN_TARGET_MAKEFILE_H
#define MAKEGEN_TARGET_MAKEFILE_H

#include <aemakegen/target.h>
#include <common/symtab/template.h>

/**
  * The target_makefile class is used to represent the processing
  * necessary to produce a Makefile.in file for this project.
  */
class target_makefile:
    public target
{
public:
    /**
      * The destructor.
      */
    virtual ~target_makefile();

    /**
      * The default constructor.
      */
    target_makefile();

protected:
    // See base class for documentation.
    void begin();

    // See base class for documentation.
    void process(const nstring &filename, bool is_a_script);

    // See base class for documentation.
    void end();

    /**
      * The c_include_dependencies method is used to scan C++ source
      * files for include dependencies.
      *
      * @param filename
      *     The name of the file to be scanned.
      * @returns
      *     a space separated list of file names.
      */
    nstring c_include_dependencies(const nstring &filename);

    /**
      * The c_include_dependencies method is used to scan a C++ source
      * file for include dependencies.
      *
      * @param filename
      *     The name of the file to be scanned.
      * @param result
      *     Where to put the names of discovered include files
      */
    void c_include_dependencies(nstring_list &result,
	const nstring &filename);

    void c_directive(const char *line, nstring_list &result);

private:
    bool seen_c;
    bool seen_c_plus_plus;
    symtab<int> dir_table;
    nstring_list clean_files;
    nstring_list programs;
    nstring_list test_files;
    nstring_list man_files;
    nstring_list all;
    nstring_list install_bin;
    symtab<nstring_list> object_list;
    bool have_groff;

    void recursive_mkdir(const nstring &a_src_dir, const nstring &a_dst_dir,
	const nstring &flavor = "datadir");
    void remember_program(const nstring &name);
    void remember_object_file(const nstring &name);
    void process_configure_dot_ac(const nstring &fn);

    /**
      * The copy constructor.  Do not use.
      */
    target_makefile(const target_makefile &);

    /**
      * The assignment operator.  Do not use.
      */
    target_makefile &operator=(const target_makefile &);
};

#endif // MAKEGEN_TARGET_MAKEFILE_H
