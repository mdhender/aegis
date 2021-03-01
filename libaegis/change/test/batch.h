//
//      aegis - project change supervisor
//      Copyright (C) 2000, 2002, 2005-2008, 2012 Peter Miller
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

#ifndef LIBAEGIS_CHANGE_TEST_BATCH_H
#define LIBAEGIS_CHANGE_TEST_BATCH_H

#include <libaegis/change/test/batch_result.h>

struct string_list_ty;
struct user_ty;
class nstring_list; // forward

/**
  * The change_test_batch function is used to run a batch of tests using
  * the batch_test_command field of the project configuration file.
  *
  * @param cp
  *     The change being tested.
  * @param wlp
  *     The list of test files names to be run.
  * @param up
  *     The user to run the tests as.
  * @param baseline
  *     true if this is a baseline (negative) test, or
  *     false if this is a regular (positive) test.
  * @param current
  *     The number of tests completed so far.
  * @param total
  *     The total number opf tests to be run.
  * @param variable_assignments
  *     variable assignments passed from the command line
  * @param remaining_seconds
  *     An array of seconds remaining, indexed by the same index as wlp.
  */
batch_result_list_ty *change_test_batch(change::pointer cp, string_list_ty *wlp,
    user_ty::pointer up, bool baseline, int current, int total,
    const nstring_list &variable_assignments, const long *remaining_seconds);

#endif // LIBAEGIS_CHANGE_TEST_BATCH_H
// vim: set ts=8 sw=4 et :
