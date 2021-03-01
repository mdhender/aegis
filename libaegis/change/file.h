/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1999 Peter Miller;
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
fstate change_fstate_get _((change_ty *));

fstate_src change_file_find _((change_ty *, string_ty *));
fstate_src change_file_find_fuzzy _((change_ty *, string_ty *));
string_ty *change_file_path _((change_ty *, string_ty *));
string_ty *change_file_source _((change_ty *, string_ty *));
void change_file_remove _((change_ty *, string_ty *));
fstate_src change_file_new _((change_ty *, string_ty *));
void change_file_remove_all _((change_ty *));
fstate_src change_file_nth _((change_ty *, size_t));
size_t change_file_count _((change_ty *));
void change_file_directory_query _((change_ty *cp, string_ty *file_name,
	struct string_list_ty *result_in, struct string_list_ty *result_out));
string_ty *change_file_directory_conflict _((change_ty *cp,
	string_ty *file_name));
void change_search_path_get _((change_ty *, struct string_list_ty *, int));

void change_file_test_time_clear _((change_ty *, fstate_src));
void change_file_test_time_set _((change_ty *, fstate_src, time_t));
time_t change_file_test_time_get _((change_ty *, fstate_src));
void change_file_test_baseline_time_clear _((change_ty *, fstate_src));
void change_file_test_baseline_time_set _((change_ty *, fstate_src, time_t));
time_t change_file_test_baseline_time_get _((change_ty *, fstate_src));

int change_fingerprint_same _((fingerprint, string_ty *, int));
void change_file_fingerprint_check _((change_ty *, fstate_src));
int change_file_up_to_date _((struct project_ty *, fstate_src));
struct metric_list *change_file_metrics_get _((change_ty *, struct string_ty *));
void change_file_list_metrics_check _((change_ty *));

#endif /* CHANGE_FILE_H */
