//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1999, 2002, 2005, 2006, 2008 Peter Miller
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

#ifndef AEGIS_INPUT_ENV_H
#define AEGIS_INPUT_ENV_H

#include <libaegis/input.h>

class input_env:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_env();

    /**
      * The constructor.
      */
    input_env(const nstring &name);

    // See base class for documentation.
    nstring name();

    // See base class for documentation.
    long length();

    // See base class for documentation.
    long read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    long ftell_inner();

private:
    nstring gname;
    nstring base;
    size_t pos;

    /**
      * The default constructor.  Do not use.
      */
    input_env();

    /**
      * The copy constructor.  Do not use.
      */
    input_env(const input_env &arg);

    /**
      * The assignment operator.  Do not use.
      */
    input_env &operator=(const input_env &arg);
};

input input_env_open(const char *name);

#endif // AEGIS_INPUT_ENV_H
