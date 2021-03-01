//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1999, 2001, 2002, 2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
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
// MANIFEST: interface definition for libaegis/input/file.c
//

#ifndef AEGIS_INPUT_FILE_H
#define AEGIS_INPUT_FILE_H

#include <input.h>

/**
  * The input_file class is used to represent an input stream which is
  * read from a file.
  */
class input_file:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_file();

    /**
      * The constructor.
      *
      * @param path
      *     The path of the file to be opened.
      * @param unlink_on_close
      *     If true, the file is to be unlinked in the destructor.
      *     Defaults to false if not specified.
      * @param empty_if_absent
      *     If true, the file is to be treated as if it was empty if it
      *     does not exist.  No warning message will be issued.  Defaults
      *     to false if not specified.
      */
    input_file(const nstring &path, bool unlink_on_close = false,
	bool empty_if_absent = false);

    // See base class for documentation.
    nstring name();

    // See base class for documentation.
    long length();

    // See base class for documentation.
    void keepalive();

    // See base class for documentation.
    long read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    long ftell_inner();

    /**
      * The unlink_on_close method may be used to specify that the file
      * is to be removed when it is closed (in the destructor).
      */
    void unlink_on_close();

private:
    nstring path;
    int fd;
    bool unlink_on_close_flag;
    long pos;
    bool empty_if_absent_flag;

    /**
      * The default constructor.  Do not use.
      */
    input_file();

    /**
      * The copy constructor.  Do not use.
      */
    input_file(const input_file &arg);

    /**
      * The assignment operator.  Do not use.
      */
    input_file &operator=(const input_file &arg);
};

/**
  * The input_file_open function is used to open thr standatd input, if
  * the path is "-", or a URL if it looks like a URL, or otherwise an
  * ordinary file.
  *
  * @param path
  *     The path of the file to be opened.
  * @param unlink_on_close
  *     If true, the file is to be unlinked in the destructor.  Defaults
  *     to false if not specified.
  * @param empty_if_absent
  *     If true, the file is to be treated as if it was empty if it does
  *     not exist.  No warning message will be issued.  Defaults to
  *     false if not specified.
  */
input_ty *input_file_open(const nstring &fn, bool unlink_on_close = false,
    bool empty_if_absent = false);

/**
  * As above, but soon to be deprecated.
  */
input_ty *input_file_open(struct string_ty *fn, bool unlink_on_close = false);

#endif // AEGIS_INPUT_FILE_H
