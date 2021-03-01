/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate shallows
 */

#include <change/file.h>
#include <error.h> /* for assert */
#include <project/file.h>


void
project_file_shallow(pp, file_name, cn)
	project_ty	*pp;
	string_ty	*file_name;
	long		cn;
{
	change_ty	*pcp;
	fstate_src	src1_data;
	fstate_src	src2_data;

	/*
	 * Will never be zero, because already magic zero encoded.
	 */
	assert(cn);

	/*
	 * Look for the file in the project.
	 * If it is there, nothing more needs to be done.
	 */
	if (!pp->parent)
		return;
	pcp = project_change_get(pp);
	src1_data = change_file_find(pcp, file_name);
	if (src1_data)
		return;

	/*
	 * The file is not part of the immediate project, see if
	 * we can find it in one of the grand*parent projects.
	 * If it is not there, nothing more needs to be done.
	 */
	src2_data = project_file_find(pp, file_name);
	if (!src2_data)
		return;

	/*
	 * Create a new file in the project, and mark it ``about to be
	 * copied''.  That way we can throw it away again, if the
	 * review fails or the integration fails.
	 */
	src1_data = change_file_new(pcp, file_name);
	src1_data->action = src2_data->action;
	src1_data->usage = src2_data->usage;
	src1_data->about_to_be_copied_by = cn;

	/*
	 * The value here is bogus (can't use the old one, it refers to
	 * the wrong branch).  The values only lasts long enough to be
	 * replaced (ipass) or removed (ifail).
	 */
	if (src2_data->deleted_by)
		src1_data->deleted_by = cn;

	/*
	 * As a branch advances, the edit_number tracks the
	 * history, but the edit_number_origin is the number when
	 * the file was first created or copied into the branch.
	 * By definition, a file in a change is out of date when
	 * it's edit_number_origin does not equal the edit_number
	 * of its project.
	 *
	 * In order to merge branches, this must be done as a
	 * cross branch merge in a change to that branch; the
	 * edit_number_origin_new field of the change is copied
	 * into the edit_number_origin origin field of the branch.
	 *
	 * branch's edit_number
	 *	The head revision of the branch.
	 * branch's edit_number_origin
	 *	The version originally copied.
	 */
	assert(src2_data->edit_number);
	if (src2_data->edit_number)
		src1_data->edit_number = str_copy(src2_data->edit_number);
	else
		src1_data->edit_number = str_from_c("bogus");
	src1_data->edit_number_origin =
		str_copy(src1_data->edit_number);

	/*
	 * pull the testing correlations across
	 */
	if (src2_data->test && src2_data->test->length)
	{
		size_t	j;

		src1_data->test = fstate_src_test_list_type.alloc();
		for (j = 0; j < src2_data->test->length; ++j)
		{
			string_ty	**addr_p;
			type_ty		*type_p;

			addr_p =
			    fstate_src_test_list_type.list_parse
				(
					src1_data->test,
					&type_p
				);
			assert(type_p = &string_type);
			*addr_p = str_copy(src2_data->test->list[j]);
		}
	}

	/*
	 * Note: locked_by is deliberatey dropped,
	 * because it refers to a different branch.
	 */
}
