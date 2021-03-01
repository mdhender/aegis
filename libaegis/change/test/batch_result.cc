//
//      aegis - project change supervisor
//      Copyright (C) 2000, 2002-2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/mem.h>
#include <common/nstring.h>
#include <common/trace.h>
#include <libaegis/change/test/batch_result.h>


batch_result_list_ty *
batch_result_list_new(void)
{
    trace(("batch_result_list_new\n"));
    batch_result_list_ty *p;

    p = (batch_result_list_ty *)mem_alloc(sizeof(batch_result_list_ty));
    p->length = 0;
    p->length_max = 0;
    p->item = 0;

    p->pass_count = 0;
    p->skip_count = 0;
    p->no_result_count = 0;
    p->fail_count = 0;
    p->elapsed = 0;
    return p;
}


void
batch_result_list_delete(batch_result_list_ty *p)
{
    trace(("batch_result_list_delete\n"));
    size_t          j;

    for (j = 0; j < p->length; ++j)
    {
        str_free(p->item[j].file_name);
        if (p->item[j].architecture)
            str_free(p->item[j].architecture);
    }
    delete [] p->item;
    p->length = 0;
    p->length_max = 0;
    p->item = 0;
    mem_free(p);
}


void
batch_result_list_append(batch_result_list_ty *p, string_ty *file_name,
    int exit_status, string_ty *architecture, double elapsed)
{
    trace(("batch_result_list_append(p = %p, file_name = %s, "
        "exit_status = %d, architecture = %s, elapsed = %g)\n{\n", p,
        nstring(file_name).quote_c().c_str(), exit_status,
        nstring(architecture).quote_c().c_str(), elapsed));
    if (p->length >= p->length_max)
    {
        size_t new_length_max = p->length_max * 2 + 4;
        batch_result_ty *new_item = new batch_result_ty [new_length_max];
        for (size_t j = 0; j < p->length; ++j)
            new_item[j] = p->item[j];
        delete [] p->item;
        p->item = new_item;
        p->length_max = new_length_max;
    }
    batch_result_ty *brp = p->item + p->length++;
    assert(file_name);
    brp->file_name = str_copy(file_name);
    brp->exit_status = exit_status;
    brp->architecture = architecture ? str_copy(architecture) : 0;
    brp->elapsed = elapsed;
    trace(("}\n"));
}


void
batch_result_list_append_list(batch_result_list_ty *p,
    const batch_result_list_ty *p2)
{
    for (size_t j = 0; j < p2->length; ++j)
    {
        const batch_result_ty *brp2 = p2->item + j;
        batch_result_list_append
        (
            p,
            brp2->file_name,
            brp2->exit_status,
            brp2->architecture,
            brp2->elapsed
        );
    }
    p->pass_count += p2->pass_count;
    p->skip_count += p2->skip_count;
    p->no_result_count += p2->no_result_count;
    p->fail_count += p2->fail_count;
    p->elapsed += p2->elapsed;
}


bool
batch_result_list_member(batch_result_list_ty *p, string_ty *file_name,
    string_ty *architecture)
{
    assert(p);
    if (!p)
        return false;
    for (size_t j = 0; j < p->length; ++j)
    {
        batch_result_ty *brp = p->item +j;
        assert(brp->file_name);
        if (!brp->file_name)
            continue;
        if
        (
            str_equal(file_name, brp->file_name)
        &&
            (
                architecture
            ?
                (
                    brp->architecture
                &&
                    str_equal(architecture, brp->architecture)
                )
            :
                (0 == brp->architecture)
            )
        )
            return true;
    }
    return false;
}


// vim: set ts=8 sw=4 et :
