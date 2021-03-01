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
// MANIFEST: interface definition for libaegis/col/unformatted.c
//

#ifndef LIBAEGIS_COL_UNFORMATTED_H
#define LIBAEGIS_COL_UNFORMATTED_H

#include <common/ac/stddef.h>

#include <libaegis/col.h>

class wide_output_ty; // forward


/**
  * The col_unformatted class is used to represent columnar output wich
  * simply has the columns concatenated on a single line, without any
  * formatting.
  */
class col_unformatted:
    public col
{
public:
    /**
      * The destructor.
      */
    virtual ~col_unformatted();

    /**
      * The constructor.
      */
    col_unformatted(wide_output_ty *deeper, bool delete_on_close);

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
      * The delcb method is used to clear out references to deleted
      * columns.  This function is registered as a callback with
      * wide_output_callback when the content filter is created.
      *
      * @param fp
      *     The output stream about to be deleted.
      * @note
      *     This method is sonly public so the output destructor can
      *     call it.  No other use of this method is allowed.
      */
    void delcb(output_ty *fp);

    // See base class for documentation.
    void flush();

private:
    struct column_ty
    {
	~column_ty();
	column_ty();
	column_ty(const column_ty &);
	column_ty &operator=(const column_ty &);

	wide_output_ty *content;
	output_ty *content_filter;
    };

    wide_output_ty *deeper;
    bool delete_on_close;
    wchar_t separator;
    size_t ncolumns;
    size_t ncolumns_max;
    column_ty *column;

    /**
      * The default constructor.  Do not use.
      */
    col_unformatted();

    /**
      * The copy constructor.  Do not use.
      */
    col_unformatted(const col_unformatted &);

    /**
      * The assignment operator.  Do not use.
      */
    col_unformatted &operator=(const col_unformatted &);
};

#endif // LIBAEGIS_COL_UNFORMATTED_H
