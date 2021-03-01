/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1997, 1999, 2000, 2002, 2003 Peter Miller;
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
 * MANIFEST: interface definition for aegis/change_file.c
 */

#ifndef CHANGE_FILE_H
#define CHANGE_FILE_H

#include <change.h>

struct string_list_ty; /* existence */

/*
 * This would be static to aegis/change_file.c if only aegis/aer/value/fstate.c
 * did not need it.  No other place should access this directly.
 */
fstate_ty *change_fstate_get(change_ty *);

fstate_src_ty *change_file_find(change_ty *, string_ty *);
fstate_src_ty *change_file_find_fuzzy(change_ty *, string_ty *);
string_ty *change_file_path(change_ty *, string_ty *);
string_ty *change_file_source(change_ty *, string_ty *);
void change_file_remove(change_ty *, string_ty *);
fstate_src_ty *change_file_new(change_ty *, string_ty *);
void change_file_remove_all(change_ty *);
fstate_src_ty *change_file_nth(change_ty *, size_t);
size_t change_file_count(change_ty *);
void change_file_directory_query(change_ty *cp, string_ty *file_name,
    struct string_list_ty *result_in, struct string_list_ty *result_out);
string_ty *change_file_directory_conflict(change_ty *cp,
    string_ty *file_name);
void change_search_path_get(change_ty *, struct string_list_ty *, int);

void change_file_test_time_clear(change_ty *, fstate_src_ty *, string_ty *);
void change_file_test_time_set(change_ty *, fstate_src_ty *, time_t,
    string_ty *);
time_t change_file_test_time_get(change_ty *, fstate_src_ty *, string_ty *);
void change_file_test_baseline_time_clear(change_ty *, fstate_src_ty *,
    string_ty *);
void change_file_test_baseline_time_set(change_ty *, fstate_src_ty *, time_t,
    string_ty *);
time_t change_file_test_baseline_time_get(change_ty *, fstate_src_ty *,
    string_ty *);

int change_fingerprint_same(fingerprint_ty *, string_ty *, int);
void change_file_fingerprint_check(change_ty *, fstate_src_ty *);
int change_file_up_to_date(struct project_ty *, fstate_src_ty *);
struct metric_list_ty *change_file_metrics_get(change_ty *,
    struct string_ty *);
void change_file_list_metrics_check(change_ty *);
void change_file_template(change_ty *, string_ty *, struct user_ty *, int);
int change_file_is_config(change_ty *, string_ty *);

#endif /* CHANGE_FILE_H */
