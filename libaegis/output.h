//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2006 Peter Miller;
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
// MANIFEST: interface definition for libaegis/output.c
//

#ifndef LIBAEGIS_OUTPUT_H
#define LIBAEGIS_OUTPUT_H

#include <common/ac/stdarg.h>
#include <common/ac/stddef.h>
#include <common/main.h> // for gcc attributes

struct string_ty; // forward
class nstring; // forward


/**
  * The output_ty class is used to describe the interface to an
  * arbitrary output destination.  It could be a file, it could be a
  * string, or many other things, including several filters.
  */
class output_ty
{
public:
    typedef void (*delete_callback_ty)(output_ty *, void *);

    /**
      * The destructor.
      */
    virtual ~output_ty();

    /**
      * The default constructor.
      */
    output_ty();

    /**
      * The filename method is used to obtain the filename of this output.
      */
    virtual struct string_ty *filename() const = 0;

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
      * The fputs method is used to write a string to the output stream,
      * encoding any XML special characters (e.g. "<" becomes "&lt;",
      * etc).
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
      * The fputs method is used to write a string to the output stream,
      * encoding any XML special characters (e.g. "<" becomes "&lt;",
      * etc).
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
      * Set the callback function to be called by the destructor.
      */
    void delete_callback(delete_callback_ty cb, void *arg);

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
      * The del_cb instance variable is used to remember the callback
      * function to be called when this output stream is destroyed.
      */
    delete_callback_ty del_cb;

    /**
      * The del_cb_arg instance variable is used to remember the
      * argument to the callback function to be called when this output
      * stream is destroyed.
      */
    void *del_cb_arg;

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
    output_ty(const output_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    output_ty &operator=(const output_ty &);
};


inline output_ty &
operator<<(output_ty &os, char c)
{
    os.fputc(c);
    return os;
}


inline output_ty &
operator<<(output_ty &os, const char *s)
{
    os.fputs(s);
    return os;
}


inline output_ty &
operator<<(output_ty &os, const nstring &s)
{
    os.fputs(s);
    return os;
}


class input; // forward

output_ty &operator<<(output_ty &os, input &is);


inline DEPRECATED void
output_delete(output_ty *op)
{
    delete op;
}

inline DEPRECATED struct string_ty *
output_filename(const output_ty *op)
{
    return op->filename();
}

inline DEPRECATED long
output_ftell(const output_ty *op)
{
    return op->ftell();
}

inline DEPRECATED void
output_fputc(output_ty *op, int c)
{
    op->fputc(c);
}

inline DEPRECATED void
output_fputs(output_ty *op, const char *s)
{
    op->fputs(s);
}

inline DEPRECATED void
output_put_str(output_ty *op, struct string_ty *s)
{
    op->fputs(s);
}

inline DEPRECATED void
output_write(output_ty *op, const void *data, size_t length)
{
    op->write(data, length);
}

inline DEPRECATED void
output_flush(output_ty *op)
{
    op->flush();
}

inline DEPRECATED int
output_page_width(const output_ty *op)
{
    return op->page_width();
}

inline DEPRECATED int
output_page_length(output_ty *op)
{
    return op->page_length();
}

void output_fprintf(output_ty *, const char *, ...)
    ATTR_PRINTF(2, 3) DEPRECATED;

inline DEPRECATED void
output_vfprintf(output_ty *op, const char *fmt, va_list ap)
{
    op->vfprintf(fmt, ap);
}

inline DEPRECATED void
output_end_of_line(output_ty *op)
{
    op->end_of_line();
}

inline DEPRECATED void
output_delete_callback(output_ty *op, output_ty::delete_callback_ty cb,
    void *arg)
{
    op->delete_callback(cb, arg);
}

#endif // LIBAEGIS_OUTPUT_H
