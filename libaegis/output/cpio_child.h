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
// MANIFEST: interface definition for aedist/output/cpio_child.c
//

#ifndef AEDIST_OUTPUT_CPIO_CHILD_H
#define AEDIST_OUTPUT_CPIO_CHILD_H

#include <common/ac/time.h>

#include <common/nstring.h>
#include <libaegis/output.h>

/**
  * The output_cpio_child_ty class is used to represent a cpio archive
  * member with a known length.
  */
class output_cpio_child_ty:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_cpio_child_ty();

    /**
      * The constructor.
      *
      * \param deeper
      *     The underlying output to which the CPIO archive is to be
      *     written.
      * \param name
      *     The the archive member name for this file.
      * \param len
      *     The length instance variable is used to remember the lenbgth
      *     of this archive member.  It is an error if you write a
      *     different number of bytes.
      * \param mtime
      *     The modify time to insert into the archive.
      */
    output_cpio_child_ty(output_ty *deeper, const nstring &name, long len,
	    time_t mtime);

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
      * The name instance variable is used to remember the archive
      * member name for this file.
      */
    nstring name;

    /**
      * The length instance variable is used to remember the lenbgth of
      * this archiove member.
      */
    long length;

    /**
      * The pos instance variable is used to remember the current output
      * position.
      */
    long pos;

    /**
      * The bol instance variable is used to remember whether or not we
      * are at the beginning of a line.
      */
    bool bol;

    /**
      * the change_size method is used to print a fatal error when the
      * data written to the output is different than the length we were
      * given in the constructor.
      */
    void changed_size();

    /**
      * The padding method is used to insert enough padding to bring the
      * archive member to a multiple of four bytes.
      */
    void padding();

    /**
      * The hex8 method is used to write a long value as eight
      * hexadecimal characters (big endian)i to the deeper output.
      */
    void hex8(long);

    /**
      * The header method is used to write the archive header to the
      * deeper output.
      */
    void header();

    /**
      * The mtime instance variable is used to remember the time to
      * insert into the file header.
      */
    time_t mtime;

    /**
      * The default constructor.  Do not use.
      */
    output_cpio_child_ty();

    /**
      * The copy constructor.  Do not use.
      */
    output_cpio_child_ty(const output_cpio_child_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    output_cpio_child_ty &operator=(const output_cpio_child_ty &);
};

output_ty *output_cpio_child_open(output_ty *, struct string_ty *, long);

#endif // AEDIST_OUTPUT_CPIO_CHILD_H
