/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001 Peter Miller;
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
 * MANIFEST: interface definition for aedist/arglex3.c
 */

#ifndef AEDIST_ARGLEX3_H
#define AEDIST_ARGLEX3_H

#include <arglex2.h>

enum
{
	arglex_token_ascii_armor = ARGLEX2_MAX,
	arglex_token_ascii_armor_not,
	arglex_token_compress,
	arglex_token_compress_not,
	arglex_token_content_transfer_encoding,
	arglex_token_description_header,
	arglex_token_description_header_not,
	arglex_token_entire_source,
	arglex_token_entire_source_not,
	arglex_token_patch,
	arglex_token_patch_not,
	arglex_token_receive,
	arglex_token_send,
	arglex_token_trojan,
	arglex_token_trojan_not,
	ARGLEX3_MAX
};

void arglex3_init _((int, char **));

#endif /* AEDIST_ARGLEX3_H */
