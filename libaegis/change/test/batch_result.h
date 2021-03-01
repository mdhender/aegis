//
//	aegis - project change supervisor
//	Copyright (C) 2000, 2002, 2004-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_CHANGE_TEST_BATCH_RESULT_H
#define LIBAEGIS_CHANGE_TEST_BATCH_RESULT_H

#include <common/str.h>

struct batch_result_ty
{
    string_ty       *file_name;
    int             exit_status;
    string_ty       *architecture;
    double          elapsed; // seconds
};

struct batch_result_list_ty
{
    size_t          length;
    size_t          length_max;
    batch_result_ty *item;

    long            pass_count;
    long            skip_count;
    long            no_result_count;
    long            fail_count;
    double          elapsed; // seconds
};

batch_result_list_ty *batch_result_list_new(void);
void batch_result_list_delete(batch_result_list_ty *);
void batch_result_list_append(batch_result_list_ty *brlp, string_ty *file_name,
    int exit_status, string_ty *architecture, double elapsed);
void batch_result_list_append_list(batch_result_list_ty *,
    const batch_result_list_ty *);

/**
  * The batch_result_list_member function is used to test whether the
  * given (file_name, architecture) pair is a member of the given test
  * result list.
  *
  * @param brlp
  *     The results list in question.
  * @param file_name
  *     The file name in question.
  * @param architecture
  *     The architecture name, or NULL
  * @returns
  *     bool; true if a member present, false if not.
  */
bool batch_result_list_member(batch_result_list_ty *brlp, string_ty *file_name,
    string_ty *architecture);

#endif // LIBAEGIS_CHANGE_TEST_BATCH_RESULT_H
