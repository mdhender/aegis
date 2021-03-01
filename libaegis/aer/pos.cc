//
//	aegis - project change supervisor
//	Copyright (C) 1996, 1999, 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate file positions
//

#include <libaegis/aer/pos.h>
#include <common/error.h>
#include <common/mem.h>
#include <common/str.h>
#include <libaegis/sub.h>


rpt_pos_ty *
rpt_pos_alloc(string_ty *file_name, long line_number)
{
    rpt_pos_ty      *pp;

    pp = (rpt_pos_ty *)mem_alloc(sizeof(rpt_pos_ty));
    pp->reference_count = 1;
    pp->file_name = str_copy(file_name);
    pp->line_number1 = line_number;
    pp->line_number2 = line_number;
    return pp;
}


rpt_pos_ty *
rpt_pos_copy(rpt_pos_ty *pp)
{
    pp->reference_count++;
    return pp;
}


void
rpt_pos_free(rpt_pos_ty *pp)
{
    assert(pp->reference_count >= 1);
    pp->reference_count--;
    if (pp->reference_count <= 0)
    {
	str_free(pp->file_name);
	mem_free(pp);
    }
}


rpt_pos_ty *
rpt_pos_union(rpt_pos_ty *p1, rpt_pos_ty *p2)
{
    rpt_pos_ty      *result;
    long            min;
    long            max;

    assert(p1);
    assert(p2);
    // assert(str_equal(p1->file_name, p2->file_name));
    min = p1->line_number1;
    max = p1->line_number2;
    if (min > p2->line_number1)
	min = p2->line_number1;
    if (max > p2->line_number2)
	max = p2->line_number2;
    if (p1->line_number1 == min && p1->line_number2 == max)
	return rpt_pos_copy(p1);
    if (p2->line_number1 == min && p2->line_number2 == max)
	return rpt_pos_copy(p2);

    result = (rpt_pos_ty *)mem_alloc(sizeof(rpt_pos_ty));
    result->reference_count = 1;
    result->file_name = str_copy(p1->file_name);
    result->line_number1 = min;
    result->line_number2 = max;
    return result;
}


void
rpt_pos_error(sub_context_ty *scp, rpt_pos_ty *pp, const char *fmt)
{
    string_ty       *s;
    int             need_to_delete;

    if (!scp)
    {
	scp = sub_context_new();
	need_to_delete = 1;
    }
    else
	need_to_delete = 0;

    s = subst_intl(scp, fmt);

    // re-use substitution context
    sub_var_set_string(scp, "Message", s);
    str_free(s);

    if (!pp)
	error_intl(scp, i18n("$message"));
    else
    {
	sub_var_set_string(scp, "File_Name", pp->file_name);
	error_intl(scp, i18n("$filename: $message"));
    }

    if (need_to_delete)
	sub_context_delete(scp);
}
