//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate lines
//

#include <libaegis/change.h>
#include <aeannotate/line.h>


void
line_constructor(line_t *lp, change_ty *cp, string_ty *text)
{
    lp->cp = cp;
    lp->text = str_copy(text);
}


void
line_destructor(line_t *lp)
{
    lp->cp = 0;
    str_free(lp->text);
    lp->text = 0;
}
