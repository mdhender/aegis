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

#ifndef LIBAEGIS_WIDE_OUTPUT_EXPAND_H
#define LIBAEGIS_WIDE_OUTPUT_EXPAND_H

#include <libaegis/wide_output.h>

/**
  * The wide_output_expand class is used to replace tabs with spaces.
  * This simplifies a lot of the internal width calculations for things
  * like truncating, wrapping and reassembling.  This internal filtering
  * uses 8-character tabs; the -tw option is exclusively for output tab
  * widths.
  */
class wide_output_expand:
    public wide_output
{
public:
    /**
      * The destructor.
      */
    virtual ~wide_output_expand();

private:
    /**
      * The constructor.  It is private on purpose, use the #open class
      * method instead.
      *
      * @param deeper
      *     when to send the expanded output
      */
    wide_output_expand(const wide_output::pointer &deeper);

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

public:
    /**
      * The open class method is sued to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     when to send the expanded output
      */
    static pointer open(const wide_output::pointer &deeper);

private:
    pointer deeper;
    int column;

    void put_hex(int n);

    /**
      * The default constructor.  Do not use.
      */
    wide_output_expand();

    /**
      * The copy constructor.  Do not use.
      */
    wide_output_expand(const wide_output_expand &);

    /**
      * The assignment operator.  Do not use.
      */
    wide_output_expand &operator=(const wide_output_expand &);
};

#endif // LIBAEGIS_WIDE_OUTPUT_EXPAND_H
