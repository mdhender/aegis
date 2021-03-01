//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001, 2002, 2004-2006, 2008 Peter Miller
//	Copyright (C) 2007 Walter Franzini
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see
//	<http://www.gnu.org/licenses>.
//

#ifndef AEDIST_ARGLEX3_H
#define AEDIST_ARGLEX3_H

#include <libaegis/arglex2.h>

enum
{
    arglex_token_aeget = ARGLEX2_MAX,
    arglex_token_all,
    arglex_token_ascii_armor,
    arglex_token_ascii_armor_not,
    arglex_token_archive,
    arglex_token_compatibility_not,
    arglex_token_content_transfer_encoding,
    arglex_token_description_header,
    arglex_token_description_header_not,
    arglex_token_entire_source,
    arglex_token_entire_source_not,
    arglex_token_exclude_uuid,
    arglex_token_exclude_uuid_not,
    arglex_token_exclude_version,
    arglex_token_exclude_version_not,
    arglex_token_ignore_uuid,
    arglex_token_ignore_uuid_not,
    arglex_token_inventory,
    arglex_token_missing,
    arglex_token_patch,
    arglex_token_patch_not,
    arglex_token_path_prefix_add,
    arglex_token_pending,
    arglex_token_receive,
    arglex_token_replay,
    arglex_token_send,
    arglex_token_trojan,
    arglex_token_trojan_not,
    ARGLEX3_MAX
};

void arglex3_init(int, char **);

#endif // AEDIST_ARGLEX3_H
