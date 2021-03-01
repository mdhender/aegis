//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2005 Peter Miller;
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
// MANIFEST: interface definition for libaegis/output/prefix.c
//

#ifndef LIBAEGIS_OUTPUT_PREFIX_H
#define LIBAEGIS_OUTPUT_PREFIX_H

#include <nstring.h>
#include <output.h>


/**
  * The output_prefix_ty class is used to represent an output stream
  * filter which adds a prefix to every line of output.
  */
class output_prefix_ty:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_prefix_ty();

    /**
      * The constructor.
      *
      * \param deeper
      *     the deeper output stream on which this filter writes to.
      * \param close_on_close
      *     whether or not the deeper output stream is to be deleted in
      *     our destructor.
      * \param prefix
      *     the prefix to be added to each line.
      */
    output_prefix_ty(output_ty *deeper, bool close_on_close,
	const char *prefix);

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

    // See base class for documentation.
    void flush_inner();

private:
    /**
      * The deeper instance variable is used to remember the deeper
      * output stream on which this filter writes to.
      */
    output_ty *deeper;

    /**
      * The close_on_close instance variable is used to remember
      * whether or not the deeper output stream is to be deleted in our
      * destructor.
      */
    bool close_on_close;

    /**
      * The prefix instance variable is used to remember the prefix to
      * be added to each line.
      */
    nstring prefix;

    /**
      * The prefix instance variable is used to remember the current
      * output column.
      */
    int column;

    /**
      * The pos instance variable is used to remember the current output
      * position.
      */
    long pos;

    output_prefix_ty();
    output_prefix_ty(const output_prefix_ty &);
    output_prefix_ty &operator=(const output_prefix_ty &);
};

inline DEPRECATED output_ty *
output_prefix(output_ty *deeper, bool coc, const char *prefix)
{
    return new output_prefix_ty(deeper, coc, prefix);
}

#endif // LIBAEGIS_OUTPUT_PREFIX_H
