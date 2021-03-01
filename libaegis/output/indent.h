//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2005, 2006 Peter Miller
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
// MANIFEST: interface definition for libaegis/output/indent.c
//

#ifndef LIBAEGIS_OUTPUT_INDENT_H
#define LIBAEGIS_OUTPUT_INDENT_H

#include <libaegis/output.h>

/**
  * The output_indent_ty class is used to represent an output stream
  * which automatically indents its C-like input.
  */
class output_indent_ty:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_indent_ty();

    /**
      * The constructor.
      *
      * \param deeper
      *     the deeper output stream on which this filter writes to.
      * \param close_on_close
      *     whether or not the deeper output stream is to be deleted in our
      *     destructor.
      */
    output_indent_ty(output_ty *deeper, bool close_on_close = true);

    /**
      * The indent_more method is used to increase the indenting
      * beyond the automatically calculated indent.
      *
      * \note
      *     There must be a matching indent_less call.
      */
    void indent_more();

    /**
      * The indent_less function is used to decrease the indenting
      * to less than the automatically calculated indent.
      *
      * \note
      *     There must be a matching indent_more call.
      */
    void indent_less();

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
    enum {
	/**
	  * This defines the width of a tab on output.
	  */
	INDENT = 8
    };

    /**
      * The deeper instance variable is used to remember the deeper
      * output stream on which this filter writes to.
      */
    output_ty *deeper;

    /**
      * The close_on_close instance variable is used to remember
      * whether or not the deeper output stream is to be deleted in out
      * destructor.
      */
    bool close_on_close;

    /**
      * The depth instance variable is used to remember the current
      * level of indenting.  Will never be negative.
      */
    int depth;

    /**
      * The in_col instance variable is used to remember the current
      * input column.  This can differe from the output column when
      * white space is being optimized.
      */
    int in_col;

    /**
      * The out_col instance variable is used to remember the current
      * output column.
      */
    int out_col;

    /**
      * The continuation_line instance variable is used to remember
      * whether or not the current line is a continuation line.
      *
      * State 0 means that it is not, state 1 means that a backslash (\)
      * has been seen, and state 2 means that "\\\n" has been seen.
      */
    int continuation_line;

    /**
      * The pos instance variable is used to remember the current output
      * position.
      */
    long pos;

    output_indent_ty();
    output_indent_ty(const output_indent_ty &);
    output_indent_ty &operator=(const output_indent_ty &);
};

inline DEPRECATED output_ty *
output_indent(output_ty *deeper)
{
    return new output_indent_ty(deeper);
}

inline DEPRECATED void
output_indent_more(output_indent_ty *op)
{
    op->indent_more();
}

inline DEPRECATED void
output_indent_less(output_indent_ty *op)
{
    op->indent_less();
}

#endif // LIBAEGIS_OUTPUT_INDENT_H
