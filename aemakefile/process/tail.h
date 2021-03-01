//
// aegis - project change supervisor
// Copyright (C) 2004-2006, 2008, 2012 Peter Miller
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
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AEMAKEFILE_PROCESS_TAIL_H
#define AEMAKEFILE_PROCESS_TAIL_H

#include <aemakefile/process.h>
#include <common/symtab/template.h>

/**
  * The process_tail class is used to represent the processing required
  * to generate the tail end of the generated Makefile.in file.
  */
class process_tail:
    public process
{
public:
    /**
      * The destructor.
      */
    virtual ~process_tail();

    /**
      * The constructor.
      */
    process_tail(printer &arg);

    // See base class for documentation.
    void per_file(const nstring &filename);

    // See base class for documentation.
    void postlude();

private:
    nstring_list aefp_files;
    nstring_list aegis_files;
    nstring_list aemeasure_files;
    nstring_list clean_files;
    nstring_list comdir_files;
    nstring_list commands;
    nstring_list commands_bin;
    nstring_list commands_install;
    nstring_list *common_files;
    nstring_list datadir_files;
    nstring_list ps_doc_files;
    nstring_list dvi_doc_files;
    nstring_list txt_doc_files;
    nstring_list fmtgen_files;
    nstring_list install_doc_file;
    nstring_list install_doc_files;
    nstring_list install_po_files;
    nstring_list *libaegis_files;
    nstring_list libdir_files;
    nstring_list man_files;
    nstring_list po_files;
    nstring_list scripts;
    nstring_list test_files;
    symtab<nstring_list> dir_st;

    void recursive_mkdir(const nstring &src_dir, const nstring &dst_dir,
        const nstring &suffix);

    /**
      * The default constructor.  Do not use.
      */
    process_tail();

    /**
      * The copy constructor.  Do not use.
      */
    process_tail(const process_tail &);

    /**
      * The assignment operator.  Do not use.
      */
    process_tail &operator=(const process_tail &);
};

#endif // AEMAKEFILE_PROCESS_TAIL_H
// vim: set ts=8 sw=4 et :
