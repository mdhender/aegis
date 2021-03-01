//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2005 Peter Miller;
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
// MANIFEST: interface definition for libaegis/output/quoted_print.c
//

#ifndef LIBAEGIS_OUTPUT_QUOTED_PRINT_H
#define LIBAEGIS_OUTPUT_QUOTED_PRINT_H

#include <output.h>

class output_quoted_printable_ty:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_quoted_printable_ty();

    /**
      * The constructor.
      *
      * \param deeper
      *      The output stream this filter uses to write its output.
      * \param close_on_close
      *     Whether or not the deeper output stream is to be deleted in
      *     this instance's destructor.
      * \param allow_international_characters
      *     Whether or not internalional characters are allowed or not.
      */
    output_quoted_printable_ty(output_ty *deeper, bool close_on_close = true,
	bool allow_international_characters = false);

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
    void flush_inner();

    // See base class for documentation.
    int page_width() const;

    // See base class for documentation.
    int page_length() const;

private:
    /**
      * The deeper instance variable is used to remember the output
      * stream this filter uses to write its output.
      */
    output_ty *deeper;

    /**
      * The close_on_c lose instance variable is used to remember
      * whether or not the deeper output stream is to be deleted in this
      * instance's destructor.
      */
    bool close_on_close;

    /**
      * The allow_international_characters instance variable is used
      * to remember whether or not internalional characters are allowed
      * (true) or to be quoted as unprintable (false).
      */
    bool allow_international_characters;

    struct glyph_t
    {
	unsigned char text;
	bool quote_it;
	int width;
	int cumulative;
    };

    enum { MAX_LINE_LEN = 76 }; // As per RFC 1521

    /**
      * The glyph instance variable is used
      * to remember
      */
    glyph_t glyph[MAX_LINE_LEN + 1];

    /**
      * The pos instance variable is used to remember the current output
      * position withing the glyph array.
      */
    int pos;

    /**
      * The eoln method is used to send the contents of the glyph array
      * to the deeper output stream.
      *
      * \param soft
      *     Either the array got too long (true),
      *     or a newline was seen (false).
      */
    void eoln(bool soft);

    /**
      * The newline_partial method is used to output a soft end-of-line
      * when the quoted line length exceeds the MAX_LINE_LEN limit.
      */
    void eoln_partial();

    /**
      * The eoln_hard method is used to output a hard end-of-line, when
      * a newline character is seen.
      */
    void eoln_hard();

    /**
      * The default constructor.  Do not use.
      */
    output_quoted_printable_ty();

    /**
      * The copy constructor.  Do not use.
      */
    output_quoted_printable_ty(const output_quoted_printable_ty &);

    /**
      * The assignment apperator.  Do not use.
      */
    output_quoted_printable_ty &operator=(const output_quoted_printable_ty &);
};


inline DEPRECATED output_ty *
output_quoted_printable(output_ty *deeper, int close_on_close,
    int allow_intl)
{
    return new output_quoted_printable_ty(deeper, close_on_close, allow_intl);
}

#endif // LIBAEGIS_OUTPUT_QUOTED_PRINT_H
