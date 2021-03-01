//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2005 Peter Miller;
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
// MANIFEST: interface definition for aetar/input/tar.c
//

#ifndef AETAR_INPUT_TAR_H
#define AETAR_INPUT_TAR_H

#include <input.h>

/**
  * The input_tar class is used to represent a tar archive.
  */
class input_tar:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_tar();

    /**
      * The constructor.
      */
    input_tar(input_ty *deeper);

    /**
      * The child method is used to obtain the next archive element.
      */
    input_ty *child(nstring &archive_name);

    // See base class for documentation.
    nstring name();

    // See base class for documentation.
    long length();

    // See base class for documentation.
    long read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    long ftell_inner();

private:
    input_ty *deeper;

    nstring read_data_as_string(size_t hsize);
    void padding();

    /**
      * The default constructor.
      */
    input_tar();

    /**
      * The copy constructor.
      */
    input_tar(const input_tar &arg);

    /**
      * The assignment operator.
      */
    input_tar &operator=(const input_tar &arg);
};

#endif // AETAR_INPUT_TAR_H
