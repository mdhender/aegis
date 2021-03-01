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
// MANIFEST: interface definition for aetar/output/tar_child.c
//

#ifndef AETAR_OUTPUT_TAR_CHILD_H
#define AETAR_OUTPUT_TAR_CHILD_H

#include <common/nstring.h>
#include <libaegis/output.h>

/**
  * The output_tar_child_ty class is used to represent an outp[ut
  * stream which writes to a member of a tar archive.
  */
class output_tar_child_ty:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_tar_child_ty();

    /**
      * The constructor.
      *
      * \param deeper
      *     The underlying output to which the tar archive is to be
      *     written.
      * \param name
      * \param length
      * \param executable
      */
    output_tar_child_ty(output_ty *deeper, const nstring &name, long length,
	bool executable);

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

private:
    /**
      * The deeper instance variable is used to remember the underlying
      * output to which the CPIO archive is to be written.
      */
    output_ty *deeper;

    /**
      * The name instance variable is used to remember the name of the
      * archive member to be written.
      */
    nstring name;

    /**
      * The length instance variable is used to remember the data length
      * for this archive member.  It is a bug if the wrong amount of data
      * is written.
      */
    long length;

    /**
      * The executable instance variable is used to remember whether or
      * not the archive member is an executable file.
      */
    bool executable;

    /**
      * The pos instance variable is used to remember the current output
      * position.
      */
    long pos;

    /**
      * the bol instance variable is used to remember whether or not we
      * are currently at the beginning of a line.
      */
    bool bol;

    /**
      * The changed_size method is used to emit a fatal error if the
      * amount of data written does not aggree with the length given to
      * the constructor.
      */
    void changed_size();

    /**
      * The header method is used to write a tar file header for this
      * archive menber.
      */
    void header();

    /**
      * The padding method is used to write NULs to the output until the
      * output position is a multiple of 512 bytes.
      */
    void padding();

    /**
      * The default constructor.  Do not use.
      */
    output_tar_child_ty();

    /**
      * The copy constructor.  Do not use.
      */
    output_tar_child_ty(const output_tar_child_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    output_tar_child_ty &operator=(const output_tar_child_ty &);
};

output_ty *output_tar_child_open(output_ty *, struct string_ty *, long, int);

#endif // AETAR_OUTPUT_TAR_CHILD_H
