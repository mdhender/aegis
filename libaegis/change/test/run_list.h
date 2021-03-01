/*
 *	aegis - project change supervisor
 *	Copyright (C) 2000, 2002, 2005 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/change/test/run_list.c
 */

#ifndef LIBAEGIS_CHANGE_TEST_RUN_LIST_H
#define LIBAEGIS_CHANGE_TEST_RUN_LIST_H

#include <ac/time.h>

#include <change/test/batch_result.h>

struct change_ty;
struct project_ty;
struct string_list_ty;
struct user_ty;

/**
  * The change_test_run_list function is used to run a list of tests.
  * It will use the batch test command if one is defined, otherwise it
  * will use the regular test command.
  *
  * \param pp
  *     The project we are testing.
  * \param wlp
  *     The list of test file names.
  * \param up
  *     The user we are running the tests as.
  * \param baseline
  *     Whether this is a baseline (negative) test or a development
  *     directory (positive) test.
  * \param progress_flag
  *     Whether or not to emot progress messages.
  * \param time_limit
  *     Zero means no limit, non-zero is a time at which to stop.
  */
batch_result_list_ty *change_test_run_list(change_ty *pp, string_list_ty *wlp,
    user_ty *up, bool baseline, bool progress_flag, time_t time_limit);

/**
  * The project_test_run_list function is used to run a list of tests.
  * It will use the batch test command if one is defined, otherwise it
  * will use the regular test command.
  *
  * \param pp
  *     The project we are testing.
  * \param wlp
  *     The list of test file names.
  * \param up
  *     The user we are running the tests as.
  * \param progress_flag
  *     Whether or not to emot progress messages.
  * \param time_limit
  *     Zero means no limit, non-zero is a time at which to stop.
  */
batch_result_list_ty *project_test_run_list(project_ty *pp, string_list_ty *wlp,
    user_ty *up, bool progress_flag, time_t time_limit);

#endif /* LIBAEGIS_CHANGE_TEST_RUN_LIST_H */
