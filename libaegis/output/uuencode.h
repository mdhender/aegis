//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2005, 2006 Peter Miller
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
// MANIFEST: interface definition for libaegis/output/uuencode.c
//

#ifndef LIBAEGIS_OUTPUT_UUENCODE_H
#define LIBAEGIS_OUTPUT_UUENCODE_H

#include <common/nstring.h>
#include <libaegis/output.h>

/**
  * The output_uuencode_ty class is used to represent the state of a
  * filter which unix-to-unix encodes the data being written.  See
  * uuencode(1) for more information.
  */
class output_uuencode_ty:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_uuencode_ty();

    /**
      * The constructor.
      *
      * \param deeper
      *     The deeper output stream this filter is to write its output to.
      * \param close_on_close
      *     Whether or not to delete the deeper output stream in our
      *     destructor.
      */
    output_uuencode_ty(output_ty *deeper, bool close_on_close);

    // See base class for documentation.
    string_ty *filename() const;

    // See base class for documentation.
    const char *type_name() const;

    // See base class for documentation.
    long ftell_inner() const;

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
      * The deeeper instance variable is used to remember the deeper
      * output stream this filter is to write its output to.
      */
    output_ty *deeper;

    /**
      * The deeeper instance variable is used to remember whether or not
      * to delete the deeper output stream in our destructor.
      */
    bool close_on_close;

    /**
      * The residual_value instance variable is used to remember the
      * cumulative bits, accumulated 8 bits per inoput characters,
      * depleted at 6 bits per output character.
      */
    unsigned residual_value;

    /**
      * The residual_bits instance variable is used to remember the
      * number of bits accumulated fo far in the residual_value.
      */
    int residual_bits;

    /**
      * The obuf instance variable is used to remember the output line
      * being accumulated.
      */
    char obuf[64 * 4 / 3];

    /**
      * The opos instance variable is used to remember how much of the
      * obuf has been consumed to date, the numer of output characters.
      */
    int opos;

    /**
      * The ipos instance variable is used to remember how many input
      * characters have been consumed to date in building the obuf.
      */
    int ipos;

    /**
      * The pos instance variable is used to remember the current output
      * file position.
      */
    long pos;

    /**
      * The bol instance variable is used to remember whether or not we
      * are at the start of a line.
      */
    bool bol;

    /**
      * The begun instance variable is used to remember whether or not
      * we have written the "begin" line yet.
      */
    bool begun;

    /**
      * The file_name instance variable is used to remember the name of
      * the input file, with any ".uu*" suffix removed.
      */
    nstring file_name;

    /**
      * The default constructor.  Do not use.
      */
    output_uuencode_ty();

    /**
      * The copy constructor.  Do not use.
      */
    output_uuencode_ty(const output_uuencode_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    output_uuencode_ty &operator=(const output_uuencode_ty &);
};


inline DEPRECATED output_ty *
output_uuencode(output_ty *deeper, bool close_on_close)
{
    return new output_uuencode_ty(deeper, close_on_close);
}

#endif // LIBAEGIS_OUTPUT_UUENCODE_H
