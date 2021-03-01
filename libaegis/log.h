//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1996, 2002, 2004 Peter Miller;
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
// MANIFEST: interface definition for aegis/log.c
//

#ifndef AEGIS_LOG_H
#define AEGIS_LOG_H

#include <main.h>

enum log_style_ty
{
	log_style_append,
	log_style_append_default,
	log_style_create,
	log_style_create_default,
	log_style_none,
	log_style_none_default,
	log_style_snuggle,
	log_style_snuggle_default
};

struct string_ty;
struct user_ty;

void log_open(struct string_ty *, struct user_ty *, log_style_ty);
void log_close(void);
void log_quitter(int);

#endif // AEGIS_LOG_H
