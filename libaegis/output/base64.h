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
// MANIFEST: interface definition for libaegis/output/base64.c
//

#ifndef LIBAEGIS_OUTPUT_BASE64_H
#define LIBAEGIS_OUTPUT_BASE64_H

#include <output.h>

class output_base64_ty:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_base64_ty();

    /**
      * The constructor.
      *
      * \param deeper
      *     The deeper output, the place this filter writes the filtered
      *     output.
      * \param delete_deeper
      *     Wherther or not to delete the deeper output in this
      *     instance's destructor.
      */
    output_base64_ty(output_ty *deeper, bool delete_deeper);

    // See base class for documentation.
    string_ty* filename() const;

    // See base class for documentation.
    const char *type_name() const;

    // See base class for documentation.
    long ftell_inner() const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void end_of_line_inner();

    // See base class for documentation.
    int output_base64_ty::page_width() const;

    // See base class for documentation.
    int output_base64_ty::page_length() const;

private:
    /**
      * The deeper instance variable is used to remember the deeper
      * output to which this filter writes its filtered output.
      */
    output_ty *deeper;

    /**
      * The delete_deeper instance variable is used to remember whether
      * or not the deeper output stream should be deleted in this
      * instance's destructor.
      */
    bool delete_deeper;

    /**
      * The residual_value instance variable is used to remember the
      * as-yet-unwritten bits of the value.
      */
    unsigned int residual_value;

    /**
      * The residual_bits instance variable is used to remember the
      * number of bits held by the residual_value instance variable.
      */
    int residual_bits;

    /**
      * The output_column instance variable is used to remember what
      * column the output is up to.
      */
    int output_column;

    /**
      * The pos instance variable is used to remember the file position.
      */
    long pos;

    /**
      * The bol instance variable is used to remember whether or not we
      * are positioned at the beginning of a line.
      */
    bool bol;

    /**
      * The default constructor.  Do not use.
      */
    output_base64_ty();

    /**
      * The copy constructor.  Do not use.
      */
    output_base64_ty(const output_base64_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    output_base64_ty operator=(const output_base64_ty &);
};

inline DEPRECATED output_ty *
output_base64(output_ty *deeper, bool delete_deeper)
{
    return new output_base64_ty(deeper, delete_deeper);
}

#endif // LIBAEGIS_OUTPUT_BASE64_H
