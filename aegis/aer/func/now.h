/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: interface definition for aegis/aer/func/now.c
 */

#ifndef AEGIS_AER_FUNC_NOW_H
#define AEGIS_AER_FUNC_NOW_H

#include <ac/time.h>

#include <aer/func.h>

extern rpt_func_ty rpt_func_now;
extern rpt_func_ty rpt_func_working_days;

double working_days _((time_t, time_t));

#endif /* AEGIS_AER_FUNC_NOW_H */
