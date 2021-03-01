//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_INPUT_STRING_H
#define LIBAEGIS_INPUT_STRING_H

#include <libaegis/input.h>

/**
  * The input_string class is used to represent an input source obtained
  * from a string.
  */
class input_string:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_string();

    /**
      * The constructor.
      */
    input_string(const nstring &arg);

    // See base class for documentation.
    nstring name();

    // See base class for documentation.
    long length();

    // See base class for documentation.
    long read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    long ftell_inner();

private:
    nstring base;
    size_t pos;

    /**
      * The default constructor.  Do not use.
      */
    input_string();

    /**
      * The copy constructor.  Do not use.
      */
    input_string(const input_string &arg);

    /**
      * The assignment operator.  Do not use.
      */
    input_string &operator=(const input_string &arg);
};

#endif // LIBAEGIS_INPUT_STRING_H
