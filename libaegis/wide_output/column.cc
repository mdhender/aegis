//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2004 Peter Miller;
//      All rights reserved.
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate columns
//
// This class of wide output is used to store columns, in preparation for
// assembing them ready for output.  The number fo lines in the column is
// tracked, as is the printing width of the column.  This simplifies the
// task of printing the columns out, particularly if a wrapping filter
// is imposed before this one.
//

#include <error.h> // for assert
#include <language.h>
#include <mem.h>
#include <page.h>
#include <str.h>
#include <trace.h>
#include <wide_output/column.h>
#include <wide_output/private.h>


struct wide_output_column_ty
{
        wide_output_ty  inherited;

        int             width;
        int             length;

        column_row_ty   *row;
        size_t          nrows;
        size_t          nrows_max;
};


static void
wide_output_column_destructor(wide_output_ty *fp)
{
        wide_output_column_ty *this_thing;
        size_t          j;

        trace(("wide_output_column::destructor(fp = %08lX)\n{\n", (long)fp));
        this_thing = (wide_output_column_ty *)fp;
        for (j = 0; j < this_thing->nrows; ++j)
        {
                column_row_ty *rp = this_thing->row + j;
                if (rp->text)
                {
                        mem_free(rp->text);
                        rp->text = 0;
                }
        }
        if (this_thing->row)
                mem_free(this_thing->row);
        this_thing->row = 0;
        this_thing->nrows = 0;
        this_thing->nrows_max = 0;
        trace(("}\n"));
}


static string_ty *
wide_output_column_filename(wide_output_ty *fp)
{
        static string_ty *s;

        if (!s)
                s = str_from_c("(column)");
        return s;
}


static void
wide_output_column_write(wide_output_ty *fp, const wchar_t *buf, size_t buflen)
{
        wide_output_column_ty *this_thing;
        wchar_t         wc;
        size_t          nbytes;
        size_t          prev;
        size_t          j;
        column_row_ty   *rp;

        trace(("wide_output_column::write(fp = %08lX, buf = %08lX, len = %ld)\n\
{\n", (long)fp, (long)buf, (long)buflen));
        this_thing = (wide_output_column_ty *)fp;
        language_human();
        while (buflen > 0)
        {
                wc = *buf++;
                --buflen;

                //
                // newlines simply advance down the column
                //
                if (wc == '\n')
                        this_thing->nrows++;

                //
                // Make sure we have a row structure to park the
                // character in.
                //
                while (this_thing->nrows >= this_thing->nrows_max)
                {
                    prev = this_thing->nrows_max;
                    this_thing->nrows_max = 4 + 2 * this_thing->nrows_max;
                    nbytes =
                        this_thing->nrows_max * sizeof(this_thing->row[0]);
                    this_thing->row =
                        (column_row_ty *)
			mem_change_size(this_thing->row, nbytes);
                    for (j = prev; j < this_thing->nrows_max; ++j)
                    {
                        rp = this_thing->row + j;
                        rp->length_max = 0;
                        rp->length = 0;
                        rp->text = 0;
                        rp->printing_width = 0;
                    }
                }

                if (wc == '\n')
                        continue;

                //
                // Make sure we have text space enough to park the character.
                //
                rp = this_thing->row + this_thing->nrows;
                while (rp->length >= rp->length_max)
                {
                        rp->length_max = 16 + 2 * rp->length_max;
                        nbytes = rp->length_max * sizeof(rp->text[0]);
                        rp->text =
                            (wchar_t *)mem_change_size(rp->text, nbytes);
                }

                //
                // Remember the character,
                // and keep a running tally of the printing width.
                //
                rp->text[rp->length++] = wc;
                rp->printing_width += wcwidth(wc);
        }
        language_C();
        trace(("}\n"));
}


static int
wide_output_column_page_width(wide_output_ty *fp)
{
        wide_output_column_ty *this_thing;

        this_thing = (wide_output_column_ty *)fp;
        return this_thing->width;
}


static void
wide_output_column_flush(wide_output_ty *fp)
{
}


static int
wide_output_column_page_length(wide_output_ty *fp)
{
        wide_output_column_ty *this_thing;

        this_thing = (wide_output_column_ty *)fp;
        return this_thing->length;
}


static void
wide_output_column_eoln(wide_output_ty *fp)
{
        wide_output_column_ty *this_thing;
        column_row_ty   *rp;

        trace(("wide_output_column::eoln(fp = %08lX)\n{\n", (long)fp));
        this_thing = (wide_output_column_ty *)fp;
        if (this_thing->nrows > 0 && this_thing->nrows < this_thing->nrows_max)
        {
                rp = this_thing->row + this_thing->nrows;
                if (rp->length > 0)
                        wide_output_putwc(fp, (wchar_t)'\n');
        }
        trace(("}\n"));
}


static wide_output_vtbl_ty vtbl =
{
        sizeof(wide_output_column_ty),
        wide_output_column_destructor,
        wide_output_column_filename,
        wide_output_column_write,
        wide_output_column_flush,
        wide_output_column_page_width,
        wide_output_column_page_length,
        wide_output_column_eoln,
        "column",
};


wide_output_ty *
wide_output_column_open(int width, int length)
{
        wide_output_ty  *result;
        wide_output_column_ty *this_thing;

        trace(("wide_output_column::new(%d, %d)\n{\n", width, length));
        result = wide_output_new(&vtbl);
        this_thing = (wide_output_column_ty *)result;
        this_thing->width = (width <= 0 ? page_width_get(-1) : width);
        this_thing->length =
            (length <= 0 ? page_length_get(-1) : length);
        this_thing->row = 0;
        this_thing->nrows = 0;
        this_thing->nrows_max = 0;
        trace(("return %08lX;\n", (long)result));
        trace(("}\n"));
        return result;
}


column_row_ty *
wide_output_column_get(wide_output_ty *fp, int n)
{
        wide_output_column_ty *this_thing;
        column_row_ty   *rp;

        trace(("wide_output_column::get(fp = %08lX, n = %d)\n{\n",
                (long)fp, n));
        if (fp->vptr != &vtbl)
        {
                trace(("WRONG!  Type is %s\n", fp->vptr->type_name));
                assert(0);
                trace(("}\n"));
                return 0;
        }
        if (n < 0)
        {
                trace(("return NULL;\n"));
                trace(("}\n"));
                return 0;
        }
        this_thing = (wide_output_column_ty *)fp;
        if (n >= (int)this_thing->nrows_max)
        {
                trace(("return NULL;\n"));
                trace(("}\n"));
                return 0;
        }
        rp = this_thing->row + n;
        if (rp->length == 0)
        {
                //
                // See if all rows after this one are empty, too.
                // If they are, pretend this row doesn't exist.
                //
                size_t k;
                int end_of_the_road = 1;
                for (k = n + 1; k < this_thing->nrows; ++k)
                {
                        if (this_thing->row[k].length)
                        {
                                end_of_the_road = 0;
                                break;
                        }
                }
                if (end_of_the_road)
                {
                        trace(("return NULL;\n"));
                        trace(("}\n"));
                        return 0;
                }
        }
        trace(("return %08lX;\n", (long)rp));
        trace(("}\n"));
        return rp;
}


void
wide_output_column_reset(wide_output_ty *fp)
{
        wide_output_column_ty *this_thing;
        size_t          j;

        trace(("wide_output_column::reset(fp = %08lX)\n{\n", (long)fp));
        if (fp->vptr != &vtbl)
        {
                trace(("WRONG!  Type is %s\n", fp->vptr->type_name));
                assert(0);
                trace(("}\n"));
                return;
        }
        this_thing = (wide_output_column_ty *)fp;
        for (j = 0; j < this_thing->nrows_max; ++j)
        {
                this_thing->row[j].length = 0;
                this_thing->row[j].printing_width = 0;
        }
        this_thing->nrows = 0;
        trace(("}\n"));
}
