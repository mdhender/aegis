//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller
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
// MANIFEST: interface of the output_bzip2 class
//

#ifndef LIBAEGIS_OUTPUT_BZIP2_H
#define LIBAEGIS_OUTPUT_BZIP2_H

#include <common/ac/bzlib.h>

#include <libaegis/output.h>

/**
  * The output_bzip2 class is used to represent an output filter which
  * compresses the data before writing it to the deeper output stream.
  */
class output_bzip2:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_bzip2();

    /**
      * The constructor.
      *
      * \param deeper
      *     the output stream this filter will write its output to.
      * \param close_on_close
      *     whether or not the deeper output stream should be deleted on
      *     the destructor.
      */
    output_bzip2(output_ty *deeper, bool close_on_close);

    // See base class for documentation.
    string_ty *filename() const;

    // See base class for documentation.
    long ftell_inner() const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    int page_width() const;

    // See base class for documentation.
    int page_length() const;

    // See base class for documentation.
    const char *type_name() const;

    // See base class for documentation.
    void end_of_line_inner();

private:
    /**
      * The deeper instance variable is used to remember the output
      * stream this filter will write its output to.
      */
    output_ty *deeper;

    /**
      * The close_on_close instance variable is used to remember whether
      * or not the deeper output stream should be deleted on the
      * destructor.
      */
    bool close_on_close;

    /**
      * The stream instance variable is used to remember somthign that
      * the bzip2 code wants to remember.  It is opaque to us.
      */
    bz_stream stream;

    enum { BUFFER_SIZE = 1 << 14 };

    /**
      * The buf instance variable is used to remember the buffered
      * output data.
      */
    char *buf;

    /**
      * The pos instance variable is used to remember the current output
      * position.
      */
    long pos;

    /**
      * The bol instance variable is used to remember whether or not the
      * output is positioned at the beginning of a lone.
      */
    bool bol;

    /**
      * The bzlib_fatal_error method is used to report a fatal error
      * from the bzip2 engine.
      */
    void bzlib_fatal_error(int err);

    /**
      * The default constructor.  Do not use.
      */
    output_bzip2();

    /**
      * The copy constructor.  Do not use.
      */
    output_bzip2(const output_bzip2 &);

    /**
      * The assignment operator.  Do not use.
      */
    output_bzip2 &operator=(const output_bzip2 &);
};

#endif // LIBAEGIS_OUTPUT_BZIP2_H
