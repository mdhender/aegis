//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2005, 2006 Peter Miller
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
// MANIFEST: interface definition for aetar/output/tar.c
//

#ifndef AETAR_OUTPUT_TAR_H
#define AETAR_OUTPUT_TAR_H

#include <libaegis/output.h>

/**
  * The output_tar_ty class is used to represent a tar archive.
  */
class output_tar_ty:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_tar_ty();

    /**
      * The constructor.
      *
      * \param deeper
      *     The underlying output to which the tar archive is to be
      *     written.
      */
    output_tar_ty(output_ty *deeper);

    /**
      * The child method is used to create a new output object into
      * which the archive member contents are written.
      *
      * \param name
      *     The name of the archive member.
      * \param len
      *     The exact length of the archive member.
      *     It is a bug if the wrong amount of data is written.
      * \param executable
      *     Wherther ot not the archive member is executable.
      */
    output_ty *child(const nstring &name, long len, bool executable);

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
      * output to which the tar archive is to be written.
      */
    output_ty *deeper;

    /**
      * The default constructor.  Do not use.
      */
    output_tar_ty();

    /**
      * The copy constructor.  Do not use.
      */
    output_tar_ty(const output_tar_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    output_tar_ty &operator=(const output_tar_ty &);
};

inline DEPRECATED output_ty *
output_tar(output_ty *deeper)
{
    return new output_tar_ty(deeper);
}

inline DEPRECATED output_ty *
output_tar_child(output_tar_ty *op, const nstring &name, long length,
    bool executable)
{
    return op->child(name, length, executable);
}

#endif // AETAR_OUTPUT_TAR_H
