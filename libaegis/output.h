//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_OUTPUT_H
#define LIBAEGIS_OUTPUT_H

#include <common/ac/stdarg.h>
#include <common/ac/stddef.h>
#include <common/main.h> // for gcc attributes
#include <common/nstring.h>
#include <common/ac/shared_ptr.h>

#include <libaegis/functor/stack.h>

struct string_ty; // forward


/**
  * The output class is used to describe the interface to an arbitrary
  * output destination.  It could be a file, it could be a string, or
  * many other things, including several filters.
  */
class output
{
public:
    /**
      * The pointer type is used to describe a pointer to an output
      * destination.
      */
    typedef aegis_shared_ptr<output> pointer;

    typedef void (*delete_callback_ty)(output *, void *);

    /**
      * The destructor.
      */
    virtual ~output();

protected:
    /**
      * The default constructor.
      * May only be called by derived classes.
      */
    output();

public:
    /**
      * The filename method is used to obtain the filename of this output.
      */
    virtual nstring filename() const = 0;

    /**
      * The ftell method is used to determine the current file position
      * of the output.
      */
    long ftell() const;

    /**
      * The write method is used to write date to the output.
      */
    void write(const void *data, size_t nbytes);

    /**
      * The flush method is used to ensure that any buffered data is
      * written to the output.
      */
    void flush();

    /**
      * The page_width method is used to obtain the width of the page of
      * the output device.
      */
    virtual int page_width() const;

    /**
      * The page_length method is used to obtain the length of the page
      * of the output device.
      */
    virtual int page_length() const;

    /**
      * The end_of_line method is used to ensure that the current output
      * position is at the beginning of a line.
      */
    void end_of_line();

    /**
      * The type_name method is used to determine the name of this
      * output device or file or type.
      */
    virtual const char *type_name() const = 0;

    /**
      * The fputc method is used to write a single character to the
      * output.  The output is buffered.
      */
    void
    fputc(char c)
    {
	if (buffer_position < buffer_end)
	    *buffer_position++ = c;
	else
	    overflow(c);
    }

    /**
      * The fputs method is used to write a NUL terminated string to the
      * output stream.
      *
      * @param str
      *     The string to be written out.
      */
    void fputs(const char *str);

    /**
      * The fputs method is used to write a NUL terminated string to the
      * output stream, encoding any XML special characters (e.g. "<"
      * becomes "&lt;", etc).
      *
      * @param str
      *     The string to be written out.
      * @param paragraphs
      *     If true, insert <P> for two newlines and <BR> for single
      *     newlines; if false, simply pass newlines through.
      */
    void fputs_xml(const char *str, bool paragraphs = false);

    /**
      * The fputs method is used to write a string to the output stream.
      *
      * @param str
      *     The string to be written out.
      */
    void fputs(string_ty *str);

    /**
      * The fputs_xml method is used to write a string to the output
      * stream, encoding any XML special characters (e.g. "<" becomes
      * "&lt;", etc).
      *
      * @param str
      *     The string to be written out.
      * @param paragraphs
      *     If true, insert <P> for two newlines and <BR> for single
      *     newlines; if false, simply pass newlines through.
      */
    void fputs_xml(string_ty *str, bool paragraphs = false);

    /**
      * The fputs method is used to write a string to the output stream.
      *
      * @param str
      *     The string to be written out.
      */
    void fputs(const nstring &str);

    /**
      * The fputs_xml method is used to write a string to the output
      * stream, encoding any XML special characters (e.g. "<" becomes
      * "&lt;", etc).
      *
      * @param str
      *     The string to be written out.
      * @param paragraphs
      *     If true, insert <P> for two newlines and <BR> for single
      *     newlines; if false, simply pass newlines through.
      */
    void fputs_xml(const nstring &str, bool paragraphs = false);

    /**
      * The fprintf method produces output according to
      * a format as described in the printf(3) man page.
      *
      * \param fmt
      *     This method writes the output under the control of a format
      *     string that specifies how subsequent arguments (or arguments
      *     accessed via the variable-length argument facilities of
      *     stdarg(3)) are converted for output.
      */
    void fprintf(const char *fmt, ...)                        ATTR_PRINTF(2, 3);

    /**
      * The vfprintf method is equivalent to the fprintf method, except
      * that it is called with a va_list instead of a variable number
      * of arguments.  This method does not call the va_end macro.
      * Consequently, the value of ap is undefined after the call.  The
      * application should call va_end(ap) itself afterwards.
      */
    void vfprintf(const char *fmt, va_list)                   ATTR_PRINTF(2, 0);

    /**
      * The register_delete_callback method is used to set the callback
      * functor to be called by the destructor.
      */
    void register_delete_callback(functor::pointer fp);

    /**
      * The unregister_delete_callback method is used to
      * forget a callback functor to be called by the destructor.
      */
    void unregister_delete_callback(functor::pointer fp);

private:
    /**
      * The ftell_inner method is used to determine the current file
      * position, without taking the buffering into account.
      */
    virtual long ftell_inner() const = 0;

    /**
      * The write_inner method is used write data to the output, without
      * taking the buffering into account.
      */
    virtual void write_inner(const void *data, size_t length) = 0;

    /**
      * The end_of_line_inner method is used to ensure that the current
      * output position is at the beginning of a line, without taking
      * the buffering into account.
      */
    virtual void end_of_line_inner() = 0;

    /**
      * The flush_inner method is called by the flush method once all
      * the data has been written.  The default implementation does
      * nothing.
      */
    virtual void flush_inner();

    /**
      * The overflow mwthod is used by the fputc method when the buffer
      * is full.  It write the buffer to the output, and then adds the
      * single character to the buffer.
      */
    void overflow(char c);

    /**
      * The callback instance variable is used to remember the
      * functor(s) to be called when this output stream is destroyed.
      */
    functor_stack callback;

    /**
      * The buffer instance variable is used to remember the base of a
      * dynamically allocated array of characters used to buffer the
      * data, to minimize the number of systems calls required.
      */
    unsigned char *buffer;

    /**
      * The buffer_size instance variable is used to remember the number
      * of characters allocated in the buffer array.
      */
    size_t buffer_size;

    /**
      * The buffer_position instance variable is used to remember the
      * current output position withing the output buffer.
      */
    unsigned char *buffer_position;

    /**
      * The buffer_end instance variable is used to remember the end of
      * the dynamically allocated buffer array.
      */
    unsigned char *buffer_end;

    /**
      * The copy constructor.  Do not use.
      */
    output(const output &);

    /**
      * The assignment operator.  Do not use.
      */
    output &operator=(const output &);
};


inline output::pointer &
operator<<(output::pointer &os, char c)
{
    os->fputc(c);
    return os;
}


inline output::pointer &
operator<<(output::pointer &os, const char *s)
{
    os->fputs(s);
    return os;
}


inline output::pointer &
operator<<(output::pointer &os, const nstring &s)
{
    os->fputs(s);
    return os;
}


class input; // forward

output::pointer &operator<<(output::pointer &os, input &is);

#endif // LIBAEGIS_OUTPUT_H
