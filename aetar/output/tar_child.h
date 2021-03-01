//
// aegis - project change supervisor
// Copyright (C) 2002, 2005, 2006, 2008, 2011, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AETAR_OUTPUT_TAR_CHILD_H
#define AETAR_OUTPUT_TAR_CHILD_H

#include <common/nstring.h>
#include <libaegis/output.h>

/**
  * The output_tar_child class is used to represent an outp[ut
  * stream which writes to a member of a tar archive.
  */
class output_tar_child:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_tar_child();

private:
    /**
      * The constructor.  It is private on purpose, use the #create
      * class method instead.
      *
      * @param deeper
      *     The underlying output to which the tar archive is to be
      *     written.
      * @param name
      *     the name of the tar archive entry.
      * @param length
      *     the length of the content
      * @param executable
      *     whether or not the file should be marked as executable
      */
    output_tar_child(const output::pointer &deeper, const nstring &name,
        long length, bool executable);

public:
    /**
      * The open class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     The underlying output to which the tar archive is to be
      *     written.
      * @param name
      *     the name of the tar archive entry.
      * @param length
      *     the length of the content
      * @param executable
      *     whether or not the file should be marked as executable
      */
    static pointer create(const output::pointer &deeper, const nstring &name,
        long length, bool executable);

protected:
    // See base class for documentation.
    nstring filename(void) const;

    // See base class for documentation.
    nstring type_name(void) const;

    // See base class for documentation.
    long ftell_inner(void) const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void end_of_line_inner(void);

private:
    /**
      * The deeper instance variable is used to remember the underlying
      * output to which the TAR archive is to be written.
      */
    output::pointer deeper;

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
    void changed_size(void);

    /**
      * The header method is used to write a tar file header for this
      * archive menber.
      */
    void header(void);

    /**
      * The padding method is used to write NULs to the output until the
      * output position is a multiple of 512 bytes.
      */
    void padding(void);

    /**
      * The default constructor.  Do not use.
      */
    output_tar_child();

    /**
      * The copy constructor.  Do not use.
      */
    output_tar_child(const output_tar_child &);

    /**
      * The assignment operator.  Do not use.
      */
    output_tar_child &operator=(const output_tar_child &);
};

#endif // AETAR_OUTPUT_TAR_CHILD_H
// vim: set ts=8 sw=4 et :
