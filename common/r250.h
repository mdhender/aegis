//
//	aegis - project change supervisor
//	Copyright (C) 1992-1995, 2005, 2006 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 1, or (at your option)
//	any later version.
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
// MANIFEST: interface definition for common/r250.c
//

#ifndef COMMON_R250_H
#define COMMON_R250_H

/** \addtogroup Misc
  * @{
  */

#include <common/main.h>

/**
  * The r250 function is used to obtain a pseudo random number.  All 32
  * bits are significant.  The generator has a minimum period of 2^250.
  */
unsigned long r250(void);

/** @} */
#endif // COMMON_R250_H
