/*
 *	aegis - project change supervisor
 *	Copyright (C) 2004 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/input/crop.c
 */

#ifndef LIBAEGIS_INPUT_CROP_H
#define LIBAEGIS_INPUT_CROP_H

#include <input.h>

/**
  * The input_crop_new creates a new input which will read exactly
  * <length> bytes of the <deeper> input, from the current position.
  */
input_ty *input_crop_new(input_ty *deeper, int delete_on_close, long length);

#endif /* LIBAEGIS_INPUT_CROP_H */
