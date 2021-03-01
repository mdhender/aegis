//
//	aegis - project change supervisor
//	Copyright (C) 2000, 2002, 2004 Peter Miller;
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
// MANIFEST: interface definition for libaegis/change/test/batch_result.c
//

#ifndef LIBAEGIS_CHANGE_TEST_BATCH_RESULT_H
#define LIBAEGIS_CHANGE_TEST_BATCH_RESULT_H

#include <str.h>

struct batch_result_ty
{
    string_ty       *file_name;
    int             exit_status;
    string_ty       *architecture;
};

struct batch_result_list_ty
{
    size_t          length;
    size_t          length_max;
    batch_result_ty *item;

    long            pass_count;
    long            no_result_count;
    long            fail_count;
};

batch_result_list_ty *batch_result_list_new(void);
void batch_result_list_delete(batch_result_list_ty *);
void batch_result_list_append(batch_result_list_ty *brlp, string_ty *file_name,
    int exit_status, string_ty *architecture);
void batch_result_list_append_list(batch_result_list_ty *,
    const batch_result_list_ty *);
int batch_result_list_member(batch_result_list_ty *, string_ty *);

#endif // LIBAEGIS_CHANGE_TEST_BATCH_RESULT_H
