/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1993, 1995, 2002 Peter Miller;
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
 * MANIFEST: interface definition for aegis/interval.c
 */

#ifndef AEGIS_INTERVAL_H
#define AEGIS_INTERVAL_H

#include <ac/stddef.h>

#include <main.h>

typedef long interval_data_ty;

typedef struct interval_ty interval_ty;
struct interval_ty
{
    size_t	    length;
    size_t	    size;
    size_t	    scan_index;
    interval_data_ty scan_next_datum;
    interval_data_ty data[1];
};

interval_ty *interval_create_empty(void);
interval_ty *interval_create_range(interval_data_ty first,
    interval_data_ty last);
void interval_free(interval_ty *);
interval_ty *interval_union(interval_ty *, interval_ty *);
interval_ty *interval_intersection(interval_ty *, interval_ty *);
interval_ty *interval_difference(interval_ty *, interval_ty *);
int interval_member(interval_ty *, interval_data_ty);

void interval_scan_begin(interval_ty *);
int interval_scan_next(interval_ty *, interval_data_ty *);
void interval_scan_end(interval_ty *);

#endif /* AEGIS_INTERVAL_H */
