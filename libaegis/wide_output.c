/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999-2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate wchar_t output through a common API
 */

#include <ac/string.h>

#include <error.h> /* for assert */
#include <mem.h>
#include <str.h>
#include <trace.h>
#include <wide_output/private.h>
#include <wstr.h>


void
wide_output_delete(wide_output_ty *fp)
{
    size_t          ncb;

    trace(("wide_output_delete(fp = %08lX)\n{\ntype is wide_output_%s\n",
	(long)fp, fp->vptr->type_name));
    wide_output_flush(fp);

    /*
     * run any delete callbacks specified
     */
    ncb = fp->ncallbacks;
    fp->ncallbacks = 0;
    while (ncb > 0)
    {
	wide_output_callback_record *crp;

	--ncb;
	crp = &fp->callback[ncb];
	crp->func(fp, crp->arg);
    }
    if (fp->callback)
	mem_free(fp->callback);
    fp->ncallbacks = 0;
    fp->ncallbacks_max = 0;
    fp->callback = 0;

    /*
     * now run the destructor
     */
    if (fp->vptr->destructor)
	fp->vptr->destructor(fp);

    /*
     * now get rid of it
     */
    fp->vptr = 0;
    mem_free(fp->buffer);
    fp->buffer_size = 0;
    fp->buffer = 0;
    fp->buffer_position = 0;
    fp->buffer_end = 0;
    mem_free(fp);
    trace(("}\n"));
}


string_ty *
wide_output_filename(wide_output_ty *fp)
{
    string_ty       *result;

    trace(("wide_output_filename(fp = %08lX)\n{\ntype is wide_output_%s\n",
	    (long)fp, fp->vptr->type_name));
    assert(fp);
    assert(fp->vptr);
    assert(fp->vptr->filename);
    result = fp->vptr->filename(fp);
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}


#ifdef wide_output_putwc
#undef wide_output_putwc
#endif

void
wide_output_putwc(wide_output_ty *fp, wint_t wc)
{
    trace(("wide_output_putwc(fp = %08lX, wc = %04lX)\n{\n\
type is wide_output_%s\n", (long)fp, (long)wc, fp->vptr->type_name));
    assert(fp);
    assert(fp->buffer);
    assert(fp->buffer_size);
    assert(fp->buffer_position >= fp->buffer);
    assert(fp->buffer_end == fp->buffer + fp->buffer_size);
    assert(fp->buffer_position <= fp->buffer_end);
    if (fp->buffer_position >= fp->buffer_end)
    {
	assert(fp->vptr);
	assert(fp->vptr->write);
	fp->vptr->write(fp, fp->buffer, fp->buffer_size);
	fp->buffer_position = fp->buffer;
    }
    *(fp->buffer_position)++ = wc;
    trace(("}\n"));
}


void
wide_output_putws(wide_output_ty *fp, const wchar_t *ws)
{
    const wchar_t   *wse;

    trace(("wide_output_putws(fp = %08lX, s = %08lX)\n{\n\
type is wide_output_%s\n", (long)fp, (long)ws, fp->vptr->type_name));
    assert(ws);
    for (wse = ws; *wse; ++wse)
	;
    if (wse > ws)
	fp->vptr->write(fp, ws, wse - ws);
    trace(("}\n"));
}


void
wide_output_write(wide_output_ty *fp, const wchar_t *data, size_t len)
{
    trace(("wide_output_write(fp = %08lX, data = %08lX, len = %ld)\n{\n\
type wide_output_%s\n", (long)fp, (long)data, (long)len, fp->vptr->type_name));
    assert(data);
    /* assert(len); ideal, but not necessary */
    assert(fp);
    assert(fp->vptr);
    assert(fp->vptr->write);
    if (fp->buffer_position + len <= fp->buffer_end)
    {
	memcpy(fp->buffer_position, data, len * sizeof(wchar_t));
	fp->buffer_position += len;
    }
    else
    {
	size_t          nwc;

	nwc = fp->buffer_position - fp->buffer;
	fp->vptr->write(fp, fp->buffer, nwc);
	fp->buffer_position = fp->buffer;

	if (len < fp->buffer_size)
	{
	    memcpy(fp->buffer, data, len * sizeof(wchar_t));
	    fp->buffer_position += len;
	}
	else
	    fp->vptr->write(fp, data, len);
    }
    trace(("}\n"));
}


void
wide_output_flush(wide_output_ty *fp)
{
    trace(("wide_output_flush(fp = %08lX)\n{\n\
type is wide_output_%s\n", (long)fp, fp->vptr->type_name));
    assert(fp);
    assert(fp->vptr);
    if (fp->buffer_position > fp->buffer)
    {
	size_t		nwc;

	nwc = fp->buffer_position - fp->buffer;
	assert(fp->vptr->write);
	fp->vptr->write(fp, fp->buffer, nwc);
	fp->buffer_position = fp->buffer;
    }
    assert(fp->vptr->flush);
    fp->vptr->flush(fp);
    trace(("}\n"));
}


int
wide_output_page_width(wide_output_ty *fp)
{
    int             result;

    trace(("wide_output_page_width(fp = %08lX)\n{\n\
type is wide_output_%s\n", (long)fp, fp->vptr->type_name));
    assert(fp);
    assert(fp->vptr);
    assert(fp->vptr->page_width);
    result = fp->vptr->page_width(fp);
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


int
wide_output_page_length(wide_output_ty *fp)
{
    int             result;

    trace(("wide_output_page_length(fp = %08lX)\n{\n\
type is wide_output_%s\n", (long)fp, fp->vptr->type_name));
    assert(fp);
    assert(fp->vptr);
    assert(fp->vptr->page_length);
    result = fp->vptr->page_length(fp);
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


void
wide_output_end_of_line(wide_output_ty *fp)
{
    /*
     * If possible, just stuff a newline into the buffer and bail.
     * This results in the fewest deeper calls.
     */
    trace(("wide_output_end_of_line(fp = %08lX)\n{\n\
type is wide_output_%s\n", (long)fp, fp->vptr->type_name));
    assert(fp);
    if
    (
	fp->buffer_position > fp->buffer
    &&
	fp->buffer_position[-1] != '\n'
    &&
	fp->buffer_position < fp->buffer_end
    )
    {
	*(fp->buffer_position)++ = '\n';
	trace(("}\n"));
	return;
    }

    /*
     * If there is something in the buffer, we need to flush it,
     * so that the deeper eoln will have the current state.
     */
    assert(fp->vptr);
    if (fp->buffer_position > fp->buffer)
    {
	size_t		nwc;

	nwc = fp->buffer_position - fp->buffer;
	assert(fp->vptr->write);
	fp->vptr->write(fp, fp->buffer, nwc);
	fp->buffer_position = fp->buffer;
    }

    /*
     * Now ask the deeper instance to do it's end of line thing.
     */
    assert(fp->vptr->end_of_line);
    fp->vptr->end_of_line(fp);
    trace(("}\n"));
}


void
wide_output_delete_callback(wide_output_ty *fp, wide_output_callback_ty func,
    void *arg)
{
    wide_output_callback_record *crp;

    if (fp->ncallbacks >= fp->ncallbacks_max)
    {
	size_t		nbytes;

	fp->ncallbacks_max = fp->ncallbacks_max * 2 + 4;
	nbytes = fp->ncallbacks_max * sizeof(fp->callback[0]);
	fp->callback = mem_change_size(fp->callback, nbytes);
    }
    crp = &fp->callback[fp->ncallbacks++];
    crp->func = func;
    crp->arg = arg;
}


void
wide_output_put_wstr(wide_output_ty *fp, wstring_ty *ws)
{
    assert(fp);
    if (!ws || !ws->wstr_length)
	return;
    wide_output_write(fp, ws->wstr_text, ws->wstr_length);
}


void
wide_output_put_str(wide_output_ty *fp, string_ty *s)
{
    wstring_ty      *ws;

    if (!s || !s->str_length)
	return;
    ws = wstr_n_from_c(s->str_text, s->str_length);
    wide_output_put_wstr(fp, ws);
    wstr_free(ws);
}


void
wide_output_put_cstr(wide_output_ty *fp, const char *s)
{
    wstring_ty      *ws;

    if (!s || !*s)
	return;
    ws = wstr_from_c(s);
    wide_output_put_wstr(fp, ws);
    wstr_free(ws);
}
