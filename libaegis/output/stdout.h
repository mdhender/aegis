//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2004, 2005 Peter Miller;
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
// MANIFEST: interface definition for libaegis/output/stdout.c
//

#ifndef LIBAEGIS_OUTPUT_STDOUT_H
#define LIBAEGIS_OUTPUT_STDOUT_H

#include <output.h>

/**
  * The output_stdout_ty class is used to repersent the state of an
  * output stream being written to the standard output.
  */
class output_stdout_ty:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_stdout_ty();

    /**
      * The default constructor.
      */
    output_stdout_ty();

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
      * The bol instance variable is used toi remember whether or not we
      * are at the beginning of a line.
      */
    bool bol;

    /**
      * The copy constructor.  Do not use.
      */
    output_stdout_ty(const output_stdout_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    output_stdout_ty &operator=(const output_stdout_ty &);
};


inline DEPRECATED output_ty *
output_stdout(void)
{
    return new output_stdout_ty();
}

#endif // LIBAEGIS_OUTPUT_STDOUT_H
