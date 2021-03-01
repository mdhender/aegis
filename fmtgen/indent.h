//
// aegis - project change supervisor
// Copyright (C) 1991-1993, 2002, 2003, 2005, 2006, 2008, 2012 Peter Miller.
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

#ifndef INDENT_H
#define INDENT_H

#include <common/ac/stdio.h>
#include <common/ac/shared_ptr.h>

#include <common/nstring.h>
#include <common/nstring/list.h>

/**
  * The indent class is used to represent an anto-indenting output stream.
  */
class indent
{
public:
    typedef aegis_shared_ptr<indent> pointer;

    /**
      * The destructor.
      */
    virtual ~indent();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create method.
      *
      * @param filename
      *     The path of the file to be written.
      */
    indent(const nstring &filename);

public:
    /**
      * The create class method is used to creare new dynamically
      * allocated instances of this class.
      *
      * @param filename
      *     The path of the file to be written.
      */
    static pointer create(const nstring &filename);

    /**
      * The less method is used to decrease the indenting
      * to less than the automatically calculated indent.
      *
      * @note
      *     There must be a matching #more call.
      */
    void less(void);

    /**
      * The more method is used to increase the indenting
      * beyond the automatically calculated indent.
      *
      * @note
      *     There must be a matching #less call.
      */
    void more(void);

    /**
      * The printf method is used to emit a formatted string.
      *
      * @param fmt
      *     The string to emit, and control for types and count of rest of
      *     parameters.
      */
    void printf(const char *fmt, ...)                         ATTR_PRINTF(2, 3);

    /**
      * The vprintf method is used to emit a formatted string.
      *
      * @param fmt
      *     The string to emit, and control for types and count of rest of
      *     parameters.
      * @param ap
      *     the location of the remaining arguments
      */
    void vprintf(const char *fmt, va_list ap)                   ATTR_VPRINTF(2);

    /**
      * The putch method is used to emit characters.  It keeps track of
      * (){}[] pairs and indents between them.  Leading whitespace is
      * suppressed and replaced with its own idea of indenting.
      *
      * @param c
      *    the character to emit.
      */
    void putch(char c);

    /**
      * The puts method is used to emit a string.
      * The characters are emitted via the #putch method.
      *
      * @param text
      *     The string to be emitted.
      */
    void puts(const char *text);

    /**
      * The puts method is used to emit a string.
      * The characters are emitted via the #putch method.
      *
      * @param text
      *     The string to be emitted.
      */
    void puts(const nstring &text);

    /**
      * The wrap_and_print method is used to wrap the given text, and
      * emit the wrapped lines, adding the given prefix.
      *
      * @param prefix
      *     The constant string to add to the start of each wrapped line.
      * @param text
      *     The text to pe wrapped and emitted.
      */
    void wrap_and_print(const nstring &prefix, const nstring &text);

    /**
      * The get_file_name method may be used to obtain the name of the
      * file being written.
      */
    nstring get_file_name() const { return fn; }

    /**
      * The include_once method is used to add a hash-include line to
      * the output, provided it hasn't already been issued.
      *
      * @param filename
      *     The name of the file to be included.
      */
    void include_once(const nstring &filename);

    /**
      * The set_indent class method is used to specify the number of
      * space charcaters to insert for each nesting increment.
      *
      * @param n
      *     The number of spaces per nesting.
      */
    static void set_indent(int n);

private:
    /**
      * The fp instance variable is used to remember the open FILE for
      * writing to.  Can be NULL because we defer opening the file in
      * case the parser finds errors.
      */
    FILE *fp;

    /**
      * The fn instance variable is used to remember the path of the
      * file to be written.
      */
    nstring fn;

    /**
      * The depth instance variable is used to remember the current
      * indent depth.
      */
    int depth;

    /**
      * The in_col instance variable is used to remember the column
      * position of the input.
      */
    int in_col;

    /**
      * The out_col instance variable is used to remember the column
      * position of the output.  Differens from in_col when processing
      * white space, otherwise they are the same.
      */
    int out_col;

    /**
      * The continuation_line instance variable is used to remember the
      * state of detecting whether or not a line is a continuation line
      * or not.  Value are
      * 0 = normal line body
      * 1 = backslash has been seen
      * 2 = backslash newline has been seen (i.e. do not auto indent at
      *     this point).
      */
    int continuation_line;

    /**
      * The within_string instance variable is used to remember whether
      * or not we are inside a string.  Values are:
      * 0    = normal, outside string,
      * '\'' = inside a single quote string (character constant)
      * '"'  = inside a double quote string.
      */
    int within_string;

    /**
      * The in_tab_width class variable is used to remember the width of
      * tabs on input.  This is also the indent::more and indent::less
      * step size.
      */
    static int in_tab_width;

    /**
      * The out_tab_width class variable is used to remember the widths
      * of tabs on output.  Set to zero for not tabs to be used on
      * output.
      */
    static int out_tab_width;

    /**
      * The included_to_date instance variable is used to remember which
      * hash-include lines have been issued so far.  This allows the
      * #include_once function to do its magic.
      */
    nstring_list included_to_date;

    /**
      * The default constructor.  Do not use.
      */
    indent();

    /**
      * The copy constructor.  Do not use.
      */
    indent(const indent &);

    /**
      * The assignment operator.  Do not use.
      */
    indent &operator=(const indent &);
};

#endif // INDENT_H
// vim: set ts=8 sw=4 et :
