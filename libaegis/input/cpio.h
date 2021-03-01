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

#ifndef LIBAEGIS_INPUT_CPIO_H
#define LIBAEGIS_INPUT_CPIO_H

#include <libaegis/input.h>

/**
  * The input_cpio class is used to represent a CPIO archive.
  */
class input_cpio:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_cpio();

    /**
      * The constructor.
      *
      * @param deeper
      *     The input data source for this filter.
      */
    input_cpio(input &deeper);

    // See base class for documentation.
    nstring name();

    // See base class for documentation.
    off_t length();

    // See base class for documentation.
    ssize_t read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    off_t ftell_inner();

    // See base class for documentation.
    bool is_remote() const;

    /**
      * The child method is used to obtain an input which may be read to
      * obtain the data of a member of the CPIO archive.
      *
      * If there are no more members, a closed input will be returned.
      * Use the input::is_open() method to work this out.
      */
    input child(nstring &name);

private:
    /**
      * The deeper instance variable is used to remember the input
      * source for this filter.
      */
    input deeper;

    /**
      * The hex8 method is used to obtain an eight digit hexadecimal
      * number from the input.
      */
    long hex8();

    /**
      * The hex_digit method is used to obtain a hexadecimal digit from
      * the input.
      */
    int hex_digit(bool &arg);

    /**
      * The get_name method is used to extract the archive name from the
      * deeper input.
      *
      * @param len
      *     The length of the name, not including the terminating NUL
      * @returns
      *     a string containing the name
      */
    nstring get_name(long len);

    /**
      * The padding method is used advance to the next 4-byte boundary,
      * if necessary.
      */
    void padding();

    /**
      * The default constructor.  Do not use.
      */
    input_cpio();

    /**
      * The copy constructor.  Do not use.
      */
    input_cpio(const input_cpio &arg);

    /**
      * The assignment operator.  Do not use.
      */
    input_cpio &operator=(const input_cpio &arg);
};

#endif // LIBAEGIS_INPUT_CPIO_H
// vim: set ts=8 sw=4 et :
