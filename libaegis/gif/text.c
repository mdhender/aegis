/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998 Peter Miller;
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
 * MANIFEST: functions to draw text in GIF files
 */

#include <gif.h>

static unsigned char data[128][14] =
{
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x00 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x01 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x02 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x03 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x04 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x05 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x06 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x07 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x08 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x09 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x0A */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x0B */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x0C */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x0D */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x0E */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x0F */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x10 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x11 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x12 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x13 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x14 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x15 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x16 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x17 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x18 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x19 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x1A */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x1B */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x1C */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x1D */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x1E */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }, /* 0x1F */

	/* ' ' */
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },

	/* '!' */
	{ 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, },

	/* '"' */
	{ 0x00, 0x48, 0x48, 0x48, 0x48, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },

	/* '#' */
	{ 0x00, 0x24, 0x24, 0x24, 0x7E, 0x24, 0x48,
	0xFC, 0x48, 0x48, 0x48, 0x00, 0x00, 0x00, },

	/* '$' */
	{ 0x00, 0x10, 0x38, 0x54, 0x50, 0x50, 0x38,
	0x14, 0x14, 0x54, 0x38, 0x10, 0x00, 0x00, },

	/* '%' */
	{ 0x00, 0x64, 0x94, 0x98, 0x68, 0x10, 0x10,
	0x2C, 0x32, 0x52, 0x4C, 0x00, 0x00, 0x00, },

	/* '&' */
	{ 0x00, 0x30, 0x48, 0x48, 0x30, 0x20, 0x52,
	0x8C, 0x88, 0x88, 0x76, 0x00, 0x00, 0x00, },

	/* '\'' */
	{ 0x00, 0x08, 0x08, 0x10, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },

	/* '(' */
	{ 0x00, 0x08, 0x10, 0x10, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x10, 0x10, 0x08, 0x00, 0x00, },

	/* ')' */
	{ 0x00, 0x20, 0x10, 0x10, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x10, 0x10, 0x20, 0x00, 0x00, },

	/* '*' */
	{ 0x00, 0x00, 0x10, 0x54, 0x38, 0x38, 0x54,
	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },

	/* '+' */
	{ 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x7C,
	0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, },

	/* ',' */
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x18, 0x18, 0x08, 0x30, 0x00, },

	/* '-' */
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },

	/* '.' */
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, },

	/* '/' */
	{ 0x00, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40, 0x00, 0x00, 0x00, },

	/* '0' */
	{ 0x00, 0x00, 0x38, 0x44, 0x44, 0x4C, 0x54,
	0x64, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, },

	/* '1' */
	{ 0x00, 0x00, 0x10, 0x30, 0x50, 0x10, 0x10,
	0x10, 0x10, 0x10, 0x7C, 0x00, 0x00, 0x00, },

	/* '2' */
	{ 0x00, 0x00, 0x38, 0x44, 0x04, 0x04, 0x08,
	0x10, 0x20, 0x40, 0x7C, 0x00, 0x00, 0x00, },

	/* '3' */
	{ 0x00, 0x00, 0x38, 0x44, 0x04, 0x04, 0x18,
	0x04, 0x04, 0x44, 0x38, 0x00, 0x00, 0x00, },

	/* '4' */
	{ 0x00, 0x00, 0x04, 0x0C, 0x14, 0x24, 0x44,
	0x44, 0x7E, 0x04, 0x04, 0x00, 0x00, 0x00, },

	/* '5' */
	{ 0x00, 0x00, 0x7C, 0x40, 0x40, 0x78, 0x04,
	0x04, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, },

	/* '6' */
	{ 0x00, 0x00, 0x38, 0x44, 0x40, 0x40, 0x78,
	0x44, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, },

	/* '7' */
	{ 0x00, 0x00, 0x7C, 0x44, 0x04, 0x08, 0x08,
	0x08, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, },

	/* '8' */
	{ 0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x38,
	0x44, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, },

	/* '9' */
	{ 0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x3C,
	0x04, 0x04, 0x44, 0x38, 0x00, 0x00, 0x00, },

	/* ':' */
	{ 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00,
	0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, },

	/* ';' */
	{ 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00,
	0x00, 0x00, 0x18, 0x18, 0x08, 0x30, 0x00, },

	/* '<' */
	{ 0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
	0x40, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00, },

	/* '=' */
	{ 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00,
	0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, },

	/* '>' */
	{ 0x00, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04,
	0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, },

	/* '?' */
	{ 0x00, 0x38, 0x44, 0x44, 0x04, 0x08, 0x10,
	0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, },

	/* '@' */
	{ 0x00, 0x10, 0x28, 0x44, 0x5C, 0x54, 0x54,
	0x5C, 0x40, 0x24, 0x18, 0x00, 0x00, 0x00, },

	/* 'A' */
	{ 0x00, 0x00, 0x10, 0x10, 0x28, 0x28, 0x28,
	0x44, 0x7C, 0x44, 0x44, 0x00, 0x00, 0x00, },

	/* 'B' */
	{ 0x00, 0x00, 0x70, 0x48, 0x48, 0x48, 0x78,
	0x44, 0x44, 0x44, 0x78, 0x00, 0x00, 0x00, },

	/* 'C' */
	{ 0x00, 0x00, 0x38, 0x44, 0x44, 0x40, 0x40,
	0x40, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, },

	/* 'D' */
	{ 0x00, 0x00, 0x70, 0x48, 0x44, 0x44, 0x44,
	0x44, 0x44, 0x48, 0x70, 0x00, 0x00, 0x00, },

	/* 'E' */
	{ 0x00, 0x00, 0x7C, 0x40, 0x40, 0x40, 0x78,
	0x40, 0x40, 0x40, 0x7C, 0x00, 0x00, 0x00, },

	/* 'F' */
	{ 0x00, 0x00, 0x7C, 0x40, 0x40, 0x40, 0x78,
	0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, },

	/* 'G' */
	{ 0x00, 0x00, 0x38, 0x44, 0x40, 0x40, 0x4C,
	0x44, 0x44, 0x4C, 0x34, 0x00, 0x00, 0x00, },

	/* 'H' */
	{ 0x00, 0x00, 0x44, 0x44, 0x44, 0x44, 0x7C,
	0x44, 0x44, 0x44, 0x44, 0x00, 0x00, 0x00, },

	/* 'I' */
	{ 0x00, 0x00, 0x7C, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x10, 0x7C, 0x00, 0x00, 0x00, },

	/* 'J' */
	{ 0x00, 0x00, 0x3C, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x48, 0x48, 0x30, 0x00, 0x00, 0x00, },

	/* 'K' */
	{ 0x00, 0x00, 0x44, 0x48, 0x50, 0x60, 0x50,
	0x48, 0x48, 0x44, 0x44, 0x00, 0x00, 0x00, },

	/* 'L' */
	{ 0x00, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40,
	0x40, 0x40, 0x40, 0x7C, 0x00, 0x00, 0x00, },

	/* 'M' */
	{ 0x00, 0x00, 0x84, 0x84, 0xCC, 0xCC, 0xB4,
	0xB4, 0x84, 0x84, 0x84, 0x00, 0x00, 0x00, },

	/* 'N' */
	{ 0x00, 0x00, 0x44, 0x64, 0x64, 0x54, 0x54,
	0x4C, 0x4C, 0x44, 0x44, 0x00, 0x00, 0x00, },

	/* 'O' */
	{ 0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x44,
	0x44, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, },

	/* 'P' */
	{ 0x00, 0x00, 0x78, 0x44, 0x44, 0x44, 0x78,
	0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, },

	/* 'Q' */
	{ 0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x44,
	0x44, 0x44, 0x44, 0x38, 0x10, 0x0C, 0x00, },

	/* 'R' */
	{ 0x00, 0x00, 0x78, 0x44, 0x44, 0x44, 0x78,
	0x48, 0x44, 0x44, 0x44, 0x00, 0x00, 0x00, },

	/* 'S' */
	{ 0x00, 0x00, 0x38, 0x44, 0x40, 0x20, 0x18,
	0x04, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, },

	/* 'T' */
	{ 0x00, 0x00, 0x7C, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, },

	/* 'U' */
	{ 0x00, 0x00, 0x44, 0x44, 0x44, 0x44, 0x44,
	0x44, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, },

	/* 'V' */
	{ 0x00, 0x00, 0x44, 0x44, 0x44, 0x28, 0x28,
	0x28, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, },

	/* 'W' */
	{ 0x00, 0x00, 0x82, 0x82, 0x82, 0x92, 0x54,
	0x54, 0x54, 0x28, 0x28, 0x00, 0x00, 0x00, },

	/* 'X' */
	{ 0x00, 0x00, 0x44, 0x44, 0x28, 0x28, 0x10,
	0x28, 0x28, 0x44, 0x44, 0x00, 0x00, 0x00, },

	/* 'Y' */
	{ 0x00, 0x00, 0x44, 0x44, 0x44, 0x28, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, },

	/* 'Z' */
	{ 0x00, 0x00, 0x7C, 0x04, 0x04, 0x08, 0x10,
	0x20, 0x40, 0x40, 0x7C, 0x00, 0x00, 0x00, },

	/* '[' */
	{ 0x00, 0x3C, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x3C, 0x00, 0x00, },

	/* '\' */
	{ 0x00, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10,
	0x08, 0x08, 0x04, 0x04, 0x00, 0x00, 0x00, },

	/* ']' */
	{ 0x00, 0x78, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x78, 0x00, 0x00, },

	/* '^' */
	{ 0x00, 0x00, 0x10, 0x38, 0x6C, 0x44, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },

	/* '_' */
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, },

	/* '`' */
	{ 0x00, 0x20, 0x20, 0x10, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },

	/* 'a' */
	{ 0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 0x04,
	0x3C, 0x44, 0x44, 0x3C, 0x00, 0x00, 0x00, },

	/* 'b' */
	{ 0x00, 0x00, 0x40, 0x40, 0x58, 0x64, 0x44,
	0x44, 0x44, 0x64, 0x58, 0x00, 0x00, 0x00, },

	/* 'c' */
	{ 0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 0x40,
	0x40, 0x40, 0x44, 0x38, 0x00, 0x00, 0x00, },

	/* 'd' */
	{ 0x00, 0x00, 0x04, 0x04, 0x34, 0x4C, 0x44,
	0x44, 0x44, 0x4C, 0x34, 0x00, 0x00, 0x00, },

	/* 'e' */
	{ 0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 0x44,
	0x7C, 0x40, 0x44, 0x38, 0x00, 0x00, 0x00, },

	/* 'f' */
	{ 0x00, 0x00, 0x0C, 0x10, 0x10, 0x7C, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, },

	/* 'g' */
	{ 0x00, 0x00, 0x00, 0x00, 0x34, 0x4C, 0x44,
	0x44, 0x44, 0x4C, 0x34, 0x04, 0x44, 0x38, },

	/* 'h' */
	{ 0x00, 0x00, 0x40, 0x40, 0x58, 0x64, 0x44,
	0x44, 0x44, 0x44, 0x44, 0x00, 0x00, 0x00, },

	/* 'i' */
	{ 0x00, 0x00, 0x08, 0x00, 0x38, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, },

	/* 'j' */
	{ 0x00, 0x00, 0x08, 0x00, 0x38, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x48, 0x30, },

	/* 'k' */
	{ 0x00, 0x00, 0x40, 0x40, 0x48, 0x50, 0x60,
	0x60, 0x50, 0x48, 0x44, 0x00, 0x00, 0x00, },

	/* 'l' */
	{ 0x00, 0x00, 0x38, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, },

	/* 'm' */
	{ 0x00, 0x00, 0x00, 0x00, 0x68, 0x54, 0x54,
	0x54, 0x54, 0x54, 0x54, 0x00, 0x00, 0x00, },

	/* 'n' */
	{ 0x00, 0x00, 0x00, 0x00, 0x58, 0x64, 0x44,
	0x44, 0x44, 0x44, 0x44, 0x00, 0x00, 0x00, },

	/* 'o' */
	{ 0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 0x44,
	0x44, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, },

	/* 'p' */
	{ 0x00, 0x00, 0x00, 0x00, 0x58, 0x64, 0x44,
	0x44, 0x44, 0x64, 0x58, 0x40, 0x40, 0x40, },

	/* 'q' */
	{ 0x00, 0x00, 0x00, 0x00, 0x34, 0x4C, 0x44,
	0x44, 0x44, 0x4C, 0x34, 0x04, 0x04, 0x04, },

	/* 'r' */
	{ 0x00, 0x00, 0x00, 0x00, 0x58, 0x64, 0x40,
	0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, },

	/* 's' */
	{ 0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 0x40,
	0x38, 0x04, 0x44, 0x38, 0x00, 0x00, 0x00, },

	/* 't' */
	{ 0x00, 0x00, 0x10, 0x10, 0x7C, 0x10, 0x10,
	0x10, 0x10, 0x10, 0x0C, 0x00, 0x00, 0x00, },

	/* 'u' */
	{ 0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x44,
	0x44, 0x44, 0x4C, 0x34, 0x00, 0x00, 0x00, },

	/* 'v' */
	{ 0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x44,
	0x28, 0x28, 0x10, 0x10, 0x00, 0x00, 0x00, },

	/* 'w' */
	{ 0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x54,
	0x54, 0x54, 0x54, 0x28, 0x00, 0x00, 0x00, },

	/* 'x' */
	{ 0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x28,
	0x10, 0x28, 0x44, 0x44, 0x00, 0x00, 0x00, },

	/* 'y' */
	{ 0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x24,
	0x24, 0x14, 0x18, 0x08, 0x08, 0x50, 0x20, },

	/* 'z' */
	{ 0x00, 0x00, 0x00, 0x00, 0x7C, 0x04, 0x08,
	0x10, 0x20, 0x40, 0x7C, 0x00, 0x00, 0x00, },

	/* '{' */
	{ 0x0C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x60,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x0C, 0x00, },

	/* '|' */
	{ 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, },

	/* '}' */
	{ 0x60, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0C,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x60, 0x00, },

	/* '~' */
	{ 0x00, 0x00, 0x24, 0x54, 0x48, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
};


static int glyph _((gif_ty *, int, int, int, int));

static int
glyph(gp, x, y, c, clr)
	gif_ty		*gp;
	int		x, y;
	int		c, clr;
{
	int		x2, y2;
	int		bit;
	unsigned char	*dp;

	dp = data[c & 0x7F];
	for (y2 = -10; y2 < 4; ++y2, ++dp)
	{
		for (x2 = 0, bit = 0x80; x2 < 7; ++x2, bit >>= 1)
		{
			if (*dp & bit)
				gif_pixel_set(gp, x + x2, y - y2, clr);
		}
	}
	return (x + 7);
}


void
gif_text(gp, x, y, s, clr)
	gif_ty		*gp;
	int		x, y;
	char		*s;
	int		clr;
{
	while (*s)
		x = glyph(gp, x, y, *s++, clr);
}
