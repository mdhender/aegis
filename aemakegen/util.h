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

#ifndef AEMAKEGEN_UTIL_H
#define AEMAKEGEN_UTIL_H

#include <common/nstring.h>

/**
  * The is_a_source_file class method is used to determine whether
  * or not a filename looks like a C or C++ source file.
  * Does not match include files.
  *
  * @param filename
  *     The filename to be tested.
  * @returns
  *     true if the filename look like a C or C++ source file,
  *     false if it is anything else (or an include file).
  */
bool is_a_source_file(const nstring &filename);

/**
  * The is_an_include_file class method is used to determine whether
  * or not a filename looks like a C or C++ include file.
  * Does not match source files.
  *
  * @param filename
  *     The filename to be tested.
  * @returns
  *     true if the filename looks like a C or C++ include file,
  *     false if it is anything else (or a source file).
  */
bool is_an_include_file(const nstring &filename);

/**
  * The is_a_c_source_file class method is used to determine whether or
  * not a filename looks like a C source file.
  * Does not match include files.
  *
  * @param filename
  *     The filename to be tested.
  * @returns
  *     true if the filename looks like a C source file,
  *     false if it is anything else (or an include file).
  */
bool is_a_c_source_file(const nstring &filename);

/**
  * The filename_implies_progname class method is used to determine
  * whether or not the given file name implies the existence of a
  * program, and the program's name.
  *
  * @param filename
  *     The file name to be tested.
  */
bool filename_implies_progname(const nstring &filename);

/**
  * The progname_from_dir_of class method is used to form the name
  * of the program, given the blah/blah/main.c filename.  This trims
  * of the basename and replaces slashes with hyphens.
  *
  * @param filename
  *     The name of the file, the directory path of which is to be
  *     converted into a program name.
  */
nstring progname_from_dir_of(const nstring &filename);

/**
  * The is_a_cxx_source_file class method is used to determine whether
  * or not a filename looks like a C++ source file.
  * Does not match include files.
  *
  * @param filename
  *     The filename to be tested.
  * @returns
  *     true if the filename looks like a C++ source file,
  *     false if it is anything else (or an include file).
  */
bool is_a_cxx_source_file(const nstring &filename);

/**
  * The extension_implies_script function may be used to see if a
  * filename looks like it would be a script (e.g. ".sh" or ".pl", etc).
  */
bool extension_implies_script(const nstring &filename);

/**
  * The looks_like_a_man_page function is used to determine
  * whether a file looks like it contains a man(1) page.
  *
  * @param filename
  *     the file in question
  */
bool looks_like_a_man_page(const nstring &filename);

/**
  * The extract_man_page_details is used to extract the man page details
  *
  * @param filename
  *     the file in question
  * @returns
  *     An empty string if doesn't look like a man page.
  *     Or the form man[1-8]/name.[1-8] if it looks like a man page.
  */
nstring extract_man_page_details(const nstring &filename);

/**
  * The make_pseudo_dir function is used to turn an actual install
  * directory into the name of the bogus file used to track the fact
  * that the corresponding install directory has been created.
  *
  * @param dst_dir
  *     Destimnation directory, i.e. it includes as its first component
  *     a make macro dereference.
  * @returns
  *     a file name, no slashes.
  */
nstring make_pseudo_dir(const nstring &dst_dir);

/**
  * The make_pseudo_dir_for function is used to turn an actual install
  * file into the name of the bogus file used to track the fact that the
  * corresponding necessary install directory has been created.
  *
  * @param dst_dir
  *     Destination file, i.e. it includes as its first component a make
  *     macro dereference, and refers to a file, not a directory.
  * @returns
  *     a file name, no slashes.
  */
nstring make_pseudo_dir_for(const nstring &dst_file);

// vim: set ts=8 sw=4 et :
#endif // AEMAKEGEN_UTIL_H
