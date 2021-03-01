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
// MANIFEST: interface definition for aedist/output/cpio.c
//

#ifndef AEDIST_OUTPUT_CPIO_H
#define AEDIST_OUTPUT_CPIO_H

#include <common/ac/time.h>

#include <libaegis/output.h>

struct string_ty; // forward

class output_cpio_ty:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_cpio_ty();

    /**
      * The constructor.
      *
      * \param deeper
      *     The underlying output to which the CPIO archive is to be
      *     written.
      * \param mtime
      *     The time stamp to attach to all archive members
      */
    output_cpio_ty(output_ty *deeper, time_t mtime);

    /**
      * The child method is used to create a child archive.
      *
      * \param name
      *     The name of the file in the archive.
      * \param len
      *     The length of the archive.  Use -1 if you don't know and it
      *     will be stached into memory until the length is known.
      */
    output_ty *child(const nstring &name, long len);

    // See base class for documentation.
    string_ty *filename() const ;

    // See base class for documentation.
    const char *type_name() const;

    // See base class for documentation.
    long ftell_inner() const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void end_of_line_inner();

private:
    /**
      * The deeper instance variable is used to remember the underlying
      * output to which the CPIO archive is to be written.
      */
    output_ty *deeper;

    /**
      * The mtime instance variable is used to remember the time to
      * attach to all archive members.
      */
    time_t mtime;

    /**
      * The default constructor.
      */
    output_cpio_ty();

    /**
      * The copy constructor.  Do not use.
      */
    output_cpio_ty(const output_cpio_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    output_cpio_ty &operator=(const output_cpio_ty &);
};

#endif // AEDIST_OUTPUT_CPIO_H
