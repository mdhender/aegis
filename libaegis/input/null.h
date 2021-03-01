//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_INPUT_NULL_H
#define LIBAEGIS_INPUT_NULL_H

#include <libaegis/input.h>

/**
  * The input_null class is used to represent and input which is always
  * empty.
  */
class input_null:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_null();

    /**
      * The default constructor.
      */
    input_null();

    // See base class for documentation.
    nstring name();

    // See base class for documentation.
    long length();

    // See base class for documentation.
    long read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    long ftell_inner();

private:
    /**
      * The copy constructor.  Do not use.
      */
    input_null(const input_null &);

    /**
      * The assignment operator.  Do not use.
      */
    input_null &operator=(const input_null &);
};

#endif // LIBAEGIS_INPUT_NULL_H
