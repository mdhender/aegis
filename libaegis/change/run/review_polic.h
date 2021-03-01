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
// MANIFEST: interface of the change_run_review_polic class
//

#ifndef LIBAEGIS_CHANGE_RUN_REVIEW_POLIC_H
#define LIBAEGIS_CHANGE_RUN_REVIEW_POLIC_H

struct change_ty; // forward
struct user_ty; // forward

/**
  * The change_run_review_policy_command function is used to run the
  * review_policy_command in the project configuration file, if present.
  *
  * This function shall only be called by the aerpass (rview pass)
  * command to find out whether we now consider this change adequately
  * reviewed
  *
  * \param cp
  *     The change to be operated upon.
  * \param up
  *     The user executing the command.
  * \returns
  *     The exit status of the command:  0 for "advance to awaiting
  *     integration" and non-zero for "do not change state".
  */
int change_run_review_policy_command(change_ty *cp, user_ty *up);

#endif // LIBAEGIS_CHANGE_RUN_REVIEW_POLIC_H
