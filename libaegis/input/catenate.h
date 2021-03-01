//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2005, 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_INPUT_CATENATE_H
#define LIBAEGIS_INPUT_CATENATE_H

#include <libaegis/input.h>

class input_catenate:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_catenate();

    /**
      * The constructor.
      */
    input_catenate(input deeper[], size_t ndeeper, bool close_on_close);

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
    input *deeper;
    size_t ndeeper;
    bool delete_on_close;
    size_t selector;
    size_t pos;

    /**
      * The default constructor.  Do not use.
      */
    input_catenate();

    /**
      * The copy constructor.  Do not use.
      */
    input_catenate(const input_catenate &arg);

    /**
      * The assignment operator.  Do not use.
      */
    input_catenate &operator=(const input_catenate &arg);
};

#endif // LIBAEGIS_INPUT_CATENATE_H
