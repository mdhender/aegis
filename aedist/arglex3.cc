//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001, 2003-2006 Peter Miller
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
// MANIFEST: functions to manipulate arglex3s
//

#include <aedist/arglex3.h>


static arglex_table_ty argtab[] =
{
	{ "-Ascii_Armor", arglex_token_ascii_armor, },
	{ "-Not_Ascii_Armor", arglex_token_ascii_armor_not, },
	{ "-Content_Transfer_Encoding", arglex_token_content_transfer_encoding},
	{ "-Description_Header", arglex_token_description_header, },
	{ "-Not_Description_Header", arglex_token_description_header_not, },
	{ "-Entire_Source", arglex_token_entire_source, },
	{ "-Not_Entire_Source", arglex_token_entire_source_not, },
        { "-EXclude_UUID", arglex_token_exclude_uuid, },
        { "-INclude_UUID", arglex_token_exclude_uuid_not, },
        { "-EXclude_VERsion", arglex_token_exclude_version, },
        { "-INclude_VERsion", arglex_token_exclude_version_not, },
        { "-MIssing", arglex_token_missing, },
        { "-PATch", arglex_token_patch, },
	{ "-Not_PATch", arglex_token_patch_not, },
	{ "-Partial_Source", arglex_token_entire_source_not, },
        { "-PENding", arglex_token_pending },
        { "-IGnore_UUID", arglex_token_ignore_uuid, },
        { "-Not_IGnore_UUID", arglex_token_ignore_uuid_not, },
        { "-Receive", arglex_token_receive, },
        { "-REPlay", arglex_token_replay, },
	{ "-Send", arglex_token_send, },
	{ "-Trojan", arglex_token_trojan, },
	{ "-Not_Trojan", arglex_token_trojan_not, },
	ARGLEX_END_MARKER
};


void
arglex3_init(int argc, char **argv)
{
	arglex2_init3(argc, argv, argtab);
}
