//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001, 2002, 2005-2008, 2012 Peter Miller
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

#ifndef LIBAEGIS_INPUT_BASE64_H
#define LIBAEGIS_INPUT_BASE64_H

#include <libaegis/input.h>

class input_base64:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_base64();

    /**
      * The constructor.
      *
      * @param deeper
      *     the data source for this filter.
      */
    input_base64(input &deeper);

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

    // See base class for documentation.
    bool is_remote() const;

    static bool recognise(input &ip);

private:
    /**
      * The deeper instance variable is used to remember the data source
      * for this filter.
      */
    input deeper;

    off_t pos;
    int residual_bits;
    int residual_value;
    bool eof;

    /**
      * The default constructor.  Do not use.
      */
    input_base64();

    /**
      * The copy constructor.  Do not use.
      */
    input_base64(const input_base64 &arg);

    /**
      * The assignment operator.  Do not use.
      */
    input_base64 &operator=(const input_base64 &arg);
};


inline DEPRECATED bool
input_base64_recognise(input &ip)
{
    return input_base64::recognise(ip);
}

#endif // LIBAEGIS_INPUT_BASE64_H
// vim: set ts=8 sw=4 et :
