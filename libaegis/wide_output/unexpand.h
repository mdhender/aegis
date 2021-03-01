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

#ifndef LIBAEGIS_WIDE_OUTPUT_UNEXPAND_H
#define LIBAEGIS_WIDE_OUTPUT_UNEXPAND_H

#include <libaegis/wide_output.h>

/**
  * This wide output class is used to convert sequences of spaces
  * into tabs, where appropriate (and smaller than the original).
  * The -Tab_Width option is used to determine the output tab width.
  */
class wide_output_unexpand:
    public wide_output
{
public:
    /**
      * The destructor.
      */
    virtual ~wide_output_unexpand();

private:
    /**
      * The constructor.  It is private on purpose, use the #open class
      * method instead.
      *
      * @param deeper
      *     where to write the filtered output
      * @param tabwidth
      *     the column width of tab characters
      */
    wide_output_unexpand(const wide_output::pointer &deeper, int tabwidth);

public:
    /**
      * The open class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     where to write the filtered output
      * @param tabwidth
      *     the column width of tab characters
      */
    static pointer open(const wide_output::pointer &deeper, int tabwidth = 8);

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
    int	icol;
    int ocol;
    int	tab_width;

    /**
      * The default constructor.  Do not use.
      */
    wide_output_unexpand();

    /**
      * The copy constructor.  Do not use.
      */
    wide_output_unexpand(const wide_output_unexpand &);

    /**
      * The assignment operator.  Do not use.
      */
    wide_output_unexpand &operator=(const wide_output_unexpand &);
};

#endif // LIBAEGIS_WIDE_OUTPUT_UNEXPAND_H
