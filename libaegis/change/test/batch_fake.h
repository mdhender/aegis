//
//	aegis - project change supervisor
//	Copyright (C) 2000, 2002, 2005, 2006 Peter Miller;
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
// MANIFEST: interface definition for libaegis/change/test/batch_fake.c
//

#ifndef LIBAEGIS_CHANGE_TEST_BATCH_FAKE_H
#define LIBAEGIS_CHANGE_TEST_BATCH_FAKE_H

#include <libaegis/change/test/batch_result.h>

struct change_ty;
struct string_list_ty;
struct user_ty;
class nstring_list; // forward

/**
  * The change_test_batch_fake function is used to run a batch of tests
  * using the test_command field of the project configuration file.  This
  * is because no batch_test_command field has been specified.
  *
  * \param cp
  *     The change being tested.
  * \param wlp
  *     The list of test files names to be run.
  * \param up
  *     The user to run the tests as.
  * \param baseline
  *     true if this is a baseline (negative) test, or
  *     false if this is a regular (positive) test.
  * \param current
  *     The number of tests completed so far.
  * \param total
  *     The total number opf tests to be run.
  * \param time_limit
  *     zero if there is no time limit, or non-zero if testing should
  *     cease at this time.
  */
batch_result_list_ty *change_test_batch_fake(change_ty *cp, string_list_ty *wlp,
    user_ty *up, bool baseline, int current, int total, time_t time_limit,
    const nstring_list &variable_assignments);

#endif // LIBAEGIS_CHANGE_TEST_BATCH_FAKE_H
