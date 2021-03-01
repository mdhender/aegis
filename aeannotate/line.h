//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004, 2005 Peter Miller;
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
// MANIFEST: interface definition for aeannotate/line.c
//

#ifndef AEANNOTATE_LINE_H
#define AEANNOTATE_LINE_H

#include <common/str.h>

struct line_t
{
    struct change_ty *cp;
    string_ty	    *text;
};

void line_constructor(line_t *, struct change_ty *, struct string_ty *);
void line_destructor(line_t *);

#endif // AEANNOTATE_LINE_H
