//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001, 2002, 2005, 2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_OUTPUT_FILE_H
#define LIBAEGIS_OUTPUT_FILE_H

#include <common/nstring.h>
#include <libaegis/output.h>

struct nstring; // forward

/**
  * The output_file class is used to represent the state of an output
  * stream to a regular file.
  */
class output_file:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_file();

private:
    /**
      * The constructor.  It is private on purpose, use one of the open
      * class methods instead.
      *
      * \param file_name
      *     The name of the file to be opened for output.
      *     (Make absolutely sure it is not NULL or empty.)
      * \param binary
      *     Whether the file is binary (true) or text (false).
      */
    output_file(const nstring &file_name, bool binary = false);

public:
    /**
      * The open class method is used to open an output file.
      *
      * \param fn
      *     The path name of the file to be opened.  If the poijnter is NULL
      *     or the string is empty, the standard output is used.
      * \param binary
      *     Wherther or not the file is binary.  Defaults to false (i.e. text).
      */
    static output::pointer open(const nstring &fn, bool binary = false);

    /**
      * The text_open class method is used to open a text output file.
      *
      * \param fn
      *     The path name of the file to be opened.  If the poijnter is NULL
      *     or the string is empty, the standard output is used.
      */
    static inline output::pointer
    text_open(string_ty *fn)
    {
        return open(nstring(fn), false);
    }

    /**
      * The text_open class method is used to open a text output file.
      *
      * \param fn
      *     The path name of the file to be opened.  If the poijnter is NULL
      *     or the string is empty, the standard output is used.
      */
    static inline output::pointer
    text_open(const nstring &fn)
    {
        return open(fn, false);
    }

    /**
      * The binary_open class method is used to open a binary output file.
      *
      * \param fn
      *     The path name of the file to be opened.  If the poijnter is NULL
      *     or the string is empty, the standard output is used.
      */
    inline static output::pointer
    binary_open(string_ty *fn)
    {
        return open(nstring(fn), true);
    }

    /**
      * The binary_open class method is used to open a binary output file.
      *
      * \param fn
      *     The path name of the file to be opened.  If the poijnter is NULL
      *     or the string is empty, the standard output is used.
      */
    inline static output::pointer
    binary_open(const nstring &fn)
    {
        return open(fn, true);
    }

protected:
    // See base class for documentation.
    nstring filename() const;

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
      * The file_name instance variable is used to remember the name of
      * the file opened for writing.
      */
    nstring file_name;

    /**
      * The fd instance variable is used to remember the number of the
      * file descriptor opened for writing.
      */
    int fd;

    /**
      * The bol instance variable is used to remember whether or not we
      * are at the start of a line.
      */
    bool bol;

    /**
      * The pos instance variable is used to remember the current
      * position in the output file.
      */
    size_t pos;

    /**
      * The default constructor.  Do not use.
      */
    output_file();

    /**
      * The copy constructor.  Do not use.
      */
    output_file(const output_file &);

    /**
      * The assignment operator.  Do not use.
      */
    output_file &operator=(const output_file &);
};

#endif // LIBAEGIS_OUTPUT_FILE_H
