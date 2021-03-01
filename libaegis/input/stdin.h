//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2005, 2006 Peter Miller
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
// MANIFEST: interface definition for libaegis/input/stdin.c
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
    long length();

    // See base class for documentation.
    void keepalive();

    // See base class for documentation.
    long read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    long ftell_inner();

private:
    long pos;
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
