//
//	aegis - project change supervisor
//	Copyright (C) 1999-2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_WIDE_OUTPUT_H
#define LIBAEGIS_WIDE_OUTPUT_H

#include <common/ac/shared_ptr.h>
#include <common/ac/stddef.h>
#include <common/ac/wchar.h>

#include <common/nstring.h>
#include <libaegis/functor/stack.h>

struct string_ty; // existence
struct wstring_ty; // existence
class wstring; // forward

/**
  * The wide_output class is used to present a generic wchar_t output
  * stream.  It could go to a file or to a chained sequence of filters,
  * it isn't known, and doesn't ned to be known.
  */
class wide_output
{
public:
    typedef aegis_shared_ptr<wide_output> pointer;

    /**
      * The destructor.
      */
    virtual ~wide_output();

protected:
    /**
      * The constructor.
      */
    wide_output();

public:
    /**
      * The put_cstr method is used to write a C string onto the output,
      * after it has been converted to wide characters.
      *
      * @param text
      *     the string to convert and write
      */
    void put_cstr(const char *text);

    /**
      * The put_ws method is used to write a C wide string (for
      * example, L"example") onto the output.
      *
      * @param wsp
      *     the string to write
      */
    void put_ws(const wchar_t *wsp);

    /**
      * The put_wc method is used to write a single side charcter (for
      * example, L'x') to the output stream.
      *
      * @param wc
      *     the wide character to be written
      */
    inline void
    put_wc(wchar_t wc)
    {
	if (buffer_position < buffer_end)
            *buffer_position++ = wc;
        else
            overflow(wc);
    }

    /**
      * The put_wstr method is used to write a wide string onto the
      * output stream.
      *
      * @param wsp
      *     the wide character string to be written
      */
    void put_wstr(wstring_ty *wsp);

    /**
      * The write method is used to write a wide string onto the
      * output stream.
      *
      * @param s
      *     the wide character string to be written
      */
    void write(const wstring &s);

    /**
      * The fputs method is used to write a string onto the output
      * stream.
      *
      * @param s
      *     the string to be converted and written
      */
    void fputs(string_ty *s);

    /**
      * The filename method is used to obtain the name of the file being
      * written by this output stream.
      *
      * @returns
      *     the name of the destination
      */
    virtual nstring filename() = 0;

    /**
      * The write method is used to write a set of wide charactres,
      * e.g. a substring.
      *
      * @param data
      *     The data to be written
      * @param len
      *     The number of wide characters to be written
      */
    void write(const wchar_t *data, size_t len);

    /**
      * The flush method is used to ensure that all output generated to
      * date is transferred from the internal buffering and delivered to
      * the output file or destination.
      */
    void flush();

    /**
      * The page-width method is used to obtain the width (in printing
      * columns) of the output destination.
      */
    virtual int page_width() = 0;

    /**
      * The length method is used to obtain the length (in whole lines)
      * of the output destination.
      */
    virtual int page_length() = 0;

    /**
      * The end_of_line method is used to add a newline to the output
      * stream if the current position of the output is not at the
      * beginning of a line.
      */
    void end_of_line();

    /**
      * The type_name method is used to return the name of the class of
      * output.  Useful for debugging.
      */
    virtual const char *type_name() const = 0;

    /**
      * The register_delete_callback method is used to register a
      * functor to be called when this output stream instance is
      * destroyed.
      *
      * @param func
      *     The functor to be called
      */
    void register_delete_callback(functor::pointer func);

    /**
      * The unregister_delete_callback method is used to rescind and
      * earlier register_delete_callback call.
      *
      * @param func
      *     The functor not to be called
      */
    void unregister_delete_callback(functor::pointer func);

protected:
    /**
      * The write_inner method is called by the #write method to emit
      * buffered data.
      *
      * @param data
      *     The data to be written
      * @param len
      *     The number of wide characters to be written
      */
    virtual void write_inner(const wchar_t *data, size_t len) = 0;

    /**
      * The flush_inner method is called by the #flush method, after it
      * writes any buffered data via the #write_inner method.
      */
    virtual void flush_inner() = 0;

    /**
      * The end_of_line_inner method is used to do end-of-line
      * processing by the derived class, if the buffered character state
      * is not sufficient for the #end_of_line method to determine
      * whether or not the output position is currently at the start of
      * a line.
      */
    virtual void end_of_line_inner() = 0;

private:
    /**
      * The buffer instance variable is used to remember the base
      * address of a dynamically allocated array of wide characters,
      * used to buffer the output and reduce the number of calles to the
      * #write_inner method.
      */
    wchar_t *buffer;

    /**
      * The buffer_size instance variable is used to remember the
      * allocated size (in wide characters) of the #buffer instance
      * variable.
      */
    size_t buffer_size;

    /**
      * The buffer_position instance variable is used to remember the
      * current write position (for the next call to #put_wc) in the
      * #buffer of data.
      *
      * assert(buffer <= buffer_position);
      * assert(buffer_position <= buffer_end);
      */
    wchar_t *buffer_position;

    /**
      * The buffer_end instance variable is used to remember the
      * address which is immediately beyond the end of the allocated
      * #buffer array.  This is used by #put_wc to know when to call the
      * #overflow method.
      *
      * assert(buffer_end == buffer + buffer_size);
      */
    wchar_t *buffer_end;

    /**
      * The callback instance variable is used to remember the
      * fallback(s), if any, to call when this instance is destroyed.
      */
    functor_stack callback;

    /**
      * The overflow method is used to flush the buffer and add the
      * given character to the buffer in the case where the #put_wc
      * method determines there is no room left in the #buffer for more
      * characters.
      *
      * @param wc
      *     the character to write after the buffer
      */
    void overflow(wchar_t wc);
};

#endif // LIBAEGIS_WIDE_OUTPUT_H
