/*
 *      aegis - project change supervisor
 *      Copyright (C) 1999-2001, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate privates
 */

#include <error.h> /* for assert */
#include <mem.h>
#include <page.h>
#include <trace.h>
#include <wide_output/private.h>


wide_output_ty *
wide_output_new(wide_output_vtbl_ty *vptr)
{
        wide_output_ty  *this_thing;

        trace(("wide_output_new(vptr = %08lX)\n{\n", (long)vptr));
        trace(("type is \"%s\"\n", vptr->type_name));
        assert(vptr);
        assert(vptr->size > sizeof(wide_output_ty));
        this_thing = (wide_output_ty *)mem_alloc(vptr->size);
        this_thing->vptr = vptr;

        this_thing->buffer_size = (size_t)1 << 11;
        this_thing->buffer =
            (wchar_t *)mem_alloc(this_thing->buffer_size * sizeof(wchar_t));
        this_thing->buffer_position = this_thing->buffer;
        this_thing->buffer_end = this_thing->buffer + this_thing->buffer_size;

        this_thing->ncallbacks = 0;
        this_thing->ncallbacks_max = 0;
        this_thing->callback = 0;
        trace(("return %08lX;", (long)this_thing));
        trace(("}\n"));
        return this_thing;
}


int
wide_output_generic_page_width(wide_output_ty *fp)
{
        return page_width_get(-1) - 1;
}


int
wide_output_generic_page_length(wide_output_ty *fp)
{
        return page_length_get(-1);
}
