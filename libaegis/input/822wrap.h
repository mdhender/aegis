//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2005 Peter Miller;
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
// MANIFEST: interface definition for aedist/input/822wrap.c
//

#ifndef AEDIST_INPUT_822WRAP_H
#define AEDIST_INPUT_822WRAP_H

#include <input.h>

/**
  * The input_822wrap class may be used to transparently join
  * continuation lines when reading rfc822 headers.
  */
class input_822wrap:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_822wrap();

    /**
      * The constructor.
      */
    input_822wrap(input_ty *deeper, bool delete_on_delete);

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

    // See base class for documentation.
    bool is_remote() const;

private:
    /**
      * The deeper instance variable is used to remember the data source
      * for this filter.
      */
    input_ty *deeper;

    /**
      * The delete_on_delete instance variable is used to remember
      * whether or not to delete the data source in the destructor.
      */
    bool delete_on_delete;

    /**
      * The pos instance variable is used to remember the byte position
      * withing this input stream.
      */
    long pos;

    /**
      * The column instance variable is used to remember the current
      * column of the cursor position.  Zero based.
      */
    int column;

    /**
      * The default constructor.  Do not use.
      */
    input_822wrap();

    /**
      * The copy constructor.  Do not use.
      */
    input_822wrap(const input_822wrap &arg);

    /**
      * The assignment operator.  Do not use.
      */
    input_822wrap &operator=(const input_822wrap &arg);
};

#endif // AEDIST_INPUT_822WRAP_H
