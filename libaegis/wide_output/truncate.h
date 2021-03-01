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

#ifndef LIBAEGIS_WIDE_OUTPUT_TRUNCATE_H
#define LIBAEGIS_WIDE_OUTPUT_TRUNCATE_H

#include <common/wstring/accumulator.h>
#include <libaegis/wide_output.h>

/**
  * The wide_output_truncate class is used to truncate lines to a
  * specified printing width.  Characters beyond on each line this limit
  * are discarded.
  */
class wide_output_truncate:
    public wide_output
{
public:
    /**
      * The destructor.
      */
    virtual ~wide_output_truncate();

private:
    /**
      * The constructor.  It is private on purpose, use the #open class
      * method instead.
      *
      * @param deeper
      *     where to write the filtered output
      * @param width
      *     the width to crop to, or -1 if should use deeper->page_width
      */
    wide_output_truncate(const wide_output::pointer &deeper, int width);

public:
    /**
      * The open class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     where to write the filtered output
      * @param width
      *     the width to crop to, or -1 if should use deeper->page_width
      */
    static pointer open(const wide_output::pointer &deeper, int width);

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
    wide_output::pointer deeper;

    int width;

    wstring_accumulator buf;

    int column;

    wide_output_truncate();
    wide_output_truncate(const wide_output_truncate &);
    wide_output_truncate &operator=(const wide_output_truncate &);
};

#endif // LIBAEGIS_WIDE_OUTPUT_TRUNCATE_H
