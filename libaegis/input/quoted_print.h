//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2005, 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_INPUT_QUOTED_PRINT_H
#define LIBAEGIS_INPUT_QUOTED_PRINT_H

#include <libaegis/input.h>

class input_quoted_printable:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_quoted_printable();

    /**
      * The constructor.
      *
      * @param deeper
      *     the source of data for this filter.
      */
    input_quoted_printable(input &deeper);

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
      * The deeper instance variable is used to remember the source of
      * data for this filter.
      */
    input deeper;

    bool eof;
    long pos;

    /**
      * The default constructor.
      */
    input_quoted_printable();

    /**
      * The copy constructor.
      */
    input_quoted_printable(const input_quoted_printable &arg);

    /**
      * The assignment operator.
      */
    input_quoted_printable &operator=(const input_quoted_printable &arg);
};

#endif // LIBAEGIS_INPUT_QUOTED_PRINT_H
