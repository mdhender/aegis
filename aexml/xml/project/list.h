/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
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
 * MANIFEST: interface definition for aexml/xml/project/list.c
 */

#ifndef AEXML_XML_PROJECT_LIST_H
#define AEXML_XML_PROJECT_LIST_H

#include <main.h>

struct string_ty; /* forward */
struct output_ty; /* forward */

void xml_project_list(struct string_ty *project_name, long change_number,
    struct output_ty *op);

#endif /* AEXML_XML_PROJECT_LIST_H */
