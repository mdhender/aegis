/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001-2003 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/project/file/roll_forward.c
 */

#ifndef LIBAEGIS_PROJECT_FILE_ROLL_FORWARD_H
#define LIBAEGIS_PROJECT_FILE_ROLL_FORWARD_H

#include <ac/time.h>
#include <project.h>
#include <fstate.h>

typedef struct file_event_ty file_event_ty;
struct file_event_ty
{
    time_t	    when;
    struct change_ty *cp;
};

typedef struct file_event_list_ty file_event_list_ty;
struct file_event_list_ty
{
    size_t	    length;
    size_t	    maximum;
    file_event_ty   *item;
};

/**
  * The project_file_roll_forward function is used to recapitilate
  * the project's history, constructing information about the state
  * of all files as it goes.  The project_file_roll_forward_get
  * function is used to extract the results.
  *
  * \param pp
  *     The project to apply the listing to.
  *     All parent branches will be visited, too.
  * \param limit
  *     The time limit for changes.  Changes on or before this
  *     time will be included.
  * \param detailed
  *     If this is false, only the parent branches and the
  *     project itself are visited.  If this is true, all branches
  *     completed within the limt will be visited.
  *
  * \caveat
  *	This function is one really big memory leak.
  *	You can't do this to two projects at the same time.
  */
void project_file_roll_forward(project_ty *pp, time_t limit, int detailed);

/**
  * The project_file_roll_forward_get function is used to obtain the
  * events for a given file, once project_file_roll_forward has been
  * called to construct the information.
  *
  * \caveat
  *    Do not free the change pointed to, as it may be referenced by
  *    other files' histories.
  */
file_event_list_ty *project_file_roll_forward_get(string_ty *);

/**
  * The project_file_roll_forward_get_last function is used to get the
  * last file event, used by most functions which deal with deltas.
  */
file_event_ty *project_file_roll_forward_get_last(string_ty *);

/**
  * The project_file_roll_forward_get_older function is used to get the
  * last-but-one file event, used by aecp -rescind to roll back a change.
  */
file_event_ty *project_file_roll_forward_get_older(string_ty *);

struct string_list_ty; /* forward */

/**
  * The project_file_roll_forward_keys function is used to get a list
  * of filenames for which file event lists are available.
  */
void project_file_roll_forward_keys(struct string_list_ty *);

#endif /* LIBAEGIS_PROJECT_FILE_ROLL_FORWARD_H */
