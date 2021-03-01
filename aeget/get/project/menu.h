//
//	aegis - project change supervisor
//	Copyright (C) 2003-2006 Peter Miller;
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
// MANIFEST: interface definition for aeget/get/project/menu.c
//

#ifndef AEGET_GET_PROJECT_MENU_H
#define AEGET_GET_PROJECT_MENU_H

#include <common/main.h>

struct project_ty; // forward
struct string_ty; // forward
struct string_list_ty; // forward

/**
  * The get_project_menu is used to respond to a
  *	GET /cgi-bin/aeget/<project-name>/?menu
  * request.  It describes and links to all other available pages.
  */
void get_project_menu(struct project_ty *pp, struct string_ty *filename,
    struct string_list_ty *modifier);

#endif // AEGET_GET_PROJECT_MENU_H
