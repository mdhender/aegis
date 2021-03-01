//
// aegis - project change supervisor
// Copyright (C) 2011 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_OUTPUT_FILTER_H
#define LIBAEGIS_OUTPUT_FILTER_H

#include <libaegis/output.h>

/**
  * The output_filter class is used to represent the processing common
  * to all output filters, so that filters need only implement the
  * necessary methods.
  */
class output_filter:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_filter();

protected:
    /**
      * The constructor.
      * For use by derived classes only.
      *
      * @param deeper
      *     Where to write the filtered output.
      */
    output_filter(const output::pointer &deeper);

    // See base class for documentation.
    nstring filename(void) const;

    // See base class for documentation.
    int page_width(void) const;

    // See base class for documentation.
    int page_length(void) const;

    // See base class for documentation.
    long ftell_inner(void) const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void end_of_line_inner(void);

    // See base class for documentation.
    void flush_inner(void);

    // See base class for documentation.
    nstring type_name(void) const;

    void deeper_fputc(char c) { deeper->fputc(c); }
    void deeper_fputs(const char *s) { deeper->fputs(s); }
    void deeper_fputs(const nstring &s) { deeper->fputs(s); }
    void deeper_write(const void *d, size_t ds) { deeper->write(d, ds); }

private:
    /**
      * The deeper instance variable is used to remember where to write
      * the filtered output.
      */
    output::pointer deeper;

    /**
      * The default constructor.  Do not use.
      */
    output_filter();

    /**
      * The copy constructor.  Do not use.
      */
    output_filter(const output_filter &);

    /**
      * The assignment operator.  Do not use.
      */
    output_filter &operator=(const output_filter &);
};

// vim: set ts=8 sw=4 et :
#endif // LIBAEGIS_OUTPUT_FILTER_H
