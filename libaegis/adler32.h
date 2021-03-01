//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: interface of the adler32 class
//

#ifndef LIBAEGIS_ADLER32_H
#define LIBAEGIS_ADLER32_H

class nstring; // forward

/**
  * The adler32 fucntion is used to calculate the Addler 32 checksum of
  * the named file.
  */
unsigned long calculate_adler32(const nstring &filename);

#endif // LIBAEGIS_ADLER32_H
