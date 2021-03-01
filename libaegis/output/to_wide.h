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
// MANIFEST: interface definition for libaegis/output/to_wide.c
//

#ifndef LIBAEGIS_OUTPUT_TO_WIDE_H
#define LIBAEGIS_OUTPUT_TO_WIDE_H

#include <output.h>
#include <wide_output.h>

/**
  * The output_to_wide_ty class is used to represent a narrow character
  * to wide character filter.
  */
class output_to_wide_ty:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_to_wide_ty();

    /**
      * The constructor.
      *
      * \param deeper
      *     the deeper wide output stream on which this filter writes to.
      * \param close_on_close
      *     whether or not the deeper wide output stream is to be
      *     deleted in our destructor.
      */
    output_to_wide_ty(wide_output_ty *deeper, bool close_on_close);

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
      * The deeper instance variable is used to remember the deeper
      * output stream on which this filter writes to.
      */
    wide_output_ty *deeper;

    /**
      * The close_on_close instance variable is used to remember
      * whether or not the deeper output stream is to be deleted in our
      * destructor.
      */
    bool close_on_close;

    char *input_buf;
    size_t input_len;
    size_t input_max;
    mbstate_t input_state;
    bool input_bol;
    wchar_t *output_buf;
    size_t output_len;
    size_t output_max;

    /**
      * The default constructor.  Do not use.
      */
    output_to_wide_ty();

    /**
      * The copy constructor.  Do not use.
      */
    output_to_wide_ty(const output_to_wide_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    output_to_wide_ty &operator=(const output_to_wide_ty &);
};

inline DEPRECATED output_ty *
output_to_wide_open(wide_output_ty *deeper, bool close_on_close)
{
    return new output_to_wide_ty(deeper, close_on_close);
}

#endif // LIBAEGIS_OUTPUT_TO_WIDE_H
