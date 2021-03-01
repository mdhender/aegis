//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef AEANNOTATE_LINE_LIST_H
#define AEANNOTATE_LINE_LIST_H

#include <aeannotate/line.h>

struct line_list_t
{
    size_t	    maximum;
    size_t	    start1;
    size_t	    length1;
    size_t	    start2;
    size_t	    length2;
    line_t	    *item;
};

void line_list_constructor(line_list_t *);
void line_list_destructor(line_list_t *);
void line_list_clear(line_list_t *);
void line_list_delete(line_list_t *, size_t, size_t);
void line_list_insert(line_list_t *, size_t, change::pointer ,
    struct string_ty *);

#endif // AEANNOTATE_LINE_LIST_H
