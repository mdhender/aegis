//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: interface of the sub_change_reviewr_list class
//

#ifndef LIBAEGIS_SUB_CHANGE_REVIEWR_LIST_H
#define LIBAEGIS_SUB_CHANGE_REVIEWR_LIST_H

struct wstring_ty; // forward
struct wstring_list_ty; // forward
struct sub_context_ty; // forward

/**
  * The sub_change_reviewer_list function implements the
  * Change_Reviewer_List substitution.  The change_reviewer_list
  * substitution is replaced by the list of reviewers of this change
  * since the last develop end.
  *
  * \param scp
  *     The context of this substitution.
  * \param arg
  *     The list of arguments, including the function name as [0]
  * \returns
  *      a pointer to a string in dynamic memory;
  *      or NULL on error, setting suberr appropriately.
  */
wstring_ty *sub_change_reviewer_list(sub_context_ty *scp, wstring_list_ty *arg);

#endif // LIBAEGIS_SUB_CHANGE_REVIEWR_LIST_H
