//
// aegis - project change supervisor
// Copyright (C) 1991-1997, 1999, 2000, 2002-2008, 2011, 2012 Peter Miller
// Copyright (C) 2007 Walter Franzini
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef CHANGE_FILE_H
#define CHANGE_FILE_H

#include <common/gcc-attribute.h>
#include <libaegis/change.h>
#include <libaegis/view_path.h>

class string_list_ty; // forward
class nstring; // forward

/** Deprecated.  Use change::file_path instead. */
string_ty *change_file_path(change::pointer cp, string_ty *file_name)
    DEPRECATED;

/** Deprecated. Use change::file_path_by_uuid instead. */
string_ty *change_file_path_by_uuid(change::pointer cp, string_ty *uuid)
    DEPRECATED;

/** Deprecated.  Use change::file_path instead. */
string_ty *change_file_path(change::pointer cp, fstate_src_ty *src)
    DEPRECATED;

string_ty *change_file_version_path(change::pointer cp, fstate_src_ty *src,
    int *unlink_p);
string_ty *change_file_source(change::pointer , string_ty *);
void change_file_remove(change::pointer , string_ty *);

void change_file_remove_all(change::pointer );
fstate_src_ty *change_file_nth(change::pointer , size_t, view_path_ty);
size_t change_file_count(change::pointer );
void change_file_directory_query(change::pointer cp, string_ty *file_name,
    string_list_ty *result_in, string_list_ty *result_out);
string_ty *change_file_directory_conflict(change::pointer cp,
    string_ty *file_name);

void change_file_test_time_clear(change::pointer cp, fstate_src_ty *src,
    string_ty *);
void change_file_test_time_set(change::pointer , fstate_src_ty *, time_t,
    string_ty *);
time_t change_file_test_time_get(change::pointer cp, fstate_src_ty *src,
    string_ty *);
void change_file_test_baseline_time_clear(change::pointer , fstate_src_ty *,
    string_ty *);
void change_file_test_baseline_time_set(change::pointer cp, fstate_src_ty *src,
    time_t, string_ty *);
time_t change_file_test_baseline_time_get(change::pointer , fstate_src_ty *,
    string_ty *);

int change_fingerprint_same(fingerprint_ty *, string_ty *, int);
void change_file_fingerprint_check(change::pointer , fstate_src_ty *);
int change_file_up_to_date(project *, fstate_src_ty *);
metric_list_ty *change_file_metrics_get(change::pointer , string_ty *);
void change_file_list_metrics_check(change::pointer );
void change_file_template(change::pointer , string_ty *, user_ty::pointer, int);

/**
  * The change_file_copy_basic_attributes function is used to copy the
  * basic change file attributes (usage, attributes and uuid) from one
  * file to another.  This is a common activity for aecp, et al.
  *
  * @param to
  *     The file meta data to receive the attributes
  * @param from
  *     The file meta data from which the attributes are to be taken.
  */
void change_file_copy_basic_attributes(fstate_src_ty *to, fstate_src_ty *from);

// vim: set ts=8 sw=4 et :
#endif // CHANGE_FILE_H
