//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1999, 2001, 2002, 2005, 2006, 2008 Peter Miller
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

#ifndef FILE_H
#define FILE_H

#include <common/nstring.h>

void copy_whole_file(string_ty *from, string_ty *to, int cmt);
void copy_whole_file(const nstring &from, const nstring &to, bool cmt);

/**
  * The read_whole_file function is used to read a file into a string.
  * The file is assumed to be a text file.
  *
  * @param filename
  *     pathname of file to be read
  * @returns
  *     Pointer to string in dynamic memory containing text of file.
  *     Any trailing white space will have been removed.
  * @note
  *     Assumes the user has already been set.
  */
string_ty *read_whole_file(string_ty *filename) DEPRECATED;

/**
  * The read_whole_file function is used to read a file into a string.
  * The file is assumed to be a text file.
  *
  * @param filename
  *     pathname of file to be read
  * @returns
  *     Pointer to string in dynamic memory containing text of file.
  *     Any trailing white space will have been removed.
  * @note
  *     Assumes the user has already been set.
  */
nstring read_whole_file(const nstring &filename);

/**
  * The files_are_different function is used to compare two files.
  * Works for both text and binary files.
  *
  * @param f1
  *     The path of a file to be compared.
  * @param f2
  *     The path of a file to be compared.
  * @returns
  *     int; non-zero (true) if the files are different in any way, zero
  *     (false) if the files are identical.
  */
int files_are_different(string_ty *f1, string_ty *f2);

/**
  * The files_are_different function is used to compare two files.
  * Works for both text and binary files.
  *
  * @param f1
  *     The path of a file to be compared.
  * @param f2
  *     The path of a file to be compared.
  * @returns
  *     bool; true if the files are different in any way, false if the
  *     files are identical.
  */
bool files_are_different(const nstring &f1, const nstring &f2);

void file_from_string(string_ty *, string_ty *, int);
void cat_string_to_stdout(string_ty *);

#endif // FILE_H
