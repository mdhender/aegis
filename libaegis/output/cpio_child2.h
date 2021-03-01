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
// MANIFEST: interface definition for aedist/output/cpio_child2.c
//

#ifndef AEDIST_OUTPUT_CPIO_CHILD2_H
#define AEDIST_OUTPUT_CPIO_CHILD2_H

#include <libaegis/output.h>

class output_memory_ty; // forward

/**
  * The output_cpio_child2_ty class is used to represent a CPIO archive
  * member of unknown length.  It will be cached in memory until the
  * length is known.
  */
class output_cpio_child2_ty:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_cpio_child2_ty();

    /**
      * The constructor.
      *
      * \param deeper
      *     The the underlying output to which the CPIO archive is to be
      *     written.
      * \param name
      *     The name of the archive member.
      */
    output_cpio_child2_ty(output_ty *deeper, const nstring &name);

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

    // See base class for documentation.
    void flush_inner();

private:
    /**
      * The deeper instance variable is used to remember the underlying
      * output to which the CPIO archive is to be written.
      */
    output_ty *deeper;

    /**
      * The name instance variable is used to remember the name of the
      * archive member.
      */
    nstring name;

    /**
      * The buffer instance variable is used to remember the data
      * written to the archive member, so that we can obtain its length.
      */
    output_memory_ty *buffer;

    /**
      * The default constructor.  Do not use.
      */
    output_cpio_child2_ty();

    /**
      * The copy constructor.  Do not use.
      */
    output_cpio_child2_ty(const output_cpio_child2_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    output_cpio_child2_ty &operator=(const output_cpio_child2_ty &);
};

inline DEPRECATED output_ty *
output_cpio_child2_open(output_ty *deeper, const nstring &name)
{
    return new output_cpio_child2_ty(deeper, name);
}

#endif // AEDIST_OUTPUT_CPIO_CHILD2_H
