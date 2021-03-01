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
// MANIFEST: interface definition for libaegis/output/bit_bucket.c
//

#ifndef LIBAEGIS_OUTPUT_BIT_BUCKET_H
#define LIBAEGIS_OUTPUT_BIT_BUCKET_H

#include <nstring.h>
#include <output.h>

/**
  * The output_bit_bucket class represents a NULL output stream.  The
  * output written to such a stream will be discarded.
  */
class output_bit_bucket:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_bit_bucket();

    /**
      * The default constructor.
      */
    output_bit_bucket();

    // See base class for documentation.
    string_ty* filename() const;

    // See base class for documentation.
    const char *type_name() const;

    // See base class for documentation.
    long int ftell_inner() const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void end_of_line_inner();

    // See base class for documentation.
    int page_width() const;

    // See base class for documentation.
    int page_length() const;

private:
    /**
      * The file_name instance variable is used to remember the name of
      * this "file".
      */
    nstring file_name;

    /**
      * The pos instance variable is used to remember the current output
      * position.
      */
    long pos;

    /**
      * The copy constructor.  Do not use.
      */
    output_bit_bucket(const output_bit_bucket &);

    /**
      * The copy constructor.  Do not use.
      */
    output_bit_bucket &operator=(const output_bit_bucket &);
};

#endif // LIBAEGIS_OUTPUT_BIT_BUCKET_H
