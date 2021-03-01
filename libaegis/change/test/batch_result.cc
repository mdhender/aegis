//
//	aegis - project change supervisor
//	Copyright (C) 2000, 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate batch_results
//

#include <change/test/batch_result.h>
#include <mem.h>


batch_result_list_ty *
batch_result_list_new(void)
{
    batch_result_list_ty *p;

    p = (batch_result_list_ty *)mem_alloc(sizeof(batch_result_list_ty));
    p->length = 0;
    p->length_max = 0;
    p->item = 0;

    p->pass_count = 0;
    p->no_result_count = 0;
    p->fail_count = 0;
    return p;
}


void
batch_result_list_delete(batch_result_list_ty *p)
{
    size_t          j;

    for (j = 0; j < p->length; ++j)
    {
	str_free(p->item[j].file_name);
	if (p->item[j].architecture)
	    str_free(p->item[j].architecture);
    }
    if (p->item)
	mem_free(p->item);
    p->length = 0;
    p->length_max = 0;
    p->item = 0;
    mem_free(p);
}


void
batch_result_list_append(batch_result_list_ty *p, string_ty *file_name,
    int exit_status, string_ty *architecture)
{
    batch_result_ty *brp;

    if (p->length >= p->length_max)
    {
	size_t		nbytes;

	p->length_max = p->length_max * 2 + 4;
	nbytes = p->length_max * sizeof(p->item[0]);
	p->item = (batch_result_ty *)mem_change_size(p->item, nbytes);
    }
    brp = p->item + p->length++;
    brp->file_name = str_copy(file_name);
    brp->exit_status = exit_status;
    brp->architecture = architecture ? str_copy(architecture) : 0;
}


void
batch_result_list_append_list(batch_result_list_ty *p,
    const batch_result_list_ty *p2)
{
    size_t          j;

    for (j = 0; j < p2->length; ++j)
    {
	const batch_result_ty *brp2 = p2->item + j;
	batch_result_list_append
	(
	    p,
	    brp2->file_name,
	    brp2->exit_status,
	    brp2->architecture
	);
    }
    p->pass_count += p2->pass_count;
    p->no_result_count += p2->no_result_count;
    p->fail_count += p2->fail_count;
}


int
batch_result_list_member(batch_result_list_ty *p, string_ty *file_name)
{
    size_t          j;

    for (j = 0; j < p->length; ++j)
    {
	if (str_equal(file_name, p->item[j].file_name))
    	    return 1;
    }
    return 0;
}
