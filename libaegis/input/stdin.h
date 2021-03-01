//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2002, 2005, 2006, 2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_INPUT_STDIN_H
#define LIBAEGIS_INPUT_STDIN_H

#include <libaegis/input.h>

/**
  * The input_stdin class is used to represent an input source which
  * reads from the standard input.
  */
class input_stdin:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_stdin();

    /**
      * The default constructor.
      */
    input_stdin();

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

private:
    off_t pos;
    bool unbuffered;

    /**
      * The copy constructor.  Do not use.
      */
    input_stdin(const input_stdin &arg);

    /**
      * The default constructor.  Do not use.
      */
    input_stdin &operator=(const input_stdin &arg);
};

#endif // LIBAEGIS_INPUT_STDIN_H
// vim: set ts=8 sw=4 et :
