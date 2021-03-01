//
// aegis - project change supervisor
// Copyright (C) 2001, 2002, 2005, 2006, 2008, 2011 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_OUTPUT_FILTER_QUOTED_PRINT_H
#define LIBAEGIS_OUTPUT_FILTER_QUOTED_PRINT_H

#include <libaegis/output/filter.h>

/**
  * The output_filter_quoted_printable class is used to filter an output
  * stream to include quored printable encoding of the content, as required.
  */
class output_filter_quoted_printable:
    public output_filter
{
public:
    /**
      * The destructor.
      */
    virtual ~output_filter_quoted_printable();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *      The output stream this filter uses to write its output.
      * @param allow_international_characters
      *     Whether or not internalional characters are allowed or not.
      */
    static pointer create(const output::pointer &deeper,
        bool allow_international_characters = false);

protected:
    // See base class for documentation.
    nstring type_name(void) const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void end_of_line_inner(void);

    // See base class for documentation.
    int page_width(void) const;

private:
    /**
      * The constructor.  It is private on purpose, use the "create" class
      * method instead.
      *
      * @param deeper
      *      The output stream this filter uses to write its output.
      * @param allow_international_characters
      *     Whether or not internalional characters are allowed or not.
      */
    output_filter_quoted_printable(const output::pointer &deeper,
        bool allow_international_characters = false);

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
      * @param soft
      *     Either the array got too long (true),
      *     or a newline was seen (false).
      */
    void eoln(bool soft);

    /**
      * The newline_partial method is used to output a soft end-of-line
      * when the quoted line length exceeds the MAX_LINE_LEN limit.
      */
    void eoln_partial(void);

    /**
      * The eoln_hard method is used to output a hard end-of-line, when
      * a newline character is seen.
      */
    void eoln_hard(void);

    /**
      * The default constructor.  Do not use.
      */
    output_filter_quoted_printable();

    /**
      * The copy constructor.  Do not use.
      */
    output_filter_quoted_printable(const output_filter_quoted_printable &);

    /**
      * The assignment apperator.  Do not use.
      */
    output_filter_quoted_printable &operator=(
        const output_filter_quoted_printable &);
};

#endif // LIBAEGIS_OUTPUT_FILTER_QUOTED_PRINT_H
