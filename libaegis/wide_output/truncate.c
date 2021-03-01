/*
 *      aegis - project change supervisor
 *      Copyright (C) 1999-2003 Peter Miller;
 *      All rights reserved.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate truncates
 *
 * This class of wide output is used to truncate lines to a specified
 * printing width.  Characters beyond on each line this limit are
 * discarded.
 */

#include <language.h>
#include <mem.h>
#include <str.h>
#include <wide_output.h>
#include <wide_output/private.h>
#include <wide_output/truncate.h>


typedef struct wide_output_truncate_ty wide_output_truncate_ty;
struct wide_output_truncate_ty
{
    wide_output_ty  inherited;
    wide_output_ty  *deeper;
    int             delete_on_close;
    int             width;

    wchar_t         *buf;
    size_t          buf_pos;
    size_t          buf_max;

    int             column;
};


static void
wide_output_truncate_destructor(wide_output_ty *fp)
{
    wide_output_truncate_ty *this_thing;

    this_thing = (wide_output_truncate_ty *)fp;
    if (this_thing->buf_pos)
        wide_output_write(
            this_thing->deeper, this_thing->buf, this_thing->buf_pos);
    if (this_thing->buf)
        mem_free(this_thing->buf);
    if (this_thing->delete_on_close)
        wide_output_delete(this_thing->deeper);
    this_thing->deeper = 0;
}


static string_ty *
wide_output_truncate_filename(wide_output_ty *fp)
{
    wide_output_truncate_ty *this_thing;

    this_thing = (wide_output_truncate_ty *)fp;
    return wide_output_filename(this_thing->deeper);
}


static void
wide_output_truncate_write(wide_output_ty *fp, const wchar_t *data, size_t len)
{
    wide_output_truncate_ty *this_thing;
    int             cwid;

    this_thing = (wide_output_truncate_ty *)fp;
    language_human();
    while (len > 0)
    {
        wchar_t wc = *data++;
        --len;

        switch (wc)
        {
        case '\n':
        case '\f':
            language_C();
            if (this_thing->buf_pos)
            {
                wide_output_write(
                    this_thing->deeper, this_thing->buf, this_thing->buf_pos);
            }
            wide_output_putwc(this_thing->deeper, wc);
            language_human();
            this_thing->buf_pos = 0;
            this_thing->column = 0;
            break;

        default:
            /*
             * If we have already become too wide, don't
             * make the deeper unnecessary function calls.
             */
            if (this_thing->column >= this_thing->width)
                break;

            /*
             * Only remember this_thing character if all of it
             * fits within the specified width.
             */
            cwid = wcwidth(wc);
            if (this_thing->column + cwid > this_thing->width)
                break;

            /*
             * Make room if necessary
             */
            if (this_thing->buf_pos >= this_thing->buf_max)
            {
                size_t          nbytes;

                this_thing->buf_max = 16 + 2 * this_thing->buf_max;
                nbytes = this_thing->buf_max * sizeof(this_thing->buf[0]);
                this_thing->buf =
                    (wchar_t *)mem_change_size(this_thing->buf, nbytes);
            }
            this_thing->buf[this_thing->buf_pos++] = wc;
            this_thing->column += cwid;
            break;
        }
    }
    language_C();
}


static void
wide_output_truncate_flush(wide_output_ty *fp)
{
    wide_output_truncate_ty *this_thing;

    this_thing = (wide_output_truncate_ty *)fp;
    wide_output_write(this_thing->deeper, this_thing->buf, this_thing->buf_pos);
    this_thing->buf_pos = 0;
    /* DO NOT reset this_thing->column */
}


static int
wide_output_truncate_page_width(wide_output_ty *fp)
{
    wide_output_truncate_ty *this_thing;

    this_thing = (wide_output_truncate_ty *)fp;
    return this_thing->width;
}


static int
wide_output_truncate_page_length(wide_output_ty *fp)
{
    wide_output_truncate_ty *this_thing;

    this_thing = (wide_output_truncate_ty *)fp;
    return wide_output_page_length(this_thing->deeper);
}


static void
wide_output_truncate_eoln(wide_output_ty *fp)
{
    wide_output_truncate_ty *this_thing;

    this_thing = (wide_output_truncate_ty *)fp;
    if (this_thing->column > 0)
        wide_output_putwc(fp, (wchar_t)'\n');
}


static wide_output_vtbl_ty vtbl =
{
    sizeof(wide_output_truncate_ty),
    wide_output_truncate_destructor,
    wide_output_truncate_filename,
    wide_output_truncate_write,
    wide_output_truncate_flush,
    wide_output_truncate_page_width,
    wide_output_truncate_page_length,
    wide_output_truncate_eoln,
    "truncate",
};


wide_output_ty *
wide_output_truncate_open(wide_output_ty *deeper, int delete_on_close,
    int width)
{
    wide_output_ty  *result;
    wide_output_truncate_ty *this_thing;

    result = wide_output_new(&vtbl);
    this_thing = (wide_output_truncate_ty *)result;
    this_thing->deeper = deeper;
    this_thing->delete_on_close = delete_on_close;
    this_thing->width = (width <= 0 ? wide_output_page_width(deeper) : width);
    this_thing->buf = 0;
    this_thing->buf_pos = 0;
    this_thing->buf_max = 0;
    this_thing->column = 0;
    return result;
}
