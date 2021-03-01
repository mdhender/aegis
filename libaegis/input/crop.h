//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: interface definition for libaegis/input/crop.c
//

#ifndef LIBAEGIS_INPUT_CROP_H
#define LIBAEGIS_INPUT_CROP_H

#include <input.h>

/**
  * The input_crop class is used to represent an input which will read
  * only a portion of the deeper input, from the current position.
  */
class input_crop:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_crop();

    /**
      * The constructor.
      */
    input_crop(input_ty *deeper, bool close_on_close, long length);

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

    void set_name(const nstring &arg) { name_cache = arg; }

private:
    input_ty *deeper;
    bool delete_on_close;
    long maximum;
    long pos;
    nstring name_cache;

    /**
      * The default constructor.  Do not use.
      */
    input_crop();

    /**
      * The copy constructor.  Do not use.
      */
    input_crop(const input_crop &arg);

    /**
      * The assignment operator.  Do not use.
      */
    input_crop &operator=(const input_crop &arg);
};

#endif // LIBAEGIS_INPUT_CROP_H
