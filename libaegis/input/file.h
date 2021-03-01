//
// aegis - project change supervisor
// Copyright (C) 1994-2001, 2002, 2005, 2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see
// <http://www.gnu.org/licenses/>.
//

#ifndef AEGIS_INPUT_FILE_H
#define AEGIS_INPUT_FILE_H

#include <libaegis/input.h>

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
    off_t length();

    // See base class for documentation.
    void keepalive();

    // See base class for documentation.
    ssize_t read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    off_t ftell_inner();

    /**
      * The unlink_on_close method may be used to specify that the file
      * is to be removed when it is closed (in the destructor).
      */
    void unlink_on_close();

private:
    nstring path;
    int fd;
    bool unlink_on_close_flag;
    off_t pos;
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
  * @param fn
  *     The path of the file to be opened.
  * @param unlink_on_close
  *     If true, the file is to be unlinked in the destructor.  Defaults
  *     to false if not specified.
  * @param empty_if_absent
  *     If true, the file is to be treated as if it was empty if it does
  *     not exist.  No warning message will be issued.  Defaults to
  *     false if not specified.
  */
input input_file_open(const nstring &fn, bool unlink_on_close = false,
    bool empty_if_absent = false);

/**
  * The input_file_open function is used to open thr standatd input, if
  * the path is "-", or a URL if it looks like a URL, or otherwise an
  * ordinary file.
  *
  * @param fn
  *     The path of the file to be opened.
  * @param unlink_on_close
  *     If true, the file is to be unlinked in the destructor.  Defaults
  *     to false if not specified.
  * @note
  *     This function will soon be deprecated.
  */
input input_file_open(struct string_ty *fn, bool unlink_on_close = false);

#endif // AEGIS_INPUT_FILE_H
// vim: set ts=8 sw=4 et :
