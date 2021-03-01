//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2005, 2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_WIDE_OUTPUT_HEAD_H
#define LIBAEGIS_WIDE_OUTPUT_HEAD_H

#include <libaegis/wide_output.h>

/**
  * The wide_output_head class is used to preserve the first few lines
  * of an output stream, and discard the rest.  It is similar to the
  * head(1) command.
  */
class wide_output_head:
    public wide_output
{
public:
    /**
      * The destructor.
      */
    virtual ~wide_output_head();

private:
    /**
      * The constructor.  It is private on purpose, use the #open class
      * method instead.
      *
      * @param deeper
      *     where to send the filtered output
      * @param nlines
      *     the number of lines to keep (at the front)
      */
    wide_output_head(const wide_output::pointer &deeper, int nlines);

public:
    /**
      * The open class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     where to send the filtered output
      * @param nlines
      *     the number of lines to keep (at the front)
      */
    static pointer open(const wide_output::pointer &deeper, int nlines);

protected:
    // See base class for documentation.
    nstring filename();

    // See base class for documentation.
    int page_width();

    // See base class for documentation.
    int page_length();

    // See base class for documentation.
    const char *type_name() const;

    // See base class for documentation.
    void write_inner(const wchar_t *data, size_t len);

    // See base class for documentation.
    void flush_inner();

    // See base class for documentation.
    void end_of_line_inner();

private:
    pointer deeper;
    int how_many_lines;
    bool prev_was_newline;

    wide_output_head();
    wide_output_head(const wide_output_head &);
    wide_output_head &operator=(const wide_output_head &);
};

#endif // LIBAEGIS_WIDE_OUTPUT_HEAD_H
