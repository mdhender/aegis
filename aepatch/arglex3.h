/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: interface definition for aepatch/arglex3.c
 */

#ifndef AEPATCH_ARGLEX3_H
#define AEPATCH_ARGLEX3_H

#include <arglex2.h>

enum
{
    arglex_token_ascii_armor = ARGLEX2_MAX,
    arglex_token_ascii_armor_not,
    arglex_token_compatibility,
    arglex_token_compress,
    arglex_token_compress_not,
    arglex_token_content_transfer_encoding,
    arglex_token_receive,
    arglex_token_send,
    arglex_token_trojan,
    arglex_token_trojan_not,
    arglex_token_path_prefix_add,
    arglex_token_path_prefix_remove,
    ARGLEX3_MAX
};

void arglex3_init(int, char **);

#endif /* AEPATCH_ARGLEX3_H */
