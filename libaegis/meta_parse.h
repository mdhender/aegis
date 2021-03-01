//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1999, 2001-2008 Peter Miller
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

#ifndef LIBAEGIS_META_PARSE_H
#define LIBAEGIS_META_PARSE_H

#include <common/main.h>
#include <common/str.h>
#include <libaegis/meta_type.h>

struct sem_ty
{
    meta_type	*type;
    sem_ty	*next;
    void	*addr;
};

void sem_push(meta_type *, void *);
void sem_pop(void);
void sem_integer(long);
void sem_real(double);
void sem_string(string_ty *);
void sem_enum(string_ty *);
void sem_list(void);
void sem_field(string_ty *);

void *parse(string_ty *, meta_type *);
void *parse_env(const char *, meta_type *);
class input_ty; // forward
void *parse_input(input &, meta_type *);

#endif // LIBAEGIS_META_PARSE_H
