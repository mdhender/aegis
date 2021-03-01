/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 * MANIFEST: interface definition for aegis/col.c
 */

#ifndef COL_H
#define COL_H

#include <main.h>

void col_open _((char *));
void col_close _((void));
int col_create _((int, int));
void col_puts _((int, char *));
void col_printf _((int, char *, ...));
void col_eoln _((void));
void col_bol _((int));
void col_heading _((int, char *));
void col_title _((char *, char *));
void col_eject _((void));
void col_need _((int));

#endif /* COL_H */
