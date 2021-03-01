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
// MANIFEST: interface definition for aedist/input/cpio.c
//

#ifndef LIBAEGIS_INPUT_CPIO_H
#define LIBAEGIS_INPUT_CPIO_H

#include <input.h>

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
      */
    input_cpio(input_ty *deeper);

    // See base class for documentation.
    nstring name();

    // See base class for documentation.
    long length();

    // See base class for documentation.
    long read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    long ftell_inner();

    // See base class for documentation.
    bool is_remote() const;

    input_ty *child(nstring &name);

private:
    /**
      * The deeper instance variabl;e is sued to remember the input
      * source for this filter.
      */
    input_ty *deeper;

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
