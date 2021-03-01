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
// MANIFEST: interface definition for libaegis/col/pretty.c
//

#ifndef LIBAEGIS_COL_PRETTY_H
#define LIBAEGIS_COL_PRETTY_H

#include <libaegis/col.h>

struct wide_output_ty; // forward

/**
  * The col_pretty class is used to represent multi-column output with
  * line wrap around the column values, and formatting across multiple
  * lines.
  */
class col_pretty:
    public col
{
public:
    /**
      * The destructor.
      */
    virtual ~col_pretty();

    /**
      * The constructor.
      */
    col_pretty(wide_output_ty *deeper, bool delete_on_close);

    // See base class for documentation.
    output_ty *create(int, int, const char*);

    // See base class for documentation.
    void title(const char*, const char*);

    // See base class for documentation.
    void eoln();

    // See base class for documentation.
    void need(int);

    // See base class for documentation.
    void eject();

    /**
      * The delcb method is used to notify us that one of the output
      * streams (which indirectly fills out column contents) is about to
      * be deleted.  This allows us to stop remembering it, and thus not
      * de-reference dangling pointers.
      *
      * @param fp
      *     The output stream about to be deleted.
      *
      * @note
      *     This method is pub;ic only so that the output_ty destructor
      *     can call it.  Nothing else may use this method.
      */
    void delcb(output_ty *fp);

    // See base class for documentation.
    void flush();

private:
    wide_output_ty *deeper;
    bool need_to_emit_headers;
    size_t ncolumns;
    size_t ncolumns_max;

    struct column_ty
    {
	~column_ty();
	column_ty();
	column_ty(const column_ty &);
	column_ty &operator=(const column_ty &);

	wide_output_ty *header;
	wide_output_ty *content;
	output_ty *content_filter;
	int left;
	int right;
    };

    column_ty *column;

    struct emit_ty
    {
	emit_ty() : content(0), left(0) { }

	wide_output_ty *content;
	int	left;
    };

    /**
      * The emit method is used to emit the given values of text out
      * of the deeper output stream.  The emitting is factored into this
      * common method, rather than reproduce the code twice, once
      * for the headers and again for the content.
      */
    void emit(size_t argc, emit_ty *argv, int minlines, bool is_the_header);

    /**
      * The emit_header method is used to emit column headers when
      * necessary.  It clears the flag which controls emitting the
      * headers, too.
      */
    void emit_header();

    /**
      * The emit_content method is used to generate the output, given
      * the content written inforctly via the wide output streams
      * privide by create (below).
      *
      * Usually, the column headers are handled by the top-of-page
      * callback, however if new columns with headers are created in the
      * middile of the page, this method also causes the new column
      * headings to be emitted, before the content.
      */
    void emit_content();

    /**
      * The default constructor.  Do not use.
      */
    col_pretty();

    /**
      * The copy constructor.  Do not use.
      */
    col_pretty(const col_pretty &);

    /**
      * The assignment operator.  Do not use.
      */
    col_pretty &operator=(const col_pretty &);
};

#endif // LIBAEGIS_COL_PRETTY_H
